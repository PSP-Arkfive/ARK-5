#include <string.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <bootloadex.h>

// Entry Point
int cfwBoot(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7)
{
    #ifdef DEBUG
    colorDebug(0xff00);
    #endif

    // check config
    checkRebootConfig();

    // scan for reboot functions
    findRebootFunctions();
    
    // patch reboot buffer
    redirect_flash = 1;
    pspemuLfatOpenExtra = &pspemuLfatOpenExtraEPSX;
    patchRebootBufferVita();
    
    // Forward Call
    return sceReboot(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
}
