#ifndef HIGHMEM_H
#define HIGHMEM_H

#include <pspsdk.h>

typedef struct _MemPart {
    u32 *meminfo;
    int offset;
    int size;
} MemPart;

extern int (*_sctrlHENApplyMemory)(u32);
extern int (*prevHandlerPlugin)(const char* path, SceUID* modid);

int prevent_highmem();
int prepatch_partitions();
int patch_partitions(u32 p2_size);
int memoryHandlerPSP(u32 p2);
int memoryHandlerPlugin(const char* path, SceUID* modid);

extern SceUID (*origAllocPartitionMemory)(int partition, char* name, int place, int size, void* addr);
SceUID extraAllocPartitionMemory(int partition, char* name, int place, int size, void* addr);

#endif