#ifndef FUNCS_H
#define FUNCS_H

#include <pspsdk.h>

int sceKernelLoadModuleBufferBootInitBtcnf661(int, void*, int, void*);

int sceKermitRegisterVirtualIntrHandler661(u32, void*);

void* sceKernelGetGameInfo661();

int sceKermitSendRequest661(void*, int, int, int, int, void*);

#endif