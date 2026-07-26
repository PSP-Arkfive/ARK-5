/*
 *  dnas.c  -- Reverse engineering of iofilemgr_dnas.prx
 *               written by tpu.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pgd.h"

/*************************************************************/

u8 dnas_key1A90[16] = { 0xED, 0xE2, 0x5D, 0x2D, 0xBB, 0xF8, 0x12, 0xE5, 0x3C, 0x5C, 0x59, 0x32, 0xFA, 0xE3, 0xE2, 0x43 };
u8 dnas_key1AA0[16] = { 0x27, 0x74, 0xFB, 0xEB, 0xA4, 0xA0, 0x01, 0xD7, 0x02, 0x56, 0x9E, 0x33, 0x8C, 0x19, 0x57, 0x83 };

PGD_DESC g_pgd;
u8 kirk_buf[0x814];

int kirk7(u8 *buf, int size, int type)
{
	int retv;
	u32 *header = (u32*)buf;

	header[0] = 5;
	header[1] = 0;
	header[2] = 0;
	header[3] = type;
	header[4] = size;

	retv = sceUtilsBufferCopyWithRange(buf, size + 0x14, buf, size, 7);

	if (retv)
		return 0x80510311;

	return 0;
}

int bbmac_getkey(MAC_KEY *mkey, u8 *bbmac, u8 *vkey)
{
	int i, retv, type, code;
	u8 *kbuf, tmp[16], tmp1[16];

	type = mkey->type;
	retv = sceDrmBBMacFinal((u8 *)mkey, tmp, NULL);

	if (retv)
		return retv;

	kbuf = kirk_buf + 0x14;

	// decrypt bbmac
	if (type == 3) {
		memcpy(kbuf, bbmac, 0x10);
		kirk7(kirk_buf, 0x10, 0x63);
	} else
		memcpy(kirk_buf, bbmac, 0x10);

	memcpy(tmp1, kirk_buf, 16);
	memcpy(kbuf, tmp1, 16);

	code = (type == 2) ? 0x3A : 0x38;
	kirk7(kirk_buf, 0x10, code);

	for (i = 0; i < 0x10; i++)
		vkey[i] = tmp[i] ^ kirk_buf[i];

	return 0;
}

int get_version_key(u8 *version_key, char *path)
{
	MAC_KEY mkey;
	u8 header[256];
	u32 psar;
	int ret = 0;

	SceUID fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
	sceIoLseek32(fd, 0x24, PSP_SEEK_SET);
	sceIoRead(fd, &psar, 4);
	sceIoLseek32(fd, psar, PSP_SEEK_SET);
	sceIoRead(fd, header, sizeof(header));

	sceDrmBBMacInit((u8 *)&mkey, 3);
	sceDrmBBMacUpdate((u8 *)&mkey, header, 0xC0);
	bbmac_getkey(&mkey, header + 0xC0, version_key);

	sceDrmBBMacInit((u8 *)&mkey, 3);
	sceDrmBBMacUpdate((u8 *)&mkey, header, 0xC0);
	ret = sceDrmBBMacFinal2((u8 *)&mkey, header + 0xC0, version_key);

	return ret;
}

int get_edat_key(u8 *vkey, u8 *pgd_buf)
{
	int pgd_flag = 2;
	PGD_DESC *pgd = &g_pgd;
	MAC_KEY mkey;
	u8 *fkey = (u8 *)NULL;
	memset(pgd, 0, sizeof(PGD_DESC));

	pgd->buf = pgd_buf;
	pgd->key_index = *(u32*)(pgd_buf + 4);
	pgd->drm_type  = *(u32*)(pgd_buf + 8);

	if (pgd->drm_type == 1) {
		pgd->mac_type = 1;
		pgd_flag |= 4;

		if (pgd->key_index > 1) {
			pgd->mac_type = 3;
			pgd_flag |= 8;
		}
	} else
		pgd->mac_type = 2;

	pgd->open_flag = pgd_flag;

	if (pgd_flag & 2)
		fkey = dnas_key1A90;

	if (pgd_flag & 1)
		fkey = dnas_key1AA0;

	if (fkey == NULL)
		return -1;

	// MAC_0x80 check
	sceDrmBBMacInit((u8 *)&mkey, pgd->mac_type);
	sceDrmBBMacUpdate((u8 *)&mkey, pgd_buf, 0x80);
	if (sceDrmBBMacFinal2((u8 *)&mkey, pgd_buf + 0x80, fkey))
		return -2;

	// MAC_0x70
	sceDrmBBMacInit((u8 *)&mkey, pgd->mac_type);
	sceDrmBBMacUpdate((u8 *)&mkey, pgd_buf, 0x70);

	if (bbmac_getkey(&mkey, pgd_buf + 0x70, vkey))
		return -3;

	return 0;
}

int dumpPS1key(const char *path, u8 *buf)
{
	int ret = 0;
	int flag = 2;
	PGD_DESC PGD;
	memset(&PGD, 0, sizeof(PGD_DESC));
	MAC_KEY mkey;

	PGD.buf = buf;
	PGD.key_index = *(u32*)(buf + 4);
	PGD.drm_type = *(u32*)(buf + 8);

	// Set the hashing, crypto and open modes.
	if (PGD.drm_type == 1) {
		PGD.mac_type = 1;
		flag |= 4;

		if (PGD.key_index > 1) {
			PGD.mac_type = 3;
			flag |= 8;
		}
	} else
		PGD.mac_type = 2;

	PGD.open_flag = flag;

	sceDrmBBMacInit((u8 *)&mkey, PGD.mac_type);
	sceDrmBBMacUpdate((u8 *)&mkey, buf, 0x70);
	bbmac_getkey(&mkey, buf + 0x70, PGD.vkey);

	char Path[256];
	strcpy(Path, path);
	int len = strlen(Path);

	while (Path[len] != '/')
		len--;

	Path[len + 1] = 0;
	strcat(Path, "KEYS.BIN");

	SceUID fd = sceIoOpen(Path, PSP_O_RDONLY, 0777);

	if (fd >= 0)
		sceIoClose(fd);
	else {
		fd = sceIoOpen(Path, PSP_O_CREAT | PSP_O_WRONLY | PSP_O_TRUNC, 0777);

		if (fd >= 0) {
			ret = sceIoWrite(fd, PGD.vkey, 16);
			sceIoClose(fd);
		} else
			ret = fd;
	}

	return ret;
}

