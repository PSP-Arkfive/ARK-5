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

struct {
    u32 cur_buttons;
    u32 button_on;
    int stop_flag;
    int menu_mode;
    int is_registered;
    int show_info;
} vshmenu;

u32 patch_addr;
char info_string[128];


int (*scePafAddClockOrig)(ScePspDateTime*, wchar_t*, int, wchar_t*) = NULL;

int EatKey(SceCtrlData *pad_data, int count)
{

    // buttons check
    vshmenu.button_on   = ~vshmenu.cur_buttons & pad_data[0].Buttons;
    vshmenu.cur_buttons = pad_data[0].Buttons;

    // mask buttons for LOCK VSH control
    for (int i=0; i<count; i++) {
        pad_data[i].Buttons &= ~(
                PSP_CTRL_SELECT|PSP_CTRL_START|
                PSP_CTRL_UP|PSP_CTRL_RIGHT|PSP_CTRL_DOWN|PSP_CTRL_LEFT|
                PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|
                PSP_CTRL_TRIANGLE|PSP_CTRL_CIRCLE|PSP_CTRL_CROSS|PSP_CTRL_SQUARE|
                PSP_CTRL_HOME|PSP_CTRL_NOTE);

    }

    return 0;
}

int scePafAddClockPatched(ScePspDateTime* time, wchar_t* str, int max_len, wchar_t* format) {
    if (vshmenu.is_registered){
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
        "Console: %s FW%d%d%d\n"
        "Bootloader: %s",
        sctrlSEGetVersion(),
        sctrlHENGetVersion(),
        sctrlHENGetMinorVersion(),
        __DATE__, __TIME__,
        console_type, major, minor, micro,
        ark_config.exploit_id
    );

    _sh(0, addr - 0x48);
    patch_addr = addr;

    sceKernelDcacheWritebackAll();
    kuKernelIcacheInvalidateAll();

}

int menu_ctrl(u32 button_on)
{
    if ((button_on & PSP_CTRL_SELECT) || (button_on & PSP_CTRL_HOME)) {
        return 1;
    }
    return 0; // continue
}

static void button_func(void)
{
    // menu controll
    switch (vshmenu.menu_mode) {
        case 0:    
            if ((vshmenu.cur_buttons & ALL_CTRL) == 0) {
                vshmenu.menu_mode = 1;
            }
            break;
        case 1:
            if (menu_ctrl(vshmenu.button_on))
				vshmenu.menu_mode = 2;
            break;
		case 2:
			if ((vshmenu.cur_buttons & ALL_CTRL) == 0)
				vshmenu.stop_flag = 1;
			break;
    }
}

int TSRThread(SceSize args, void *argp)
{

    sceKernelChangeThreadPriority(0, 8);
    vctrlVSHRegisterVshMenu(EatKey);

    vshmenu.is_registered = 1;
    while (!vshmenu.stop_flag) {
        if( sceDisplayWaitVblankStart() < 0)
            break; // end of VSH ?

        button_func();
    }
    vshmenu.is_registered = 0;

    vshcube_stop();
    
	vctrlVSHExitVSHMenu(NULL, NULL, 0);
    return sceKernelExitDeleteThread(0);
}

int xmbctrlEnterVshMenuMode(){

    memset(&vshmenu, 0, sizeof(vshmenu));
    vshmenu.cur_buttons = 0xFFFFFFFF;

    if (scePafAddClockOrig == NULL){
        scePafAddClockOrig = (void*)U_EXTRACT_CALL(patch_addr + 4);
        MAKE_CALL(patch_addr + 4, (u32)&scePafAddClockPatched);
        sceKernelDcacheWritebackAll();
        kuKernelIcacheInvalidateAll();
    }

    vshcube_start();

    SceUID thread_id = sceKernelCreateThread("VshMenu_Thread", (void*)KERNELIFY(TSRThread), 16 , 0x1000 , 0 , 0);
    return sceKernelStartThread(thread_id, 0, 0);
}
