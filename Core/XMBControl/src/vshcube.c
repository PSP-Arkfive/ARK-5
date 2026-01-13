// credits to mcidclan

#include <psppower.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <pspsdk.h>
#include <pspgu.h>
#include <pspgum.h>
#include <psputilsforkernel.h>

#include <systemctrl.h>
#include <vshctrl.h>

static inline void* hook(char* mod, char* lib, u32 nid, void* hf) {
  void* f = (void*)sctrlHENFindFunction(mod, lib, nid);
  if (f) {
    sctrlHENPatchSyscall(f, hf);
    return f;
  }
  return NULL;
}
static inline void unhook(void* hook_orig, void* hook_patch){
  sctrlHENPatchSyscall(hook_patch, hook_orig);
}

struct Vertex {
  unsigned int color;
  float nx, ny, nz;
  float x, y, z;
} __attribute__((aligned(4), packed));

#define VRAM_BACKUP_BYTE_COUNT  0x00044000
#define DEPTH_BUFFER            0x00000000 /*0x001c0000*/
#define CUBE_VERT_COUNT         36
#define BUF_WIDTH               512
#define SCR_WIDTH               480
#define SCR_HEIGHT              272

u32 magic            = 0;
u32* list            = NULL;
void* vramBackup     = NULL;
struct Vertex* cube  = NULL;
PspGeContext* ctx    = NULL;

static int vsh_cube_running = 0;

int (*_displaySetFrameBuf)(void*, int, int, int);

int displaySetFrameBuf(void *frameBuf, int bufferwidth, int pixelformat, int sync) {
  
  void* frame = (void*)(0x1fffffff & (u32)frameBuf);

  if (frame) {
    
    if (list && magic == 1) {
      
      magic = 2;
      
      sceGeSaveContext(ctx);
      int state = sceKernelSuspendDispatchThread();
      int intr = sceKernelCpuSuspendIntr();
      
      // pspDebugScreenSetBase((u32*)(0x40000000 | (u32)frame));
      // pspDebugScreenSetXY(41, 31);
      // pspDebugScreenKprintf("GU cube on VSH");
        
      // frame = (void*)(0x40000000 | (u32)frame);
      void* const depthBuf = (void*)(DEPTH_BUFFER + (u32)frame);

      sceGuStart(GU_DIRECT, list);
      
      sceGuDisable(GU_DEPTH_TEST);
      sceGuDisable(GU_BLEND);
      sceGuDisable(GU_TEXTURE_2D);
      sceGuDisable(GU_CULL_FACE);
      
      sceGuCopyImage(GU_PSM_8888,
        0, 0, 480, 64, 512, depthBuf,
        0, 0, 512, vramBackup
      );

      sceGuOffset(2048 - (SCR_WIDTH/2), 2048 - (SCR_HEIGHT/2));
      sceGuViewport(2048, 2048, SCR_WIDTH, SCR_HEIGHT);

      sceGumMatrixMode(GU_PROJECTION);
      sceGumLoadIdentity();
      sceGumPerspective(50.0f, 16.0f/9.0f, 1.0f, 100.0f);

      sceGumMatrixMode(GU_VIEW);
      sceGumLoadIdentity();

      sceGumMatrixMode(GU_MODEL);
      sceGumLoadIdentity();

      {
        static float deg = 45.0f;
        const float rad = 3.14f/180.0f;
        const ScePspFVector3 t = {1.435f, -0.72f, -2.0f};
        const ScePspFVector3 s = {0.09f, 0.09f, 0.09f};
        sceGumTranslate(&t);
        sceGumScale(&s);
        sceGumRotateY(rad * deg * 1.2f);
        sceGumRotateZ(rad * deg * 1.4f);
        deg += 0.25f;
      }
      
      sceGuDepthBuffer(depthBuf, 64);
      sceGuDepthRange(65535, 0);
      sceGuClearDepth(0.0f);
      sceGuDepthFunc(GU_GEQUAL);
      sceGuEnable(GU_DEPTH_TEST);

      sceGuDisable(GU_STENCIL_TEST);

      sceGuDrawBuffer(GU_PSM_8888, frame, BUF_WIDTH);
      sceGuEnable(GU_SCISSOR_TEST);
      sceGuScissor(416, 208, 64, 64);
      // sceGuClearColor(0);
      
      sceGuClear(/*GU_COLOR_BUFFER_BIT | */GU_DEPTH_BUFFER_BIT);
      
      sceGuEnable(GU_LIGHTING);
      sceGuEnable(GU_LIGHT0);
      sceGuAmbient(0xffeeeeee);
      sceGuLightColor(GU_LIGHT0, GU_DIFFUSE, 0xffffffff);

      ScePspFVector3 lightDir = { 0.0f, -1.0f, -1.0f };
      sceGuLight(GU_LIGHT0, GU_DIRECTIONAL, GU_DIFFUSE_AND_SPECULAR, &lightDir);

      sceGumDrawArray(GU_TRIANGLES, GU_COLOR_8888 | GU_NORMAL_32BITF |
      GU_VERTEX_32BITF | GU_TRANSFORM_3D, CUBE_VERT_COUNT, NULL, cube);
      
      sceGuDisable(GU_LIGHT0);
      sceGuDisable(GU_LIGHTING);
       
      {
        const ScePspFVector3 s = {1.09f, 1.09f, 1.09f};
        sceGumScale(&s);
      }
      
      sceGuEnable(GU_BLEND);
      sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
  
      sceGumDrawArray(GU_TRIANGLES, GU_COLOR_8888 | GU_NORMAL_32BITF |
      GU_VERTEX_32BITF | GU_TRANSFORM_3D, CUBE_VERT_COUNT, NULL, cube);
      
      sceGuCopyImage(GU_PSM_8888,
        0, 0, 480, 64, 512, vramBackup,
        0, 0, 512, depthBuf
      );
      
      sceGuFinish();
      sceGuSync(GU_SYNC_FINISH, GU_SYNC_WHAT_DONE);
      
      sceKernelCpuResumeIntrWithSync(intr);
      sceKernelResumeDispatchThread(state);
      sceGeRestoreContext(ctx);
      
      magic = 1;
    }
  }
  
  int ret = _displaySetFrameBuf(frameBuf, bufferwidth, pixelformat, sync);

  return ret;
}

int thread(SceSize ags, void *agp) {
  void *frame = NULL;
  int width, format;
  while (vsh_cube_running && !frame) {
    sceDisplayGetFrameBuf(&frame, &width, &format, 0);
    if (frame) {
      magic = 1;
    }
    sceKernelDelayThread(1);
  }
  sceKernelExitDeleteThread(0);
  return 0;
}

int vshcube_start() {
  magic = 0;
  vramBackup = NULL;
  list = NULL;
  cube = NULL;
  ctx = NULL;
  
  vramBackup = user_memalign(64, VRAM_BACKUP_BYTE_COUNT);
  list = (u32*)user_memalign(64, 2048);
  ctx = user_memalign(64, sizeof(PspGeContext));
  
  const unsigned int cubeSize = CUBE_VERT_COUNT * sizeof(struct Vertex);
  cube = (struct Vertex*)user_memalign(64, cubeSize);
  
  unsigned int cubeColor = 0x808b4513;

  // front
  cube[0]  = (struct Vertex){ cubeColor,  0.0f,  0.0f,  1.0f,   1.0f, -1.0f,  1.0f };
  cube[1]  = (struct Vertex){ cubeColor,  0.0f,  0.0f,  1.0f,  -1.0f,  1.0f,  1.0f };
  cube[2]  = (struct Vertex){ cubeColor,  0.0f,  0.0f,  1.0f,  -1.0f, -1.0f,  1.0f };
  cube[3]  = (struct Vertex){ cubeColor,  0.0f,  0.0f,  1.0f,   1.0f,  1.0f,  1.0f };
  cube[4]  = (struct Vertex){ cubeColor,  0.0f,  0.0f,  1.0f,  -1.0f,  1.0f,  1.0f };
  cube[5]  = (struct Vertex){ cubeColor,  0.0f,  0.0f,  1.0f,   1.0f, -1.0f,  1.0f };

  // back
  cube[6]  = (struct Vertex){ cubeColor,  0.0f,  0.0f, -1.0f,  -1.0f,  1.0f, -1.0f };
  cube[7]  = (struct Vertex){ cubeColor,  0.0f,  0.0f, -1.0f,   1.0f, -1.0f, -1.0f };
  cube[8]  = (struct Vertex){ cubeColor,  0.0f,  0.0f, -1.0f,  -1.0f, -1.0f, -1.0f };
  cube[9]  = (struct Vertex){ cubeColor,  0.0f,  0.0f, -1.0f,  -1.0f,  1.0f, -1.0f };
  cube[10] = (struct Vertex){ cubeColor,  0.0f,  0.0f, -1.0f,   1.0f,  1.0f, -1.0f };
  cube[11] = (struct Vertex){ cubeColor,  0.0f,  0.0f, -1.0f,   1.0f, -1.0f, -1.0f };

  // left
  cube[12] = (struct Vertex){ cubeColor, -1.0f,  0.0f,  0.0f,  -1.0f, -1.0f, -1.0f };
  cube[13] = (struct Vertex){ cubeColor, -1.0f,  0.0f,  0.0f,  -1.0f, -1.0f,  1.0f };
  cube[14] = (struct Vertex){ cubeColor, -1.0f,  0.0f,  0.0f,  -1.0f,  1.0f, -1.0f };
  cube[15] = (struct Vertex){ cubeColor, -1.0f,  0.0f,  0.0f,  -1.0f, -1.0f,  1.0f };
  cube[16] = (struct Vertex){ cubeColor, -1.0f,  0.0f,  0.0f,  -1.0f,  1.0f,  1.0f };
  cube[17] = (struct Vertex){ cubeColor, -1.0f,  0.0f,  0.0f,  -1.0f,  1.0f, -1.0f };

  // right
  cube[18] = (struct Vertex){ cubeColor,  1.0f,  0.0f,  0.0f,   1.0f, -1.0f, -1.0f };
  cube[19] = (struct Vertex){ cubeColor,  1.0f,  0.0f,  0.0f,   1.0f,  1.0f, -1.0f };
  cube[20] = (struct Vertex){ cubeColor,  1.0f,  0.0f,  0.0f,   1.0f, -1.0f,  1.0f };
  cube[21] = (struct Vertex){ cubeColor,  1.0f,  0.0f,  0.0f,   1.0f, -1.0f,  1.0f };
  cube[22] = (struct Vertex){ cubeColor,  1.0f,  0.0f,  0.0f,   1.0f,  1.0f, -1.0f };
  cube[23] = (struct Vertex){ cubeColor,  1.0f,  0.0f,  0.0f,   1.0f,  1.0f,  1.0f };

  // bottom
  cube[24] = (struct Vertex){ cubeColor,  0.0f, -1.0f,  0.0f,   1.0f, -1.0f,  1.0f };
  cube[25] = (struct Vertex){ cubeColor,  0.0f, -1.0f,  0.0f,  -1.0f, -1.0f,  1.0f };
  cube[26] = (struct Vertex){ cubeColor,  0.0f, -1.0f,  0.0f,   1.0f, -1.0f, -1.0f };
  cube[27] = (struct Vertex){ cubeColor,  0.0f, -1.0f,  0.0f,   1.0f, -1.0f, -1.0f };
  cube[28] = (struct Vertex){ cubeColor,  0.0f, -1.0f,  0.0f,  -1.0f, -1.0f,  1.0f };
  cube[29] = (struct Vertex){ cubeColor,  0.0f, -1.0f,  0.0f,  -1.0f, -1.0f, -1.0f };

  // top
  cube[30] = (struct Vertex){ cubeColor,  0.0f,  1.0f,  0.0f,  -1.0f,  1.0f,  1.0f };
  cube[31] = (struct Vertex){ cubeColor,  0.0f,  1.0f,  0.0f,   1.0f,  1.0f, -1.0f };
  cube[32] = (struct Vertex){ cubeColor,  0.0f,  1.0f,  0.0f,  -1.0f,  1.0f, -1.0f };
  cube[33] = (struct Vertex){ cubeColor,  0.0f,  1.0f,  0.0f,  -1.0f,  1.0f,  1.0f };
  cube[34] = (struct Vertex){ cubeColor,  0.0f,  1.0f,  0.0f,   1.0f,  1.0f,  1.0f };
  cube[35] = (struct Vertex){ cubeColor,  0.0f,  1.0f,  0.0f,   1.0f,  1.0f, -1.0f };


  sceKernelDcacheWritebackRange(cube, (cubeSize + 63) & ~63);
  
  // pspDebugScreenInitEx(0, PSP_DISPLAY_PIXEL_FORMAT_8888, 1);
  // pspDebugScreenEnableBackColor(0);
  
  _displaySetFrameBuf = hook("sceDisplay_Service", "sceDisplay", 0x289D82FE, (void*)displaySetFrameBuf);

  SceUID id = sceKernelCreateThread("thread", thread, 0x12, 0x10000, 0, NULL);
  if (id >= 0) {
    sceKernelStartThread(id, 0, NULL);
  }
  
  return 0;
}

int vshcube_stop(SceSize args, void *argp) {

  unhook(_displaySetFrameBuf, displaySetFrameBuf);

  return 0;
}
