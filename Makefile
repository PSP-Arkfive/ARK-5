PY = $(shell which python3)

.PHONY : mkdist libraries FlashPackage \
	SystemControl VSHControl XMBControl Inferno PopCorn Stargate \
	PSPCompat VitaCompat VitaPopsCompat Pentazemin


all: mkdist libraries FlashPackage
	$(Q)echo "Build Done"

mkdist:
	$(Q)mkdir -p dist
	$(Q)mkdir -p dist/flash0

libraries:
	$(MAKE) -C Libs/ark-dev-sdk/

SystemControl: libraries
	$(Q)cp Libs/ark-dev-sdk/include/*.h Core/SystemControl/external/include/
	$(Q)cp Libs/ark-dev-sdk/libs/*.a Core/SystemControl/external/libs/
	$(Q)cp Libs/ark-dev-sdk/src/ansi-c/*.c Core/SystemControl/external/src/
	$(MAKE) -C Core/SystemControl

VSHControl: libraries
	$(Q)cp Libs/ark-dev-sdk/include/*.h Core/VSHControl/external/include/
	$(Q)cp Libs/ark-dev-sdk/libs/*.a Core/VSHControl/external/libs/
	$(MAKE) -C Core/VSHControl

XMBControl: libraries
	$(Q)cp Libs/ark-dev-sdk/include/*.h Core/XMBControl/external/include/
	$(Q)cp Libs/ark-dev-sdk/libs/*.a Core/XMBControl/external/libs/
	$(MAKE) -C Core/XMBControl

Inferno: libraries
	$(Q)cp Libs/ark-dev-sdk/include/*.h Core/Inferno/external/include/
	$(Q)cp Libs/ark-dev-sdk/libs/*.a Core/Inferno/external/libs/
	$(MAKE) -C Core/Inferno

PopCorn: libraries
	$(Q)cp Libs/ark-dev-sdk/include/*.h Core/PopCorn/external/include/
	$(Q)cp Libs/ark-dev-sdk/libs/*.a Core/PopCorn/external/libs/
	$(MAKE) -C Core/PopCorn

Stargate: libraries
	$(Q)cp Libs/ark-dev-sdk/include/*.h Core/Stargate/external/include/
	$(Q)cp Libs/ark-dev-sdk/libs/*.a Core/Stargate/external/libs/
	$(MAKE) -C Core/Stargate

PSPCompat: libraries
	$(Q)cp Libs/ark-dev-sdk/include/*.h Core/Compat/PSP/external/include/
	$(Q)cp Libs/ark-dev-sdk/libs/*.a Core/Compat/PSP/external/libs/
	$(MAKE) REBOOTEXDIR="$(CURDIR)/Libs/BootLoadEx" -C Core/Compat/PSP/rebootex
	$(MAKE) -C Core/Compat/PSP/

VitaCompat: libraries
	$(Q)cp Libs/ark-dev-sdk/include/*.h Core/Compat/ePSP/external/include/
	$(Q)cp Libs/ark-dev-sdk/libs/*.a Core/Compat/ePSP/external/libs/
	$(MAKE) REBOOTEXDIR="$(CURDIR)/Libs/BootLoadEx" -C Core/Compat/ePSP/rebootex
	$(MAKE) -C Core/Compat/ePSP/
	$(PY) build-tools/btcnf.py build Core/Compat/ePSP/btcnf/psvbtcnf.txt
	$(PY) build-tools/btcnf.py build Core/Compat/ePSP/btcnf/psvbtinf.txt
	$(Q)mv Core/Compat/ePSP/btcnf/*.bin dist/flash0/

VitaPopsCompat: libraries
	$(Q)cp Libs/ark-dev-sdk/include/*.h Core/Compat/ePSX/external/include/
	$(Q)cp Libs/ark-dev-sdk/libs/*.a Core/Compat/ePSX/external/libs/
	$(MAKE) REBOOTEXDIR="$(CURDIR)/Libs/BootLoadEx" -C Core/Compat/ePSX/rebootex
	$(MAKE) -C Core/Compat/ePSX/
	$(PY) build-tools/btcnf.py build Core/Compat/ePSX/btcnf/psxbtcnf.txt
	$(Q)mv Core/Compat/ePSX/btcnf/*.bin dist/flash0/

Pentazemin: libraries
	$(Q)cp Libs/ark-dev-sdk/include/*.h Core/Compat/vPSP/external/include/
	$(Q)cp Libs/ark-dev-sdk/libs/*.a Core/Compat/vPSP/external/libs/
	$(MAKE) REBOOTEXDIR="$(CURDIR)/Libs/BootLoadEx" -C Core/Compat/vPSP/rebootex
	$(MAKE) -C Core/Compat/vPSP/
	$(PY) build-tools/btcnf.py build Core/Compat/vPSP/btcnf/psvbtjnf.txt
	$(PY) build-tools/btcnf.py build Core/Compat/vPSP/btcnf/psvbtknf.txt
	$(Q)mv Core/Compat/vPSP/btcnf/*.bin dist/flash0/

FlashPackage: mkdist \
	SystemControl VSHControl XMBControl Inferno PopCorn Stargate \
	PSPCompat VitaCompat VitaPopsCompat Pentazemin
	$(PY) build-tools/pspgz/pspgz.py dist/flash0/ark_systemctrl.prx build-tools/pspgz/SystemControl.hdr Core/SystemControl/systemctrl.prx SystemControl 0x3007
	$(PY) build-tools/pspgz/pspgz.py dist/flash0/ark_vshctrl.prx build-tools/pspgz/SystemControl.hdr Core/VSHControl/vshctrl.prx VshControl 0x3007
	$(PY) build-tools/pspgz/pspgz.py dist/flash0/ark_xmbctrl.prx build-tools/pspgz/UserModule.hdr Core/XMBControl/xmbctrl.prx XmbControl 0x0000
	$(PY) build-tools/pspgz/pspgz.py dist/flash0/ark_inferno.prx build-tools/pspgz/SystemControl.hdr Core/Inferno/inferno.prx PRO_Inferno_Driver 0x3007
	$(PY) build-tools/pspgz/pspgz.py dist/flash0/ark_popcorn.prx build-tools/pspgz/SystemControl.hdr Core/PopCorn/popcorn.prx PROPopcornManager 0x3007
	$(PY) build-tools/pspgz/pspgz.py dist/flash0/ark_stargate.prx build-tools/pspgz/SystemControl.hdr Core/Stargate/stargate.prx Stargate 0x3007
	$(PY) build-tools/pspgz/pspgz.py dist/flash0/ark_pspcompat.prx build-tools/pspgz/SystemControl.hdr Core/Compat/PSP/pspcompat.prx PSPCompat 0x3007
	$(PY) build-tools/pspgz/pspgz.py dist/flash0/ark_vitacompat.prx build-tools/pspgz/SystemControl.hdr Core/Compat/ePSP/vitacompat.prx VitaCompat 0x3007
	$(PY) build-tools/pspgz/pspgz.py dist/flash0/ark_vitapops.prx build-tools/pspgz/SystemControl.hdr Core/Compat/ePSX/vitapops.prx VitaPopsCompat 0x3007
	$(PY) build-tools/pspgz/pspgz.py dist/flash0/ark_vitaplus.prx build-tools/pspgz/SystemControl.hdr Core/Compat/vPSP/pentazemin.prx Pentazemin 0x3007
	$(PY) build-tools/pack/pack.py -p dist/FLASH0.ARK build-tools/pack/flash0.txt -s


clean:
	# SystemControl
	$(Q)rm -f Core/SystemControl/external/include/*.h
	$(Q)rm -f Core/SystemControl/external/libs/*.a
	$(Q)rm -f Core/SystemControl/external/src/*.c
	$(MAKE) -C Core/SystemControl clean
	# VSHControl
	$(Q)rm -f Core/VSHControl/external/include/*.h
	$(Q)rm -f Core/VSHControl/external/libs/*.a
	$(MAKE) -C Core/VSHControl clean
	# XMBControl
	$(Q)rm -f Core/XMBControl/external/include/*.h
	$(Q)rm -f Core/XMBControl/external/libs/*.a
	$(MAKE) -C Core/XMBControl clean
	# Inferno
	$(Q)rm -f Core/Inferno/external/include/*.h
	$(Q)rm -f Core/Inferno/external/libs/*.a
	$(MAKE) -C Core/Inferno clean
	# PopCorn
	$(Q)rm -f Core/PopCorn/external/include/*.h
	$(Q)rm -f Core/PopCorn/external/libs/*.a
	$(MAKE) -C Core/PopCorn clean
	# Stargate
	$(Q)rm -f Core/Stargate/external/include/*.h
	$(Q)rm -f Core/Stargate/external/libs/*.a
	$(MAKE) -C Core/Stargate clean
	# PSPCompat
	$(Q)rm -f Core/Compat/PSP/external/include/*.h
	$(Q)rm -f Core/Compat/PSP/external/libs/*.a
	$(MAKE) -C Core/Compat/PSP clean
	$(MAKE) REBOOTEXDIR="$(CURDIR)/Libs/BootLoadEx" -C Core/Compat/PSP/rebootex clean
	# VitaCompat
	$(Q)rm -f Core/Compat/ePSP/external/include/*.h
	$(Q)rm -f Core/Compat/ePSP/external/libs/*.a
	$(MAKE) -C Core/Compat/ePSP clean
	$(MAKE) REBOOTEXDIR="$(CURDIR)/Libs/BootLoadEx" -C Core/Compat/ePSP/rebootex clean
	# VitaPopsCompat
	$(Q)rm -f Core/Compat/ePSX/external/include/*.h
	$(Q)rm -f Core/Compat/ePSX/external/libs/*.a
	$(MAKE) -C Core/Compat/ePSX clean
	$(MAKE) REBOOTEXDIR="$(CURDIR)/Libs/BootLoadEx" -C Core/Compat/ePSX/rebootex clean
	# Pentazemin
	$(Q)rm -f Core/Compat/vPSP/external/include/*.h
	$(Q)rm -f Core/Compat/vPSP/external/libs/*.a
	$(MAKE) -C Core/Compat/vPSP clean
	$(MAKE) REBOOTEXDIR="$(CURDIR)/Libs/BootLoadEx" -C Core/Compat/vPSP/rebootex clean
	# Libs
	$(MAKE) -C Libs/ark-dev-sdk clean
	$(MAKE) -C Libs/BootLoadEx clean
	# Rest
	$(Q)rm -rf dist