PSPSDK = $(shell psp-config --pspsdk-path)

TARGET = inferno

C_OBJS = \
	main.o \
	src/iodrv_funcs.o \
	src/umd.o \
	src/isoread.o \
	src/isocache.o

OBJS = $(C_OBJS) imports.o

all: $(TARGET).prx
INCDIR = include
CFLAGS = -std=c99 -Os -G0 -Wall

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

LDFLAGS =  -nostartfiles
LIBDIR = 
LIBS = -lcisoread -lpspsystemctrl_kernel

PSP_FW_VERSION = 660

PRX_EXPORTS = exports.exp

BUILD_PRX=1
USE_KERNEL_LIBS=1
USE_KERNEL_LIBC=1

include $(PSPSDK)/lib/build.mak
