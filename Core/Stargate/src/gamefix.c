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

#include <string.h>
#include <strings.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <pspthreadman_kernel.h>
#include <psputility_sysparam.h>

#include <ark.h>
#include <cfwmacros.h>
#include <rebootconfig.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_private.h>

extern SEConfig* se_config;

static STMOD_HANDLER game_previous;

static int (*utilityGetParam)(int, int*) = NULL;
static int getParamFixed_ULJM05221(int param, int* value){
    int res = utilityGetParam(param, value);
    if (param == PSP_SYSTEMPARAM_ID_INT_LANGUAGE && *value > 1){
        *value = 0;
    }
    return res;
}

static int wweModuleOnStart(SceModule * mod)
{
    // Boot Complete Action not done yet
    if (strcmp(mod->modname, "mainPSP") == 0)
    {
        sctrlHookImportByNID(mod, "scePower", 0x34F9C463, (void*)222); // scePowerGetPllClockFrequencyInt
        sctrlHookImportByNID(mod, "scePower", 0x843FBF43, (void*)0);   // scePowerSetCpuClockFrequency
        sctrlHookImportByNID(mod, "scePower", 0xFDB5BFE9, (void*)222); // scePowerGetCpuClockFrequencyInt
        sctrlHookImportByNID(mod, "scePower", 0xBD681969, (void*)111); // scePowerGetBusClockFrequencyInt
    }

    // Call Previous Module Start Handler
    if(game_previous) return game_previous(mod);
    return 0;
}


void applyFixesByModule(SceModule* mod){

    // fix black screen in Tekken 6
    if (strcmp(mod->modname, "tekken") == 0) {
        sctrlHookImportByNID(mod, "scePower", 0x34F9C463, (void*)222); // scePowerGetPllClockFrequencyInt
    }

    // remove "overclock" message in ATV PRO
    else if (strcmp(mod->modname, "ATVPRO") == 0){
        sctrlHookImportByNID(mod, "scePower", 0x843FBF43, (void*)0);   // scePowerSetCpuClockFrequency
        sctrlHookImportByNID(mod, "scePower", 0xFDB5BFE9, (void*)222); // scePowerGetCpuClockFrequencyInt
        sctrlHookImportByNID(mod, "scePower", 0xBD681969, (void*)111); // scePowerGetBusClockFrequencyInt
    }

    // disable anti-CFW code
    else if (strcasecmp(mod->modname, "DJMAX") == 0) {
        SEConfigARK* se_config = (SEConfigARK*)sctrlSEGetConfig(NULL);

        if (se_config->umdseek == 0 && se_config->umdspeed == 0){
            // enable UMD reading speed
            void (*SetUmdDelay)(int, int) = (void*)sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0xB6522E93);
            if (SetUmdDelay) SetUmdDelay(2, 2);
            se_config->umdseek = 2;
            se_config->umdspeed = 2;
        }

        // disable Inferno Cache
        int (*CacheInit)(int, int, int) = (void*)sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0x8CDE7F95);
        if (CacheInit) CacheInit(0, 0, 0);

        // disable memory stick cache
        msstorCacheInit(NULL);

        // prevent Inferno Cache and MS Cache from being re-enabled
        se_config->iso_cache_type = 0;
        se_config->msspeed = 0;
    }

    sctrlFlushCache();
}

void applyFixesByGameId(){
    // Obtain game ID for other patches
    RebootConfigARK* reboot_config = sctrlHENGetRebootexConfig(NULL);
    char gameid[10]; memset(gameid, 0, sizeof(gameid));
    strncpy(gameid, reboot_config->game_id, 9);

    // Fix TwinBee Portable when not using English or Japanese language
    if (strcasecmp("ULJM05221", gameid) == 0){
        utilityGetParam = (void*)sctrlHENFindFunction("sceUtility_Driver", "sceUtility", 0xA5DA2406);
        sctrlHENPatchSyscall(utilityGetParam, getParamFixed_ULJM05221);
    }

    // Patch Smakdown vs RAW 2011 anti-CFW check (CPU speed)
    else if (strcasecmp("ULES01472", gameid) == 0 || strcasecmp("ULUS10543", gameid) == 0){
        game_previous = sctrlHENSetStartModuleHandler(wweModuleOnStart);
    }

    // Patch Aces of War anti-CFW check (UMD speed)
    else if (strcasecmp("ULES00590", gameid) == 0 || strcasecmp("ULJM05075", gameid) == 0){
        SEConfigARK* se_config = (SEConfigARK*)sctrlSEGetConfig(NULL);
        if (se_config->umdseek == 0 && se_config->umdspeed == 0){
            void (*SetUmdDelay)(int, int) = (void*)sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0xB6522E93);
            if (SetUmdDelay) SetUmdDelay(1, 1);
            se_config->umdseek = 1;
            se_config->umdspeed = 1;
        }
    }
}