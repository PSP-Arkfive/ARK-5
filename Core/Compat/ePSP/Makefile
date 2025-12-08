TARGET = vitacompat

PSPSDK = $(shell psp-config --pspsdk-path)

C_OBJS =         \
	main.o        \
	src/vlf.o      \
	src/syspatch.o  \
	src/vitamem.o    \
	src/vitaflash.o   \
	src/popspatch.o    \
	src/filesystem.o    \
	src/fatef.o          \
	src/flashfs.o         \
		
OBJS = \
	$(C_OBJS) imports.o

all: $(TARGET).prx
INCDIR = include rebootex
CFLAGS = -std=c99 -Os -G0 -Wall -fno-pic

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR =
LDFLAGS =  -nostartfiles
LIBS = -lpspsystemctrl_kernel -lpspkermit_driver -lpspkermitperipheral_driver

PSP_FW_VERSION = 660

include $(PSPSDK)/lib/build.mak
