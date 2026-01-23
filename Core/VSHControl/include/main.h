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

#ifndef MAIN_H
#define MAIN_H

#include <pspsdk.h>
#include <systemctrl_se.h>
#include <systemctrl_ark.h>

extern u32 psp_model;
extern u32 psp_fw_version;

extern SEConfig conf;
extern ARKConfig* ark_config;

#define ISO_RUNLEVEL 0x123
#define ISO_RUNLEVEL_GO 0x125
#define ISO_PBOOT_RUNLEVEL 0x124
#define ISO_PBOOT_RUNLEVEL_GO 0x126

int vshpatch_init(void);
int get_device_name(char *device, int size, const char* path);

#endif
