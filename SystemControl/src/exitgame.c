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
#include <pspsdk.h>
#include <pspinit.h>
#include <pspkernel.h>
#include <pspctrl.h>

#include <systemctrl_ark.h>
#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>


extern ARKConfig ark_config;


int sctrlArkIsLauncher(){
    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config.arkpath);
    if (ark_config.launcher[0]) strcat(path, ark_config.launcher);
    else                         strcat(path, VBOOT_PBP);
    return (strcmp(path, sceKernelInitFileName())==0);
}

int sctrlArkExitLauncher()
{

    int k1 = pspSdkSetK1(0);

    // Refuse Operation in Save dialog
    if(sceKernelFindModuleByName("sceVshSDUtility_Module") != NULL){
        pspSdkSetK1(k1);
        return 0;
    }
    // Refuse Operation in Dialog
    if(sceKernelFindModuleByName("sceDialogmain_Module") != NULL){
        pspSdkSetK1(k1);
        return 0;
    }

    // set exit app
    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config.arkpath);
    if (ark_config.recovery) strcat(path, ARK_RECOVERY);
    else if (ark_config.launcher[0]) strcat(path, ark_config.launcher);
    else strcat(path, VBOOT_PBP);

    int (*setHoldMode)(int) = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x7ED59BC4);
    if (setHoldMode) setHoldMode(0);

    // reset some flags
    SetUmdFile(NULL);
    sctrlSESetBootConfFileIndex(MODE_UMD);

    SceIoStat stat; int res = sceIoGetstat(path, &stat);
    if (res >= 0){
        // Load Execute Parameter
        struct SceKernelLoadExecVSHParam param;
        // Clear Memory
        memset(&param, 0, sizeof(param));
        // Configure Parameters
        param.size = sizeof(param);
        param.args = strlen(path) + 1;
        param.argp = path;
        param.key = "game";
        // Trigger Reboot
        ark_config.recovery = 0; // reset recovery mode for next reboot
        sctrlKernelLoadExecVSHWithApitype(0x141, path, &param);
    }
    else if (ark_config.recovery){
        // no recovery app? try classic module
        strcpy(path, ark_config.arkpath);
        strcat(path, RECOVERY_PRX);
        res = sceIoGetstat(path, &stat);
        if (res < 0){
        	// try flash0
        	strcpy(path, RECOVERY_PRX_FLASH);
        }
        SceUID modid = sceKernelLoadModule(path, 0, NULL);
        if(modid >= 0) {
        	sceKernelStartModule(modid, strlen(path) + 1, path, NULL, NULL);
        	ark_config.recovery = 0; // reset recovery mode for next reboot
        	ark_config.launcher[0] = 0; // reset launcher mode for next reboot
        	pspSdkSetK1(k1);
        	return 0;
        }
    }

    ark_config.recovery = 0;
    return sctrlKernelExitVSH(NULL);
}
