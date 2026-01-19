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
#include <systemctrl_private.h>

#include "imports.h"
#include "modulemanager.h"
#include "cryptography.h"
#include "mediasync.h"
#include "msstor_cache.h"
#include "rebootex.h"
#include "sysmem.h"
#include "psnfix.h"
#include "loadexec.h"
#include "gameinfo.h"
#include "exitgame.h"

extern u32 sctrlHENFakeDevkitVersion();
extern int is_plugins_loading;
extern SEConfigARK se_config;

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;

#ifdef DEBUG
#include <screenprinter.h>
#include <colordebugger.h>
// for screen debugging
int (* DisplaySetFrameBuf)(void*, int, int, int) = NULL;
#endif

static unsigned int fakeFindFunction(char * szMod, char * szLib, unsigned int nid){
    if (nid == 0x221400A6 && strcmp(szMod, "SystemControl") == 0)
        return 0; // Popsloader V4 looks for this function to check for ME, let's pretend ARK doesn't have it ;)
    return sctrlHENFindFunction(szMod, szLib, nid);
}

int _sceChkreg_6894A027(u8* a0, u32 a1){
    if (a0 && a1 == 0){
        *a0 = 1;
        return 0;
    }
    return -1;
}

void patch_qaflags(){
    u32 fp;

    // sceChkregGetPsCode
    fp = sctrlHENFindFunction("sceChkreg", "sceChkreg_driver", 0x6894A027);

    if (fp) {
        _sw(JUMP(_sceChkreg_6894A027), fp);
        _sw(NOP, fp+4);
    }
}

// Module Start Handler
static int ARKSyspatchOnModuleStart(SceModule * mod)
{

    // System fully booted Status
    static int booted = 0;

    patchGameInfoGetter(mod);

    // Fix 6.60 plugins on 6.61
    if (is_plugins_loading){
        sctrlHookImportByNID(mod, "SysMemForKernel", 0x3FC9AE6A, &sctrlHENFakeDevkitVersion);
        sctrlHookImportByNID(mod, "SysMemUserForUser", 0x3FC9AE6A, &sctrlHENFakeDevkitVersion);
    }

    #ifdef DEBUG
    printk("syspatch: %s(0x%04X)\r\n", mod->modname, sceKernelInitApitype());
    sctrlHookImportByNID(mod, "KDebugForKernel", 0x84F370BC, printk);

    if (sceKernelFindModuleByName("vsh_module") == NULL){
        initScreen(DisplaySetFrameBuf);
        PRTSTR1("Module: %s", mod->modname);
    }

    if(strcmp(mod->modname, "sceDisplay_Service") == 0)
    {
        // can use screen now
        DisplaySetFrameBuf = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);
        goto flush;
    }

    #endif

    if (strcmp(mod->modname, "sceController_Service") == 0){
        // Allow exiting through key combo
        patchController(mod);
        goto flush;
    }

    if(strcmp(mod->modname, "sceLoadExec") == 0)
    {
        // Find Reboot Loader Function
        OrigLoadReboot = (void *)mod->text_addr;
        // Patch loadexec
        patchLoadExec(mod, (u32)LoadReboot, (u32)sctrlHENFindFunction("sceThreadManager", "ThreadManForKernel", 0xF6427665), 3);

        // Hijack all execute calls
        extern int (* _sceLoadExecVSHWithApitype)(int, const char*, struct SceKernelLoadExecVSHParam*, unsigned int);
        extern int sctrlKernelLoadExecVSHWithApitype(int apitype, const char * file, struct SceKernelLoadExecVSHParam * param);
        u32 _LoadExecVSHWithApitype = sctrlHENFindFirstJAL(sctrlHENFindFunction("sceLoadExec", "LoadExecForKernel", 0xD8320A28));
        HIJACK_FUNCTION(_LoadExecVSHWithApitype, sctrlKernelLoadExecVSHWithApitype, _sceLoadExecVSHWithApitype);

        // Hijack exit calls
        extern int (*_sceKernelExitVSH)(void*);
        extern int sctrlKernelExitVSH(struct SceKernelLoadExecVSHParam *param);
        u32 _KernelExitVSH = sctrlHENFindFunction("sceLoadExec", "LoadExecForKernel", 0x08F7166C);
        HIJACK_FUNCTION(_KernelExitVSH, sctrlKernelExitVSH, _sceKernelExitVSH);
        goto flush;
    }

    if (strcmp(mod->modname, "sceImpose_Driver") == 0){
        // Handle extra ram setting
        if (se_config.force_high_memory){
            sctrlHENApplyMemory(MAX_HIGH_MEMSIZE);
        }
    }

    // Media Sync about to start...
    if(strcmp(mod->modname, "sceMediaSync") == 0)
    {
        // Patch mediasync.prx
        patchMediaSync(mod);
        // Exit Handler
        goto flush;
    }

    // MesgLed Cryptography about to start...
    if(strcmp(mod->modname, "sceMesgLed") == 0)
    {
        // Patch mesg_led_01g.prx
        patchMesgLed(mod);
        // Exit Handler
        goto flush;
    }

    // unlocks mp3 variable bitrate and qwerty osk on old games/homebrew
    if (strcmp(mod->modname, "sceMp3_Library") == 0 || strcmp(mod->modname, "sceVshOSK_Module") == 0){
        sctrlHookImportByNID(mod, "SysMemUserForUser", 0xFC114573, &sctrlHENFakeDevkitVersion);
        goto flush;
    }

    if (strcmp(mod->modname, "sceNpSignupPlugin_Module") == 0) {
        patch_npsignup(mod);
        goto flush;
    }

    if (strcmp(mod->modname, "sceVshNpSignin_Module") == 0) {
        patch_npsignin(mod);
        goto flush;
    }

    if (strcmp(mod->modname, "sceNp") == 0) {
        patch_np(mod, 9, 90);
        goto flush;
    }

    if (strcmp(mod->modname, "popsloader") == 0 || strcmp(mod->modname, "popscore") == 0){
        // fix for 6.60 check on 6.61
        sctrlHookImportByNID(mod, "SysMemForKernel", 0x3FC9AE6A, &sctrlHENFakeDevkitVersion);
        // fix to prevent ME detection
        sctrlHookImportByNID(mod, "SystemCtrlForKernel", 0x159AF5CC, &fakeFindFunction);
        goto flush;
    }

    // Boot Complete Action not done yet
    if(booted == 0)
    {
        // Boot is complete
        if(sctrlHENIsSystemBooted())
        {

            // remember last played game
            extern int isLauncher();
            if (sceKernelInitKeyConfig() != PSP_INIT_KEYCONFIG_VSH && !isLauncher()){
                rebootex_config.last_played.apitype = sceKernelInitApitype();
                memcpy(rebootex_config.last_played.game_id, rebootex_config.game_id, 10);
                strcpy(rebootex_config.last_played.path, sceKernelInitFileName());
            }

            // extend utility modules
            extern void extendUtilityModules();
            extendUtilityModules();

            // patch QA flags settings
            if (se_config.qaflags){
                patch_qaflags();
            }

            // handle UMD seek and UMD speed settings
            if (se_config.umdseek || se_config.umdspeed){
                se_config.iso_cache_type = 0;
                void (*SetUmdDelay)(int, int) = (void*)sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0xB6522E93);
                if (SetUmdDelay) SetUmdDelay(se_config.umdseek, se_config.umdspeed);
            }

            // handle inferno cache settings
            if (se_config.iso_cache_type){
                extern int p2_size;
                if (p2_size>24 || se_config.force_high_memory){
                    se_config.iso_cache_partition = 2;
                }
                // set cache policy first
                int (*CacheSetPolicy)(int) = (void*)sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0xC0736FD6);
                if (CacheSetPolicy){
                    CacheSetPolicy(se_config.iso_cache_type);
                }
                // enable cache
                int (*CacheInit)(u32, u32, u32) = (void*)sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0x8CDE7F95);
                if (CacheInit){
                    CacheInit((u32)se_config.iso_cache_size_kb*1024, se_config.iso_cache_num, se_config.iso_cache_partition);
                }
            }

            if (sctrlHENIsToolKit() != PSP_TOOLKIT_TYPE_DEV){
                // handle CPU speed settings
                switch (se_config.cpubus_clock){
                    case 1: sctrlHENSetSpeed(333, 166); break;
                    case 2: sctrlHENSetSpeed(133, 66); break;
                    case 3: sctrlHENSetSpeed(222, 111); break;
                }
            }

            #ifdef DEBUG
            // syncronize printk
            printkSync();
            #endif

            ark_config->recovery = 0; // reset recovery mode for next reboot

            // Boot Complete Action done
            booted = 1;
            goto flush;
        }
    }

    // No need to flush
    goto exit;

flush:
    // Flush Cache
    sctrlFlushCache();

exit:
    // Forward to previous Handler
    if(previous) return previous(mod);
    return 0;
}

// Add Module Start Patcher
void syspatchInit(void)
{
    // Register Module Start Handler
    previous = sctrlHENSetStartModuleHandler(ARKSyspatchOnModuleStart);
}
