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
#include <pspsysmem_kernel.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <pspumd.h>

#include <ark.h>
#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>

#include "main.h"
#include "custom_update.h"


PSP_MODULE_INFO("VshCtrl", 0x1007, 1, 2);

u32 psp_model = 0;
ARKConfig* ark_config = NULL;
SEConfig* se_config = NULL;
int has_umd_iso = 0;
int _150_addon_enabled = 0;
char mounted_iso[64];

void enable_150_addon()
{
    SceIoStat stat;

    if (psp_model == PSP_1000 && sceKernelDevkitVersion() == FW_661) {
        memset(&stat, 0, sizeof(stat));
        if (sceIoGetstat(ARK_DC_PATH "/150", &stat) >= 0)
            _150_addon_enabled = 1;
    }
}

int get_device_name(char *device, int size, const char* path)
{
    const char *p;

    if (path == NULL || device == NULL) {
        return -1;
    }

    p = strchr(path, '/');

    if (p == NULL) {
        return -2;
    }

    strncpy(device, path, MIN(size, p-path+1));
    device[MIN(size-1, p-path)] = '\0';

    return 0;
}

void unlockGeMemory() {
    const unsigned int start = 0xbc000000;
    const unsigned int end   = 0xbc00002c;
    for (unsigned int reg = start; reg <= end; reg += 4) {
        (*((volatile unsigned int*)(reg))) = -1;
    }
    asm volatile("sync");
}

int module_start(SceSize args, void* argp)
{
    #ifdef DEBUG
    printk_init("ms0:/log_vshctrl.txt");
    printk("VshCtrl started\n");
    #endif
    
    psp_model = sceKernelGetModel();
    ark_config = sctrlArkGetConfig(NULL);
    se_config = sctrlSEGetConfigInternal();

    if (ark_config->recovery) return 0;

    vshpatch_init();
    load_server_file();
    
    // always reset to NORMAL mode in VSH
    // to avoid ISO mode is used in homebrews in next reboot
    char* umdfile = sctrlSEGetUmdFile();
    has_umd_iso = (umdfile[0] != 0 && sctrlSEGetBootConfFileIndex() == MODE_VSHUMD);
    if (has_umd_iso) strcpy(mounted_iso, umdfile);
    sctrlSESetUmdFile("");
    sctrlSESetBootConfFileIndex(MODE_UMD);

    if (has_umd_iso){
        // disable launcher mode if using VSH ISO
        ark_config->launcher[0] = 0;
    }

    enable_150_addon();

    unlockGeMemory();

    return 0;
}
