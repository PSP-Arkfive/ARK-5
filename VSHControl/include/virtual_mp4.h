#ifndef VIRTUAL_MP4_H
#define VIRTUAL_MP4_H

#include <pspsdk.h>

SceUID videoIoOpen(const char* file, u32 flags, u32 mode);
SceUID videoIoDopen(const char* dir);
int videoIoGetstat(const char* path, SceIoStat* stat);
int videoIoRead(SceUID fd, void* buf, u32 size);
int videoIoDread(SceUID fd, SceIoDirent *dir);
int videoIoClose(SceUID fd);
int videoIoDclose(SceUID fd);
SceOff videoIoLseek(SceUID fd, SceOff offset, int whence);
int videoRemove(const char * file);
int is_video_path(const char* path);
int is_video_file(SceUID fd);
int is_video_folder(SceUID dd);

#endif