#ifndef PSNFIX_H
#define PSNFIX_H

void patch_npsignup(SceModule* mod);
void patch_npsignin(SceModule* mod);
void patch_np(SceModule* mod, u8 major, u8 minor);

#endif