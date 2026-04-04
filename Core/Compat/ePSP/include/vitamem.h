#ifndef VITAMEM_H
#define VITAMEM_H

extern void unprotectVitaMemory();

extern int (*_sctrlHENApplyMemory)(u32);
extern int memoryHandlerVita(u32 p2);

extern SceUID (*origAllocPartitionMemory)(int partition, char* name, int place, int size, void* addr);
extern SceUID extraAllocPartitionMemory(int partition, char* name, int place, int size, void* addr);

#endif