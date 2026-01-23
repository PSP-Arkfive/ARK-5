#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspthreadman_kernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <pspsysmem_kernel.h>
#include <pspinit.h>

#include <systemctrl_ark.h>
#include <cfwmacros.h>

#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_private.h>

#include "rebootex.h"
#include "nidresolver.h"
#include "modulemanager.h"
#include "loadercore.h"
#include "imports.h"
#include "sysmem.h"

extern SEConfig se_config;

int p2_size = 24;

int sctrlHENSetMemory(u32 p2, u32 p9){
    return 0; // unused in ARK
}

int sctrlHENApplyMemory(u32 p2) // stub (to be highjacked and implemented by compat layer)
{
    // can't modify ram after boot
    if (sctrlHENIsSystemBooted()) return -3;
    // check for unlock
    if (p2 > 24){
        if (p2_size > 24) return -2; // already enabled
        p2_size = p2;
        return 0;
    }
    // check for default
    else if (p2 == 24){
        if (p2_size == 24) return -2; // already enabled
        p2_size = p2;
        return 0;
    }
    return -1;
}

// Get HEN Version
int sctrlHENGetVersion()
{
    return ( (ARK_MAJOR_VERSION << 24) | (ARK_MINOR_VERSION << 16) | (ARK_MICRO_VERSION << 8) );
}

// Get HEN Minor Version
int sctrlHENGetMinorVersion()
{
    return ARK_REVISION;
}

int sctrlHENIsSE()
{
    return 1;
}

int sctrlHENIsDevhook()
{
    return 0;
}

// Find Filesystem Driver
PspIoDrv * sctrlHENFindDriver(const char * drvname)
{
    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);

    // Find Function
    int * (* findDriver)(const char * drvname) = (void*)sctrlHENFindFirstJAL(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x76DA16E3));

    // Find Driver Wrapper
    int * wrapper = findDriver(drvname);

    // Search Result
    PspIoDrv * driver = NULL;

    // Found Driver Wrapper
    if(wrapper != NULL)
    {
        // Save Driver Pointer
        driver = (PspIoDrv *)(wrapper[1]);
    }

    // Restore Permission Level
    pspSdkSetK1(k1);

    if (driver == NULL){
        if(0 == strcasecmp(drvname, "msstor")) {
            return sctrlHENFindDriver("eflash0a0f");
        }

        if(0 == strcasecmp(drvname, "msstor0p")) {
            return sctrlHENFindDriver("eflash0a0f1p");
        }
    }

    // Return Driver
    return driver;
}

// Replace Function in Syscall Table
void sctrlHENPatchSyscall(void * addr, void * newaddr)
{
    // Syscall Table
    unsigned int * syscalls = NULL;

    // Get Syscall Table Pointer from Coprocessor
    __asm__ volatile("cfc0 %0, $12\n" : "=r"(syscalls));

    // Invalid Syscall Table
    if(syscalls == NULL) return;

    // Skip Table Header
    syscalls += 4; // 4 * 4 = 16

    // Iterate Syscalls
    for(int i = 0; i < 0xFF4; ++i)
    {
        // Found Matching Function
        if((syscalls[i] & 0x0FFFFFFF) == (((unsigned int)addr) & 0x0FFFFFFF))
        {
            // Replace Syscall Function
            syscalls[i] = (unsigned int)newaddr;
        }
    }
}

// Register Prologue Module Start Handler
STMOD_HANDLER sctrlHENSetStartModuleHandler(STMOD_HANDLER new_handler)
{
    // Get Previous Handler
    STMOD_HANDLER on_module_start = g_module_start_handler;

    // Register Handler
    g_module_start_handler = (void *)(KERNELIFY(new_handler));

    // Return Previous Handler
    return on_module_start;
}

u32 sctrlHENFindFunction(const char * modname, const char *library, u32 nid){
    // Find Target Module
    SceModule * mod = (SceModule *)sceKernelFindModuleByName(modname);
    
    // Module not found
    if (mod == NULL)
    {
        // Attempt to find it by Address
        mod = (SceModule *)sceKernelFindModuleByAddress((unsigned int)modname);
        
        // Module not found
        if (mod == NULL) return 0;
    }

    return sctrlHENFindFunctionInMod(mod, library, nid);
}

// Find Function Address
u32 sctrlHENFindFunctionInMod(SceModule * mod, const char * library, u32 nid)
{

    if (mod == NULL) return 0;

    // Get NID Resolver
    NidResolverLib * resolver = getNidResolverLib(library);

    // Found Resolver for Library
    if(resolver != NULL)
    {
        // Resolve NID
        nid = getNidReplacement(resolver, nid);
    }
    

    // Fetch Export Table Start Address
    void * entTab = mod->ent_top;

    // Iterate Exports
    for (int i = 0; i < mod->ent_size;)
    {
        // Cast Export Table Entry
        struct SceLibraryEntryTable * entry = (struct SceLibraryEntryTable *)(entTab + i);

        // Found Matching Library
        if(entry->libname != NULL && library != NULL && 0 == strcmp(entry->libname, library))
        {
            // Accumulate Function and Variable Exports
            unsigned int total = entry->stubcount + entry->vstubcount;

            // NID + Address Table
            u32 * vars = entry->entrytable;
            
            // Exports available
            if(total > 0)
            {
                // Iterate Exports
                for(int j = 0; j < total; j++)
                {
                    // Found Matching NID
                    if(vars[j] == nid) return vars[total + j];
                }
            }
        }

        // Move Pointer
        i += (entry->len * 4);
    }

    // Function not found
    return 0;
}

u32 sctrlHENFindFunctionOnSystem(const char *libname, u32 nid, int user_mods_only) {
    SceUID mod_list[128] = {0};
    u32 mod_count = 0;
    int res = sceKernelGetModuleIdListForKernel(mod_list, 128, &mod_count, user_mods_only);

    if (res != 0) {
        return 0;
    }

    if (mod_count > 128) {
        // logmsg("[WARN]: %s: System has more than 128 modules, result will be incomplete\n", __func__);
    }

    for (int i = 0; i < MIN(mod_count, 128); i++) {
        SceUID mod_id = mod_list[i];
        SceModule* mod = sceKernelFindModuleByUID(mod_id);
        u32 fn = sctrlHENFindFunctionInMod(mod, libname, nid);

        if (fn != 0) {
            return fn;
        }
    }

    return 0;
}

u32 sctrlHENGetInitControl()
{
    if (kernel_init_apitype == NULL)
        return 0;
    return (u32)kernel_init_apitype - 8;
}

u32 sctrlHENFindImport(const char *szMod, const char *szLib, u32 nid)
{
    SceModule *mod = (SceModule*)sceKernelFindModuleByName(szMod);
    if(!mod) return 0;

    return sctrlHENFindImportInMod(mod, szLib, nid);
}

u32 sctrlHENFindImportInMod(SceModule * mod, const char *library, u32 nid) {
    // Invalid Arguments
    if (mod == NULL || library == NULL) {
        return 0;
    }

    for(int i = 0; i < mod->stub_size;) {
        SceLibraryStubTable *stub = (SceLibraryStubTable *)(mod->stub_top + i);

        if(stub->libname && strcmp(stub->libname, library) == 0) {
            unsigned int *table = stub->nidtable;

            for(int j = 0; j < stub->stubcount; j++) {
                if(table[j] == nid) {
                    return ((u32)stub->stubtable + (j * 8));
                }
            }
        }

        i += (stub->len * 4);
    }

    return 0;
}

void sctrlHENHijackFunction(FunctionPatchData* patch_data, void* func_addr, void* patch_func, void** orig_func) {

    void* ptr = patch_data;

    int is_kernel_patch = IS_KERNEL_ADDR(func_addr);
    if (is_kernel_patch){
        patch_func = (void*)KERNELIFY(patch_func);
        ptr = (void*)KERNELIFY(ptr);
    }

    // do hijack
    {
        u32 _pb_ = (u32)patch_data;
        u32 a = (u32)func_addr;
        u32 f = (u32)patch_func;

        _sw(_lw(a), _pb_);
        _sw(_lw(a + 4), _pb_ + 4);
        _sw(NOP, _pb_ + 8);
        _sw(NOP, _pb_ + 16);
        MAKE_JUMP_PATCH(_pb_ + 12, a + 8);
        _sw(0x08000000 | ((f >> 2) & 0x03FFFFFF), a); \
        _sw(NOP, a + 4);

    }

    *orig_func = ptr;

    sctrlFlushCache();
}

u32 sctrlHENMakeSyscallStub(void *function) {
    
	u32 stub = (u32)user_malloc(2*sizeof(u32));
	s32 syscall_num = sceKernelQuerySystemCall(function);

	if (stub == 0) {
		return 0;
	}

	if (syscall_num < 0) {
		return 0;
	}

	MAKE_SYSCALL_FUNCTION(stub, syscall_num);
	return stub;
}

void sctrlHENLoadModuleOnReboot(char *module_before, void *buf, int size, int flags)
{
    rebootex_config.rtm_mod.before = module_before;
    rebootex_config.rtm_mod.buffer = buf;
    rebootex_config.rtm_mod.size = size;
    rebootex_config.rtm_mod.flags = flags;
}

extern void* custom_rebootex;
void sctrlHENSetRebootexOverride(const u8 *rebootex)
{
    if (rebootex != NULL) // override rebootex
        custom_rebootex = (void*)rebootex;
}

extern int (* LoadRebootOverrideHandler)(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4);
void sctrlHENSetLoadRebootOverrideHandler(int (* func)(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4))
{
    LoadRebootOverrideHandler = func;
}

extern int (*ExtendDecryption)();
void* sctrlHENRegisterKDecryptHandler(int (* func)())
{
    int (* r)() = ExtendDecryption;
    ExtendDecryption = func;
    return r;
}

extern int (*MesgLedDecryptEX)();
void* sctrlHENRegisterMDecryptHandler(int (* func)())
{
    int (* r)() = (void *)MesgLedDecryptEX;
    MesgLedDecryptEX = (void *)func;
    return (void *)r;
}

extern void (*lle_handler)(void*);
void sctrlHENRegisterLLEHandler(void* handler)
{
    lle_handler = handler;
}

int sctrlHENRegisterHomebrewLoader(void* handler)
{
    // register handler and patch leda
    extern void patchLedaPlugin(void* handler);
    patchLedaPlugin(handler);
    return 0;
}

extern void* plugin_handler;
void* sctrlHENSetPluginHandler(void* handler){
    void* ret = plugin_handler;
    plugin_handler = handler;
    return ret;
}

RebootexConfig* sctrlHENGetRebootexConfig(RebootexConfig* config){
    if (config){
        memcpy(config, &rebootex_config, sizeof(RebootexConfigARK));
    }
    return (RebootexConfig*)&rebootex_config;
}

void sctrlHENSetRebootexConfig(RebootexConfig* config){
    if (config){
        memcpy(&rebootex_config, config, sizeof(RebootexConfigARK));
    }
}

u32 sctrlHENFakeDevkitVersion(){
    return FW_660;
}

int sctrlHENIsToolKit()
{
    int ret = 0; // Retail
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    if (ark_config->exec_mode == PSP_ORIG){
        SceIoStat stat;
        if (sceIoGetstat("flash0:/kd/vshbridge_tool.prx", &stat) >= 0)
            ark_config->exec_mode = PSP_TOOL;
    }

    if (ark_config->exec_mode == PSP_TOOL){
        int baryon_ver = 0;
        int (*getBaryonVer)(void*) = (void*)sctrlHENFindFunction("sceSYSCON_Driver", "sceSyscon_driver", 0x7EC5A957);
        if (getBaryonVer) getBaryonVer(&baryon_ver);
        if (baryon_ver == 0x00020601){
            ret = 2; // DevelopmentTool
        }
        else {
            ret = 1; // TestingTool
        }
    }

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return ret;
}

int sctrlHENIsSystemBooted() {
	int res = sceKernelGetSystemStatus();

	return (res == 0x20000);
}
