PSPSDK = $(shell psp-config --pspsdk-path)
PSPDEV = $(shell psp-config --pspdev-path)
PYTHON = $(shell which python3)
ARKSDK = $(PSPDEV)/share/ark-dev-sdk

TARGET = reboot.bin.gz

all: $(TARGET)

C_OBJS = main.o

CC = psp-gcc
LD = psp-ld
STRIP = psp-strip
OBJCOPY = psp-objcopy
LINKFILE = $(ARKSDK)/build-tools/bootloadex.l
INCDIR = $(PSPSDK)/include
CFLAGS = -std=c99 -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR))
LIBS = -L $(PSPSDK)/lib -lbootloadex -lansic


ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
LIBS += -lcolordebugger
endif

main.elf: $(C_OBJS)

$(TARGET): main.elf
	$(Q)$(STRIP) -s $<
	$(Q)$(OBJCOPY) -O binary $< $(patsubst %.gz,%,$(TARGET))
	$(Q)$(PYTHON) $(ARKSDK)/build-tools/gz/gz.py $(patsubst %.gz,%,$(TARGET)) $@
	$(Q)bin2c $@ payload.h rebootbuffer_pentazemin
	@echo GET $@

clean:
	$(Q)rm -rf *~ *.s *.o *.elf reboot.bin reboot.bin.gz payload.h $(EXTRA_CLEAN)

include $(ARKSDK)/build-tools/beauty_bin.mak
