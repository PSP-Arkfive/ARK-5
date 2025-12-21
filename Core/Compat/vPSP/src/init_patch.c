#include <string.h>
#include <stdio.h>
#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <pspinit.h>

#include <ark.h> 
#include <cfwmacros.h>
#include <rebootconfig.h>
#include <bootloadex.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_private.h>

#include "adrenaline_compat.h"
#include "funcs.h"

typedef struct {
    char *name;
    void *buf;
    int size;
    int unk_12;
    int attr;
    int unk_20;
    int argSize;
    int argPartId;
} SceLoadCoreBootModuleInfo;

// Sony flash0 files
BootFileList* boot_files = (BootFileList*)BOOT_FILE_LIST_ADDR;

static int cur_file = 14;

SceUID sceKernelLoadModuleBufferBootInitBtcnfPatched(SceLoadCoreBootModuleInfo *info, void *buf, int flags, SceKernelLMOption *option) {

    char path[64];

    sprintf(path, "ms0:/__ADRENALINE__/flash0%s", (char*)&(boot_files->bootfile[cur_file])); //not use flash0 cause of cxmb

    cur_file++;

    SceUID mod = sceKernelLoadModule(path, 0, NULL);
    if (mod >= 0)
        return mod;

    return sceKernelLoadModuleBufferBootInitBtcnf661(info->size, buf, flags, option);
}

SceUID (* LoadModuleBufferAnchorInBtcnf)(void *buf, int a1);
SceUID LoadModuleBufferAnchorInBtcnfPatched(void *buf, SceLoadCoreBootModuleInfo *info) {

    char path[64];

    sprintf(path, "ms0:/__ADRENALINE__/flash0%s", (char*)&(boot_files->bootfile[cur_file]));

    cur_file++;

    SceUID mod = sceKernelLoadModule(path, 0, NULL);
    if (mod >= 0)
        return mod;

    return LoadModuleBufferAnchorInBtcnf(buf, (info->attr >> 8) & 1);
}

int (*ARKPatchInit)(int (* module_bootstart)(SceSize, void *), void *argp) = NULL;
int AdrenalinePatchInit(int (* module_bootstart)(SceSize, void *), void *argp) {
    u32 init_addr = ((u32)module_bootstart) - 0x1A54;

    // Ignore StopInit
    _sw(0, init_addr + 0x18EC);

    // Redirect load functions to load from MS
    LoadModuleBufferAnchorInBtcnf = (void *)init_addr + 0x1038;
    MAKE_CALL(init_addr + 0x17E4, LoadModuleBufferAnchorInBtcnfPatched);
    _sw(0x02402821, init_addr + 0x17E8); //move $a1, $s2

    _sw(0x02402021, init_addr + 0x1868); //move $a0, $s2
    MAKE_CALL(init_addr + 0x1878, sceKernelLoadModuleBufferBootInitBtcnfPatched);

    sctrlFlushCache();

    return ARKPatchInit(module_bootstart, argp);
}

// Patch Loader Core Module
SceModule* patchLoaderCore(void)
{
    // Find Module
    SceModule* mod = (SceModule *)sceKernelFindModuleByName("sceLoaderCore");

    for (int i=0; i<boot_files->nfiles; i++){
        if (strcmp((char*)&(boot_files->bootfile[i]), "/kd/init.prx") == 0){
        	cur_file = i+1;
        	break;
        }
    }

    // Fetch Text Address
    u32 start_addr = mod->text_addr;
    u32 topaddr = mod->text_addr+mod->text_size;

    for (u32 addr = start_addr; addr<topaddr; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x02E02021){
            ARKPatchInit = (void*)K_EXTRACT_CALL(addr-4);
            _sw(JAL(AdrenalinePatchInit), addr-4);
            break;
        }
    }
    sctrlFlushCache();
    return mod;
}