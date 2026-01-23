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
#include <stdlib.h>
#include <stdio.h>
#include <pspkernel.h>
#include <pspinit.h>
#include <psputilsforkernel.h>

#include <cfwmacros.h>
#include <systemctrl.h>

PSP_MODULE_INFO("PROPopcornManager", 0x1007, 1, 2);

extern STMOD_HANDLER g_previous;
extern unsigned int g_pspFwVersion;
extern int g_isCustomPBP;
extern int g_icon0Status;

extern int popcornSyspatch(SceModule *mod);
extern void patchPopsMgr(void);
extern void getKeys(void);
extern void readCustomConfig();
extern unsigned int isCustomPBP(void);
extern int getIcon0Status(void);
extern void setupPsxFwVersion(unsigned int fw_version);

int module_start(SceSize args, void* argp)
{
    #if DEBUG >= 3
    printk("popcorn: init_file = %s\r\n", sceKernelInitFileName());

    char g_DiscID[32];
    u16 paramType = 0;
    u32 paramLength = sizeof(g_DiscID);
    sctrlGetInitPARAM("DISC_ID", &paramType, &paramLength, g_DiscID);
    
    printk("pops disc id: %s\r\n", g_DiscID);
    #endif

    g_pspFwVersion = sceKernelDevkitVersion();
    
    getKeys();
    readCustomConfig();
    g_isCustomPBP = isCustomPBP();
    g_icon0Status = getIcon0Status();

    if(g_isCustomPBP)
    {
        setupPsxFwVersion(g_pspFwVersion);
    }
    
    g_previous = sctrlHENSetStartModuleHandler(popcornSyspatch);
    patchPopsMgr();
    
    sctrlFlushCache();
    
    return 0;
}
