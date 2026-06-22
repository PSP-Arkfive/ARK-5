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
#include <pspsdk.h>
#include <psploadcore.h>
#include <pspiofilemgr.h>

#include <cfwmacros.h>

#include <systemctrl.h>

static char *g_blacklist[] = {
    "iso",
    "seplugins",
    "isocache.bin",
    "irshell",
	"cso",
	"dax",
	"pbp",
	"ini",
};

static inline int is_in_blacklist(const char *dname)
{
    // lower string
    char temp[255]; memset(temp, 0, sizeof(temp));
    strncpy(temp, dname, sizeof(temp));

    extern int lowerString(char*, char*, int);
    lowerString(temp, temp, strlen(temp)+1);

    for (int i=0; i<NELEMS(g_blacklist); ++i) {
        if (strstr(temp, g_blacklist[i])) {
        	return 1;
        }
    }

    return 0;
}

int hideIoDread(SceUID fd, SceIoDirent * dir)
{
    int k1 = pspSdkSetK1(0);
    int result = sceIoDread(fd, dir);

    if(result >= 0 && is_in_blacklist(dir->d_name)) {
		memset(dir, 0, sizeof(SceIoDirent));
		if (result == 0) {
			return 0x80020142;
		}
        result = hideIoDread(fd, dir);
    }

    pspSdkSetK1(k1);
    return result;
}

int hideIoGetstat(const char *file, SceIoStat *stat)
{
    int k1 = pspSdkSetK1(0);
    int result = sceIoGetstat(file, stat);

    if(result > 0 && is_in_blacklist(file)) {
        result = 0x80020142;
    }

    pspSdkSetK1(k1);
    return result;
}

SceUID hideIoOpen(const char *file, int flags, SceMode mode)
{
    int k1 = pspSdkSetK1(0);
    int result = sceIoOpen(file, flags, mode);

    if(result > 0 && is_in_blacklist(file)) {
        result = 0x80020142;
    }

    pspSdkSetK1(k1);
    return result;
}

SceUID hideIoDopen(const char *dirname)
{
    int k1 = pspSdkSetK1(0);
    int result = sceIoDopen(dirname);

    if(result > 0 && is_in_blacklist(dirname)) {
        result = 0x80020142;
    }

    pspSdkSetK1(k1);
    return result;
}

// hide cfw folders to fix djmax
void hide_cfw_folder(SceModule * mod)
{
    // hide dread
    sctrlHookImportByNID(mod, "IoFileMgrForUser", 0xE3EB004C, &hideIoDread);
	// hide dopen
	sctrlHookImportByNID(mod, "IoFileMgrForUser", 0xB29DDF9C, &hideIoDopen);
	// hide open
	sctrlHookImportByNID(mod, "IoFileMgrForUser", 0x109F50BC, &hideIoOpen);
	// hide getstat
	sctrlHookImportByNID(mod, "IoFileMgrForUser", 0xACE946E8, &hideIoGetstat);
}
