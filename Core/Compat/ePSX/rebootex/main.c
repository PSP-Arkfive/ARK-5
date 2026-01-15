#include <string.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <bootloadex.h>
#include <bootloadex_ark.h>


int pspemuLfatOpenArkEPSX(BootFile* file)
{
    char* p = file->name;
    if (strcmp(p, "pspbtcnf.bin") == 0){
        p[2] = 'x'; // custom btcnf for VitaPops
    }
    return -1;
}


BootLoadExConfig bleconf = {
    .boot_type = TYPE_REBOOTEX,
    .boot_storage = FLASH_BOOT,
    .extra_io = {
        .vita_io = {
            .redirect_flash = 1,
            .pspemuLfatOpenExtra = &pspemuLfatOpenArkEPSX
        }
    }
};


// Entry Point
int cfwBoot(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7)
{
    #ifdef DEBUG
    _sw(0x44000000, 0xBC800100);
    colorDebug(0xFF00);
    #endif

    // Configure
    configureBoot(&bleconf);

    // scan functions
    findBootFunctions();
    
    // patch sceboot
    patchBootVita();
    
    // Forward Call
    return sceBoot(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
}
