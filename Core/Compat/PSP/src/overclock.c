// m-c/d 2026, for more information on this project see:
// https://github.com/mcidclan/psp-undocumented-sorcery/tree/main/experimental-overclock


#include <string.h>
#include <pspsdk.h>
#include <pspkernel.h>

#include <cfwmacros.h>
#include <systemctrl.h>

#include "overclock.h"


#define PLL_DEN                       20
#define PLL_DEN_STREET                2
#define PLL_BASE_FREQ                 37
#define PLL_BASE_FREQ_STREET          12
#define PLL_RATIO_INDEX               5
#define PLL_RATIO_INDEX_STREET        5
#define PLL_MUL_MSB                   0x0124
#define PLL_MUL_MSB_STREET            0x0122

//#define PLL_CUSTOM_FLAG             27
#define FREQUENCY_STEP                10  /*PLL_BASE_FREQ / 2*/

static unsigned int pll_den           = PLL_DEN;
static unsigned int pll_base_freq     = PLL_BASE_FREQ;
static unsigned int pll_mul_msb       = PLL_MUL_MSB;
static unsigned int pll_ratio_index   = PLL_RATIO_INDEX;
static unsigned int freq_step         = FREQUENCY_STEP;


int currFreq = DEFAULT_FREQUENCY, targetFreq = DEFAULT_FREQUENCY;

void (*origSetClockFrequency)(int cpu, int bus) = NULL;
u32 (*origGetClockFrequency)() = NULL;

#define hw(addr)                      \
  (*((volatile unsigned int*)(addr)))

#define sync()          \
  __asm__ volatile(         \
    "sync       \n"     \
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
  __asm__ volatile(            \
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

#define updatePLLMultiplier(num, msb)               \
{                                                   \
  const u32 lsb = (num) << 8 | pll_den;             \
  const u32 multiplier = (msb << 16) | lsb;         \
  hw(0xbc1000fc) = multiplier;                      \
  sync();                                           \
}

#define updatePLLControl()                          \
{                                                   \
  if (!(hw(0xbc100068) & pll_ratio_index)) {        \
    hw(0xbc100068) = 0x80 | pll_ratio_index;        \
    /*hw(0xbc100068) &= 0xfffffff0;*/               \
    /*hw(0xbc100068) |= (0x80 | pll_ratio_index);*/ \
    sync();                                         \
    do {                                            \
      delayPipeline();                              \
    } while (hw(0xbc100068) & 0x80);                \
    sync();                                         \
  }                                                 \
}

static inline void adjustPLLMultiplier() {
  
  const u32 defaultNum = (u32)(((float)(DEFAULT_FREQUENCY * pll_den)) / ((float)pll_base_freq));
  hw(0xbc1000fc) = (hw(0xbc1000fc) & 0xffff0000) | (defaultNum << 8) | pll_den;
  sync();
  hw(0xbc1000fc) = (pll_mul_msb << 16) | (hw(0xbc1000fc) & 0xffff);
  settle();
}

static inline void adjustPLLRatio() {
  
  u32 index = hw(0xbc100068) & 0x0f;
  sync();

  if (index != pll_ratio_index) {
    
    const int step = (index > pll_ratio_index) ? -1 : 1;
    while (((step < 0) == (index > pll_ratio_index)) || index == pll_ratio_index) {
        
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

static void adjustValues(){
  extern int psp_model;
  if (psp_model == PSP_STREET){
    pll_den           = PLL_DEN_STREET;
    pll_base_freq     = PLL_BASE_FREQ_STREET;
    pll_mul_msb       = PLL_MUL_MSB_STREET;
    pll_ratio_index   = PLL_RATIO_INDEX_STREET;
  }
}

void doOverclock() {

  origSetClockFrequency(DEFAULT_FREQUENCY, DEFAULT_FREQUENCY/2);
  adjustInitialFrequencies();
  
  int defaultFreq = DEFAULT_FREQUENCY;
  const int freqStep = freq_step;
  int theoreticalFreq = defaultFreq + freqStep;
  
  while (theoreticalFreq <= targetFreq) {
    
    int intr, state;
    state = sceKernelSuspendDispatchThread();
    suspendCpuIntr(intr);
    
    // clearTags();
    
    u32 _num = (u32)(((float)(defaultFreq * pll_den)) / ((float)pll_base_freq));
    const u32 num = (u32)(((float)(theoreticalFreq * pll_den)) / ((float)pll_base_freq));
    
    updatePLLControl();
    
    //const u32 msb = pll_mul_msb | (1 << (PLL_CUSTOM_FLAG - 16));
    while (_num <= num) {
      updatePLLMultiplier(_num, pll_mul_msb);
      _num++;
    }
    settle();
    
    defaultFreq += freqStep;
    theoreticalFreq = defaultFreq + freqStep;
    
    resumeCpuIntr(intr);
    sceKernelResumeDispatchThread(state);
    sceKernelDelayThread(100);
  }
  currFreq = theoreticalFreq;
}

void cancelOverclock() {
  u32 _num = (u32)(((float)(currFreq * pll_den)) / ((float)pll_base_freq));
  const u32 num = (u32)(((float)(DEFAULT_FREQUENCY * pll_den)) / ((float)pll_base_freq));
  
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
  const int overclocked = ((pllCtl & pll_ratio_index) && (m > 9.0f)) ? 1 : 0;
  sceKernelDelayThread(1000);

  //const u32 pllMul = hw(0xbc1000fc); sync();
  //const int overclocked = pllMul & (1 << PLL_CUSTOM_FLAG);
  
  if (overclocked) {
    state = sceKernelSuspendDispatchThread();
    suspendCpuIntr(intr);
    
    updatePLLControl();

    while (_num >= num) {
      updatePLLMultiplier(_num, pll_mul_msb);
      _num--;
    }
    settle();
    
    resumeCpuIntr(intr);
    sceKernelResumeDispatchThread(state);
  }
}

void overclockHandler(int cpu, int bus){    
    if (cpu > DEFAULT_FREQUENCY && cpu <= MAX_ALLOWED_FREQUENCY && cpu > currFreq) {
        targetFreq = cpu;
        doOverclock();
    }
    else {
        if (currFreq > DEFAULT_FREQUENCY && cpu < currFreq) return;
        origSetClockFrequency(cpu, bus);
        currFreq = cpu;
    }
}

u32 getOverclockSpeed(){
    if (currFreq > DEFAULT_FREQUENCY) return currFreq;
    return origGetClockFrequency();
}

void initOverclock() {
  // override clock set/get functions
  HIJACK_FUNCTION(K_EXTRACT_IMPORT(sctrlHENSetSpeed), overclockHandler, origSetClockFrequency);
  HIJACK_FUNCTION(K_EXTRACT_IMPORT(sctrlHENGetSpeed), getOverclockSpeed, origGetClockFrequency);
  adjustValues();
  sctrlFlushCache();
}
