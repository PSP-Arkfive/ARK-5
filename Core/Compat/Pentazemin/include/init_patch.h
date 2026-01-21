#ifndef INIT_PATCH_H
#define INIT_PATCH_H

#include <pspsdk.h>



SceModule* patchLoaderCore(void);
int AdrenalinePatchInit(int (* module_bootstart)(SceSize, void *), void *argp);

#endif