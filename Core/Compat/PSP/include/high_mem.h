#ifndef HIGHMEM_H
#define HIGHMEM_H

#include <pspsdk.h>

typedef struct _MemPart {
    u32 *meminfo;
    int offset;
    int size;
} MemPart;

extern int (*_sctrlHENApplyMemory)(u32);

int prevent_highmem();
int prepatch_partitions();
int patch_partitions(u32 p2_size);
int memoryHandlerPSP(u32 p2);

#endif