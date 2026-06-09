#include <string.h>
#include <pspsdk.h>
#include <pspinit.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <pspaudio.h>

#include <systemctrl_ark.h>
#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>

// Exit Button Mask
#define EXIT_MASK_CL (PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_START | PSP_CTRL_DOWN)
#define EXIT_MASK_VSH (PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_SELECT | PSP_CTRL_DOWN)
#define MUTE_MASK (PSP_CTRL_VOLUP | PSP_CTRL_VOLDOWN)

extern SEConfigARK se_config;
extern ARKConfig* ark_config;
extern int disable_plugins;
extern int disable_settings;

static int (*_sceImposeSetParam)(u32, int);


static int exitVsh(){

    int k1 = pspSdkSetK1(0);

    // Refuse Operation in Save dialog
    if(sceKernelFindModuleByName("sceVshSDUtility_Module") != NULL) return 0;
    
    // Refuse Operation in Dialog
    if(sceKernelFindModuleByName("sceDialogmain_Module") != NULL) return 0;

    int (*setHoldMode)(int) = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x7ED59BC4);
    if (setHoldMode) setHoldMode(0);

    // reset some flags
    ark_config->recovery = 0;
    SetUmdFile(NULL);
    sctrlSESetBootConfFileIndex(MODE_UMD);

    int res = sctrlKernelExitVSH(NULL);

    pspSdkSetK1(k1);
    return res;
}

static void startExitThread(void* exitFunc){
    // Exit to custom launcher
    int k1 = pspSdkSetK1(0);
    int intc = pspSdkDisableInterrupts();
    if (sctrlGetThreadUIDByName("ExitGamePollThread") >= 0){
        pspSdkEnableInterrupts(intc);
        return; // already exiting
    }
    int uid = sceKernelCreateThread("ExitGamePollThread", exitFunc, 1, 4096, 0, NULL);
    pspSdkEnableInterrupts(intc);
    sceKernelStartThread(uid, 0, NULL);
    sceKernelWaitThreadEnd(uid, NULL);
    sceKernelDeleteThread(uid);
    pspSdkSetK1(k1);
}

static void remove_analog_input(SceCtrlData *data, int count)
{
    for (int i=0; i<count; i++){
        data[i].Lx = 0xFF/2;
        data[i].Ly = 0xFF/2;
    }
}

// Gamepad Hook #1
int (*CtrlPeekBufferPositive)(SceCtrlData *, int) = NULL;
int peek_positive(SceCtrlData * pad_data, int count)
{
    // Capture Gamepad Input
    count = CtrlPeekBufferPositive(pad_data, count);
    
    // Check for Exit Mask
    if ((pad_data[0].Buttons & EXIT_MASK_CL) == EXIT_MASK_CL)
    {
        startExitThread(sctrlArkExitLauncher);
    }

    // Check for Exit Mask
    if ((pad_data[0].Buttons & EXIT_MASK_VSH) == EXIT_MASK_VSH)
    {
        startExitThread(exitVsh);
    }

    if (se_config.noanalog){
        remove_analog_input(pad_data, count);
    }

    if (se_config.vitamute && (pad_data[0].Buttons & MUTE_MASK) == MUTE_MASK){
        //positive_note_button(pad_data, count);
        if (_sceImposeSetParam){
            _sceImposeSetParam(0x8, 1);
        }
    }
    
    // Return Number of Input Frames
    return count;
}

// Gamepad Hook #2
int (*CtrlPeekBufferNegative)(SceCtrlData *, int) = NULL;
int peek_negative(SceCtrlData * pad_data, int count)
{
    // Capture Gamepad Input
    count = CtrlPeekBufferNegative(pad_data, count);
    
    // Check for Exit Mask
    if((pad_data[0].Buttons & EXIT_MASK_CL) == 0)
    {
        startExitThread(sctrlArkExitLauncher);
    }
    
    // Check for Exit Mask
    if((pad_data[0].Buttons & EXIT_MASK_VSH) == 0)
    {
        startExitThread(exitVsh);
    }

    if (se_config.noanalog){
        remove_analog_input(pad_data, count);
    }

    if (se_config.vitamute && (pad_data[0].Buttons & MUTE_MASK) == 0){
        //negative_note_button(pad_data, count);
        if (_sceImposeSetParam){
            _sceImposeSetParam(0x8, 1);
        }
    }

    // Return Number of Input Frames
    return count;
}

// Gamepad Hook #3
int (*CtrlReadBufferPositive)(SceCtrlData *, int) = NULL;
int read_positive(SceCtrlData * pad_data, int count)
{
    // Capture Gamepad Input
    count = CtrlReadBufferPositive(pad_data, count);
    
    // Check for Exit Mask
    if((pad_data[0].Buttons & EXIT_MASK_CL) == EXIT_MASK_CL)
    {
        startExitThread(sctrlArkExitLauncher);
    }

    // Check for Exit Mask
    if((pad_data[0].Buttons & EXIT_MASK_VSH) == EXIT_MASK_VSH)
    {
        startExitThread(exitVsh);
    }

    if (se_config.noanalog){
        remove_analog_input(pad_data, count);
    }

    if (se_config.vitamute && (pad_data[0].Buttons & MUTE_MASK) == MUTE_MASK){
        //positive_note_button(pad_data, count);
        if (_sceImposeSetParam){
            _sceImposeSetParam(0x8, 1);
        }
    }
    
    // Return Number of Input Frames
    return count;
}

// Gamepad Hook #4
int (*CtrlReadBufferNegative)(SceCtrlData *, int) = NULL;
int read_negative(SceCtrlData * pad_data, int count)
{
    // Capture Gamepad Input
    count = CtrlReadBufferNegative(pad_data, count);
    
    // Check for Exit Mask
    if((pad_data[0].Buttons & EXIT_MASK_CL) == 0)
    {
        startExitThread(sctrlArkExitLauncher);
    }

    // Check for Exit Mask
    if((pad_data[0].Buttons & EXIT_MASK_VSH) == 0)
    {
        startExitThread(exitVsh);
    }

    if (se_config.noanalog){
        remove_analog_input(pad_data, count);
    }

    if (se_config.vitamute && (pad_data[0].Buttons & MUTE_MASK) == 0){
        //negative_note_button(pad_data, count);
        if (_sceImposeSetParam){
            _sceImposeSetParam(0x8, 1);
        }
    }
    
    // Return Number of Input Frames
    return count;
}
static int isRecoveryMode(){
    if (ark_config->recovery) return 1; // recovery mode set
    char* filename = sceKernelInitFileName();
    if (filename == NULL) return 0; // not running any app
    // check if running a recovery app
    return (strstr(filename, "RECOVERY") != NULL || strstr(filename, "recovery") != NULL);
}

void checkControllerInput(){
    if (isRecoveryMode()){
        disable_plugins = 1;
        disable_settings = 1;
    }
    else {
        SceCtrlData pad_data;
        CtrlPeekBufferPositive(&pad_data, 1);
        if ((pad_data.Buttons & PSP_CTRL_START) == PSP_CTRL_START) disable_plugins = 1;
        if ((pad_data.Buttons & PSP_CTRL_SELECT) == PSP_CTRL_SELECT) disable_settings = 1;
    }
    _sceImposeSetParam = (void *)sctrlHENFindFunction("sceImpose_Driver", "sceImpose_driver", 0x3C318569);
}

// Hook Gamepad Input
void patchController(SceModule* mod)
{
    CtrlPeekBufferPositive = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl_driver", 0x3A622550);
    CtrlPeekBufferNegative = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl_driver", 0xC152080A);
    CtrlReadBufferPositive = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl_driver", 0x1F803938);
    CtrlReadBufferNegative = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl_driver", 0x60B81F86);

    // Hook Gamepad Input
    HIJACK_FUNCTION(CtrlPeekBufferPositive, peek_positive, CtrlPeekBufferPositive);
    HIJACK_FUNCTION(CtrlPeekBufferNegative, peek_negative, CtrlPeekBufferNegative);
    HIJACK_FUNCTION(CtrlReadBufferPositive, read_positive, CtrlReadBufferPositive);
    HIJACK_FUNCTION(CtrlReadBufferNegative, read_negative, CtrlReadBufferNegative);
}
