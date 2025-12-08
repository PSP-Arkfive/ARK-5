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

#include <string.h>
#include <stdio.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <pspthreadman_kernel.h>
#include <pspdebug.h>
#include <pspinit.h>

#include <ark.h>
#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_private.h>

struct DeviceSize {
    u32 maxClusters;
    u32 freeClusters;
    u32 maxSectors;
    u32 sectorSize;
    u32 sectorCount;
};

// this fixes old games that report "not enough space" when more than 2GB are available
static u32 (*_sceIoDevctl)(const char *name, int cmd, u32 argAddr, int argLen, u32 outPtr, int outLen);
static u32 myIoDevctl(const char *name, int cmd, u32 argAddr, int argLen, u32 outPtr, int outLen){
    u32 res = _sceIoDevctl(name, cmd, argAddr, argLen, outPtr, outLen);

    if (cmd == 0x02425818 && res >= 0){

        struct DeviceSize* deviceSize = *(struct DeviceSize**)argAddr;

        u32 sectorSize = deviceSize->sectorSize;
        if (sectorSize){
            u32 memStickSectorSize = 32 * 1024;
            u32 sectorCount = memStickSectorSize / sectorSize;
            u32 freeSize = 1900 * 1024 * 1024; // pretend to have 1GB, enough for any game
            u32 clusterSize = sectorSize * sectorCount;
            if (clusterSize){
                u32 maxClusters = (u32)(freeSize / clusterSize);
                if (deviceSize->freeClusters > maxClusters){
                    deviceSize->maxClusters = maxClusters;
                    deviceSize->freeClusters = maxClusters;
                    deviceSize->maxSectors = maxClusters;
                    deviceSize->sectorCount = sectorCount;
                }
            }
        }
    }

    return res;
}

void patch_ioDevCtl(){
    u32 io_ctrl = sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x54F5FB11);
    HIJACK_FUNCTION(io_ctrl, myIoDevctl, _sceIoDevctl);
}