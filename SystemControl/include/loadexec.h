#ifndef LOADEXEC_PATCH_H
#define LOADEXEC_PATCH_H

#include <psploadcore.h>

extern void patchLoadExec(SceModule* loadexec, u32 LoadReboot, u32 GetUserLevel, int k1_checks);

#endif
