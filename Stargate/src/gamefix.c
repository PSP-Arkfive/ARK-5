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

#include <cfwmacros.h>
#include <rebootexconfig.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_ark.h>

#include "main.h"


static int (*utilityGetParam)(int, int*) = NULL;
static int getParamFixed_ULJM05221(int param, int* value){
    int res = utilityGetParam(param, value);
    if (param == PSP_SYSTEMPARAM_ID_INT_LANGUAGE && *value > 1){
        *value = 0;
    }
    return res;
}

void applyFixesByModule(SceModule* mod){

    // Obtain game ID for other patches
    char* gameid = reboot_config->game_id;

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
        // Patch CFW dirs
        hide_cfw_folder(mod);
		//Read speed fixes under cmd 0x01F30003
    }

	else if (strcmp(mod->modname, "Megpoid") == 0) {
        if (se_config->umdseek == 0 && se_config->umdspeed == 0){
            void (*SetUmdDelay)(int, int) = (void*)sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0xB6522E93);
            if (SetUmdDelay) {
				SetUmdDelay(1, 1);
			}
            se_config->umdseek = 1;
            se_config->umdspeed = 1;
        }
	}

    // fix Pangya
    else if (strcmp(mod->modname, "projectg_psp") == 0){
        // TODO: port from Adrenaline
    }

    // Patch Smakdown vs RAW 2010 & 2011 anti-CFW check (CPU speed)
    else if (strcmp(mod->modname, "mainPSP") == 0 &&
            (strcasecmp("ULES01472", gameid) == 0 ||
            strcasecmp("ULUS10543", gameid) == 0 ||
            strcasecmp("ULES01339", gameid) == 0 ||
            strcasecmp("ULUS10452", gameid) == 0 ))
    {
        sctrlHookImportByNID(mod, "scePower", 0x34F9C463, (void*)222); // scePowerGetPllClockFrequencyInt
        sctrlHookImportByNID(mod, "scePower", 0x843FBF43, (void*)0);   // scePowerSetCpuClockFrequency
        sctrlHookImportByNID(mod, "scePower", 0xFDB5BFE9, (void*)222); // scePowerGetCpuClockFrequencyInt
        sctrlHookImportByNID(mod, "scePower", 0xBD681969, (void*)111); // scePowerGetBusClockFrequencyInt
    }

    sctrlFlushCache();
}

void applyFixesByGameId(){
    
    // Obtain game ID for other patches
    char* gameid = reboot_config->game_id;

    // Fix TwinBee Portable when not using English or Japanese language
    if (strcasecmp("ULJM05221", gameid) == 0){
        utilityGetParam = (void*)sctrlHENFindFunction("sceUtility_Driver", "sceUtility", 0xA5DA2406);
        sctrlHENPatchSyscall(utilityGetParam, getParamFixed_ULJM05221);
    }

    // Patch games that have issues with high read speeds
    else if (strcasecmp("ULES00590", gameid) == 0   // Aces of War (EU)
            || strcasecmp("ULJM05075", gameid) == 0 // Aces of War (JP)
            || strcasecmp("ULES00233", gameid) == 0 // Bust-a-Move Ghost (EU)
            || strcasecmp("ULUS10438", gameid) == 0 // Pangya (US)
    ){
        if (se_config->umdseek == 0 && se_config->umdspeed == 0){
            void (*SetUmdDelay)(int, int) = (void*)sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0xB6522E93);
            if (SetUmdDelay) SetUmdDelay(1, 1);
            se_config->umdseek = 1;
            se_config->umdspeed = 1;
        }
    }

    // patch games that have issues with inferno cache
    else if (strcasecmp("ULUS10201", gameid) == 0   // Luxor - The Wrath of Set (US)
            || strcasecmp("ULUS10328", gameid) == 0 // Flat-Out Head On (US)
            || strcasecmp("ULES00968", gameid) == 0 // Flat-Out Head On (EU)
    ){
        // enable cache
        int (*CacheInit)(u32, u32, u32) = (void*)sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0x8CDE7F95);
        if (CacheInit){
            CacheInit(0, 0, 0);
        }
        se_config->iso_cache_type = 0;
    }
}
