#include <stdio.h>
#include <string.h>
#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <pspsysmem_kernel.h>

#include <systemctrl_ark.h>
#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>


extern SEConfigARK* se_config;

static PspSysMemPartition *(* GetPartition)(int partition) = NULL;
static u32 findGetPartition(){
    for (u32 addr = SYSMEM_TEXT; ; addr+=4){
        if (_lw(addr) == 0x2C85000D){
            return addr-4;
        }
    }
    return 0;
}

// modify extra ram partitions to be user-allocateable
void unprotectVitaMemory(){
    if (GetPartition == NULL) GetPartition = (void*)findGetPartition();
    for (int i=8; i<12; i++){
        PspSysMemPartition* partition = (void*)GetPartition(i);
        if (partition){
            partition->address &= 0x7FFFFFFF;
        }
    }
}

int unlockVitaMemory(u32 user_size_mib){

    int apitype = sceKernelInitApitype(); // prevent in pops and vsh
    if (apitype == 0x144 || apitype == 0x155 || apitype >= 0x200)
        return -1;

    if (GetPartition == NULL) GetPartition = (void*)findGetPartition();

    PspSysMemPartition *partition;
    u32 user_size = user_size_mib * 1024 * 1024; // new p2 size

    // modify p2
    partition = GetPartition(PSP_MEMORY_PARTITION_USER);
    partition->size = user_size;
    partition->data->size = (((user_size >> 8) << 9) | 0xFC);

    // modify p11
    for (int i=8; i<12; i++){
        partition = GetPartition(i);
        if (partition){
            partition->size = 0;
            partition->address = 0x88800000 + user_size;
            partition->data->size = (((partition->size >> 8) << 9) | 0xFC);
        }
    }

    return 0;
}

int (*_sctrlHENApplyMemory)(u32) = NULL;
int memoryHandlerVita(u32 p2){
    // sanity checks
    if (p2<=24) return -1;

    // the first 16MB are stable and good enough for most use cases
    // but homebrew that require extra ram will be allowed to use (some of) the upper 16MB
    if (p2 > 52){
        p2 = (se_config->force_high_memory == 2)? 52 : 40;
    }

    // call orig function to determine if can unlock
    int res = _sctrlHENApplyMemory(p2);
    if (res<0) return res;

    // unlock
    res = unlockVitaMemory(p2);
    
    // unlock fail? revert back to 24MB
    if (res<0) _sctrlHENApplyMemory(24);

    return res;
}

// This patch forces user plugins to allocate on extra ram
SceUID (*origAllocPartitionMemory)(int partition, char* name, int place, int size, void* addr) = NULL;
SceUID extraAllocPartitionMemory(int partition, char* name, int place, int size, void* addr){
    if (!se_config->force_high_memory && partition == 2 && addr == NULL && sctrlIsLoadingPlugins()){
        partition = 11;
    }
    SceUID res = origAllocPartitionMemory(partition, name, place, size, addr);

    // if memory has been allocated into p11, disable growing of p2
    if (partition == 11 && res >= 0){
        MAKE_DUMMY_FUNCTION(memoryHandlerVita, 0xFFFF); // make it return -1
    }

    return res;
}
