#include <string.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <bootloadex.h>

ExtraIoFuncs iofuncs = {
    .vita_io = {
        .redirect_flash = 1,
        .pspemuLfatOpenExtra = &pspemuLfatOpenExtraEPSX
    }
};


// Entry Point
int cfwBoot(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7)
{
    #ifdef DEBUG
    colorDebug(0xff00);
    #endif

    // Configure
    bootConfig(FLASH_BOOT, TYPE_REBOOTEX, &iofuncs);

    // scan for reboot functions
    findBootFunctions();
    
    // patch reboot buffer
    patchBootVita();
    
    // Forward Call
    return sceReboot(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
}
