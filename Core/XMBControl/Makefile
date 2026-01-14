PSPSDK = $(shell psp-config --pspsdk-path)

TARGET = xmbctrl
OBJS = main.o stub.o \
	src/xmbpatch.o \
	src/list.o \
	src/settings.o \
	src/config.o \
	src/plugins.o \
	src/battery.o \
	src/vshmenu.o \
	src/utils.o \
	src/vshcube.o

CFLAGS = -std=c99 -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

INCDIR = include
LIBDIR =

USE_PSPSDK_LIBS = 1
USE_PSPSDK_LIBC = 1

LIBS = -lpspgum -lpspgu -lpspsystemctrl_user -lpspkubridge -lpspvshctrl -lpspreg -lpspsysc_user -lpsppower
LDFLAGS = -nostartfiles

include $(PSPSDK)/lib/build.mak
