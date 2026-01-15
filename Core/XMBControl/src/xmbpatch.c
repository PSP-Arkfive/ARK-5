/*
    6.39 TN-A, XmbControl
    Copyright (C) 2011, Total_Noob
    Copyright (C) 2011, Frostegater
    Copyright (C) 2011, codestation

    main.c: XmbControl main code
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <psppower.h>
#include <psputility_sysparam.h>

#include <systemctrl_ark.h>
#include <vshctrl.h>
#include <kubridge.h>
#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <rebootexconfig.h>

#include "main.h"
#include "utils.h"
#include "list.h"
#include "settings.h"
#include "plugins.h"
#include "battery.h"

// TODO: send to pspsdk
#define PSP_INIT_APITYPE_EF2 0x152
extern int scePowerRequestColdReset(int unk);

ARKConfig ark_config;
RebootexConfigARK rebootex_config;
extern List plugins;

STMOD_HANDLER previous = NULL;
CFWConfig config;

int psp_model;
SEConfigARK se_config;
int codecs_active = 0;
int battery_type = -1;
int has_hibernation = 0;

char * strtrim(char * text);

char* custom_app_path = "ms0:/PSP/APP/CUSTOM/EBOOT.PBP";
const char* PLUGIN_PATH_GO = "ef0:/SEPLUGINS/PLUGINS.TXT";
const char* PLUGIN_PATH = "ms0:/SEPLUGINS/PLUGINS.TXT";
const char* GAME_PLUGIN_PATH_GO = "ef0:/SEPLUGINS/GAME.TXT";
const char* GAME_PLUGIN_PATH = "ms0:/SEPLUGINS/GAME.TXT";
const char* VSH_PLUGIN_PATH_GO = "ef0:/SEPLUGINS/VSH.TXT";
const char* VSH_PLUGIN_PATH = "ms0:/SEPLUGINS/VSH.TXT";
const char* POPS_PLUGIN_PATH_GO = "ef0:/SEPLUGINS/POPS.TXT";
const char* POPS_PLUGIN_PATH = "ms0:/SEPLUGINS/POPS.TXT";


enum{
    SYSTEM_OPTIONS,
    ACTIVATE_CODECS,
    USB_DEVICE,
    USB_READONLY,
    USB_CHARGE,
    CPU_CLOCK_GAME,
    CPU_CLOCK_VSH,
    WPA2_SUPPORT,
    AUTOBOOT_LAUNCHER,
    USE_EXTRA_MEM,
    MEM_STICK_SPEEDUP,
    INFERNO_CACHE,
    DISABLE_GO_PAUSE,
    OLD_GO_PLUGINS,
    NO_HIB_DELETE,
    SKIP_LOGOS,
    HIDE_PICS,
    HIDE_MAC,
    HIDE_DLC,
    DISABLE_LED,
    DISABLE_UMD,
    DISABLE_ANALOG,
    UMD_REGION,
    VSH_REGION,
    CONFIRM_BUTTON,
    BATTERY_CONVERT,
    QA_FLAGS,
    GO_PAUSE_DELETE,
    RESET_SETTINGS,
};

enum {
    SOFT_RESET = 1,
    HARD_RESET,
    SUSPEND_DEVICE,
    SHUTDOWN_DEVICE,
};

#define PLUGINS_CONTEXT 2

typedef struct
{
    int mode;
    int negative;
    char *item;
} GetItem;

GetItem GetItemes[] =
{
    { SYSTEM_OPTIONS      +PLUGINS_CONTEXT+2, 0, "System Options" },
    { ACTIVATE_CODECS     +PLUGINS_CONTEXT+2, 0, "Activate Flash and WMA Codecs" },
    { USB_DEVICE          +PLUGINS_CONTEXT+2, 0, "USB Device" },
    { USB_READONLY        +PLUGINS_CONTEXT+2, 0, "USB Read-Only" },
    { USB_CHARGE          +PLUGINS_CONTEXT+2, 0, "USB Charge" },
    { CPU_CLOCK_GAME      +PLUGINS_CONTEXT+2, 0, "CPU Clock in Game" },
    { CPU_CLOCK_VSH       +PLUGINS_CONTEXT+2, 0, "CPU Clock in XMB" },
    { WPA2_SUPPORT        +PLUGINS_CONTEXT+2, 0, "WPA2 Support" },
    { AUTOBOOT_LAUNCHER   +PLUGINS_CONTEXT+2, 0, "Autoboot Launcher" },
    { USE_EXTRA_MEM       +PLUGINS_CONTEXT+2, 0, "Use Extra Memory" },
    { MEM_STICK_SPEEDUP   +PLUGINS_CONTEXT+2, 0, "Memory Stick Speedup" },
    { INFERNO_CACHE       +PLUGINS_CONTEXT+2, 0, "Inferno Cache" },
    { DISABLE_GO_PAUSE    +PLUGINS_CONTEXT+2, 0, "Disable PSP Go Pause" },
    { OLD_GO_PLUGINS      +PLUGINS_CONTEXT+2, 0, "Old Plugins on ef0" },
    { NO_HIB_DELETE       +PLUGINS_CONTEXT+2, 0, "Prevent hibernation deletion on PSP Go" },
    { SKIP_LOGOS          +PLUGINS_CONTEXT+2, 0, "Skip Sony logos" },
    { HIDE_PICS           +PLUGINS_CONTEXT+2, 0, "Hide PIC0 and PIC1" },
    { HIDE_MAC            +PLUGINS_CONTEXT+2, 0, "Hide MAC Address" },
    { HIDE_DLC            +PLUGINS_CONTEXT+2, 0, "Hide DLC" },
    { DISABLE_LED         +PLUGINS_CONTEXT+2, 0, "Turn off LEDs" },
    { DISABLE_UMD         +PLUGINS_CONTEXT+2, 0, "Disable UMD Drive" },
    { DISABLE_ANALOG      +PLUGINS_CONTEXT+2, 0, "Disable Analog Stick" },
    { UMD_REGION          +PLUGINS_CONTEXT+2, 0, "UMD Region" },
    { VSH_REGION          +PLUGINS_CONTEXT+2, 0, "VSH Region" },
    { CONFIRM_BUTTON      +PLUGINS_CONTEXT+2, 0, "Confirm Button" },
    { BATTERY_CONVERT     +PLUGINS_CONTEXT+2, 0, "Battery Convert" },
    { QA_FLAGS            +PLUGINS_CONTEXT+2, 0, "QA Flags" },
    { GO_PAUSE_DELETE     +PLUGINS_CONTEXT+2, 0, "Delete PSP Go Pause" },
    { RESET_SETTINGS      +PLUGINS_CONTEXT+2, 0, "Reset Settings" },
};

char* system_opts[] = {
    "Cancel",
    "Soft Reset",
    "Hard Reset",
    "Suspend",
    "Shutdown",
};

char* system_opts_vita[] = {
    "Cancel",
    "Soft Reset",
    "Suspend",
};

char* boolean_settings[] = {
    "Disabled",
    "Enabled"
};

char* boolean_settings2[] = {
    "Auto",
    "Forced"
};

char* boolean_settings3[] = {
    "Off",
    "On",
    "Auto",
};

char* boolean_settings4[] = {
    "Cancel",
    "Confirm",
};

char* usbdev_settings[] = {
    "Memory Stick",
    "Flash 0",
    "Flash 1",
    "Flash 2",
    "Flash 3",
    "UMD",
    "Internal Storage",
};

char* clock_settings[] = {
    "Auto",
    "OverClock",
    "Balanced",
    "PowerSave"
};

char* skiplogos_settings[] = {
    "Disabled",
    "Enabled",
    "GameBoot",
    "ColdBoot"
};

char* hidepics_settings[] = {
    "Disabled",
    "Enabled",
    "PIC0",
    "PIC1"
};

char* infernocache_settings[] = {
    "Disabled",
    "LRU",
    "RR"
};

char* umdregion_settings[] = {
    "Default",
    "America",
    "Europe",
    "Japan",
};

char* vshregion_settings[] = {
    "Default", "Japan", "America", "Europe", "Korea",
    "United Kingdom", "Latin America", "Australia", "Hong Kong",
    "Taiwan", "Russia", "China", "Debug I", "Debug II"
};

char* confirmbutton_settings[] = {
    "O", "X"
};

char* convert_battery_opts[] = {
    "Normal",
    "Pandora",
};

char* classic_plugins_opts[] = {
    "No Cleanup",
    "With Cleanup",
};

char* plugins_options[] = {
    "Disabled",
    "Enabled",
    "Remove",
};

char* plugins_install_options[] = {
    "None",
    "Always",
    "GAME",
    "VSH/XMB",
    "POPS/PS1",
    "UMD/ISO",
    "Homebrew",
    NULL, // Last Played Game
};


#define ITEM_OPT(i) { NELEMS(i), i }
struct {
    int n;
    char** c;
} item_opts[] = {
    {0, NULL}, // None
    ITEM_OPT(classic_plugins_opts), // Import Plugins
    ITEM_OPT(plugins_options), // Plugins
    ITEM_OPT(plugins_install_options), // Plugins Install
    ITEM_OPT(system_opts), // System Options
    ITEM_OPT(boolean_settings4), // Activate Codecs
    ITEM_OPT(usbdev_settings), // USB Device
    ITEM_OPT(boolean_settings3), // USB Read-Only
    ITEM_OPT(boolean_settings), // USB Charge
    ITEM_OPT(clock_settings), // Clock Game
    ITEM_OPT(clock_settings), // Clock VSH
    ITEM_OPT(boolean_settings), // WPA2 ( Thanks again @Moment )
    ITEM_OPT(boolean_settings), // Autoboot Launcher
    ITEM_OPT(boolean_settings2), // Extra RAM
    ITEM_OPT(boolean_settings), // MS Speedup
    ITEM_OPT(infernocache_settings), // Inferno Cache
    ITEM_OPT(boolean_settings2), // Disable Go Pause
    ITEM_OPT(boolean_settings), // Old Plugins on ef0
    ITEM_OPT(boolean_settings), // Prevent hib delete
    ITEM_OPT(skiplogos_settings), // Skip Sony logos
    ITEM_OPT(hidepics_settings), // Hide PIC0 and PIC1
    ITEM_OPT(boolean_settings), // Hide MAC
    ITEM_OPT(boolean_settings), // Hide DLC
    ITEM_OPT(boolean_settings), // Turn off LEDs
    ITEM_OPT(boolean_settings), // Disable UMD Drive
    ITEM_OPT(boolean_settings), // Disable Analog Stick 
    ITEM_OPT(umdregion_settings), // UMD Region
    ITEM_OPT(vshregion_settings), // VSH Region
    ITEM_OPT(confirmbutton_settings), // Confirmation Button
    ITEM_OPT(convert_battery_opts), // Convert Battery
    ITEM_OPT(boolean_settings), // QA Flags
    ITEM_OPT(boolean_settings4), // PSP Go Pause Delete
    ITEM_OPT(boolean_settings4), // Reset Settings
};

typedef struct {
    char* orig;
    char* translated;
} StringContainer;
StringContainer language_strings[MAX_LANG_STRINGS];
int n_translated = 0;

#define N_STRINGS ((sizeof(string) / sizeof(char **)))

int count = 0;

int (* AddVshItem)(void *a0, int topitem, SceVshItem *item);
SceSysconfItem *(*GetSysconfItem)(void *a0, void *a1);
int (* ExecuteAction)(int action, int action_arg);
int (* UnloadModule)(int skip);
int (* OnXmbPush)(void *arg0, void *arg1);
int (* OnXmbContextMenu)(void *arg0, void *arg1);

void (* LoadStartAuth)();
int (* auth_handler)(int a0);
void (* OnRetry)();

void (* AddSysconfItem)(u32 *option, SceSysconfItem **item);
void (* OnInitMenuPspConfig)();

extern int readLine(char* source, char *str);
extern int utf8_to_unicode(wchar_t *dest, char *src);

u32 sysconf_unk, sysconf_option;

int is_cfw_config = 0;
int unload = 0;

u32 backup[4];
int context_mode = 0;

char user_buffer[LINE_BUFFER_SIZE];

int startup = 1;

SceContextItem *context;
SceVshItem *new_item;
SceVshItem *new_item2;
SceVshItem *new_item3;
SceVshItem *new_item4;
SceVshItem *new_item5;
void *xmb_arg0, *xmb_arg1;
int sysconf_action = 0;

static unsigned char signup_item[] __attribute__((aligned(16))) = {
    0x2a, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x46, 
    0x55, 0x00, 0x00, 0x46, 0x59, 0x00, 0x00, 0x47, 0x43, 0x00, 0x00, 0x6d, 0x73, 0x67, 0x5f, 0x73, 
    0x69, 0x67, 0x6e, 0x75, 0x70,
};


static unsigned char ps_store_item[] __attribute__((aligned(16))) = {
    0x2c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x46, 
    0x56, 0x00, 0x00, 0x46, 0x5a, 0x00, 0x00, 0x47, 0x44, 0x00, 0x00, 0x6d, 0x73, 0x67, 0x5f, 0x70, 
    0x73, 0x5f, 0x73, 0x74, 0x6f, 0x72, 0x65,
};

static unsigned char information_board_item[] __attribute__((aligned(16))) = {
    0x2e, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 
    0x08, 0xdf, 0x09, 0x0a, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x46, 
    0x58, 0x00, 0x00, 0x47, 0x42, 0x00, 0x00, 0x47, 0x46, 0x00, 0x00, 0x6d, 0x73, 0x67, 0x5f, 0x69, 
    0x6e, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x5f, 0x62, 0x6f, 0x61, 0x72, 0x64,
};

void ClearCaches()
{
    sceKernelDcacheWritebackAll();
    kuKernelIcacheInvalidateAll();
}

char* getLastGameForPlugin(){
    char* opt = NULL;
    int apitype = rebootex_config.last_played.apitype;
    if (apitype == PSP_INIT_APITYPE_MS2 || apitype == PSP_INIT_APITYPE_EF2){ // homebrew
        if (rebootex_config.last_played.path[0]){
            opt = rebootex_config.last_played.path;
        }
    }
    else if (rebootex_config.last_played.game_id[0]){
        opt = rebootex_config.last_played.game_id;
    }
    return opt;
}

char* getLastGameForPluginFormatted(){
    char* opt = getLastGameForPlugin();
    if (opt){
        char* hbn = sce_paf_private_strrchr(opt, '/');
        if (hbn) opt = hbn;
    }
    return opt;
}

void exec_custom_launcher() {
    char menupath[ARK_PATH_SIZE];
    sce_paf_private_strcpy(menupath, ark_config.arkpath);
    strcat(menupath, VBOOT_PBP);

    SceIoStat stat; int res = sceIoGetstat(menupath, &stat);
    if (res >= 0){
        struct SceKernelLoadExecVSHParam param;
        sce_paf_private_memset(&param, 0, sizeof(param));
        param.size = sizeof(param);
        param.args = sce_paf_private_strlen(menupath) + 1;
        param.argp = menupath;
        param.key = "game";
        sctrlKernelLoadExecVSHWithApitype(0x141, menupath, &param);
    }
}

void exec_150_reboot(void) {
    int k1 = pspSdkSetK1(0);
    SceUID mod = sceKernelLoadModule(ARK_DC_PATH "/150/reboot150.prx", 0, NULL);
    if(mod < 0) {
        pspSdkSetK1(k1);
        return;
    }
    int res = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
    pspSdkSetK1(k1);
    if (res >= 0) sctrlKernelExitVSH(NULL);
    else sceKernelUnloadModule(mod);
}

void exec_custom_app(char *path) {
    struct SceKernelLoadExecVSHParam param;
    sce_paf_private_memset(&param, 0, sizeof(param));
    param.size = sizeof(param);
    param.args = sce_paf_private_strlen(path) + 1;
    param.argp = path;
    param.key = "game";
    sctrlKernelLoadExecVSHWithApitype(0x141, path, &param);
}

void recreate_umd_keys(void) {
    struct KernelCallArg args;
    sce_paf_private_memset(&args, 0, sizeof(args));

    sctrlSEGetConfig((SEConfig*)&se_config);
    se_config.umdregion = config.umdregion;
    sctrlSESetConfig((SEConfig*)&se_config);
    
    void* generate_umd_keys = (void*)sctrlHENFindFunction("ARKCompatLayer", "PSPCompat", 0x2EE76C36);
    if (!generate_umd_keys) return;
    kuKernelCall(generate_umd_keys, &args);

    // patch region check if not done already
    SceModule mod; kuKernelFindModuleByName("vsh_module", &mod);
    sctrlHookImportByNID(&mod, "sceVshBridge", 0x5C2983C2, (void*)1);
}

int codecs_activated() {
    u32 flash_activated = 0;
    u32 flash_play = 0;
    u32 wma_play = 0;

    vctrlGetRegistryValue("/CONFIG/BROWSER", "flash_activated", &flash_activated);
    vctrlGetRegistryValue("/CONFIG/BROWSER", "flash_play", &flash_play);
    vctrlGetRegistryValue("/CONFIG/MUSIC", "wma_play", &wma_play);

    if (!flash_activated || !flash_play || !wma_play){
        return 0;
    }
    
    return 1;
}

int activate_codecs() {

    if (!codecs_activated()){
        vctrlSetRegistryValue("/CONFIG/BROWSER", "flash_activated", 1);
        vctrlSetRegistryValue("/CONFIG/BROWSER", "flash_play", 1);
        vctrlSetRegistryValue("/CONFIG/MUSIC", "wma_play", 1);
        return 1;
    }
    
    return 0;
}

void reset_ark_settings(){
    const char settings[] =
        "always, usbcharge, on\n"
        "always, overclock, on\n"
        "always, wpa2, on\n"
        "always, launcher, off\n"
        "always, highmem, off\n"
        "always, mscache, on\n"
        "always, infernocache, on\n"
        "always, disablepause, off\n"
        "always, oldplugin, on\n"
        "always, skiplogos, off\n"
        "always, hidepics, off\n"
        "always, hibblock, on\n"
        "always, hidemac, on\n"
        "always, hidedlc, on\n"
        "always, noled, off\n"
        "always, noumd, off\n"
        "always, noanalog, off\n"
        "always, qaflags, on\n"
        "\n"
        "# The following games don't like Inferno Cache\n"
        "# Luxor - The Wrath of Set (the other Luxor game works fine)\n"
        "ULUS10201, infernocache, off\n"
        "# Flat-Out Head On (both US and EU)\n"
        "ULUS10328 ULES00968, infernocache, off\n"
        "\n"
        "# Enable Extra RAM on GTA LCS and VCS for CheatDeviceRemastered\n"
        "ULUS10041 ULUS10160 ULES00151 ULES00502, highmem, on\n"
    ;

    char arkMenuPath[ARK_PATH_SIZE];
    char arkSettingsPath[ARK_PATH_SIZE];
    sce_paf_private_strcpy(arkMenuPath, ark_config.arkpath);
    sce_paf_private_strcpy(arkSettingsPath, ark_config.arkpath);
    strcat(arkMenuPath, MENU_SETTINGS);
    strcat(arkSettingsPath, ARK_SETTINGS);
    
    int fd = sceIoOpen(arkMenuPath, PSP_O_RDONLY, 0);
    if(fd) {
        sceIoClose(fd);
        sceIoRemove(arkMenuPath);
        sceIoRemove(arkSettingsPath);
        sceKernelDelayThread(8000);
        int settings_file = sceIoOpen(arkSettingsPath, PSP_O_CREAT | PSP_O_WRONLY, 0777);
        sceIoWrite(settings_file, settings, sizeof(settings)-1);
        sceKernelDelayThread(8000);
        sceIoClose(settings_file);

    }
}

int has_classic_plugins(){
    SceIoStat stat;
    return (
        sceIoGetstat(GAME_PLUGIN_PATH_GO, &stat) >= 0 ||
        sceIoGetstat(GAME_PLUGIN_PATH, &stat) >= 0 ||
        sceIoGetstat(VSH_PLUGIN_PATH_GO, &stat) >= 0 ||
        sceIoGetstat(VSH_PLUGIN_PATH, &stat) >= 0 ||
        sceIoGetstat(POPS_PLUGIN_PATH_GO, &stat) >= 0 ||
        sceIoGetstat(POPS_PLUGIN_PATH, &stat) >= 0
    );
}

void import_classic_plugins(int devpath, int cleanup) {
    SceUID game, vsh, pops, plugins;
    int i = 0;
    int chunksize = 512;
    int bytesRead;
    char *buf = sce_paf_private_malloc(chunksize);
    char *gameChar = "game, ";
    int gameCharLength = sce_paf_private_strlen(gameChar);
    char *vshChar = "vsh, ";
    int vshCharLength = sce_paf_private_strlen(vshChar);
    char *popsChar = "pops, ";
    int popsCharLength = sce_paf_private_strlen(popsChar);
    
    const char* filename = (devpath)? PLUGIN_PATH_GO : PLUGIN_PATH;
    const char* gamepath = (devpath)? GAME_PLUGIN_PATH_GO : GAME_PLUGIN_PATH;
    const char* vshpath = (devpath)? VSH_PLUGIN_PATH_GO : VSH_PLUGIN_PATH;
    const char* popspath = (devpath)? POPS_PLUGIN_PATH_GO : POPS_PLUGIN_PATH;

    game = sceIoOpen(gamepath, PSP_O_RDONLY, 0777);
    vsh = sceIoOpen(vshpath, PSP_O_RDONLY, 0777);
    pops = sceIoOpen(popspath, PSP_O_RDONLY, 0777);
    plugins = sceIoOpen(filename, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);

    // GAME.txt
    sce_paf_private_memset(buf, 0, chunksize);
    while ((bytesRead = sceIoRead(game, buf, chunksize)) > 0) {
        for(i = 0; i < bytesRead; i++) {
        	if (i == 0 || buf[i-1] == '\n' || buf[i-1] == '\0'){
        		sceIoWrite(plugins, gameChar, gameCharLength);
        	}
        	if (buf[i] == ' ' && i != 0)
        		sceIoWrite(plugins, ",", 1);
        	sceIoWrite(plugins, &buf[i], 1);
        }
    }
    sceIoClose(game);


    sce_paf_private_memset(buf, 0, chunksize);

    // VSH.txt
    while ((bytesRead = sceIoRead(vsh, buf, chunksize)) > 0) {
        for(i = 0; i < bytesRead; i++) {
        	if (i == 0 || buf[i-1] == '\n' || buf[i-1] == '\0'){
        		sceIoWrite(plugins, vshChar, vshCharLength);
        	}
        	if (buf[i] == ' ' && i != 0)
        		sceIoWrite(plugins, ",", 1);
        	sceIoWrite(plugins, &buf[i], 1);
        }
    }
    sceIoClose(vsh);

    sce_paf_private_memset(buf, 0, chunksize);

    // POP.txt
    while ((bytesRead = sceIoRead(pops, buf, chunksize)) > 0) {
        for(i = 0; i < bytesRead; i++) {
        	if (i == 0 || buf[i-1] == '\n' || buf[i-1] == '\0'){
        		sceIoWrite(plugins, popsChar, popsCharLength);
        	}
        	if (buf[i] == ' ' && i != 0)
        		sceIoWrite(plugins, ",", 1);
        	sceIoWrite(plugins, &buf[i], 1);
        }
    }
    sceIoClose(pops);

    sceIoClose(plugins);
    sce_paf_private_free(buf);

    if (cleanup){
        sceIoRemove(gamepath);
        sceIoRemove(vshpath);
        sceIoRemove(popspath);
    }
}


SceOff findPkgOffset(const char* filename, unsigned* size, const char* pkgpath){

    int pkg = sceIoOpen(pkgpath, PSP_O_RDONLY, 0777);
    if (pkg < 0)
        return 0;
     
    unsigned pkgsize = sceIoLseek32(pkg, 0, PSP_SEEK_END);
    unsigned size2 = 0;
     
    sceIoLseek32(pkg, 0, PSP_SEEK_SET);

    if (size != NULL)
        *size = 0;

    unsigned offset = 0;
    char name[64];
           
    while (offset != 0xFFFFFFFF){
        sceIoRead(pkg, &offset, 4);
        if (offset == 0xFFFFFFFF){
            sceIoClose(pkg);
            return 0;
        }
        unsigned namelength;
        sceIoRead(pkg, &namelength, 4);
        sceIoRead(pkg, name, namelength+1);
                   
        if (!strncmp(name, filename, namelength)){
            sceIoRead(pkg, &size2, 4);
    
            if (size2 == 0xFFFFFFFF)
                size2 = pkgsize;

            if (size != NULL)
                *size = size2 - offset;
     
            sceIoClose(pkg);
            return offset;
        }
    }
    return 0;
}

void findAllTranslatableStrings(){
    sce_paf_private_memset(language_strings, 0, sizeof(language_strings));
    n_translated = 0;

    language_strings[n_translated++].orig = "xmbmsg_system_update";
    language_strings[n_translated++].orig = "xmbmsgtop_sysconf_configuration";
    language_strings[n_translated++].orig = "xmbmsgtop_sysconf_plugins";
    language_strings[n_translated++].orig = "xmbmsgtop_custom_launcher";
    language_strings[n_translated++].orig = "xmbmsgtop_custom_app";
    language_strings[n_translated++].orig = "xmbmsgtop_150_reboot";
    
    for (int i=0; i<NELEMS(GetItemes); i++){
        language_strings[n_translated++].orig = GetItemes[i].item;
    }

    for (int i=0; i<NELEMS(item_opts); i++){
        for (int j=0; j<item_opts[i].n; j++){
            language_strings[n_translated++].orig = item_opts[i].c[j];
        }
    }
}

static int findTranslatableStringIndex(char* line){
    char* txt_start = sce_paf_private_strchr(line, '"');
    if (!txt_start) return -1;
    for (int i=0; i<n_translated; i++){
        char* item = language_strings[i].orig;
        if (item == NULL) continue;
        if (sce_paf_private_strcmp(line, item) == 0) return i;
        char* sub = strstr(line, item);
        if (sub == NULL) continue;
        int l = sce_paf_private_strlen(item);
        if (sub == txt_start+1 && sub[l] == '"')
            return i;
    }
    return -1;
}

static char* findTranslation(char* text){
    for (int i=0; i<n_translated; i++)
    {
        if (sce_paf_private_strcmp(text, language_strings[i].orig) == 0){
            return language_strings[i].translated;
        }
    }
    return NULL;
}

static int isTranslatableString(char* text){
    for (int i=0; i<n_translated; i++)
    {
        if (sce_paf_private_strcmp(text, language_strings[i].orig) == 0){
            return 1;
        }
    }
    return 0;
}

int LoadTextLanguage(int new_id)
{
    static char *languages[] = { "jp", "en", "fr", "es", "de", "it", "nl", "pt", "ru", "ko", "cht", "chs" };

    int id; sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &id);

    if(new_id >= 0)
    {
        if(new_id == id) return 0;
        id = new_id;
    }

    for (int i=0; i<n_translated; i++){
        sce_paf_private_free(language_strings[i].translated);
        language_strings[i].translated = NULL;
    }

    SceUID fd = -1;
    SceOff offset = 0;
    unsigned size = 0;
    if (id < NELEMS(languages)){
        char file[64];
        char pkgpath[ARK_PATH_SIZE];
    
        sce_paf_private_strcpy(pkgpath, ark_config.arkpath);
        strcat(pkgpath, "LANG.ARK");
        sce_paf_private_sprintf(file, "lang_%s.json", languages[id]);
        offset = findPkgOffset(file, &size, pkgpath);
        if (!offset && !size)
            pkgpath[0] = 0;
        
        fd = sceIoOpen(pkgpath, PSP_O_RDONLY, 0);
    }

    if(fd < 0) return 0;

    u8* buf = sce_paf_private_malloc(size+1);
    sceIoLseek(fd, offset, PSP_SEEK_SET);
    sceIoRead(fd, buf, size);
    sceIoClose(fd);
    buf[size] = 0;
    

    int counter = 0;
    char line[LINE_BUFFER_SIZE];
    int buf_pos = 0;

    // Skip UTF8 magic
    u32 magic = *(u32*)buf;
    if ((magic & 0xFFFFFF) == 0xBFBBEF){
        buf_pos = 3;
    }

    while (counter < n_translated)
    {
        if (buf_pos >= size) break;

        int n_read = readLine((char*)buf+buf_pos, line);
        buf_pos += n_read;

        if (n_read == 0) break;
        if (sce_paf_private_strchr(line, '"') == NULL) continue;

        char* sep = NULL;
        int text_idx = findTranslatableStringIndex(line);

        if (text_idx>=0){
            char* aux = language_strings[text_idx].orig;
            sep = sce_paf_private_strchr(sce_paf_private_strchr(line, '"')+sce_paf_private_strlen(aux)+1, ':');
            if (!sep) continue;
        }
        else continue;

        char* start = sce_paf_private_strchr(sep, '"');
        if (!start) continue;

        char* translated = sce_paf_private_malloc(strlen(start+1)+1);
        sce_paf_private_strcpy(translated, start+1);

        char* ending = sce_paf_private_strrchr(translated, '"');
        if (ending) *ending = 0;

        language_strings[text_idx].translated = translated;
        counter++;

    }

    sce_paf_private_free(buf);

    return 1;
}

void* addCustomVshItem(int id, char* text, int action_arg, SceVshItem* orig){
    SceVshItem* item = (SceVshItem *)sce_paf_private_malloc(sizeof(SceVshItem));
    sce_paf_private_memset(item, 0, sizeof(SceVshItem));
    sce_paf_private_memcpy(item, orig, sizeof(SceVshItem));

    item->id = id; // custom id
    item->action = sysconf_action;
    item->action_arg = action_arg;
    item->play_sound = 1;
    item->context = NULL;
    sce_paf_private_strcpy(item->text, text);

    return item;
}

int AddVshItemPatched(void *a0, int topitem, SceVshItem *item)
{

    static int items_added = 0;

    if (sce_paf_private_strcmp(item->text, "msgtop_sysconf_console")==0){
        sysconf_action = item->action;
        LoadTextLanguage(-1);
    }

    if ( !items_added && // prevent adding more than once
        // Game Items
        (sce_paf_private_strcmp(item->text, "msgtop_game_gamedl")==0 ||
        sce_paf_private_strcmp(item->text, "msgtop_game_savedata")==0 ||
        // Extras Items
        sce_paf_private_strcmp(item->text, "msg_digitalcomics")==0 ||
        sce_paf_private_strcmp(item->text, "msg_bookreader")==0 ||
        sce_paf_private_strcmp(item->text, "msg_1seg")==0 ||
        sce_paf_private_strcmp(item->text, "msg_xradar_portable")==0 ||
        sce_paf_private_strcmp(item->text, "msg_tdmb")==0)
        )
    {
        items_added = 1;
        startup = 0;

        int cur_icon = 0;
        SceIoStat stat;
        int ebootFound;

        if (psp_model == PSP_11000){
            u32 value = 0;
            vctrlGetRegistryValue("/CONFIG/SYSTEM/XMB/THEME", "custom_theme_mode", &value);
            cur_icon = !value;
        }

        // Add CFW Settings
        new_item = addCustomVshItem(81, "xmbmsgtop_sysconf_configuration", sysconf_tnconfig_action_arg, (cur_icon)?item:(SceVshItem*)signup_item);
        AddVshItem(a0, topitem, new_item);

        // Add Plugins Manager
        new_item2 = addCustomVshItem(82, "xmbmsgtop_sysconf_plugins", sysconf_plugins_action_arg, (cur_icon)?item:(SceVshItem*)ps_store_item);
        AddVshItem(a0, topitem, new_item2);

        // Add Custom Launcher (if found)
        char launcher_path[ARK_PATH_SIZE];
        sce_paf_private_strcpy(launcher_path, ark_config.arkpath);
        strcat(launcher_path, VBOOT_PBP);
        ebootFound = sceIoGetstat(launcher_path, &stat);
        if (ebootFound >= 0){
            new_item3 = addCustomVshItem(83, "xmbmsgtop_custom_launcher", sysconf_custom_launcher_arg, (cur_icon)?item:(SceVshItem*)information_board_item);
            AddVshItem(a0, topitem, new_item3);
        }
        
        // Add Custom App (if found)
        custom_app_path[0] = 'e';
        custom_app_path[1] = 'f';
        ebootFound = sceIoGetstat(custom_app_path, &stat);
        if(ebootFound < 0) {
            custom_app_path[0] = 'm'; 
            custom_app_path[1] = 's';
            ebootFound = sceIoGetstat(custom_app_path, &stat);
        }
        
        if(ebootFound >= 0) {
            new_item4 = addCustomVshItem(84, "xmbmsgtop_custom_app", sysconf_custom_app_arg, (SceVshItem*)information_board_item);
            AddVshItem(a0, topitem, new_item4);
        }

        // Add 1.50 Kernel Addon (if found)
        SceIoStat _150_file;
        int _1k_file = sceIoGetstat("ms0:/TM/DCARK/150/reboot150.prx", &_150_file);
        if((psp_model == PSP_1000) && _1k_file >= 0 && !IS_VITA_ADR((&ark_config))) {
            new_item5 = addCustomVshItem(84, "xmbmsgtop_150_reboot", sysconf_150_reboot_arg, item);
            AddVshItem(a0, topitem, new_item5);
        }

    }
    
    return AddVshItem(a0, topitem, item);

}

int OnXmbPushPatched(void *arg0, void *arg1)
{
    xmb_arg0 = arg0;
    xmb_arg1 = arg1;
    return OnXmbPush(arg0, arg1);
}

int OnXmbContextMenuPatched(void *arg0, void *arg1)
{
    new_item->context = NULL;
    new_item2->context = NULL;
    return OnXmbContextMenu(arg0, arg1);
}

int ExecuteActionPatched(int action, int action_arg)
{
    int old_is_cfw_config = is_cfw_config;

    if(action == sysconf_console_action)
    {
        if(action_arg == sysconf_tnconfig_action_arg)
        {
            is_cfw_config = 1;
            action = sysconf_console_action;
            action_arg = sysconf_console_action_arg;
        }
        else if (action_arg == sysconf_plugins_action_arg)
        {
            is_cfw_config = 2;
            action = sysconf_console_action;
            action_arg = sysconf_console_action_arg;
        }
        else if (action_arg == sysconf_custom_launcher_arg){
            exec_custom_launcher();
        }
        else if (action_arg == sysconf_custom_app_arg){
            exec_custom_app(custom_app_path);
        }
        else if (action_arg == sysconf_150_reboot_arg){
        	exec_150_reboot();
        }
        else is_cfw_config = 0;
    }
    if(old_is_cfw_config != is_cfw_config)
    {
        sce_paf_private_memset(backup, 0, sizeof(backup));
        context_mode = 0;

        unload = 1;
    }

    return ExecuteAction(action, action_arg);
}

int UnloadModulePatched(int skip)
{
    if(unload)
    {
        skip = -1;
        unload = 0;
        //clear_list(&plugins, &plugin_list_cleaner);
        //clear_list(&iplugins, &plugin_list_cleaner);
    }
    return UnloadModule(skip);
}

void AddSysconfContextItem(char *text, char *subtitle, char *regkey)
{
    SceSysconfItem *item = (SceSysconfItem *)sce_paf_private_malloc(sizeof(SceSysconfItem));

    item->id = 5;
    item->unk = (u32 *)sysconf_unk;
    item->regkey = regkey;
    item->text = text;
    item->subtitle = subtitle;
    item->page = "page_psp_config_umd_autoboot";

    ((u32 *)sysconf_option)[2] = 1;

    AddSysconfItem((u32 *)sysconf_option, &item);
}

int skipSetting(int i){
    if (i == ACTIVATE_CODECS && codecs_active) return 1;
    if (i == BATTERY_CONVERT && battery_type < 0) return 1;
    if (IS_VITA((&ark_config))) return (
        i == USB_DEVICE ||
        i == USB_READONLY ||
        i == USB_CHARGE ||
        i == DISABLE_GO_PAUSE ||
        i == OLD_GO_PLUGINS ||
        i == NO_HIB_DELETE ||
        i == DISABLE_LED ||
        i == DISABLE_UMD ||
        i == WPA2_SUPPORT ||
        i == UMD_REGION ||
        i == GO_PAUSE_DELETE
    );
    else if (psp_model == PSP_1000) return (
        i == USB_CHARGE ||
        i == DISABLE_GO_PAUSE ||
        i == USE_EXTRA_MEM ||
        i == OLD_GO_PLUGINS ||
        i == NO_HIB_DELETE ||
        i == GO_PAUSE_DELETE
    );
    else if (psp_model == PSP_11000) return (
        i == DISABLE_GO_PAUSE ||
        i == OLD_GO_PLUGINS ||
        i == NO_HIB_DELETE ||
        i == HIDE_MAC ||
        i == WPA2_SUPPORT
    );
    else if (psp_model != PSP_GO) return (
        i == DISABLE_GO_PAUSE ||
        i == OLD_GO_PLUGINS ||
        i == NO_HIB_DELETE  ||
        i == GO_PAUSE_DELETE
    );
    else if (psp_model == PSP_GO) return (
        i == DISABLE_UMD ||
        i == UMD_REGION ||
        (i == GO_PAUSE_DELETE && !has_hibernation) 
    );
    return 0;
}

void OnInitMenuPspConfigPatched()
{
    if(is_cfw_config == 1)
    {
        if (((u32*)sysconf_option)[2] == 0)
        {
            codecs_active = codecs_activated();
            battery_type = battery_init();
            has_hibernation = vshCtrlHibernationExists();
            
            // handle USB Devices
            int n_usbdev = NELEMS(usbdev_settings);
            if (psp_model == PSP_GO){ // Switch "Memory Stick" with "Internal Storage" on PSP Go
                usbdev_settings[0] = usbdev_settings[n_usbdev-1];
            }
            // remove "Internal Storage" option (if not there)
            if (n_usbdev == item_opts[USB_DEVICE+PLUGINS_CONTEXT+2].n)
                item_opts[USB_DEVICE+PLUGINS_CONTEXT+2].n--;
            
            // remove Shutdown option from PS Vita
            if (IS_VITA((&ark_config))){
                item_opts[SYSTEM_OPTIONS+PLUGINS_CONTEXT+2].n = NELEMS(system_opts_vita);
                item_opts[SYSTEM_OPTIONS+PLUGINS_CONTEXT+2].c = system_opts_vita;
            }
            
            loadSettings();
            int i;
            for(i = 0; i < NELEMS(GetItemes); i++)
            {
                if (skipSetting(i)){
                    continue;
                }
                else{
                    AddSysconfContextItem(GetItemes[i].item, NULL, GetItemes[i].item);
                }
            }
        }
    }
    else if (is_cfw_config == 2){
        if (((u32*)sysconf_option)[2] == 0)
        {
            if (has_classic_plugins()){
                AddSysconfContextItem("Import Classic Plugins", NULL, "Import Classic Plugins");
            }
            loadPlugins();
            findInstallablePlugins();
            for (int i=0; i<plugins.count; i++){
                Plugin* plugin = (Plugin*)(plugins.table[i]);
                if (plugin->name != NULL){
                    AddSysconfContextItem(plugin->name, plugin->surname, plugin->name);
                }
            }
            for (int i=0; i<iplugins.count; i++){
                Plugin* plugin = (Plugin*)(iplugins.table[i]);
                if (plugin->name != NULL){
                    AddSysconfContextItem(plugin->name, plugin->surname, plugin->name);
                }
            }
            char* last_played = getLastGameForPluginFormatted();
            if (last_played){
                plugins_install_options[NELEMS(plugins_install_options)-1] = last_played;
            }
            else {
                item_opts[PLUGINS_CONTEXT+1].n--;
            }
        }
    }
    else
    {
        OnInitMenuPspConfig();
    }
}

SceSysconfItem *GetSysconfItemPatched(void *a0, void *a1)
{
    SceSysconfItem *item = GetSysconfItem(a0, a1);

    if(is_cfw_config == 1)
    {
        int i;
        for(i = 0; i < NELEMS(GetItemes); i++)
        {
            if(sce_paf_private_strcmp(item->text, GetItemes[i].item) == 0)
            {
                context_mode = GetItemes[i].mode;
            }
        }
    }
    else if (is_cfw_config == 2){
        if (sce_paf_private_strcmp(item->text, "Import Classic Plugins") == 0){
            context_mode = PLUGINS_CONTEXT-1;
        }
        else if (sce_paf_private_strncmp(item->text, "iplugin_", 8) == 0){
            context_mode = PLUGINS_CONTEXT+1;
        }
        else {
            context_mode = PLUGINS_CONTEXT;
        }
    }
    return item;
}

wchar_t *scePafGetTextPatched(void *a0, char *name)
{
    if(name)
    {
        if(is_cfw_config == 1 || sce_paf_private_strncmp(name, "xmbmsg", 6)==0)
        {
            char* translated = findTranslation(name);
            char* star = NULL;
            if (!translated){
                if(sce_paf_private_strcmp(name, "xmbmsgtop_sysconf_configuration") == 0){
                    translated = "Custom Firmware Settings";
                    star = STAR;
                }
                else if(sce_paf_private_strcmp(name, "xmbmsgtop_sysconf_plugins") == 0){
                    translated = "Plugins Manager";
                    star = STAR;
                }
                else if(sce_paf_private_strcmp(name, "xmbmsgtop_custom_launcher") == 0){
                    translated = "Custom Launcher";
                    star = STAR;
                }
                else if(sce_paf_private_strcmp(name, "xmbmsgtop_custom_app") == 0){
                    translated = "Custom App";
                    star = STAR;
                }
                else if(sce_paf_private_strcmp(name, "xmbmsgtop_150_reboot") == 0){
                    translated = "Reboot to 1.50 ARK";
                    star = STAR;
                }
                else if (isTranslatableString(name))
                    translated = name; // should have been translated but wasn't
            }
            if (translated){
                int offset = 0;
                if (star) offset = utf8_to_unicode((wchar_t *)user_buffer, star);
                utf8_to_unicode((wchar_t *)((u8*)user_buffer+offset), translated);
                return (wchar_t *)user_buffer;
            }
        }
        else if (is_cfw_config == 2){
            if (sce_paf_private_strncmp(name, "plugin_", 7) == 0){
                u32 i = sce_paf_private_strtoul(name + 7, NULL, 10);
                Plugin* plugin = (Plugin*)(plugins.table[i]);
                char file[128];

        		utf8_to_unicode((wchar_t *)user_buffer, getPluginName(plugin->path, file));
        		return (wchar_t *)user_buffer;
            }
            else if (sce_paf_private_strncmp(name, "plugins", 7) == 0){
                u32 i = sce_paf_private_strtoul(name + 7, NULL, 10);
                Plugin* plugin = (Plugin*)(plugins.table[i]);
                char plugin_path[128];
                if (sce_paf_private_strchr(plugin->path, ':') == NULL){
                    sce_paf_private_sprintf(plugin_path, "<%s> %s, %s", plugins_paths[plugin->place], plugin->runlevel, plugin->path);
                }
                else{
                    sce_paf_private_sprintf(plugin_path, "%s, %s", plugin->runlevel, plugin->path);
                }
                utf8_to_unicode((wchar_t *)user_buffer, plugin_path);
        		return (wchar_t *)user_buffer;
            }
            else if (sce_paf_private_strncmp(name, "iplugin_", 8) == 0){
                u32 i = sce_paf_private_strtoul(name + 8, NULL, 10);
                Plugin* plugin = (Plugin*)(iplugins.table[i]);
                utf8_to_unicode((wchar_t *)user_buffer, plugin->path);
        		return (wchar_t *)user_buffer;
            }
            else if (sce_paf_private_strncmp(name, "iplugins", 8) == 0){
                u32 i = sce_paf_private_strtoul(name + 8, NULL, 10);
                Plugin* plugin = (Plugin*)(iplugins.table[i]);
                utf8_to_unicode((wchar_t *)user_buffer, plugins_paths[plugin->place]);
        		return (wchar_t *)user_buffer;
            }
            else if (sce_paf_private_strcmp(name, "Import Classic Plugins") == 0){
                char* translated = findTranslation(name);
                utf8_to_unicode((wchar_t *)user_buffer, (translated)? translated:name);
                return (wchar_t *)user_buffer;
            }
            else {
                char* translated = findTranslation(name);
                if (translated){
                    utf8_to_unicode((wchar_t *)user_buffer, translated);
                    return (wchar_t *)user_buffer;
                }
            }
        }
        else if (sce_paf_private_strcmp(name, "msg_system_update") == 0 && se_config.custom_update)
        {
            char* translated = findTranslation("xmbmsg_system_update");
            if (!translated) translated = "ARK Updater";
            utf8_to_unicode((wchar_t *)user_buffer, translated);
        	return (wchar_t *)user_buffer;
        }
    }

    return scePafGetText(a0, name);
}

int vshGetRegistryValuePatched(u32 *option, char *name, void *arg2, int size, int *value)
{
    if(name)
    {

        if(is_cfw_config == 1)
        {
            if (battery_type>=0)
                config.convert_battery = battery_type;
            else config.convert_battery = 0;

            u8 configs[] =
            {
                config.sysopt,
                config.activate_codecs,
                config.usbdevice,
                config.usbreadonly,
                config.usbcharge,        
                config.clock_game,        
                config.clock_vsh, 
                config.wpa2,           
                config.launcher,        
                config.highmem,        	
                config.mscache,        	
                config.infernocache,   
                config.disablepause,     
                config.oldplugin,        
                config.hibblock,    
                config.skiplogos,        
                config.hidepics,         	
                config.hidemac,         
                config.hidedlc,        	
                config.noled,        	
                config.noumd,        	
                config.noanalog,
                config.umdregion,
                config.vshregion,
                config.confirmbtn,
                config.convert_battery,
                config.qaflags,
                config.delete_go_pause,   
                config.reset_settings,     
            };
            
            int i;
            for(i = 0; i < NELEMS(GetItemes); i++)
            {
                if (sce_paf_private_strcmp(name, GetItemes[i].item) == 0)
                {
                    context_mode = GetItemes[i].mode;
                    *value = configs[i];
                    return 0;
                }
            }

        }
        else if (is_cfw_config == 2){
            if (sce_paf_private_strcmp(name, "Import Classic Plugins") == 0){
                *value = config.import_plugins;
                context_mode = PLUGINS_CONTEXT-1;
                return 0;
            }
            if (sce_paf_private_strncmp(name, "plugin_", 7) == 0)
        	{
        		u32 i = sce_paf_private_strtoul(name + 7, NULL, 10);
                Plugin* plugin = (Plugin*)(plugins.table[i]);
        		context_mode = PLUGINS_CONTEXT;
        		*value = plugin->active;
        		return 0;
        	}
            if (sce_paf_private_strncmp(name, "iplugin_", 8) == 0)
        	{
        		u32 i = sce_paf_private_strtoul(name + 8, NULL, 10);
                Plugin* plugin = (Plugin*)(iplugins.table[i]);
        		context_mode = PLUGINS_CONTEXT+1;
        		*value = plugin->active;
        		return 0;
        	}
        }
    }

    int res = vshGetRegistryValue(option, name, arg2, size, value);

    return res;
}

int vshSetRegistryValuePatched(u32 *option, char *name, int size, int *value)
{
    if (name)
    {
        if (is_cfw_config == 1)
        {
            static u8 *configs[] =
            {
                &config.sysopt,
                &config.activate_codecs,
                &config.usbdevice,
                &config.usbreadonly,
                &config.usbcharge,
                &config.clock_game,
                &config.clock_vsh,
                &config.wpa2,
                &config.launcher,
                &config.highmem,
                &config.mscache,
                &config.infernocache,
                &config.disablepause,
                &config.oldplugin,
                &config.hibblock,
                &config.skiplogos,
                &config.hidepics,
                &config.hidemac,
                &config.hidedlc,
                &config.noled,
                &config.noumd,
                &config.noanalog,
                &config.umdregion,
                &config.vshregion,
                &config.confirmbtn,
                &config.convert_battery,
                &config.qaflags,
                &config.delete_go_pause,
                &config.reset_settings,
            };
        
            for (int i = 0; i < NELEMS(GetItemes); i++)
            {
                if (sce_paf_private_strcmp(name, GetItemes[i].item) == 0)
                {
                    *configs[i] = GetItemes[i].negative ? !(*value) : *value;
                    saveSettings();
                    
                    if (i == SYSTEM_OPTIONS){
                        if (IS_VITA((&ark_config))){
                            switch (*value){
                                case SOFT_RESET:         sctrlKernelExitVSH(NULL); break;
                                case HARD_RESET:         scePowerRequestSuspend(); break;
                            }
                        }
                        else {
                            switch (*value){
                                case SOFT_RESET:         sctrlKernelExitVSH(NULL); break;
                                case HARD_RESET:         scePowerRequestColdReset(0); break;
                                case SUSPEND_DEVICE:     scePowerRequestSuspend(); break;
                                case SHUTDOWN_DEVICE:    scePowerRequestStandby(); break;
                            }
                        }
                    }
                    else if (i == UMD_REGION && config.umdregion){
                        recreate_umd_keys();
                    }
                    else if (i == USB_DEVICE || i == USB_READONLY){
                        se_config.usbdevice = config.usbdevice;
                        se_config.usbdevice_rdonly = config.usbreadonly;
                        sctrlSESetConfig((SEConfig*)&se_config);
                    }
                    else if (i == CONFIRM_BUTTON){
                        u32 value = config.confirmbtn;
                        vctrlSetRegistryValue("/CONFIG/SYSTEM/XMB", "button_assign", value);
                        sctrlKernelExitVSH(NULL);
                    }
                    else if (i == ACTIVATE_CODECS){
                        if (config.activate_codecs && activate_codecs())
                            sctrlKernelExitVSH(NULL);
                    }
                    else if (i == BATTERY_CONVERT){
                        battery_convert(config.convert_battery);
                        battery_type = config.convert_battery;
                    }
                    else if (i == GO_PAUSE_DELETE){
                        vshCtrlDeleteHibernation();
                        sctrlKernelExitVSH(NULL);
                    }
                    else if (i == RESET_SETTINGS){
                        if (config.reset_settings){
                            reset_ark_settings();
                            sctrlKernelExitVSH(NULL);
                        }
                    }
                    return 0;
                }
            }
        }
        else if (is_cfw_config == 2){
            if (sce_paf_private_strcmp(name, "Import Classic Plugins") == 0){
                config.import_plugins = *value;
                context_mode = PLUGINS_CONTEXT-1;
                import_classic_plugins(0, *value);
                import_classic_plugins(1, *value);
                sctrlKernelExitVSH(NULL);
                return 0;
            }
            if(sce_paf_private_strncmp(name, "plugin_", 7) == 0)
        	{
        		u32 i = sce_paf_private_strtoul(name + 7, NULL, 10);
                Plugin* plugin = (Plugin*)(plugins.table[i]);
        		context_mode = PLUGINS_CONTEXT;
        		plugin->active = *value;
                savePlugins();
                if (*value == PLUGIN_REMOVED){
                    sctrlKernelExitVSH(NULL);
                }
                return 0;
        	}
            if (sce_paf_private_strncmp(name, "iplugin_", 8) == 0)
        	{
        		u32 i = sce_paf_private_strtoul(name + 8, NULL, 10);
                Plugin* plugin = (Plugin*)(iplugins.table[i]);
        		context_mode = PLUGINS_CONTEXT+1;
        		plugin->active = *value;
                if (*value > 0){
                    char* opt = NULL;
                    if (*value == NELEMS(plugins_install_options)-1){
                        opt = getLastGameForPlugin();
                    }
                    installPlugin(plugin, opt);
                    sctrlKernelExitVSH(NULL);
                }
        		return 0;
        	}
        }
        if (sce_paf_private_strcmp(name, "/CONFIG/SYSTEM/XMB/language") == 0)
        {
            LoadTextLanguage(*value);
        }
    }

    return vshSetRegistryValue(option, name, size, value);
}

void HijackContext(SceRcoEntry *src, char **options, int n)
{
    SceRcoEntry *plane = (SceRcoEntry *)((u32)src + src->first_child);
    SceRcoEntry *mlist = (SceRcoEntry *)((u32)plane + plane->first_child);
    u32 *mlist_param = (u32 *)((u32)mlist + mlist->param);

    /* Backup */
    if(backup[0] == 0 && backup[1] == 0 && backup[2] == 0 && backup[3] == 0)
    {
        backup[0] = mlist->first_child;
        backup[1] = mlist->child_count;
        backup[2] = mlist_param[16];
        backup[3] = mlist_param[18];
    }

    if(context_mode)
    {
        SceRcoEntry *base = (SceRcoEntry *)((u32)mlist + mlist->first_child);

        SceRcoEntry *item = (SceRcoEntry *)sce_paf_private_malloc(base->next_entry * n);
        u32 *item_param = (u32 *)((u32)item + base->param);

        mlist->first_child = (u32)item - (u32)mlist;
        mlist->child_count = n;
        mlist_param[16] = 13;
        mlist_param[18] = 6;

        int i;
        for(i = 0; i < n; i++)
        {
            char* opt = options[i];
            char* translated = findTranslation(opt);
            sce_paf_private_memcpy(item, base, base->next_entry);

            item_param[0] = 0xDEAD;
            item_param[1] = (u32)((translated)? translated : opt);

            if(i != 0) item->prev_entry = item->next_entry;
            if(i == n - 1) item->next_entry = 0;

            item = (SceRcoEntry *)((u32)item + base->next_entry);
            item_param = (u32 *)((u32)item + base->param);
        }
    }
    else
    {
        // Restore
        mlist->first_child = backup[0];
        mlist->child_count = backup[1];
        mlist_param[16] = backup[2];
        mlist_param[18] = backup[3];
    }

    sceKernelDcacheWritebackAll();
}

int PAF_Resource_GetPageNodeByID_Patched(void *resource, char *name, SceRcoEntry **child)
{
    int res = PAF_Resource_GetPageNodeByID(resource, name, child);

    if(name)
    {
        if(is_cfw_config == 1 || is_cfw_config == 2)
        {
            if(sce_paf_private_strcmp(name, "page_psp_config_umd_autoboot") == 0)
            {
                HijackContext(*child, item_opts[context_mode].c, item_opts[context_mode].n);
            }
        }
    }

    return res;
}

int PAF_Resource_ResolveRefWString_Patched(void *resource, u32 *data, int *a2, char **string, int *t0)
{
    if(data[0] == 0xDEAD)
    {
        utf8_to_unicode((wchar_t *)user_buffer, (char *)data[1]);
        *(wchar_t **)string = (wchar_t *)user_buffer;
        return 0;
    }

    return PAF_Resource_ResolveRefWString(resource, data, a2, string, t0);
}

int auth_handler_new(int a0)
{
    startup = a0;
    return auth_handler(a0);
}

int OnInitAuthPatched(void *a0, int (* handler)(), void *a2, void *a3, int (* OnInitAuth)())
{
    return OnInitAuth(a0, startup ? auth_handler_new : handler, a2, a3);
}

int sceVshCommonGuiBottomDialogPatched(void *a0, void *a1, void *a2, int (* cancel_handler)(), void *t0, void *t1, int (* handler)(), void *t3)
{
    return sceVshCommonGuiBottomDialog(a0, a1, a2, startup ? OnRetry : (void *)cancel_handler, t0, t1, handler, t3);
}

void PatchVshMain(u32 text_addr, u32 text_size)
{
    int patches = 14;
    u32 scePafGetText_call = _lw((u32)&scePafGetText);

    for (u32 addr=text_addr; addr<text_addr+text_size && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x00063100){
            AddVshItem = (void*)(U_EXTRACT_CALL(addr+12));
            MAKE_CALL(addr + 12, AddVshItemPatched);
            patches--;
        }
        else if (data == 0x3A14000F){
            ExecuteAction = (void*)addr-72;
            MAKE_CALL(addr - 72 - 36, ExecuteActionPatched);
            patches--;
        }
        else if (data == 0xA0C3019C){
            UnloadModule = (void*)addr-52;
            patches--;
        }
        else if (data == 0x9042001C){
            OnXmbPush = (void*)addr-124;
            patches--;
        }
        else if (data == 0x00021202 && OnXmbContextMenu==NULL){
            OnXmbContextMenu = (void*)addr-24;
            patches--;
        }
        else if (data == 0x34420080 && LoadStartAuth==NULL){
            LoadStartAuth = (void*)addr-208;
            patches--;
        }
        else if (data == 0xA040014D){
            auth_handler = (void*)addr-32;
            patches--;
        }
        else if (data == 0x8E050038){
            MAKE_CALL(addr + 4, ExecuteActionPatched);
            patches--;
        }
        else if (data == 0xAC520124){
            MAKE_CALL(addr + 4, UnloadModulePatched);
            patches--;
        }
        else if (data == 0x24060064){
            patchVshClock(addr);
            patches--;
        }
        else if (data == 0x24040010 && _lw(addr+20) == 0x0040F809){
            _sw(0x8C48000C, addr + 16); //lw $t0, 12($v0)
            MAKE_CALL(addr + 20, OnInitAuthPatched);
            patches--;
        }
        else if (data == scePafGetText_call){
            REDIRECT_FUNCTION(addr, scePafGetTextPatched);
            patches--;
        }
        else if (data == (u32)OnXmbPush && OnXmbPush != NULL && addr > text_addr+0x50000){
            _sw((u32)OnXmbPushPatched, addr);
            patches--;
        }
        else if (data == (u32)OnXmbContextMenu && OnXmbContextMenu != NULL && addr > text_addr+0x50000){
            _sw((u32)OnXmbContextMenuPatched, addr);
            patches--;
        }
    }
    ClearCaches();
}

void PatchAuthPlugin(u32 text_addr, u32 text_size)
{
    for (u32 addr=text_addr; addr<text_addr+text_size; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x27BE0040){
            u32 a = addr-4;
            do {a-=4;} while (_lw(a) != 0x27BDFFF0);
            OnRetry = (void*)a;
        }
        else if (data == 0x44816000 && _lw(addr-4) == 0x3C0141F0){
            MAKE_CALL(addr+4, sceVshCommonGuiBottomDialogPatched);
            break;
        }
    }
    ClearCaches();
}

void PatchSysconfPlugin(u32 text_addr, u32 text_size)
{
    u32 PAF_Resource_GetPageNodeByID_call = _lw((u32)&PAF_Resource_GetPageNodeByID);
    u32 PAF_Resource_ResolveRefWString_call = _lw((u32)&PAF_Resource_ResolveRefWString);
    u32 scePafGetText_call = _lw((u32)&scePafGetText);
    int patches = 10;
    for (u32 addr=text_addr; addr<text_addr+text_size && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x24420008 && _lw(addr-4) == 0x00402821){
            AddSysconfItem = (void*)addr-36;
            patches--;
        }
        else if (data == 0x8C840008 && _lw(addr+4) == 0x27BDFFD0){
            GetSysconfItem = (void*)addr;
            patches--;
        }
        else if (data == 0xAFBF0060 && _lw(addr+4) == 0xAFB3005C && _lw(addr-12) == 0xAFB00050){
            OnInitMenuPspConfig = (void*)addr-20;
            patches--;
        }
        else if (data == 0x2C420012){
            // Allows more than 18 items
            _sh(0xFF, addr);
            patches--;
        }
        else if (data == 0x01202821){
            MAKE_CALL(addr + 8, vshGetRegistryValuePatched);
            MAKE_CALL(addr + 44, vshSetRegistryValuePatched);
            patches--;
        }
        else if (data == 0x2C620012 && _lw(addr-4) == 0x00408821){
            MAKE_CALL(addr - 16, GetSysconfItemPatched);
            patches--;
        }
        else if (data == (u32)OnInitMenuPspConfig && OnInitMenuPspConfig != NULL){
            _sw((u32)OnInitMenuPspConfigPatched, addr);
            patches--;
        }
        else if (data == PAF_Resource_GetPageNodeByID_call){
            REDIRECT_FUNCTION(addr, PAF_Resource_GetPageNodeByID_Patched);
            patches--;
        }
        else if (data == PAF_Resource_ResolveRefWString_call){
            REDIRECT_FUNCTION(addr, PAF_Resource_ResolveRefWString_Patched);
            patches--;
        }
        else if (data == scePafGetText_call){
            REDIRECT_FUNCTION(addr, scePafGetTextPatched);
            patches--;
        }
    }

    for (u32 addr=text_addr+0x33000; addr<text_addr+0x40000; addr++){
        if (strcmp((char*)addr, "fiji") == 0){
            sysconf_unk = addr+216;
            if (_lw(sysconf_unk+4) == 0) sysconf_unk -= 4; // adjust on TT/DT firmware
            sysconf_option = sysconf_unk + 0x4cc; //CHECK
            break;
        }
    }

    ClearCaches();
}

int OnModuleStart(SceModule *mod)
{

    char *modname = mod->modname;
    u32 text_addr = mod->text_addr;
    u32 text_size = mod->text_size;

    if(strcmp(modname, "vsh_module") == 0){
        PatchVshMain(text_addr, text_size);
    }
    
    else if(strcmp(modname, "sceVshAuthPlugin_Module") == 0){
        PatchAuthPlugin(text_addr, text_size);
    }
    
    else if(strcmp(modname, "sysconf_plugin_module") == 0){
        PatchSysconfPlugin(text_addr, text_size);
    }

    if (previous) return previous(mod);
    return 0;
}
