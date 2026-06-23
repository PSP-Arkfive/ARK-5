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



extern ARKConfig* ark_config;
extern SEConfigARK se_config;

int settingsLoaded = 0;
int disable_settings = 0;

static void settingsHandler(const char* path, u8 enabled){
    int apitype = sceKernelInitApitype();
    if (strcasecmp(path, "cpuclock:333") == 0 || strcasecmp(path, "overclock") == 0){ // set CPU speed to max
        if (enabled)
            se_config.cpubus_clock = CPU_BUS_CLOCK_333;
        else if (se_config.cpubus_clock == CPU_BUS_CLOCK_333) se_config.cpubus_clock = 0;
    }
    else if (strcasecmp(path, "cpuclock:133") == 0 || strcasecmp(path, "powersave") == 0){ // underclock to save battery
        if (apitype != 0x144 && apitype != 0x155){ // prevent operation in pops
            if (enabled)
                se_config.cpubus_clock = CPU_BUS_CLOCK_133;
            else if (se_config.cpubus_clock == CPU_BUS_CLOCK_133) se_config.cpubus_clock = 0;
        }
    }
    else if (strcasecmp(path, "cpuclock:222") == 0 || strcasecmp(path, "defaultclock") == 0){
        if (apitype != 0x144 && apitype != 0x155){ // prevent operation in pops
            if (enabled)
                se_config.cpubus_clock = CPU_BUS_CLOCK_222;
            else if (se_config.cpubus_clock == CPU_BUS_CLOCK_222) se_config.cpubus_clock = 0;
        }
    }
    else if (strncasecmp(path, "cpuclock:", 9) == 0){ // custom cpu clock
        if (enabled){
            unsigned long r = strtoul(path+9, NULL, 10);
            se_config.custom_cpu_clock = r;
            se_config.cpubus_clock = CPU_BUS_CLOCK_CUSTOM;
        }
        else se_config.custom_cpu_clock = 0;
    }
    else if (strncasecmp(path, "busclock:", 9) == 0){ // custom bus clock
        if (enabled){
            unsigned long r = strtoul(path+9, NULL, 10);
            se_config.custom_bus_clock = r;
        }
        else se_config.custom_bus_clock = 0;
    }
    else if (strcasecmp(path, "wpa2") == 0){ // wpa2 support
        se_config.wpa2 = enabled;
    }
    else if (strcasecmp(path, "usbcharge") == 0){ // enable usb charging
        se_config.usbcharge = enabled;
    }
    else if (strcasecmp(path, "highmem") == 0){ // enable high memory
        if (sceKernelFindModuleByName("sceUmdCache_driver") != NULL){
            // don't allow high memory in UMD when cache is enabled
            return;
        }
        se_config.force_high_memory = enabled;
    }
    else if (strcasecmp(path, "mscache") == 0){
        se_config.msspeed = enabled; // enable ms cache for speedup
    }
    else if (strcasecmp(path, "disablepause") == 0){ // disable pause game feature on psp go
        se_config.disable_pause = enabled;
    }
    else if (strcasecmp(path, "launcher") == 0){ // replace XMB with custom launcher
        se_config.launcher_mode = enabled;
    }
    else if (strcasecmp(path, "oldplugin") == 0){ // redirect ms0 to ef0 on psp go
        se_config.oldplugin = enabled;
    }
    else if (strncasecmp(path, "infernocache", 12) == 0){
        char* c = strchr(path, ':');
        se_config.iso_cache_type = enabled;
        if (enabled && c){
            if (strcasecmp(c+1, "lru") == 0) se_config.iso_cache_type = 1;
            else if (strcasecmp(c+1, "rr") == 0) se_config.iso_cache_type = 2;
        }
    }
    else if (strcasecmp(path, "noled") == 0){
        se_config.noled = enabled;
    }
    else if (strcasecmp(path, "noumd") == 0){
        se_config.noumd = enabled;
    }
    else if (strcasecmp(path, "noanalog") == 0){
        se_config.noanalog = enabled;
    }
    else if (strcasecmp(path, "vitamute") == 0){ // vita-style mute
        se_config.vitamute = enabled;
    }
    else if (strcasecmp(path, "hidemac") == 0){ // hide mac address
        se_config.hidemac = enabled;
    }
    else if (strcasecmp(path, "hidedlc") == 0){ // hide mac address
        se_config.hidedlc = enabled;
    }
    else if (strcasecmp(path, "qaflags") == 0){ // QA Flags
        se_config.qaflags = enabled;
    }
    else if (strncasecmp(path, "region_", 7) == 0){
        char* c = strchr(path, '_')+1;
        if (strcasecmp(c, "jp") == 0){
            se_config.umdregion = (enabled)?UMD_REGION_JAPAN:0;
        }
        else if (strcasecmp(c, "us") == 0){
            se_config.umdregion = (enabled)?UMD_REGION_AMERICA:0;
        }
        else if (strcasecmp(c, "eu") == 0){
            se_config.umdregion = (enabled)?UMD_REGION_EUROPE:0;
        }
    }
    else if (strncasecmp(path, "fakeregion_", 11) == 0){
        unsigned long r = strtoul(path+11, NULL, 10);
        se_config.vshregion = (enabled)?r:0;
    }
    else if (strncasecmp(path, "umdseek_", 8) == 0){
        unsigned long r = strtoul(path+8, NULL, 10);
        se_config.umdseek = (enabled)?r:0;
    }
    else if (strncasecmp(path, "umdspeed_", 9) == 0){
        unsigned long r = strtoul(path+9, NULL, 10);
        se_config.umdspeed = (enabled)?r:0;
    }
    else if (strcasecmp(path, "hibblock") == 0){ // block hibernation
        se_config.hibblock = enabled;
    }
    else if (strncasecmp(path, "skiplogos", 9) == 0){
        char* c = strchr(path, ':');
        se_config.skiplogos = enabled;
        if (enabled && c){
            if (strcasecmp(c+1, "gameboot") == 0) se_config.skiplogos = 2;
            else if (strcasecmp(c+1, "coldboot") == 0) se_config.skiplogos = 3;
        }
    }
    else if (strncasecmp(path, "hidepics", 8) == 0){ // hide PIC0 and PIC1
        char* c = strchr(path, ':');
        se_config.hidepics = enabled;
        if (enabled && c){
            if (strcasecmp(c+1, "pic0") == 0) se_config.hidepics = 2;
            else if (strcasecmp(c+1, "pic1") == 0) se_config.hidepics = 3;
        }
    }
}

static void settingsEnabler(const char* path){
    settingsHandler(path, 1);
}

static void settingsDisabler(const char* path){
    settingsHandler(path, 0);
}

void loadSettings(){
    if (disable_settings || settingsLoaded)
        return; // don't load settings in recovery mode

        // process settings file
    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, ARK_SETTINGS);
    if (processConfigFile(NULL, path, settingsEnabler, settingsDisabler) < 0) // try external settings
        processConfigFile(NULL, ARK_SETTINGS_FLASH, settingsEnabler, settingsDisabler); // retry flash1 settings

    int apitype = sceKernelInitApitype();
    if (apitype == 0x141 || apitype == 0x152){
        u32 paramsize=4;
        int use_highmem = 0;
        if (sctrlGetInitPARAM("MEMSIZE", NULL, &paramsize, &use_highmem) >= 0 && use_highmem){
            se_config.force_high_memory = 2;
        }
    }

    if (se_config.force_high_memory){
        se_config.disable_pause = 1; // unless we figure out how to fix this
    }

    // Remember it
    settingsLoaded = 1;
}
