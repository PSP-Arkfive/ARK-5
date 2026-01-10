#include <string.h>
#include <pspsdk.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psputility_modules.h>

#include <ark.h>
#include <systemctrl.h>
#include <cfwmacros.h>


typedef struct {
    int id;
    int modid;
    char* prxname; // prx name in ARK path
    char* libname; // prx name in LIBS folder
} CustomUtilityModule;

extern ARKConfig* ark_config;

static CustomUtilityModule custom_utility_modules[] = {
    {PSP_MODULE_NET_FTP,           -1, PSPFTP_PRX},
    {PSP_MODULE_AV_PNG,            -1, LIBPNG_PRX},
    {PSP_MODULE_AV_HELPER,         -1, PSPAV_PRX},
    {PSP_MODULE_VLF,               -1, VLF_PRX},
    {PSP_MODULE_INTRAFONT_VLF,     -1, "VLFFONT.PRX", "intraFont-vlf.prx"},
    {PSP_MODULE_INTRAFONT_GU,      -1, INTRAFONT_PRX, "intraFont-gu.prx"},
    {PSP_MODULE_UNARCHIVER,        -1, UNARCHIVE_PRX, "unarchiver.prx"},
    {PSP_MODULE_USB_DEV_DRV,       -1, USBDEV_PRX, "usbdevice.prx"},
    {PSP_MODULE_IDS_REGEN,         -1, IDSREG_PRX, "idsregeneration.prx"},
    {PSP_MODULE_IO_PRIVILEGED,     -1, "IOP.PRX"},
    {PSP_MODULE_PSAR_DUMPER,       -1, "PSARDUMP.PRX", "libpsardumper.prx"},
    {PSP_MODULE_PSPDECRYPT,        -1, "DECRYPT.PRX", "pspdecrypt.prx"},
    {PSP_MODULE_KPSPIDENT,         -1, "KPSPID.PRX", "kpspident.prx"},
    {PSP_MODULE_IPL_UPDATER,       -1, "IPL_UPDT.PRX", "ipl_update.prx"},
    {PSP_MODULE_KBOOTI_UPDATER,    -1, "KBI_UPDT.PRX", "kbooti_update.prx"},
};

static int (*origUtilityLoadModule)(int);
static int (*origUtilityUnloadModule)(int);

static CustomUtilityModule* findCustomModuleById(int id){
    if (id>=0x0700 || (id&0xFF) >= 0x80){ // custom module
        for (int i=0; i<NELEMS(custom_utility_modules); i++){
            CustomUtilityModule* module = &custom_utility_modules[i];
            if (module->id == id) return module;
        }
    }
    return NULL;
}

static int findModulePath(CustomUtilityModule* module, char* path){
    SceIoStat stat;
    char* libname = module->libname? module->libname : module->prxname;

    // try in init path
    char* initpath = sceKernelInitFileName();
    if (initpath){
        strcpy(path, initpath);
        char* fname = strrchr(path, '/');
        if (fname){
            strcpy(fname+1, libname);

            if (sceIoGetstat(path, &stat) >= 0) return 0; // found
        }
    }

    // try in libs path
    strcpy(path, "ms0:/PSP/LIBS/");
    strcat(path, libname);

    if (sceIoGetstat(path, &stat) >= 0) return 0; // found
    
    // try in ark path
    strcpy(path, ark_config->arkpath);
    strcat(path, module->prxname);

    if (sceIoGetstat(path, &stat) >= 0) return 0; // found

    // try in flash0
    strcpy(path, "flash0:/kd/");
    strcat(path, libname);

    return sceIoGetstat(path, &stat);
}

static int loadstartCustomUtilityModule(CustomUtilityModule* module){
    if (module->modid >= 0) return 0x80111102; // SCE_UTILITY_MODULE_ERROR_ALREADY_LOADED

    char modpath[ARK_PATH_SIZE];
    findModulePath(module, modpath);    

    module->modid = sceKernelLoadModule(modpath, 0, NULL);
    if (module->modid < 0) return module->modid;

    int res = sceKernelStartModule(module->modid, strlen(modpath)+1, (void*)modpath, NULL, NULL);
    if (res < 0){
        sceKernelUnloadModule(module->modid);
        module->modid = -1;
        return res;
    }
    return 0;
}

static int stopunloadCustomUtilityModule(CustomUtilityModule* module){
    if (module->modid < 0) return 0x80111103; // SCE_UTILITY_MODULE_ERROR_NOT_LOADED

    int res = sceKernelStopModule(module->modid, 0, NULL, NULL, NULL);
    if (res >= 0){
        res = sceKernelUnloadModule(module->modid);
        if (res >= 0){
            module->modid = -1;
        }
    }
    
    return res;
}

static int extendedUtilityLoadModule(int module){
    int k1 = pspSdkSetK1(0);
    int res = -1;

    CustomUtilityModule* mod = findCustomModuleById(module);
    if (mod) res = loadstartCustomUtilityModule(mod);
    else res = origUtilityLoadModule(module);

    pspSdkSetK1(k1);
    return res;
}

static int extendedUtilityUnloadModule(int module){
    int k1 = pspSdkSetK1(0);
    int res = -1;
    
    CustomUtilityModule* mod = findCustomModuleById(module);
    if (mod) res = stopunloadCustomUtilityModule(mod);
    else res = origUtilityUnloadModule(module);

    pspSdkSetK1(k1);
    return res;
}

void extendUtilityModules(){
    u32 utilityLoadModule = sctrlHENFindFunction("sceUtility_Driver", "sceUtility", 0x2A2B3DE0);
    u32 utilityUnloadModule = sctrlHENFindFunction("sceUtility_Driver", "sceUtility", 0xE49BFE92);

    HIJACK_FUNCTION(utilityLoadModule, extendedUtilityLoadModule, origUtilityLoadModule);
    HIJACK_FUNCTION(utilityUnloadModule, extendedUtilityUnloadModule, origUtilityUnloadModule);
}

int sctrlUtilityFindModulePath(int module, char* path){
    CustomUtilityModule* mod = findCustomModuleById(module);
    if (mod)
        return findModulePath(mod, path);
    return -1;
}