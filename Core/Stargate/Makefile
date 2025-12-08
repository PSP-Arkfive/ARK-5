PSPSDK = $(shell psp-config --pspsdk-path)

TARGET = stargate

C_OBJS = main.o \
	imports.o \
	src/loadmodule_patch.o \
	src/nodrm_patch.o \
	src/io_patch.o \
	src/key_decrypt.o \
	src/pspcipher.o \
	src/gamefix.o \
	src/hide.o \
	src/chn_iso.o \

OBJS = $(C_OBJS)

all: $(TARGET).prx

INCDIR = include
CFLAGS = -std=c99 -Os -G0 -Wall -fno-pic

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

PSP_FW_VERSION = 660

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR =
LDFLAGS = -nostartfiles
LIBS = -lpspsystemctrl_kernel -lpspsysc_user -lpspsemaphore_660

include $(PSPSDK)/lib/build.mak
