PSPSDK = $(shell psp-config --pspsdk-path)

TARGET = systemctrl

OBJS = \
	main.o \
	src/modulemanager.o \
	src/elf.o \
	src/flushcache.o \
	src/loadercore.o \
	src/cryptography.o \
	src/interruptman.o \
	src/kubridge.o \
	src/sctrl.o \
	src/sctrl_se.o \
	src/sctrl_hen.o \
	src/oe_malloc.o \
	src/syspatch.o \
	src/mediasync.o \
	src/hooknids.o \
	src/nidresolver.o \
	src/nid_660_data.o \
	src/rebootex.o \
	src/sysmem.o \
	src/loadexec.o \
	src/exitgame.o \
	src/gameinfo.o \
	src/plugin.o \
	src/leda.o \
	src/psnfix.o \
	src/cpuclock.o   \
	src/msstor_cache.o \
	src/threadman.o \
	src/utility.o \
	src/patches.o \
	src/missingfunc.o \
	src/setlongjmp.o
	
all: $(TARGET).prx
INCDIR = include
CFLAGS = -std=c99 -Os -G0 -Wall -fno-pic
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR =
LDFLAGS = -nostartfiles
LIBS = -lmicrolz \
	-lpspsemaphore_660 \
	-lpspinit_kernel_660   \
	-lpspsysmem_kernel_660  \
	-lpspsysclib_kernel_660  \
	-lpsploadcore_kernel_660  \
	-lpspmodulemgr_kernel_660  \
	-lpspthreadman_kernel_660   \
	-lpspinterruptmanager_kernel_660 \

PSP_FW_VERSION = 660

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
OBJS += debug/exception.o debug/exception_asm.o debug/printk.o debug/jumper.o
LIBS += -lcolordebugger -lscreenprinter
else
OBJS += debug/dummy.o
endif

include $(PSPSDK)/lib/build.mak
