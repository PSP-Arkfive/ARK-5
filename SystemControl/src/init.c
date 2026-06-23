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

#include "config.h"
#include "controller.h"
#include "gameinfo.h"


extern SEConfigARK se_config;
extern ARKConfig ark_config;

// init.prx Text Address
u32 sceInitTextAddr = 0;

// init.prx Custom sceKernelStartModule Handler
int (* customStartModule)(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt) = NULL;


static void checkArkPath(){
    if (strcmp(ark_config.arkpath, SEPLUGINS_MS0) == 0){ // attempt revert to default path 
        strcpy(ark_config.arkpath, DEFAULT_ARK_PATH_GO);
    }
    int res = sceIoDopen(ark_config.arkpath);
    if (res < 0){
        // fix for PSP-Go with dead ef (or non-Go units)
        if (ark_config.arkpath[0]=='e' && ark_config.arkpath[1]=='f'){
            ark_config.arkpath[0] = 'm'; ark_config.arkpath[1] = 's';
        }
        else {
            ark_config.arkpath[0] = 'e'; ark_config.arkpath[1] = 'f';
        }
        if ((res=sceIoDopen(ark_config.arkpath))>=0){
            sceIoDclose(res);
            return;
        }
        // no ARK install folder, default to SEPLUGINS
        strcpy(ark_config.arkpath, SEPLUGINS_MS0);
        sceIoMkdir(SEPLUGINS_MS0, 0777);
    }
    else{
        sceIoDclose(res);
    }
}

// Init Start Module Hook
static int InitKernelStartModule(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt)
{
    char modname[28]; memset(modname, 0, sizeof(modname));
    SceModule* mod = (SceModule*) sceKernelFindModuleByUID(modid);
    strncpy(modname, mod->modname, sizeof(modname));

    int result = -1;
    u32* vshmain_args = NULL;

    // VSH replacement
    if (strcmp(modname, "vsh_module") == 0){
        // system in recovery or launcher mode
        if (ark_config.recovery || ark_config.launcher[0] || se_config.launcher_mode){
            int (*LoadExecForKernel_AA2029EC)() = (int(*)())sctrlHENFindFunction("sceLoadExec", "LoadExecForKernel", 0xAA2029EC);
            if (LoadExecForKernel_AA2029EC) LoadExecForKernel_AA2029EC();
            sctrlArkExitLauncher(); // reboot VSH into custom menu
            MAKE_DUMMY_FUNCTION_RETURN_0(mod->entry_addr);
        }
        // skip bootup animation
        if (se_config.skiplogos == 1 || se_config.skiplogos == 3) {
            vshmain_args = oe_malloc(1024);
            memset(vshmain_args, 0, 1024);
    
            if (argp != NULL && argsize != 0 ) {
                memcpy( vshmain_args , argp ,  argsize);
            }
    
            vshmain_args[0] = 1024;
            vshmain_args[1] = 0x20;
            vshmain_args[16] = 1;
            argp = vshmain_args;
            argsize = 1024;
        }
    }

    // Custom Handler registered
    if(customStartModule != NULL)
    {
        // Forward to Handler
        result = customStartModule(modid, argsize, argp, modstatus, opt);
    }

    // load settings before impose module
    if (strcmp(modname, "sceImpose_Driver") == 0){
        // Read Game ID
        findGameId();
        // Check ARK install path
        checkArkPath();
        // Check controller input to disable settings and/or plugins
        checkControllerInput();
        // load settings
        loadSettings();
    }

    // load plugins before starting mediasync
    if (strcmp(modname, "sceMediaSync") == 0)
    {
        // Load Plugins
        loadPlugins();
    }
    
    // start module
    if (result < 0) result = sceKernelStartModule(modid, argsize, argp, modstatus, opt);

    // cleanup
    if (vshmain_args) oe_free(vshmain_args);

    return result;
}

// sceKernelStartModule Hook
int patch_sceKernelStartModule_in_bootstart(int (* bootstart)(SceSize, void *), void * argp)
{
    u32 StartModule = JUMP(sctrlHENFindFunction("sceModuleManager", "ModuleMgrForUser", 0x50F0C1EC));
    u32 addr = (u32)bootstart;
    int patches = 1;
    for (;patches; addr+=4){
        if (_lw(addr) == StartModule){
            // Replace Stub
            _sw(JUMP(InitKernelStartModule), addr);
            _sw(NOP, addr + 4);
            patches--;
        }
    }
    // Passthrough
    return bootstart(4, argp);
}
