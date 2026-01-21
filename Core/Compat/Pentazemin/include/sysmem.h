#ifndef SYSMEM_H
#define SYSMEM_H

#include <pspsdk.h>

typedef struct {
    u32 addr;
    u32 size;
} SceSysmemPartInfo;

typedef struct {
    u32 memSize;
    u32 unk4;
    u32 unk8;
    SceSysmemPartInfo other1; // 12
    SceSysmemPartInfo other2; // 20
    SceSysmemPartInfo vshell; // 28
    SceSysmemPartInfo scUser; // 36
    SceSysmemPartInfo meUser; // 44
    SceSysmemPartInfo extSc2Kernel; // 52
    SceSysmemPartInfo extScKernel; // 60
    SceSysmemPartInfo extMeKernel; // 68
    SceSysmemPartInfo extVshell; // 76
} SceSysmemPartTable;

typedef struct PartitionData {
    u32 unk[5];
    u32 size;
} PartitionData;

typedef struct SysMemPartition {
    struct SysMemPartition *next;
    u32    address;
    u32 size;
    u32 attributes;
    PartitionData *data;
} SysMemPartition;

#endif