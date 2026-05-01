/*
Copyright (c) 2025 m-c/d & Acid_Snake

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <malloc.h>
#include <psppower.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <pspsdk.h>
#include <pspgu.h>
#include <pspgum.h>
#include <psputilsforkernel.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <vshctrl.h>
#include <kubridge.h>


#define BUF_WIDTH               512

static FunctionPatchData display_patch;
static int (*prevDisplaySetFrameBuf)(void*, int, int, int) = NULL;

static void* list = NULL;

extern int vshmenu_running;
extern void (*vshmenu_draw)(void* frame);
extern void vshcube_draw(void* frame);


static int vshDisplaySetFrameBuf(void *frameBuf, int bufferwidth, int pixelformat, int sync) {  
    void* frame = (void*)(0x1fffffff & (u32)frameBuf);

    if (frame && vshmenu_running){
        list = memalign(64, 2048);
        // save context
        PspGeContext* gectx = memalign(64, sizeof(PspGeContext));
        int state = sceKernelSuspendDispatchThread();
        int intr = sceKernelCpuSuspendIntr();
        sceGeSaveContext(gectx);
        // draw
        sceGuStart(GU_DIRECT, list);
        sceGuDrawBuffer(GU_PSM_8888, frame, BUF_WIDTH);
        if (vshmenu_draw){
            vshmenu_draw(frame);
        }
        vshcube_draw(frame);
        // sync
        {
            u32 a=0x1fff;
            while(--a) {__asm__("nop; sync");}
        }
        sceGuFinish();
        sceGuSync(GU_SYNC_FINISH, GU_SYNC_WHAT_DONE);
        // restore context
        sceKernelCpuResumeIntrWithSync(intr);
        sceKernelResumeDispatchThread(state);
        sceGeRestoreContext(gectx);
        free(gectx);
        free(list);
    }

    return prevDisplaySetFrameBuf(frameBuf, bufferwidth, pixelformat, sync);
}

int vshgu_init() {
    sceGuInit();
    sceGuDisplay(GU_FALSE);

    void* func_addr = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);
    sctrlHENHijackFunction(&display_patch, func_addr, vshDisplaySetFrameBuf, (void**)&prevDisplaySetFrameBuf);

    return 0;
}
