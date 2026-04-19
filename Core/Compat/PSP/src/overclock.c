
// m-c/d 2026, MIT License, for more information on this project see:
// https://github.com/mcidclan/psp-undocumented-sorcery/tree/main/experimental-overclock

#include <string.h>
#include <stdio.h>
#include <pspsdk.h>
#include <pspkernel.h>

#include <cfwmacros.h>
#include <systemctrl.h>

#include "overclock.h"


#define PLL_MUL_MSB                 0x0124
#define PLL_RATIO_INDEX             5
#define PLL_BASE_FREQ               37
#define PLL_DEN                     20
#define OVERCLOCK_FREQUENCY_STEP    5
//#define PLL_CUSTOM_FLAG           27

int current_frequency = DEFAULT_FREQUENCY;

#define updatePLLMultiplier(num, msb)               \
{                                                   \
  const u32 lsb = (num) << 8 | PLL_DEN;             \
  const u32 multiplier = (msb << 16) | lsb;         \
  hw(0xbc1000fc) = multiplier;                      \
  sync();                                           \
}

#define updatePLLControl()                          \
{                                                   \
  if (!(hw(0xbc100068) & PLL_RATIO_INDEX)) {        \
    hw(0xbc100068) = 0x80 | PLL_RATIO_INDEX;        \
    /*hw(0xbc100068) &= 0xfffffff0;*/               \
    /*hw(0xbc100068) |= (0x80 | PLL_RATIO_INDEX);*/ \
    sync();                                         \
    do {                                            \
      delayPipeline();                              \
    } while (hw(0xbc100068) & 0x80);                \
    sync();                                         \
  }                                                 \
}

#define hw(addr)                      \
  (*((volatile unsigned int*)(addr)))

#define sync()          \
  __asm__ volatile(         \
    "sync       \n"     \
  )

#define clearTags()       \
  __asm__ volatile (          \
    "mtc0 $0, $28   \n"   \
    "mtc0 $0, $29   \n"   \
    "sync           \n"   \
  )
  
#define delayPipeline()                    \
  __asm__ volatile(                            \
    "nop; nop; nop; nop; nop; nop; nop \n" \
  )

#define suspendCpuIntr(var)    \
  __asm__ volatile(                \
    ".set push             \n" \
    ".set noreorder        \n" \
    ".set volatile         \n" \
    ".set noat             \n" \
    "mfc0  %0, $12         \n" \
    "sync                  \n" \
    "li    $t0, 0xfffffffe \n" \
    "and   $t0, %0, $t0    \n" \
    "mtc0  $t0, $12        \n" \
    "sync                  \n" \
    "nop                   \n" \
    "nop                   \n" \
    "nop                   \n" \
    ".set pop              \n" \
    : "=r"(var)                \
    :                          \
    : "$t0", "memory"          \
  )

#define resumeCpuIntr(var) \
  __asm__ volatile (            \
    ".set push      \n"    \
    ".set noreorder \n"    \
    ".set volatile  \n"    \
    ".set noat      \n"    \
    "mtc0  %0, $12  \n"    \
    "sync           \n"    \
    "nop            \n"    \
    "nop            \n"    \
    "nop            \n"    \
    ".set pop       \n"    \
    :                      \
    : "r"(var)             \
    : "memory"             \
  )

// Set clock domains to ratio 1:1
#define resetDomainRatios()          \
  sync();                            \
  hw(0xbc200000) = 511 << 16 | 511;  \
  hw(0xBC200004) = 511 << 16 | 511;  \
  hw(0xBC200008) = 511 << 16 | 511;  \
  sync();

// Wait for clock stability, signal propagation and pipeline drain
/*
 #define settle()            \
{                           \
  sync();                   \
  u32 i = 0x1fffff;         \
  while (--i) {             \
    delayPipeline();        \
  }                         \
}
*/
#define settle()                \
  __asm__ volatile(                 \
    ".set push              \n" \
    ".set noreorder         \n" \
    ".set nomacro           \n" \
    ".set volatile          \n" \
    ".set noat              \n" \
                                \
    "sync                   \n" \
    "lui  $t0, 0x02         \n" \
    "ori  $t0, $t0, 0xffff  \n" \
                                \
    "1:                     \n" \
    "  nop                  \n" \
    "  nop                  \n" \
    "  nop                  \n" \
    "  nop                  \n" \
    "  nop                  \n" \
    "  nop                  \n" \
    "  nop                  \n" \
    "  addiu $t0, $t0, -1   \n" \
    "  bnez  $t0, 1b        \n" \
    "  nop                  \n" \
                                \
    ".set pop               \n" \
    :                           \
    :                           \
    : "$t0", "memory"           \
  )


void (*origSetClockFrequency)(int cpu, int bus) = NULL;
u32 (*origGetClockFrequency)() = NULL;

static inline void unlockMemory() {
  const u32 start = 0xbc000000;
  const u32 end   = 0xbc00002c;
  for (u32 reg = start; reg <= end; reg += 4) {
    hw(reg) = -1;
  }
  sync();
}


static inline void adjustPLLMultiplier() {
  
  const u32 defaultNum = (u32)(((float)(DEFAULT_FREQUENCY * PLL_DEN)) / ((float)PLL_BASE_FREQ));
  hw(0xbc1000fc) = (PLL_MUL_MSB << 16) | (defaultNum << 8) | PLL_DEN;
  settle();
}

static inline void adjustPLLRatio() {
  
  u32 index = hw(0xbc100068) & 0x0f;
  sync();

  if (index != PLL_RATIO_INDEX) {
    
    const int step = (index > 5) ? -1 : 1;
    while (((step < 0) == (index > 5)) || index == 5) {
        
      hw(0xbc100068) = 0x80 | index;
      sync();
      
      do {
        delayPipeline();
      } while ((hw(0xbc100068) & 0x80));
      settle();
      
      index += step;
    }
  }
  
}

static inline void adjustDomainRatios() {
  
  const u32 cpu = hw(0xbc200000);
  const u32 bus = hw(0xBC200004);
  sync();
  
  u32 cpuDen = cpu & 0x1ff;
  u32 cpuNum = (cpu >> 16) & 0x1ff;
  u32 busDen = bus & 0x1ff;
  u32 busNum = (bus >> 16) & 0x1ff;
  
  hw(0xbc200000) = (cpuNum << 16) | cpuDen;
  hw(0xBC200004) = (busNum << 16) | busDen;
  settle();
    
  const int step = 18;
  while ((cpuNum & cpuDen & busNum & busDen) != 0x1ff) {
    
    const u32 nextCpuNum = cpuNum + step;
    const u32 nextCpuDen = cpuDen + step;
    const u32 nextBusNum = busNum + step;
    const u32 nextBusDen = busDen + step;
    
    cpuNum = (nextCpuNum > 0x1ff) ? 0x1ff : nextCpuNum;
    cpuDen = (nextCpuDen > 0x1ff) ? 0x1ff : nextCpuDen;
    busNum = (nextBusNum > 0x1ff) ? 0x1ff : nextBusNum;
    busDen = (nextBusDen > 0x1ff) ? 0x1ff : nextBusDen;
    
    hw(0xbc200000) = (cpuNum << 16) | cpuDen;
    hw(0xBC200004) = (busNum << 16) | busDen;
    settle();
  }
}

static void adjustInitialFrequencies() {
  
  sceKernelDelayThread(100);

  int intr, state;
  state = sceKernelSuspendDispatchThread();
  suspendCpuIntr(intr);

  adjustPLLMultiplier();
  adjustPLLRatio();
  adjustDomainRatios();

  resumeCpuIntr(intr);
  sceKernelResumeDispatchThread(state);
}

void doOverclock() {

  origSetClockFrequency(DEFAULT_FREQUENCY, DEFAULT_FREQUENCY/2);
  adjustInitialFrequencies();
  
  int defaultFreq = DEFAULT_FREQUENCY;
  const int freqStep = OVERCLOCK_FREQUENCY_STEP;
  int theoreticalFreq = defaultFreq + freqStep;
  
  while (theoreticalFreq <= current_frequency) {
    
    int intr, state;
    state = sceKernelSuspendDispatchThread();
    suspendCpuIntr(intr);
    
    // clearTags();
    
    u32 _num = (u32)(((float)(defaultFreq * PLL_DEN)) / ((float)PLL_BASE_FREQ));
    const u32 num = (u32)(((float)(theoreticalFreq * PLL_DEN)) / ((float)PLL_BASE_FREQ));

    updatePLLControl();
    
    //const u32 msb = PLL_MUL_MSB | (1 << (PLL_CUSTOM_FLAG - 16));
    while (_num <= num) {
      updatePLLMultiplier(_num, PLL_MUL_MSB);
      _num++;
    }

    settle();
    
    defaultFreq += freqStep;
    theoreticalFreq = defaultFreq + freqStep;
    
    resumeCpuIntr(intr);
    sceKernelResumeDispatchThread(state);
  
    // scePowerTick(PSP_POWER_TICK_ALL);
    sceKernelDelayThread(100);
  }
}

int cancelOverclock() {
  
  u32 _num = (u32)(((float)(current_frequency * PLL_DEN)) / ((float)PLL_BASE_FREQ));
  const u32 num = (u32)(((float)(current_frequency * PLL_DEN)) / ((float)PLL_BASE_FREQ));
  
  int intr, state;
  state = sceKernelSuspendDispatchThread();
  suspendCpuIntr(intr);
  
  const u32 pllCtl = hw(0xbc100068) & 0x0f;
  const u32 pllMul = hw(0xbc1000fc) & 0xffff;
  sync();
  
  resumeCpuIntr(intr);
  sceKernelResumeDispatchThread(state);
  
  const float n = (float)((pllMul & 0xff00) >> 8);
  const float d = (float)((pllMul & 0x00ff));
  const float m = (d > 0.0f) ? (n / d) : 9.0f;
  const int overclocked = ((pllCtl & PLL_RATIO_INDEX) && (m > 9.0f)) ? 1 : 0;
  sceKernelDelayThread(1000);

  //const u32 pllMul = hw(0xbc1000fc); sync();
  //const int overclocked = pllMul & (1 << PLL_CUSTOM_FLAG);
  
  if (overclocked) {
    state = sceKernelSuspendDispatchThread();
    suspendCpuIntr(intr);
    
    updatePLLControl();

    while (_num >= num) {
      updatePLLMultiplier(_num, PLL_MUL_MSB);
      _num--;
    }
    settle();
    
    resumeCpuIntr(intr);
    sceKernelResumeDispatchThread(state);
  }
  
  return overclocked;
}

void overclockHandler(int cpu, int bus){
    // disallow changing CPU clock on devkits
    if (sctrlHENIsToolKit() == PSP_TOOLKIT_TYPE_DEV) return;

    current_frequency = cpu;
    if (cpu > DEFAULT_FREQUENCY && cpu <= THEORETICAL_FREQUENCY) {
        doOverclock();
    }
    else {
        cancelOverclock();
        origSetClockFrequency(cpu, bus);
    }
}

u32 getOverclockSpeed(){
    if (current_frequency > DEFAULT_FREQUENCY) return current_frequency;
    return origGetClockFrequency();
}

void initOverclock() {
  // override clock set/get functions
  HIJACK_FUNCTION(K_EXTRACT_IMPORT(sctrlHENSetSpeed), overclockHandler, origSetClockFrequency);
  HIJACK_FUNCTION(K_EXTRACT_IMPORT(sctrlHENGetSpeed), getOverclockSpeed, origGetClockFrequency);
  // set up
  unlockMemory();
  sctrlFlushCache();
}
