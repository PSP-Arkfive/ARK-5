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

#ifndef _SEPLUGINS_H_
#define _SEPLUGINS_H_

extern int pluginsLoaded;
extern int settingsLoaded;
extern int disable_plugins;
extern int disable_settings;
extern int is_plugins_loading;

void loadPlugins();
void loadSettings();

int processConfigFile(
    const char* parent,
    const char* path,
    void (*enabler)(const char*),
    void (*disabler)(const char*)
);

#endif

