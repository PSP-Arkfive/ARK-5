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

#include "isoreader.h"
#include <string.h>
#include <pspiofilemgr.h>
#include <psputilsforkernel.h>
#include <pspsysmem_kernel.h>

#include <ciso.h>
#include <cfwmacros.h>
#include <vshctrl.h>
#include <systemctrl.h>
#include <systemctrl_se.h>


#define MAX_RETRIES 8
#define MAX_DIR_LEVEL 8
#define ISO_STANDARD_ID "CD001"



static int read_raw_data(void* arg, void* addr, u32 size, u32 offset);

// Ciso File Handler
CisoFile g_ciso_file = {
    .read_data = &read_raw_data,
    .memalign = &user_memalign,
    .free = &oe_free,
};

// file vars
static char* g_sector_buffer = NULL; // ISO sector
static const char * g_filename = NULL;
static SceUID g_isofd = -1;
static u32 g_total_sectors = 0;
static int is_compressed = 0;

static Iso9660DirectoryRecord g_root_record;

// for libcisoread
int zlib_inflate(void* dst, int dst_len, void* src){
    return sceKernelDeflateDecompress(dst, dst_len, src, NULL);
}

static void isoAlloc(){
    g_sector_buffer = user_malloc(SECTOR_SIZE);
}

static void isoFree(){
    if (g_sector_buffer) oe_free(g_sector_buffer);
    g_sector_buffer = NULL;
    ciso_close(&g_ciso_file);
}

static inline u32 isoPos2LBA(u32 pos)
{
    return pos / SECTOR_SIZE;
}

static inline u32 isoLBA2Pos(u32 lba, int offset)
{
    return lba * SECTOR_SIZE + offset;
}

static inline u32 isoPos2OffsetInSector(u32 pos)
{
    return pos & (SECTOR_SIZE - 1);
}

static inline u32 isoPos2RestSize(u32 pos)
{
    return SECTOR_SIZE - isoPos2OffsetInSector(pos);
}

static int reOpen(void)
{
    int retries = MAX_RETRIES, fd = -1;

    sceIoClose(g_isofd);

    while(retries -- > 0) {
        fd = sceIoOpen(g_filename, PSP_O_RDONLY, 0777);

        if (fd >= 0) {
            break;
        }

        sceKernelDelayThread(100000);
    }

    if (fd >= 0) {
        g_isofd = fd;
    }

    return fd;
}

static int read_raw_data(void* arg, void* addr, u32 size, u32 offset)
{
    int ret, i;
    SceOff ofs;

    for(i=0; i<MAX_RETRIES; ++i) {
        ofs = sceIoLseek(g_isofd, offset, PSP_SEEK_SET);

        if (ofs >= 0) {
            break;
        } else {
            #ifdef DEBUG
            printk("%s: sceIoLseek -> 0x%08X\n", __func__, (int)ofs);
            #endif
            reOpen();
        }

        sceKernelDelayThread(100000);
    }

    for(i=0; i<MAX_RETRIES; ++i) {
        ret = sceIoRead(g_isofd, addr, size);

        if(ret >= 0) {
            break;
        } else {
            #ifdef DEBUG
            printk("%s: sceIoRead -> 0x%08X\n", __func__, ret);
            #endif
            reOpen();
            sceIoLseek(g_isofd, offset, PSP_SEEK_SET);
        }

        sceKernelDelayThread(100000);
    }

    return ret;
}

static int readSector(u32 sector, u8* buf){
    if (is_compressed){
        return ciso_read(&g_ciso_file, buf, SECTOR_SIZE, sector*SECTOR_SIZE);
    }
    return read_raw_data(NULL, buf, SECTOR_SIZE, sector*SECTOR_SIZE);
}

static void normalizeName(char *filename)
{
    char *p;
   
    p = strstr(filename, ";1");

    if (p) {
        *p = '\0';
    }
}

static int findFile(const char * file, u32 lba, u32 dir_size, u32 is_dir, Iso9660DirectoryRecord *result_record)
{
    u32 pos;
    int ret;
    Iso9660DirectoryRecord *rec;
    char name[32];
    int re;

    pos = isoLBA2Pos(lba, 0);
    re = lba = 0;

    while ( re < dir_size ) {
        if (isoPos2LBA(pos) != lba) {
            lba = isoPos2LBA(pos);
            ret = readSector(lba, (u8*)g_sector_buffer);

            if (ret != SECTOR_SIZE) {
                return -23;
            }
        }

        rec = (Iso9660DirectoryRecord*)&g_sector_buffer[isoPos2OffsetInSector(pos)];

        if(rec->len_dr == 0) {
            u32 remaining;

            remaining = isoPos2RestSize(pos);
            pos += remaining;
            re += remaining;
            continue;
        }
        
        #ifdef DEBUG
        if(rec->len_dr < rec->len_fi + sizeof(*rec)) {
            printk("%s: Corrupted directory record found in %s, LBA %d\n", __func__, g_filename, (int)lba);
        }
        #endif

        if(rec->len_fi > 32) {
            return -11;
        }

        if(rec->len_fi == 1 && rec->fi == 0) {
            if (0 == strcmp(file, ".")) {
                memcpy(result_record, rec, sizeof(*result_record));

                return 0;
            }
        } else if(rec->len_fi == 1 && rec->fi == 1) {
            if (0 == strcmp(file, "..")) {
                // didn't support ..
                return -19;
            }
        } else {
            memset(name, 0, sizeof(name));
            memcpy(name, &rec->fi, rec->len_fi);
            normalizeName(name);

            if (0 == strcmp(name, file)) {
                if (is_dir) {
                    if(!(rec->fileFlags & ISO9660_FILEFLAGS_DIR)) {
                        return -14;
                    }
                }

                memcpy(result_record, rec, sizeof(*result_record));

                return 0;
            }
        }

        pos += rec->len_dr;
        re += rec->len_dr;
    }

    return -18;
}

static int findPath(const char *path, Iso9660DirectoryRecord *result_record)
{
    int level = 0, ret;
    const char *cur_path, *next;
    u32 lba, dir_size;
    char cur_dir[32];

    if (result_record == NULL) {
        return -17;
    }

    memset(result_record, 0, sizeof(*result_record));
    lba = g_root_record.lsbStart;
    dir_size = g_root_record.lsbDataLength;

    cur_path = path;

    while(*cur_path == '/') {
        cur_path++;
    }

    next = strchr(cur_path, '/');

    while (next != NULL) {
        if (next-cur_path >= sizeof(cur_dir)) {
            return -15;
        }

        memset(cur_dir, 0, sizeof(cur_dir));
        strncpy(cur_dir, cur_path, next-cur_path);
        cur_dir[next-cur_path] = '\0';

        if (0 == strcmp(cur_dir, ".")) {
        } else if (0 == strcmp(cur_dir, "..")) {
            level--;
        } else {
            level++;
        }

        if(level > MAX_DIR_LEVEL) {
            return -16;
        }

        ret = findFile(cur_dir, lba, dir_size, 1, result_record);

        if (ret < 0) {
            return ret;
        }

        lba = result_record->lsbStart;
        dir_size = result_record->lsbDataLength;

        cur_path=next+1;

        // skip unwant path separator
        while(*cur_path == '/') {
            cur_path++;
        }
        
        next = strchr(cur_path, '/');
    }

    ret = findFile(cur_path, lba, dir_size, 0, result_record);

    return ret;
}

int isoOpen(const char *path)
{
    int ret = -1;

    int k1 = pspSdkSetK1(0);

    if (g_isofd >= 0) {
        isoClose();
    }

    g_filename = path;

    if (g_filename == NULL || reOpen() < 0) {
        #ifdef DEBUG
        printk("%s: open failed %s -> 0x%08X\n", __func__, g_filename, g_isofd);
        #endif
        ret = -2;
        goto error;
    }

    is_compressed = ciso_open(&g_ciso_file);

    if (is_compressed < 0) return is_compressed;
    // total number of DVD sectors (2K) in the original ISO.
    else if (is_compressed > 0){
        g_total_sectors = g_ciso_file.uncompressed_size / ISO_SECTOR_SIZE;
    }
    else {
        SceOff size, orig;
        orig = sceIoLseek(g_isofd, 0, PSP_SEEK_CUR);
        size = sceIoLseek(g_isofd, 0, PSP_SEEK_END);
        sceIoLseek(g_isofd, orig, PSP_SEEK_SET);
        g_total_sectors = isoPos2LBA((u32)size);
    }

    isoAlloc();

    ret = readSector(16, (u8*)g_sector_buffer);

    if (ret != SECTOR_SIZE) {
        ret = -7;
        goto error;
    }

    if (memcmp(&g_sector_buffer[1], ISO_STANDARD_ID, sizeof(ISO_STANDARD_ID)-1)) {
        #ifdef DEBUG
        printk("%s: vol descriptor not found\n", __func__);
        #endif
        ret = -10;

        goto error;
    }

    memcpy(&g_root_record, &g_sector_buffer[0x9C], sizeof(g_root_record));

    return 0;

error:
    if (g_isofd >= 0) {
        isoClose();
    }
    g_filename = NULL;
    pspSdkSetK1(k1);
    return ret;
}

int isoGetTotalSectorSize(void)
{
    return g_total_sectors;
}

void isoClose(void)
{
    int k1 = pspSdkSetK1(0);
    
    sceIoClose(g_isofd);
    g_isofd = -1;
    g_filename = NULL;

    isoFree();
    g_total_sectors = 0;
    
    pspSdkSetK1(k1);
}

int isoGetFileInfo(const char * path, u32 *filesize, u32 *lba)
{
    int ret = 0;
    Iso9660DirectoryRecord rec;
    int k1 = pspSdkSetK1(0);

    ret = findPath(path, &rec);

    if (ret >= 0) {
        if (lba){
           *lba = rec.lsbStart;
        }
        if (filesize) {
            *filesize = rec.lsbDataLength;
        }
    }
    
    pspSdkSetK1(k1);
    return ret;
}

int isoRead(void *buffer, u32 lba, int offset, u32 size)
{
    int k1 = pspSdkSetK1(0);
    u32 pos = isoLBA2Pos(lba, offset);
    int res = 0;
    if (is_compressed){
        res = ciso_read(&g_ciso_file, buffer, size, pos);
    }
    else{
        res = read_raw_data(NULL, buffer, size, pos);
    }
    pspSdkSetK1(k1);
    return res;
}
