#include <string.h>
#include <pspsdk.h>
#include <pspdisplay_kernel.h>
#include <pspsysmem_kernel.h>
#include <pspiofilemgr.h>
#include <pspsysevent.h>

#include <cfwmacros.h>
#include <rebootexconfig.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_ark.h>
#include <popsdisplay.h>

#include "payload.h"

PSP_MODULE_INFO("ARKCompatLayer", 0x3007, 1, 0);

ARKConfig* ark_config = NULL;
SEConfig* se_config = NULL;
RebootexConfigARK* reboot_config = NULL;

extern void initVitaPopsSysPatch();
extern void copyPSPVram(u32* psp_vram);

static void processArkConfig(){
    if (ark_config->exec_mode == DEV_UNK){
        ark_config->exec_mode = PSV_POPS; // assume running on PS Vita Pops
    }
    if (ark_config->launcher[0] == '\0'){
        strcpy(ark_config->launcher, ARK_XMENU);
    }
}

// Boot Time Entry Point
int module_start(SceSize args, void * argp)
{

    se_config = sctrlSEGetConfigInternal();
    ark_config = sctrlArkGetConfig(NULL);
    reboot_config = (RebootexConfigARK*)sctrlHENGetRebootexConfig(NULL);

    if (ark_config == NULL){
        return 1;
    }
    
    // copy configuration
    processArkConfig();

    if (ark_config->exec_mode != PSV_POPS){
        return 2;
    }

    if (size_rebootbuffer_vitapops == 0){
        return 3;
    }

    #if 0
    #include <colordebugger.h>
    _sw(0x44000000, 0xBC800100);
    popsDisplayInit();
    colorDebug(0xFF00);
    copyPSPVram((void*)0x44000000);
    #endif

    // set rebootex for VitaPOPS
    sctrlHENSetRebootexOverride(rebootbuffer_vitapops);

    initVitaPopsSysPatch();
    
    // Return Success
    return 0;
}
