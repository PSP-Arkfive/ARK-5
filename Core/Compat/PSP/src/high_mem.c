/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <stdio.h>
#include <string.h>
#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>

#include <systemctrl_ark.h>
#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>

#include "high_mem.h"

extern u32 psp_model;
extern SEConfigARK* se_config;

static u8 g_p8_size = 4;

static void modify_partition(MemPart *part)
{
    PspSysMemPartition *meminfo;
    u32 offset, size;

    meminfo = part->meminfo;
    offset = part->offset;
    size = part->size;

    if(meminfo == NULL) {
        return;
    }

    if (offset != 0) {
        meminfo->address = (offset << 20) + 0x88800000;
    }

    meminfo->size = size << 20;
    meminfo->data->size = (size << 21) | 0xFC;
}

int prevent_highmem(){
    if (psp_model == PSP_1000) return 1;
    int apitype = sceKernelInitApitype();
    return (apitype == 0x144 || apitype == 0x155 || apitype > 0x200);
}

int prepatch_partitions()
{
    if(prevent_highmem()){
        return -1;
    }

    MemPart p8, p11;

    p8.meminfo = sctrlGetMemoryPartition(8);
    p11.meminfo = sctrlGetMemoryPartition(11);

    g_p8_size = p8.size = 1;
    
    if(p11.meminfo != NULL) {
        p8.offset = 56-4-p8.size;
    } else {
        p8.offset = 56-p8.size;
    }

    modify_partition(&p8);

    p11.size = 4;
    p11.offset = 56-4;
    modify_partition(&p11);

    return 0;
}

int patch_partitions(u32 p2_size) 
{

    if(prevent_highmem()){
        return -1;
    }

    MemPart p2, p9;
    int max_user_part_size;

    p2.meminfo = sctrlGetMemoryPartition(2);
    p9.meminfo = sctrlGetMemoryPartition(9);

    p2.size = p2_size;
    p9.size = 0;

    if(sctrlGetMemoryPartition(11) != NULL) {
        max_user_part_size = 56 - 4 - g_p8_size;
    } else {
        max_user_part_size = 56 - g_p8_size;
    }

    if (p2.size + p9.size > max_user_part_size) {
        // reserved 4MB for P11
        int reserved_len;

        reserved_len = p2.size + p9.size - max_user_part_size;

        if(p9.size > reserved_len) {
            p9.size -= reserved_len;
        } else {
            p2.size -= reserved_len - p9.size; 
            p9.size = 0;
        }
    }

    p2.offset = 0;
    modify_partition(&p2);

    p9.offset = p2.size;
    modify_partition(&p9);

    return 0;
}

int (*_sctrlHENApplyMemory)(u32) = NULL;
int memoryHandlerPSP(u32 p2){
    
    // sanity checks
    if (p2<=24) return -1;

    if (p2 > MAX_HIGH_MEMSIZE) p2 = MAX_HIGH_MEMSIZE;

    // call orig function to determine if can unlock
    int res = _sctrlHENApplyMemory(p2);
    if (res<0) return res;

    // unlock
    res = patch_partitions(p2);

    // unlock fail? revert back to 24MB
    if (res<0) _sctrlHENApplyMemory(24);

    return res;
}

// This patch forces user plugins to allocate on extra ram
SceUID (*origAllocPartitionMemory)(int partition, char* name, int place, int size, void* addr) = NULL;
SceUID extraAllocPartitionMemory(int partition, char* name, int place, int size, void* addr){
    if (partition == 2 && addr == NULL && // redirect p2 allocations
        !se_config->force_high_memory && // don't redirect if extra ram is forced onto p2
        sctrlIsLoadingPlugins() && // only when loading plugins
        sceKernelInitApitype() < 0x200 && // not in VSH
        (psp_model != PSP_GO || se_config->disable_pause) // disable on PSP Go with pause feature
    ){
        partition = 9;
    }
    SceUID res = origAllocPartitionMemory(partition, name, place, size, addr);

    // if memory has been allocated into p9, disable growing of p2
    if (partition == 9 && res >= 0){
        MAKE_DUMMY_FUNCTION(memoryHandlerPSP, 0xFFFF); // make it return -1
    }

    return res;
}
