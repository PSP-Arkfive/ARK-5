#ifndef REGION_FREE_H
#define REGION_FREE_H

int sctrlArkReplaceUmdKeys();
void patch_umd_idslookup(SceModule* mod);
int patch_umd_thread(SceSize args, void *argp);
void patch_vsh_region_check(SceModule* mod);
void patch_sceChkreg();

#endif