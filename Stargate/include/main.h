#ifndef STARGATE_MAIN_H
#define STARGATE_MAIN_H

#include <pspsdk.h>
#include <pspkernel.h>

extern void patch_ioDevCtl();
extern void patch_IsoDrivers();
extern void patch_sceMesgLed();
extern void applyFixesByGameId();
extern void patch_ISODriverModule(SceModule* mod);
extern void applyFixesByModule(SceModule* mod);
extern void hide_cfw_folder(SceModule * mod);

#endif