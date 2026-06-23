/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <pspinit.h>
#include <pspmodulemgr.h>
#include <pspiofilemgr.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_ark.h>

#include "rebootex.h"
#include "config.h"



extern ARKConfig ark_config;
extern SEConfigARK se_config;

#define MAX_PLUGINS 64
#define MAX_PLUGIN_PATH 256

typedef struct{
    int count;
    char paths[MAX_PLUGINS][MAX_PLUGIN_PATH];
}Plugins;

Plugins* plugins = NULL;

static int ef0PluginHandler(const char* path, SceUID* modid);
int (*plugin_handler)(const char* path, SceUID* modid) = &ef0PluginHandler;


int pluginsLoaded = 0;
int disable_plugins = 0;
int is_plugins_loading = 0;


static void addPlugin(const char* path){
    for (int i=0; i<plugins->count; i++){
        const char* cmp1 = strchr(plugins->paths[i], ':');
        const char* cmp2 = strchr(path, ':');
        if (!cmp1) cmp1 = plugins->paths[i]; 
        if (!cmp2) cmp2 = path;
        if (strcasecmp(cmp1, cmp2) == 0)
            return; // plugin already added
    }
    if (plugins->count < MAX_PLUGINS)
        strcpy(plugins->paths[plugins->count++], path);
}

static void removePlugin(const char* path){
    for (int i=0; i<plugins->count; i++){
        if (strcasecmp(plugins->paths[i], path) == 0){
            if (--plugins->count > i){
                strcpy(plugins->paths[i], plugins->paths[plugins->count]);
            }
            break;
        }
    }
}

// Load and Start Plugin Module
static void startPlugins()
{
    for (int i=0; i<plugins->count; i++){
        int res = 0;
        char* path = plugins->paths[i];
        // Load Module
        SceUID uid = sceKernelLoadModule(path, 0, NULL);
        if (uid >= 0){
            // Call handler
            if (plugin_handler){
                res = plugin_handler(path, &uid);
                // Unload Module on Error
                if (res < 0){
                    sceKernelUnloadModule(uid);
                    continue;
                }
            }
            // Start Module
            res = sceKernelStartModule(uid, strlen(path) + 1, path, NULL, NULL);
            // Unload Module on Error
            if (res < 0) sceKernelUnloadModule(uid);
        }
    }
}

void loadPlugins(){
    if (disable_plugins || pluginsLoaded || sceKernelFindModuleByName("DesCemManager")!=NULL)
        return; // don't load plugins in recovery mode

    is_plugins_loading = 1;
    // allocate resources
    plugins = oe_malloc(sizeof(Plugins));
    plugins->count = 0; // initialize plugins table
    
    // Open Plugin Config from ARK's installation folder
    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config.arkpath);
    strcat(path, PLUGINS_FILE);
    processConfigFile(ark_config.arkpath, path, addPlugin, removePlugin);
    
    // Open Plugin Config from SEPLUGINS
    processConfigFile(SEPLUGINS_MS0, PLUGINS_PATH, addPlugin, removePlugin);
    
    // On PSP Go (only if ms0 isn't already redirected to ef0)
    processConfigFile(SEPLUGINS_EF0, PLUGINS_PATH_GO, addPlugin, removePlugin);
    
    // Flash0 plugins
    processConfigFile(FLASH0_PATH, PLUGINS_PATH_FLASH, addPlugin, removePlugin);
    
    // start all loaded plugins
    startPlugins();
    
    // free resources
    oe_free(plugins);
    plugins = NULL;
    is_plugins_loading = 0;
    
    // Remember it
    pluginsLoaded = 1;
}

static int needs_devicename_patch(SceModule* mod){
    for (int i=0; i<mod->nsegment; ++i) {
        u32 end = mod->segmentaddr[i] + mod->segmentsize[i];
        for(u32 addr = mod->segmentaddr[i]; addr < end; addr ++) {
        	char *str = (char*)addr;
        	if (0 == strncmp(str, "ef0", 3)) {
        		return 0;
        	} else if (0 == strncmp(str, "fatef", 5)) {
        		return 0;
        	}
        }
    }
    
    u32 start = mod->text_addr+mod->text_size;
    u32 end = start + mod->data_size;
    for (u32 addr=start; addr<end; addr++){
        char *str = (char*)addr;
        if (0 == strncmp(str, "ef0", 3)) {
        	return 0;
        } else if (0 == strncmp(str, "fatef", 5)) {
        	return 0;
        }
    }
    return 1;
}

static void patch_devicename(SceUID modid)
{
    SceModule* mod = (SceModule*)sceKernelFindModuleByUID(modid);

    if(mod == NULL || !needs_devicename_patch(mod)) {
        return;
    }

    for(int i=0; i<mod->nsegment; ++i) {
        u32 end = mod->segmentaddr[i] + mod->segmentsize[i];
        for(u32 addr = mod->segmentaddr[i]; addr < end; addr ++) {
        	char *str = (char*)addr;
        	if (0 == strncmp(str, "ms0", 3)) {
        		str[0] = 'e';
        		str[1] = 'f';
        	} else if (0 == strncmp(str, "fatms", 5)) {
        		str[3] = 'e';
        		str[4] = 'f';
        	}
        }
    }
    
    u32 start = mod->text_addr+mod->text_size;
    u32 end = start + mod->data_size;
    for (u32 addr=start; addr<end; addr++){
        char *str = (char*)addr;
        if (0 == strncmp(str, "ms0", 3)) {
        	str[0] = 'e';
        	str[1] = 'f';
        } else if (0 == strncmp(str, "fatms", 5)) {
        	str[3] = 'e';
        	str[4] = 'f';
        }
    }

    sctrlFlushCache();
}

static int ef0PluginHandler(const char* path, SceUID* modid){
    if(se_config.oldplugin && path[0] == 'e' && path[1] == 'f') {
        patch_devicename(*modid);
    }
    return 0;
}
