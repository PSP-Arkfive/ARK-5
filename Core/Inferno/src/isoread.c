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
#include <pspkernel.h>
#include <pspreg.h>
#include <pspsysmem_kernel.h>
#include <psprtc.h>
#include <psputilsforkernel.h>
#include <pspthreadman_kernel.h>

#include <ark.h>
#include <ciso.h>
#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_private.h>

#include "inferno.h"


// 0x00002784
struct IoReadArg g_read_arg;

static int read_raw_data(void* arg, void* addr, u32 size, u32 offset);

// 0x0000248C
int g_iso_opened = 0;

// 0x000023D4
int g_total_sectors = -1;

// 0x000023D0
SceUID g_iso_fd = -1;

// ciso data
CisoFile g_ciso_file = {
    .read_data = &read_raw_data,
    .memalign = &oe_memalign,
    .free = &oe_free,
};

// reader functions
static int is_compressed = 0;

// UMD delay
unsigned char umd_seek = 0;
unsigned char umd_speed = 0;
u32 cur_offset = 0;
u32 last_read_offset = 0;


// for libcisoread
int zlib_inflate(void* dst, int dst_len, void* src){
    return sceKernelDeflateDecompress(dst, dst_len, src, NULL);
}

// 0x00000368
static void wait_until_ms0_ready(void)
{
    int ret, status = 0;

    if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_VSH) return; // no wait on VSH

    const char *drvname = (
        (g_iso_fn[0] == 'm' || g_iso_fn[0] == 'M') &&
        (g_iso_fn[1] == 's' || g_iso_fn[1] == 'S')
    )?  "mscmhc0:" : "mscmhcemu0:";

    while (1) {
        ret = sceIoDevctl(drvname, 0x02025801, 0, 0, &status, sizeof(status));

        if (ret<0){
            sceKernelDelayThread(20000);
            continue;
        }

        if(status == 4) {
            break;
        }

        sceKernelDelayThread(20000);
    }
}

#ifdef DEBUG
static int io_calls = 0;
#endif

// 0x00000BB4
static int read_raw_data(void* arg, void* addr, u32 size, u32 offset)
{
    int ret, i;
    SceOff ofs;
    i = 0;

    #ifdef DEBUG
    io_calls++;
    #endif
    
    do {
        i++;
        ofs = sceIoLseek(g_iso_fd, offset, PSP_SEEK_SET);

        if(ofs >= 0) {
            i = 0;
            break;
        } else {
            #ifdef DEBUG
            printk("%s: lseek retry %d error 0x%08X\n", __func__, i, (int)ofs);
            #endif
            iso_open();
        }
    } while(i < 16);

    if(i == 16) {
        ret = 0x80010013;
        goto exit;
    }

    for(i=0; i<16; ++i) {
        ret = sceIoRead(g_iso_fd, addr, size);

        if(ret >= 0) {
            i = 0;
            break;
        } else {
            #ifdef DEBUG
            printk("%s: read retry %d error 0x%08X\n", __func__, i, ret);
            #endif
            iso_open();
        }
    }

    if(i == 16) {
        ret = 0x80010013;
        goto exit;
    }

exit:
    return ret;
}

// 0x000009D4
int iso_open(void)
{
    if (g_iso_fn[0] == 0) return -1;

    wait_until_ms0_ready();
    sceIoClose(g_iso_fd);
    g_iso_opened = 0;

    int retries = 0;
    do {
        g_iso_fd = sceIoOpen(g_iso_fn, 0x000F0001, 0777);

        if(g_iso_fd < 0) {
            if(++retries >= 16) {
                return -1;
            }

            sceKernelDelayThread(20000);
        }
    } while(g_iso_fd < 0);

    if(g_iso_fd < 0) {
        return -1;
    }

    g_ciso_file.reader_arg = (void*)g_iso_fd;

    is_compressed = ciso_open(&g_ciso_file);

    if (is_compressed < 0) return is_compressed;
    // total number of DVD sectors (2K) in the original ISO.
    else if (is_compressed > 0){
        g_total_sectors = g_ciso_file.uncompressed_size / ISO_SECTOR_SIZE;
    }
    else {
        SceOff off = sceIoLseek(g_iso_fd, 0, PSP_SEEK_CUR);
        SceOff total = sceIoLseek(g_iso_fd, 0, PSP_SEEK_END);
        sceIoLseek(g_iso_fd, off, PSP_SEEK_SET);
        g_total_sectors = total / ISO_SECTOR_SIZE;
    }

    g_iso_opened = 1;

    return 0;
}

// 0x00000C7C
int iso_read(struct IoReadArg *args)
{
    if (is_compressed)
        return ciso_read(&g_ciso_file, args->address, args->size, args->offset);
    return read_raw_data(NULL, args->address, args->size, args->offset);
}

// 0x000003E0
int (*iso_reader)(struct IoReadArg *args) = &iso_read;
int iso_read_with_stack(u32 offset, void *ptr, u32 data_len)
{
    int ret, retv;

    ret = sceKernelWaitSema(g_umd9660_sema_id, 1, 0);

    if(ret < 0) {
        return -1;
    }

    g_read_arg.offset = offset;
    g_read_arg.address = ptr;
    g_read_arg.size = data_len;

    retv = sceKernelExtendKernelStack(0x2000, (void*)iso_reader, &g_read_arg);

    ret = sceKernelSignalSema(g_umd9660_sema_id, 1);

    if(ret < 0) {
        return -1;
    }

    if (umd_seek){
        // simulate seek time
        u32 diff = 0;
        last_read_offset = offset+data_len;
        if (cur_offset>last_read_offset) diff = cur_offset-last_read_offset;
        else diff = last_read_offset-cur_offset;
        cur_offset = last_read_offset;
        u32 seek_time = (diff*umd_seek)/1024;
        sceKernelDelayThread(seek_time);
    }
    if (umd_speed){
        // simulate read time
        sceKernelDelayThread(data_len*umd_speed);
    }

    return retv;
}

void infernoSetUmdDelay(int seek, int speed){
    umd_seek = seek;
    umd_speed = speed;
}