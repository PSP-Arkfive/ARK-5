/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <stdio.h>
#include <string.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_ark.h>

#include "modulemanager.h"
#include "nidresolver.h"
#include "config.h"
#include "elf.h"
#include "loadercore.h"
#include "cryptography.h"
#include "rebootex.h"
#include "systemctrl_private.h"
#include "init.h"


// Real Executable Check Function Pointer
int (* ProbeExec1)(u8 *buffer, int *check) = NULL;
int (* ProbeExec2)(u8 *buffer, int *check) = NULL;

// Sony PRX Decrypter Function Pointer
int (* SonyPRXDecrypt)(void *, unsigned int, unsigned int *) = NULL;
int (* origCheckExecFile)(unsigned char * addr, void * arg2) = NULL;

// Executable Check #1
int _ProbeExec1(u8 *buffer, int *check)
{
    // Check Executable (we patched our files with shifted attributes so this works)
    int result = ProbeExec1(buffer, check);
    
    // Grab Executable Magic
    unsigned int magic = *(unsigned int *)(buffer);
    
    // ELF File
    if(magic == 0x464C457F)
    {
        // Recover Attributes (which we shifted before)
        unsigned short realattr = *(unsigned short *)(buffer + check[19]);
        
        // Mask Attributes
        unsigned short attr = realattr & 0x1E00;
        
        // Kernel Module
        if(attr != 0)
        {
            // Fetch OFW-detected Attributes
            unsigned short attr2 = *(u16*)((void*)(check)+0x58);
            
            // OFW Attributes don't match
            if((attr2 & 0x1E00) != attr)
            {
                // Now they do. :)
                *(u16*)((void*)(check)+0x58) = realattr;
            }
        }
        
        // Flip Switch
        if(check[18] == 0) check[18] = 1;
    }
    
    // Return Result
    return result;
}

// Executable Check #2
int _ProbeExec2(u8 *buffer, int *check)
{
    // Check Executable
    int result = ProbeExec2(buffer, check);
    
    // Grab Executable Magic
    unsigned int magic = *(unsigned int *)(buffer);
    
    // Plain Static ELF Executable
    if(magic == 0x464C457F && IsStaticElf(buffer))
    {
        // Fake UMD Apitype (as its the only one that allows Static ELFs... and even that, only as LoadExec Target)
        check[2] = 0x120;
        
        // Invalid Module Info Section
        if(check[19] == 0)
        {
            // Search String Table
            char * strtab = GetStrTab(buffer);
            
            // Found it! :D
            if(strtab != NULL)
            {
                // Cast ELF Header
                Elf32_Ehdr * header = (Elf32_Ehdr *)buffer;
                
                // Section Header Start Pointer
                unsigned char * pData = buffer + header->e_shoff;
                
                // Iterate Section Headers
                for (int i = 0; i < header->e_shnum; i++)
                {
                    // Cast Section Header
                    Elf32_Shdr * section = (Elf32_Shdr *)pData;
                    
                    // Found Module Info Section
                    if(strcmp(strtab + section->sh_name, ".rodata.sceModuleInfo") == 0)
                    {
                        // Fix Section Pointer
                        check[19] = section->sh_offset;
                        check[22] = 0;
                        
                        // Stop Search
                        break;
                    }
                    
                    // Move to next Section
                    pData += header->e_shentsize;
                }
            }
        }
    }
    
    // Return Result
    return result;
}

// Executable File Check
int KernelCheckExecFile(unsigned char * buffer, SceLoadCoreExecFileInfo * check)
{
    // Patch Executable
    int result = PatchExec1(buffer, check);
    
    // PatchExec1 isn't enough... :(
    if(result != 0)
    {
        // Check Executable
        int checkresult = sceKernelCheckExecFile(buffer, check);
        
        // Grab Executable Magic
        unsigned int magic = *(unsigned int *)(buffer);
        
        // Patch Executable
        result = PatchExec3(buffer, check, magic == 0x464C457F, checkresult);
    }
    
    // Return Result
    return result;
}

// Patch Loader Core Module
SceModule* patchLoaderCore(void)
{

    // Find Module
    SceModule* mod = (SceModule *)sceKernelFindModuleByName("sceLoaderCore");

    // Fetch Text Address
    u32 start_addr = mod->text_addr;
    u32 topaddr = mod->text_addr+mod->text_size;

    // restore rebootex pointers to original
    u32 rebootex_decrypt_call = JAL(0);
    u32 rebootex_checkexec_call = JAL(0);
    SonyPRXDecrypt = (void*)sctrlHENFindFunction("sceMemlmd", "memlmd", 0xEF73E85B);
    origCheckExecFile = (void*)sctrlHENFindFunction("sceMemlmd", "memlmd", 0x6192F715);
    // find patched functions pointing to rebootex
    int found = 0;
    for (u32 addr = start_addr; addr<topaddr&&!found; addr+=4){
        u32 data = _lw(addr);
        switch (data){
        case 0x35450200: rebootex_checkexec_call = _lw(addr+12);
        case 0x35250200: rebootex_decrypt_call = _lw(addr-0x18); found=1;
        default: break;
        }
    }

    // override the checkExec reference in the module globals
    u32 checkExec = sctrlHENFindFunction("sceLoaderCore", "LoadCoreForKernel", 0xD3353EC4);
    u32 ref = sctrlHENFindRefInGlobals("LoadCoreForKernel", checkExec, checkExec);
    _sw((unsigned int)KernelCheckExecFile, ref);
    // Flush Cache
    sctrlFlushCache();

    // start the dynamic patching
    for (u32 addr = start_addr; addr<topaddr; addr+=4){
        u32 data = _lw(addr);
        if (data == JAL(checkExec)){
            // Hook sceKernelCheckExecFile
            _sw(JAL(KernelCheckExecFile), addr);
        }
        else if (data == rebootex_decrypt_call){ // Not doing this will keep them pointing into Reboot Buffer... which gets unloaded...
            _sw(JAL(SonyPRXDecrypt), addr); // Fix memlmd_EF73E85B Calls that we broke intentionally in Reboot Buffer
        }
        else if (data == rebootex_checkexec_call){
            _sw(JAL(origCheckExecFile), addr); // Fix memlmd_6192F715 Calls that we broke intentionally in Reboot Buffer
        }
        else{
            switch (data){
            case 0x02E0F809: 
                // Hook sceInit StartModule Call
                _sw(JAL(patch_sceKernelStartModule_in_bootstart), addr);
                // Move Real Bootstart into Argument #1
                _sw(0x02E02021, addr+4);
                break;
            case 0x30ABFFFF:    ProbeExec1 = (void *)addr-0x100;     break;        // Executable Check Function #1
            case 0x01E63823:    ProbeExec2 = (void *)addr-0x78;      break;        // Executable Check Function #2
            case 0x30894000:    _sw(0x3C090000, addr);               break;        // Allow Syscalls
            case 0x00E8282B:    _sh(0x1000, addr + 6);               break;        // Remove POPS Check
            case 0x01A3302B:    _sw(NOP, addr+4);                    break;        // Remove Invalid PRX Type (0x80020148) Check
            }
        }
    }
    // Flush Cache
    sctrlFlushCache();
    
    // Patch Relocation Type 7 to 0 (this makes more homebrews load)
    {
    u32 addr = ref; // addr = mod->text_addr would also work, we generally just want it to be pointing at the code
    while (strcmp((char*)addr, "sceSystemModule")) addr++; // scan for this string, reloc_type comes a few fixed bytes after
    _sw(_lw(addr+0x7C), addr+0x98);
    }
    
    // Flush Cache
    sctrlFlushCache();
    
    // Hook Executable Checks
    for (u32 addr=start_addr; addr<topaddr; addr+=4){
        if (_lw(addr) == JAL(ProbeExec1))
            _sw(JAL(_ProbeExec1), addr);
        else if (_lw(addr) == JAL(ProbeExec2))
            _sw (JAL(_ProbeExec2), addr);
    }

    // Flush Cache
    sctrlFlushCache();

    return mod;
}
