.PHONY : libraries \
	SystemControl \
	Inferno \
	PopCorn

all: libraries \
	SystemControl \
	Inferno \
	PopCorn


libraries:
	$(MAKE) -C libs/ark-dev-sdk/

SystemControl: libraries
	$(Q)cp libs/ark-dev-sdk/include/* core/SystemControl/external/include/
	$(Q)cp libs/ark-dev-sdk/libs/* core/SystemControl/external/libs/
	$(Q)cp libs/ark-dev-sdk/src/ansi-c/*.c core/SystemControl/external/src/
	$(MAKE) -C core/SystemControl

Inferno: libraries
	$(Q)cp libs/ark-dev-sdk/include/* core/Inferno/external/include/
	$(Q)cp libs/ark-dev-sdk/libs/* core/Inferno/external/libs/
	$(MAKE) -C core/Inferno

PopCorn: libraries
	$(Q)cp libs/ark-dev-sdk/include/* core/PopCorn/external/include/
	$(Q)cp libs/ark-dev-sdk/libs/* core/PopCorn/external/libs/
	$(MAKE) -C core/PopCorn


clean:
	# SystemControl
	$(Q)rm -f core/SystemControl/external/include/*.h
	$(Q)rm -f core/SystemControl/external/include/guglue.exp
	$(Q)rm -f core/SystemControl/external/libs/*.a
	$(Q)rm -f core/SystemControl/external/src/*.c
	$(MAKE) -C core/SystemControl clean
	# Inferno
	$(Q)rm -f core/Inferno/external/include/*.h
	$(Q)rm -f core/Inferno/external/include/guglue.exp
	$(Q)rm -f core/Inferno/external/libs/*.a
	$(MAKE) -C core/Inferno clean
	# PopCorn
	$(Q)rm -f core/PopCorn/external/include/*.h
	$(Q)rm -f core/PopCorn/external/include/guglue.exp
	$(Q)rm -f core/PopCorn/external/libs/*.a
	$(MAKE) -C core/PopCorn clean
	# Libs
	$(MAKE) -C libs/ark-dev-sdk clean