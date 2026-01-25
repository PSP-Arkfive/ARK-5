/*
    6.39 TN-A, XmbControl
    Copyright (C) 2011, Total_Noob
    Copyright (C) 2011, Frostegater

    main.h: XmbControl main header file
    
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

#ifndef __MAIN_H__
#define __MAIN_H__

#include <systemctrl_ark.h>
#include <rebootexconfig.h>
#include <systemctrl.h>
#include <systemctrl_se.h>

#define STAR "★"
#define WSTAR L"★"

#define sysconf_console_id 4
#define sysconf_console_action 2
#define sysconf_console_action_arg 2

enum {
    sysconf_tnconfig_action_arg = 0x1000,
    sysconf_plugins_action_arg = 0x1002,
    sysconf_custom_launcher_arg = 0x1003,
    sysconf_custom_app_arg = 0x1004,
    sysconf_150_reboot_arg = 0x1005,
};

typedef struct
{
    u32 magic;
    u8 sysopt;
    u8 usbdevice;
    u8 usbreadonly;
    u8 usbcharge;
    u8 clock_game;
    u8 clock_vsh;
    u8 wpa2;
    u8 launcher;
    u8 highmem;
    u8 mscache;
    u8 infernocache;
    u8 disablepause;
    u8 oldplugin;
    u8 hibblock;
    u8 skiplogos;
    u8 hidepics;
    u8 hidemac;
    u8 hidedlc;
    u8 noled;
    u8 noumd;
    u8 noanalog;
    u8 umdregion;
    u8 vshregion;
    u8 confirmbtn;
    u8 qaflags;
    u8 import_plugins;
    u8 activate_codecs;
    u8 convert_battery;
    u8 delete_go_pause;
    u8 reset_settings;
} CFWConfig;

// TODO: send some of these to pspsdk
typedef struct
{
    char text[48];
    int play_sound;
    int action;
    int action_arg;
} SceContextItem;

typedef struct
{
    int id;
    int relocate;
    int action;
    int action_arg;
    SceContextItem *context;
    char *subtitle;
    int unk;
    char play_sound;
    char memstick;
    char umd_icon;
    char image[4];
    char image_shadow[4];
    char image_glow[4];
    char text[0x25];
} SceVshItem;

typedef struct
{
    void *unk;
    int id;
    char *regkey;
    char *text;
    char *subtitle;
    char *page;
} SceSysconfItem;

typedef struct
{
    u8 id;
    u8 type;
    u16 unk1;
    u32 label;
    u32 param;
    u32 first_child;
    int child_count;
    u32 next_entry;
    u32 prev_entry;
    u32 parent;
    u32 unknown[2];
} SceRcoEntry;

extern int psp_model;
extern ARKConfig ark_config;
extern SEConfigARK se_config;
extern RebootexConfigARK rebootex_config;
extern STMOD_HANDLER previous;

int vshcube_init();
int vshcube_start();
int vshcube_stop();
void findAllTranslatableStrings();
int OnModuleStart(SceModule *mod);

int sce_paf_private_wcslen(wchar_t *);
int sce_paf_private_wcscpy(wchar_t* str1, wchar_t* str2);
int sce_paf_private_wcsprintf(wchar_t* string, int len, wchar_t* buf, ...);
int sce_paf_private_sprintf(char *, const char *, ...);
int sce_paf_private_snprintf(char *, int, const char *, ...);
void *sce_paf_private_memcpy(void *, void *, int);
void *sce_paf_private_memset(void *, char, int);
int sce_paf_private_strlen(char *);
char *sce_paf_private_strcpy(char *, const char *);
char *sce_paf_private_strncpy(char *, const char *, int);
int sce_paf_private_strcmp(const char *, const char *);
int sce_paf_private_strncmp(const char *, const char *, int);
char *sce_paf_private_strchr(const char *, int);
char *sce_paf_private_strrchr(const char *, int);
int sce_paf_private_strpbrk(const char *, const char *);
int sce_paf_private_strtoul(const char *, char **, int);
void *sce_paf_private_malloc(int);
void sce_paf_private_free(void *);

wchar_t *scePafGetText(void *, char *);
int PAF_Resource_GetPageNodeByID(void *, char *, SceRcoEntry **);
int PAF_Resource_ResolveRefWString(void *, u32 *, int *, char **, int *);

int vshGetRegistryValue(u32 *, char *, void *, int , int *);
int vshSetRegistryValue(u32 *, char *, int , int *);

int sceVshCommonGuiBottomDialog(void *a0, void *a1, void *a2, int (* cancel_handler)(), void *t0, void *t1, int (* handler)(), void *t3);

void patchVshClock(u32 addr);

#endif
