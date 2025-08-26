.PHONY : libraries SystemControl

all: libraries SystemControl

libraries:
	$(MAKE) -C libs/ark-dev-sdk/


SystemControl: libraries
	$(Q)cp libs/ark-dev-sdk/include/* core/SystemControl/external/include/
	$(Q)cp libs/ark-dev-sdk/libs/* core/SystemControl/external/libs/
	$(Q)cp libs/ark-dev-sdk/src/ansi-c/*.c core/SystemControl/external/src/
	$(MAKE) -C core/SystemControl

clean:
	$(Q)rm -f core/SystemControl/external/include/*.h
	$(Q)rm -f core/SystemControl/external/include/guglue.exp
	$(Q)rm -f core/SystemControl/external/libs/*.a
	$(Q)rm -f core/SystemControl/external/src/*.c
	$(MAKE) -C core/SystemControl clean
	$(MAKE) -C libs/ark-dev-sdk clean