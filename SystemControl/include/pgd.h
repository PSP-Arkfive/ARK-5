#ifndef _PGD_H
#define _PGD_H

#include <pspkernel.h>
#include <pspamctrl.h>

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

int sceUtilsBufferCopyWithRange(u8 *outbuf, int outlen, u8 *inbuf, int inlen, int cmd);

int kirk7(u8 *buf, int size, int type);
int nodrmGetVersionKey(unsigned char * key, char *path);
int nodrmGetEdatKey(u8 *vkey, u8 *pgd_buf);
int nodrmBBMacGetKey(SceMacKey *mkey, u8 *bbmac, u8 *vkey);

#endif /* _PGD_H */
