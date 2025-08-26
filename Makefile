.PHONY : libraries \
	SystemControl \
	Inferno \
	PopCorn \
	Stargate

all: libraries \
	SystemControl \
	Inferno \
	PopCorn \
	Stargate


libraries:
	$(MAKE) -C libs/ark-dev-sdk/

SystemControl: libraries
	$(Q)cp libs/ark-dev-sdk/include/*.h core/SystemControl/external/include/
	$(Q)cp libs/ark-dev-sdk/libs/*.a core/SystemControl/external/libs/
	$(Q)cp libs/ark-dev-sdk/src/ansi-c/*.c core/SystemControl/external/src/
	$(MAKE) -C core/SystemControl

Inferno: libraries
	$(Q)cp libs/ark-dev-sdk/include/*.h core/Inferno/external/include/
	$(Q)cp libs/ark-dev-sdk/libs/*.a core/Inferno/external/libs/
	$(MAKE) -C core/Inferno

PopCorn: libraries
	$(Q)cp libs/ark-dev-sdk/include/*.h core/PopCorn/external/include/
	$(Q)cp libs/ark-dev-sdk/libs/*.a core/PopCorn/external/libs/
	$(MAKE) -C core/PopCorn

Stargate: libraries
	$(Q)cp libs/ark-dev-sdk/include/*.h core/Stargate/external/include/
	$(Q)cp libs/ark-dev-sdk/libs/*.a core/Stargate/external/libs/
	$(MAKE) -C core/Stargate


clean:
	# SystemControl
	$(Q)rm -f core/SystemControl/external/include/*.h
	$(Q)rm -f core/SystemControl/external/libs/*.a
	$(Q)rm -f core/SystemControl/external/src/*.c
	$(MAKE) -C core/SystemControl clean
	# Inferno
	$(Q)rm -f core/Inferno/external/include/*.h
	$(Q)rm -f core/Inferno/external/libs/*.a
	$(MAKE) -C core/Inferno clean
	# PopCorn
	$(Q)rm -f core/PopCorn/external/include/*.h
	$(Q)rm -f core/PopCorn/external/libs/*.a
	$(MAKE) -C core/PopCorn clean
	# Stargate
	$(Q)rm -f core/Stargate/external/include/*.h
	$(Q)rm -f core/Stargate/external/libs/*.a
	$(MAKE) -C core/Stargate clean
	# Libs
	$(MAKE) -C libs/ark-dev-sdk clean