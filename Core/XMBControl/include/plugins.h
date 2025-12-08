#ifndef PLUGINS_H
#define PLUGINS_H

#include <pspsdk.h>
#include <pspkernel.h>
#include <psputility_sysparam.h>
#include <systemctrl.h>
#include <kubridge.h>
#include <stddef.h>

enum{
    PLACE_MS0,
    PLACE_EF0,
    PLACE_ARK_PATH
};

enum{
    PLUGIN_OFF,
    PLUGIN_ON,
    PLUGIN_REMOVED,
};

typedef struct{
    char* name;
    char* surname;
    char* path;
    char* runlevel;
    int active;
    int place;
} Plugin;

extern char* plugins_paths[];

extern List plugins;
extern List iplugins;

void loadPlugins();
void savePlugins();
void installPlugin(Plugin* plugin, char* opt);
void findInstallablePlugins();
char* getPluginName(char* plugin_path, char* file);
void plugin_list_cleaner(void* item);

#endif
