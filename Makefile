PY = $(shell which python3)

.PHONY : mkdist libraries \
	SystemControl \
	VSHControl \
	Inferno \
	PopCorn \
	Stargate \
	PSPCompat \
	VitaCompat


all: mkdist libraries \
	FakeSignFlashModules
	$(Q)echo "Build Done"

mkdist:
	$(Q)mkdir -p dist
	$(Q)mkdir -p dist/flash0

libraries:
	$(MAKE) -C libs/ark-dev-sdk/

SystemControl: libraries
	$(Q)cp libs/ark-dev-sdk/include/*.h core/SystemControl/external/include/
	$(Q)cp libs/ark-dev-sdk/libs/*.a core/SystemControl/external/libs/
	$(Q)cp libs/ark-dev-sdk/src/ansi-c/*.c core/SystemControl/external/src/
	$(MAKE) -C core/SystemControl

VSHControl: libraries
	$(Q)cp libs/ark-dev-sdk/include/*.h core/VSHControl/external/include/
	$(Q)cp libs/ark-dev-sdk/libs/*.a core/VSHControl/external/libs/
	$(MAKE) -C core/VSHControl

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

PSPCompat: libraries
	$(Q)cp libs/ark-dev-sdk/include/*.h core/Compat/PSP/external/include/
	$(Q)cp libs/ark-dev-sdk/libs/*.a core/Compat/PSP/external/libs/
	$(MAKE) REBOOTEXDIR="$(CURDIR)/libs/BootLoadEx" -C core/Compat/PSP/rebootex
	$(MAKE) -C core/Compat/PSP/

VitaCompat: libraries
	$(Q)cp libs/ark-dev-sdk/include/*.h core/Compat/ePSP/external/include/
	$(Q)cp libs/ark-dev-sdk/libs/*.a core/Compat/ePSP/external/libs/
	$(MAKE) REBOOTEXDIR="$(CURDIR)/libs/BootLoadEx" -C core/Compat/ePSP/rebootex
	$(MAKE) -C core/Compat/ePSP/

FakeSignFlashModules: mkdist \
	SystemControl \
	VSHControl \
	Inferno \
	PopCorn \
	Stargate \
	PSPCompat \
	VitaCompat
	$(PY) build-tools/pspgz/pspgz.py dist/flash0/ark_systemctrl.prx build-tools/pspgz/SystemControl.hdr core/SystemControl/systemctrl.prx SystemControl 0x3007
	$(PY) build-tools/pspgz/pspgz.py dist/flash0/ark_vshctrl.prx build-tools/pspgz/SystemControl.hdr core/VSHControl/vshctrl.prx VshControl 0x3007
	$(PY) build-tools/pspgz/pspgz.py dist/flash0/ark_inferno.prx build-tools/pspgz/SystemControl.hdr core/Inferno/inferno.prx PRO_Inferno_Driver 0x3007
	$(PY) build-tools/pspgz/pspgz.py dist/flash0/ark_popcorn.prx build-tools/pspgz/SystemControl.hdr core/PopCorn/popcorn.prx PROPopcornManager 0x3007
	$(PY) build-tools/pspgz/pspgz.py dist/flash0/ark_stargate.prx build-tools/pspgz/SystemControl.hdr core/Stargate/stargate.prx Stargate 0x3007
	$(PY) build-tools/pspgz/pspgz.py dist/flash0/ark_pspcompat.prx build-tools/pspgz/SystemControl.hdr core/Compat/PSP/pspcompat.prx PSPCompat 0x3007
	$(PY) build-tools/pspgz/pspgz.py dist/flash0/ark_vitacompat.prx build-tools/pspgz/SystemControl.hdr core/Compat/ePSP/vitacompat.prx VitaCompat 0x3007


clean:
	# SystemControl
	$(Q)rm -f core/SystemControl/external/include/*.h
	$(Q)rm -f core/SystemControl/external/libs/*.a
	$(Q)rm -f core/SystemControl/external/src/*.c
	$(MAKE) -C core/SystemControl clean
	# VSHControl
	$(Q)rm -f core/VSHControl/external/include/*.h
	$(Q)rm -f core/VSHControl/external/libs/*.a
	$(MAKE) -C core/VSHControl clean
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
	# PSPCompat
	$(Q)rm -f core/Compat/PSP/external/include/*.h
	$(Q)rm -f core/Compat/PSP/external/libs/*.a
	$(MAKE) -C core/Compat/PSP clean
	$(MAKE) REBOOTEXDIR="$(CURDIR)/libs/BootLoadEx" -C core/Compat/PSP/rebootex clean
	# VitaCompat
	$(Q)rm -f core/Compat/ePSP/external/include/*.h
	$(Q)rm -f core/Compat/ePSP/external/libs/*.a
	$(MAKE) -C core/Compat/ePSP clean
	$(MAKE) REBOOTEXDIR="$(CURDIR)/libs/BootLoadEx" -C core/Compat/ePSP/rebootex clean
	# Libs
	$(MAKE) -C libs/ark-dev-sdk clean
	$(MAKE) -C libs/BootLoadEx clean
	# Rest
	$(Q)rm -rf dist