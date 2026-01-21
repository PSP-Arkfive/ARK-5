#include <string.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <bootloadex.h>
#include <bootloadex_ark.h>


int findArkFlashFile(BootFile* file, const char* path){
    u32 nfiles = *(u32*)(VITA_FLASH_ARK);
    ArkFlashFile* cur = (ArkFlashFile*)((size_t)(VITA_FLASH_ARK)+4);

    for (int i=0; i<nfiles; i++){
        size_t filesize = (cur->filesize[0]) + (cur->filesize[1]<<8) + (cur->filesize[2]<<16) + (cur->filesize[3]<<24);
        if (strncmp(path, cur->name, cur->namelen) == 0){
            file->buffer = (void*)((size_t)(&(cur->name[0])) + cur->namelen);
            file->size = filesize;
            return 0;
        }
        cur = (ArkFlashFile*)((size_t)(cur)+filesize+cur->namelen+5);
    }
    return -1;
}

int pspemuLfatOpenArkVPSP(BootFile* file){
    
    int ret = -1;
    char* p = file->name;
    
    if (strcmp(p, "pspbtcnf.bin") == 0){
        p[2] = 'v'; // custom btcnf for PS Vita
        switch(reboot_conf->iso_mode) {
            case MODE_ME:
            case MODE_INFERNO:
            case MODE_MARCH33:
            case MODE_OE_LEGACY:
            case MODE_VSHUMD: // inferno ISO mode
                reboot_conf->iso_mode = MODE_INFERNO;
                p[5] = 'j'; // use inferno ISO mode (psvbtjnf.bin)
                ret = findArkFlashFile(file, p);
                break;
            default: // np9660 mode
                p[5] = 'k'; // use np9660 ISO mode (psvbtknf.bin)
                ret = findArkFlashFile(file, p);
                break;
        }
        if (ret == 0){
            relocateFlashFile(file);
        }
    }
    else if (strncmp(p, "/kd/ark_", 8) == 0){ // ARK module
        ret = findArkFlashFile(file, p);
        if (ret == 0){
            relocateFlashFile(file);
        }
    }

    return ret;
}


BootLoadExConfig bleconf = {
    .boot_type = TYPE_REBOOTEX,
    .boot_storage = FLASH_BOOT,
    .extra_io = {
        .vita_io = {
            .redirect_flash = 0,
            .pspemuLfatOpenExtra = &pspemuLfatOpenArkVPSP
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

    // initialize ARK reboot config
    initArkRebootConfig(&bleconf);

    // Configure
    configureBoot(&bleconf);

    // scan for reboot functions
    findBootFunctions();
    
    // patch reboot buffer
    patchBootVita();
    
    // Forward Call
    return sceBoot(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
}
