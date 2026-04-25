#include <stdio.h>
#include <string.h>
#include <pspkernel.h>
#include <psputility.h>
#include <pspdisplay.h>
#include <psprtc.h>
#include <psppower.h>

#include <systemctrl_ark.h>
#include <cfwmacros.h>
#include <vshctrl.h>
#include <kubridge.h>
#include <systemctrl.h>

#include "main.h"
#include "utils.h"

extern int psp_model;
extern ARKConfig ark_config;

u32 patch_addr;
char info_string[128];
int vshmenu_running  = 0;
int menu_mode = 0;
u32 button_on = 0;
u32 cur_buttons = 0xFFFFFFFF;
void (*vshmenu_draw)(void* frame) = NULL;
int (*scePafAddClockOrig)(ScePspDateTime*, wchar_t*, int, wchar_t*) = NULL;

int scePafAddClockPatched(ScePspDateTime* time, wchar_t* str, int max_len, wchar_t* format) {
    if (vshmenu_running){
        return utf8_to_unicode(str, info_string);
    }
    else {
        return scePafAddClockOrig(time, str, max_len, format);
    }
}

void patchVshClock(u32 addr){

    u32 fw = sceKernelDevkitVersion();
    u32 major = fw>>24;
    u32 minor = (fw>>16)&0xF;
    u32 micro = (fw>>8)&0xF;
    
    char* console_type = "";

    if (IS_VITA((&ark_config))){
        console_type = "PS Vita";
    }
    else {
        switch (sctrlHENIsToolKit()){
            default:
            case 0:
                switch (psp_model) {
                    default:
                    case 0: console_type = "PSP Phat"; break;
                    case 1: console_type = "PSP Lite"; break;
                    case 2: case 3: case 6:
                    case 8: console_type = "PSP Brite"; break;
                    case 4: console_type = "PSP Go"; break;
                    case 10: console_type = "PSP Street"; break;
                    
                }
                break;
            case 1: console_type = "PSP TestingTool"; break;
            case 2: console_type = "PSP DevelopmentTool"; break;
        }
    }

    sce_paf_private_sprintf(info_string, "\n\n\n\n"
        "CFW: ARK %d.%d.%d\n"
        "Built: %s %s\n"
        "Console: %s (%02dg) FW%d%d%d\n"
        "Bootloader: %s",
        sctrlSEGetVersion(),
        sctrlHENGetVersion(),
        sctrlHENGetMinorVersion(),
        __DATE__, __TIME__,
        console_type, psp_model+1, major, minor, micro,
        ark_config.exploit_id
    );

    _sh(0, addr - 0x48);
    patch_addr = addr;

    sceKernelDcacheWritebackAll();
    kuKernelIcacheInvalidateAll();

}

int EatKey(SceCtrlData *pad_data, int count)
{

    button_on   = ~cur_buttons & pad_data[0].Buttons;
    cur_buttons = pad_data[0].Buttons;

    // menu control
    switch (menu_mode) {
        case 0:    
            if ((cur_buttons & ALL_CTRL) == 0) {
                menu_mode = 1;
            }
            break;
        case 1:
            if ((button_on & PSP_CTRL_SELECT) || (button_on & PSP_CTRL_HOME))
				menu_mode = 2;
            break;
		case 2:
			if ((cur_buttons & ALL_CTRL) == 0)
				vctrlVSHExitVSHMenu(NULL, NULL, 0);
			break;
    }

    // mask buttons for LOCK VSH control
    for (int i=0; i<count; i++) {
        pad_data[i].Buttons = 0;
    }

    return 0;
}

int xmbctrlEnterVshMenuMode(){
    menu_mode = 0;
    button_on = 0;
    cur_buttons = 0xFFFFFFFF;
    vshmenu_draw = NULL;

    vctrlVSHRegisterVshMenu(EatKey);

    if (scePafAddClockOrig == NULL){
        scePafAddClockOrig = (void*)U_EXTRACT_CALL(patch_addr + 4);
        MAKE_CALL(patch_addr + 4, (u32)&scePafAddClockPatched);
        sctrlFlushCache();
    }

    vshmenu_running = 1;
}

int xmbctrlExitVshMenuMode(){
    vshmenu_running = 0;
}

int xmbctrlRegisterVshMenu(void (*draw_func)(void*)){
    vshmenu_draw = draw_func;
}
