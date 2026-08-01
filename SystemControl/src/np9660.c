/*
 *  Copyright (C) 2014-2018 qwikrazor87
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <systemctrl.h>
#include <pspiofilemgr.h>
#include <pspinit.h>
#include <stdio.h>
#include <string.h>
#include <cfwmacros.h>
#include "pgd.h"
#include "np9660.h"

char ebootpath[256], g_pgd_path[256];
u8 pgdbuf[0x90], g_eboot_key[16];
SceModule *npmod;
int licensed_eboot = 0, g_is_key = 0;

int is_licensed_eboot(const char *path)
{
	int ret = 0;
	u8 buf[0x28];
	SceUID fd = sceIoOpen(path, PSP_O_RDONLY, 0);
	sceIoRead(fd, buf, sizeof(buf));

	if (!memcmp(buf, "\x00PBP", 4)) {
		sceIoLseek(fd, *(u32*)(buf + 0x24), 0);
		sceIoRead(fd, buf, 0xC);

		if (!memcmp(buf, "NPUMDIMG", 8))
			ret = memcmp(buf + 8, "\x03\x00\x00\x01", 4); //disable patch if fixed key version is detected.
	}

	sceIoClose(fd);

	return ret;
}

int (* setup_eboot_version_key)(u8 *vkey, u8 *cid, u32 type, u8 *act);
int setup_eboot_version_key_hook(u8 *vkey, u8 *cid, u32 type, u8 *act)
{
	if (g_is_key) { //prevent SceNpUmdMount thread from crashing during suspend/resume process if rif/act.dat fails.
		memcpy(vkey, g_eboot_key, 16); //copy generated key from first call since there is only one version key per eboot.
		return 0;
	}

	int ret = setup_eboot_version_key(vkey, cid, type, act);

	if (ret < 0) //generate key from mac if official method fails.
		ret = get_version_key(vkey, ebootpath);

	if (ret >= 0) {
		memcpy(g_eboot_key, vkey, 16); //backup eboot vkey for later calls.
		g_is_key = 1;
	}

	return ret;
}

void patch_drm()
{
	u32 addr, data;

	for (addr = npmod->text_addr; addr < (npmod->text_addr + npmod->text_size); addr += 4) {
		data = _lw(addr);

		if (data == 0x3C118021) { //lui        $s1, 0x8021
			_sw(0x1021, addr - 4); //move $v0, $zr //patch memcmp, ensures the kernel continues to decrypt the eboot.
		} else if (data == 0x3C098055) { //lui        $t1, 0x8055
			HIJACK_FUNCTION(addr - 4, setup_eboot_version_key_hook, setup_eboot_version_key);
			break;
		}
	}

	sctrlFlushCache();
}

int (* init_eboot)(const char *eboot, u32 a1) = (void *)NULL;
int init_eboot_hook(const char *eboot, u32 a1)
{
	if ((licensed_eboot = is_licensed_eboot(eboot))) {
		strcpy(ebootpath, eboot);
		patch_drm();
	}

	return init_eboot(eboot, a1);
}

void patch_np9660(SceModule *mod)
{
	u32 addr, jalcount = 0, data;
	npmod = mod;

	//prevent from patching again, should fix suspend issue in most cases.
	if (init_eboot)
		return;

	for (addr = mod->text_addr; addr < (mod->text_addr + mod->text_size); addr += 4) {
		data = _lw(addr);

		if (((data & 0xFC000000) == 0x0C000000) && (jalcount < 2))
			jalcount++;

		if (jalcount == 2) {
			init_eboot = (void *)(((data & 0x03FFFFFF) << 2) | 0x80000000);
            MAKE_CALL(addr, init_eboot_hook);
			break;
		}
	}

	sctrlFlushCache();
}
