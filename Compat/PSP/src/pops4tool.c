#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <pspinit.h>
#include <pspiofilemgr.h>
#include <psploadcore.h>

#include <systemctrl_ark.h>
#include <cfwmacros.h>

#include <systemctrl.h>
#include <systemctrl_se.h>


static s32 (*aLinkLibEntries)(SceStubLibrary *stubLib);
static s32 myLinkLibEntries(SceStubLibrary *stubLib){

    int res = aLinkLibEntries(stubLib);
    if (res < 0){
        // this should not get executed in normal circumstances
        // we do this for toolkits to load just enough of pops to trigger popsloader
        if (sceKernelFindModuleByName("pops") != NULL && sceKernelFindModuleByName("popscore") == NULL){
            return 0;
        }
    }

    return res;
}

void patchPops4Tool(){
    SceModule* mod = (SceModule*)sceKernelFindModuleByName("sceLoaderCore");
    u32 start_addr = mod->text_addr;
    u32 topaddr = mod->text_addr+mod->text_size;
    
    for (u32 addr = start_addr; addr<topaddr; addr+=4){
        if (_lw(addr) == 0x3452013C){
            HIJACK_FUNCTION(addr-44, myLinkLibEntries, aLinkLibEntries);
            break;
        }
    }
}