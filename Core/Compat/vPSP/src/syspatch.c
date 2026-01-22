#include <string.h>
#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <pspinit.h>
#include <pspumd.h>
#include <pspdisplay.h>
#include <pspkermit.h>

#include <cfwmacros.h>
#include <rebootexconfig.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_ark.h>
#include <systemctrl_private.h>


extern SEConfigARK* se_config;

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;


// kermit_peripheral's sub_000007CC clone, called by loadexec + 0x0000299C with a0=8 (was a0=7 for fw <210)
// Returns 0 on success
u64 kermit_flash_load(int cmd)
{
    u8 buf[128];
    u64 resp;
    void *alignedBuf = (void*)ALIGN_64((int)buf + 63);
    sceKernelDcacheInvalidateRange(alignedBuf, 0x40);
    SceKermitRequest *packet = (SceKermitRequest *)KERMIT_PACKET((int)alignedBuf);
    u32 argc = 0;
    sceKermitSendRequest(packet, KERMIT_MODE_PERIPHERAL, cmd, argc, KERMIT_CALLBACK_DISABLE, &resp);
    return resp;
}

int flashLoadPatch(int cmd)
{
    int ret = kermit_flash_load(cmd);
    // Custom handling on loadFlash mode, else nothing
    if ( cmd == KERMIT_CMD_ERROR_EXIT || cmd == KERMIT_CMD_ERROR_EXIT_2 )
    {
        // Wait for flash to load
        sceKernelDelayThread(10000);

        // Load FLASH0.ARK
        char archive[ARK_PATH_SIZE];
        strcpy(archive, ark_config->arkpath);
        strcat(archive, FLASH0_ARK);

        int fd = sceIoOpen(archive, PSP_O_RDONLY, 0777);
        if (fd >= 0){
            sceIoRead(fd, (void*)VITA_FLASH_ARK, MAX_FLASH0_SIZE);
            sceIoClose(fd);
        }
        else {
            memset((void*)VITA_FLASH_ARK, 0, 32);
        }

        sctrlFlushCache();
    }
    return ret;
}

int patchKermitPeripheral()
{
    // Redirect KERMIT_CMD_ERROR_EXIT loadFlash function
    u32 swaddress = sctrlHENFindFirstJAL(sctrlHENFindFunction("sceKermitPeripheral_Driver", "sceKermitPeripheral_driver", 0x0648E1A3));
    REDIRECT_FUNCTION(swaddress, flashLoadPatch);
    return 0;
}

static u8 get_pscode_from_region(int region)
{
    u8 code;

    code = region;

    if(code < 12) {
        code += 2;
    } else {
        code -= 11;
    }

    if(code == 2) {
        code = 3;
    }

    return code;
}

int (* _sceChkregGetPsCode)(u8 *pscode);
int sceChkregGetPsCodePatched(u8 *pscode) {
    int res = _sceChkregGetPsCode(pscode);

    pscode[0] = 0x01;
    pscode[1] = 0x00;
    pscode[3] = 0x00;
    pscode[4] = 0x01;
    pscode[5] = 0x00;
    pscode[6] = 0x01;
    pscode[7] = 0x00;

    if (se_config->vshregion)
        pscode[2] = get_pscode_from_region(se_config->vshregion);

    return res;
}

void patch_GameBoot(SceModule* mod){
    u32 p1 = 0;
    u32 p2 = 0;
    int patches = 2;
    for (u32 addr=mod->text_addr; addr<mod->text_addr+mod->text_size && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x2C43000D){
            p1 = addr-36;
            patches--;
        }
        else if (data == 0x27BDFF20 && _lw(addr-4) == 0x27BD0040){
            p2 = addr-24;
            patches--;
        }
    }
    _sw(JAL(p1), p2);
    _sw(0x24040002, p2 + 4);
}

void exit_game_patched(){
    sctrlSESetBootConfFileIndex(MODE_UMD);
    if (se_config->launcher_mode)
        sctrlArkExitLauncher();
    else
        sctrlKernelExitVSH(NULL);
}

int AdrenalineOnModuleStart(SceModule * mod){

    // System fully booted Status
    static int booted = 0;

    // Patch Kermit Peripheral Module to load flash0
    if(strcmp(mod->modname, "sceKermitPeripheral_Driver") == 0)
    {
        patchKermitPeripheral();
        goto flush;
    }

    if (strcmp(mod->modname, "sceLoadExec") == 0) {
        // fix exitgame
        REDIRECT_FUNCTION(sctrlHENFindFunction(mod->modname, "LoadExecForUser", 0x05572A5F), exit_game_patched);
        REDIRECT_FUNCTION(sctrlHENFindFunction(mod->modname, "LoadExecForUser", 0x2AC9954B), exit_game_patched);
        goto flush;
    }
    if (strcmp(mod->modname, "sceChkreg") == 0) {
        HIJACK_FUNCTION(sctrlHENFindFunction("sceChkreg", "sceChkreg_driver", 0x59F8491D), sceChkregGetPsCodePatched, _sceChkregGetPsCode);
        goto flush;
    }

    if (strcmp(mod->modname, "sceImpose_Driver") == 0) {
        // configure inferno cache
        se_config->iso_cache_size_kb = 32;
        se_config->iso_cache_num = 32;
        se_config->iso_cache_partition = (se_config->force_high_memory)? 2:11;
        goto flush;
    }

    if(strcmp(mod->modname, "game_plugin_module") == 0) {
        if (se_config->skiplogos == 1 || se_config->skiplogos == 2) {
            patch_GameBoot(mod);
        }
        goto flush;
    }
       
    // Boot Complete Action not done yet
    if(booted == 0)
    {
        // Boot is complete
        if(sctrlHENIsSystemBooted())
        {
            // Initialize Memory Stick Speedup Cache
            if (se_config->msspeed)
        		msstorCacheInit("ms");

            // Boot Complete Action done
            booted = 1;
        }
    }

flush:
    sctrlFlushCache();

    // Forward to previous Handler
    if(previous) return previous(mod);
    return 0;
}

void initAdrenalineSysPatch(){
    // Register Module Start Handler
    previous = sctrlHENSetStartModuleHandler(AdrenalineOnModuleStart);
}
