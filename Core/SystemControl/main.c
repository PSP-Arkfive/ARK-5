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

#include <pspsdk.h>

#include <systemctrl.h>
#include <systemctrl_ark.h>

#include "rebootex.h"
#include "modulemanager.h"
#include "loadercore.h"
#include "interruptman.h"
#include "cryptography.h"
#include "syspatch.h"
#include "sysmem.h"
#include "nidresolver.h"
#include "exception.h"


PSP_MODULE_INFO("SystemControl", 0x3007, 4, 0);

// default config when none provided by the bootloader
static ARKConfig _ark_conf = {
    .magic = ARK_CONFIG_MAGIC,
    .arkpath = DEFAULT_ARK_PATH,
    .exploit_id = {0}, // None by default
    .exec_mode = DEV_UNK, // set by compat layer
    .recovery = 0,
};
ARKConfig* ark_config = &_ark_conf;


// Boot Time Entry Point
int module_start(SceSize args, void * argp)
{

    #if 0
    // set LCD framebuffer in hardware reg so we can do color debbuging
    _sw(0x44000000, 0xBC800100);
    colorDebug(0xFF00);
    #endif

    // Apply Module Patches
    patchSystemMemoryManager();
    SceModule* loadcore = patchLoaderCore();
    patchModuleManager();
    patchInterruptMan();
    patchMemlmd();

    // Flush Cache
    sctrlFlushCache();

    // setup NID resolver on loadercore
    setupNidResolver(loadcore);

    // Initialize Module Start Patching
    syspatchInit();

    // Backup Reboot Buffer (including configuration)
    backupRebootBuffer();

    uprotectExtraMemory();

    // Flush Cache
    sctrlFlushCache();

    // Return Success
    return 0;
}

