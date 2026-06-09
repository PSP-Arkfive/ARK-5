#ifndef VITAFLASH_H
#define VITAFLASH_H

#include <cfwmacros.h>

#define FLASH_BACKUP "ms0:/flash.bak"

// Vita Buffered RAM flash0 Filesystem Structure
typedef struct VitaFlashBufferFile
{
    char * name;
    void * content;
    unsigned int size;
} VitaFlashBufferFile;

int patchKermitPeripheral();

#endif
