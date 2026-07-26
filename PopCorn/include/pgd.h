#ifndef _PGD_H
#define _PGD_H

#include <pspkernel.h>

typedef struct {
	int type;
	u8 key[16];
	u8 pad[16];
	int pad_size;
} MAC_KEY;

typedef struct
{
	u32 type;
	u32 seed;
	u8 key[16];
} CIPHER_KEY;

typedef struct {
	u8  vkey[16];

	int open_flag;
	int key_index;
	int drm_type;
	int mac_type;
	int cipher_type;

	int data_size;
	int align_size;
	int block_size;
	int block_nr;
	int data_offset;
	int table_offset;

	u8 *buf;
} PGD_DESC;

int sceDrmBBMacInit(u8 *mac_key, int type);
int sceDrmBBMacUpdate(u8 *mac_key, u8 *buf, int size);
int sceAmctrl_driver_9227EA79(u8 *mac_key, u8 *buf, int size);
int sceDrmBBMacFinal(u8 *mac_key, u8 *buf, u8 *version_key);
int sceDrmBBMacFinal2(u8 *mac_key, u8 *buf, u8 *version_key);

int sceDrmBBCipherInit(u8 *cipher_key, int type, int mode, u8 *header_key, u8 *version_key, int seed);
int sceDrmBBCipherUpdate(u8 *cipher_key, u8 *buf, int size);
int sceAmctrl_driver_E04ADD4C(u8 *cipher_key, u8 *buf, int size);
int sceDrmBBCipherFinal(u8 *cipher_key);

int sceUtilsBufferCopyWithRange(u8 *outbuf, int outlen, u8 *inbuf, int inlen, int cmd);

int kirk7(u8 *buf, int size, int type);

int get_version_key(u8 *version_key, char *path);
int get_edat_key(u8 *vkey, u8 *pgd_buf);
int dumpPS1key(const char *path, u8 *buf);

#endif /* _PGD_H */
