PY = $(shell which python3)

.PHONY : full fullclean mkdist libraries FlashPackage \
	SystemControl VSHControl XMBControl Inferno PopCorn Stargate \
	PSPCompat VitaCompat VitaPopsCompat Pentazemin


all: mkdist FlashPackage
	$(Q)echo "Build Done"

full: libraries all


mkdist:
	$(Q)mkdir -p dist
	$(Q)mkdir -p dist/flash0

libraries:
	$(MAKE) -C Libs/ark-dev-sdk/

SystemControl: libraries
	$(MAKE) ARKSDK="$(CURDIR)/Libs/ark-dev-sdk" -C Core/SystemControl

VSHControl: libraries
	$(MAKE) ARKSDK="$(CURDIR)/Libs/ark-dev-sdk" -C Core/VSHControl

XMBControl: libraries
	$(MAKE) ARKSDK="$(CURDIR)/Libs/ark-dev-sdk" -C Core/XMBControl

Inferno: libraries
	$(MAKE) ARKSDK="$(CURDIR)/Libs/ark-dev-sdk" -C Core/Inferno

PopCorn: libraries
	$(MAKE) ARKSDK="$(CURDIR)/Libs/ark-dev-sdk" -C Core/PopCorn

Stargate: libraries
	$(MAKE) ARKSDK="$(CURDIR)/Libs/ark-dev-sdk" -C Core/Stargate

PSPCompat: libraries
	$(MAKE) ARKSDK="$(CURDIR)/Libs/ark-dev-sdk" REBOOTEXDIR="$(CURDIR)/Libs/BootLoadEx" -C Core/Compat/PSP/rebootex
	$(MAKE) ARKSDK="$(CURDIR)/Libs/ark-dev-sdk" -C Core/Compat/PSP/

VitaCompat: libraries
	$(MAKE) ARKSDK="$(CURDIR)/Libs/ark-dev-sdk" REBOOTEXDIR="$(CURDIR)/Libs/BootLoadEx" -C Core/Compat/ePSP/rebootex
	$(MAKE) ARKSDK="$(CURDIR)/Libs/ark-dev-sdk" -C Core/Compat/ePSP/
	$(PY) build-tools/btcnf.py build Core/Compat/ePSP/btcnf/psvbtcnf.txt
	$(PY) build-tools/btcnf.py build Core/Compat/ePSP/btcnf/psvbtinf.txt
	$(Q)mv Core/Compat/ePSP/btcnf/*.bin dist/flash0/

VitaPopsCompat: libraries
	$(MAKE) ARKSDK="$(CURDIR)/Libs/ark-dev-sdk" REBOOTEXDIR="$(CURDIR)/Libs/BootLoadEx" -C Core/Compat/ePSX/rebootex
	$(MAKE) ARKSDK="$(CURDIR)/Libs/ark-dev-sdk" -C Core/Compat/ePSX/
	$(PY) build-tools/btcnf.py build Core/Compat/ePSX/btcnf/psxbtcnf.txt
	$(Q)mv Core/Compat/ePSX/btcnf/*.bin dist/flash0/

Pentazemin: libraries
	$(MAKE) ARKSDK="$(CURDIR)/Libs/ark-dev-sdk" REBOOTEXDIR="$(CURDIR)/Libs/BootLoadEx" -C Core/Compat/vPSP/rebootex
	$(MAKE) ARKSDK="$(CURDIR)/Libs/ark-dev-sdk" -C Core/Compat/vPSP/
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
	$(MAKE) -C Core/SystemControl clean
	# VSHControl
	$(MAKE) -C Core/VSHControl clean
	# XMBControl
	$(MAKE) -C Core/XMBControl clean
	# Inferno
	$(MAKE) -C Core/Inferno clean
	# PopCorn
	$(MAKE) -C Core/PopCorn clean
	# Stargate
	$(MAKE) -C Core/Stargate clean
	# PSPCompat
	$(MAKE) -C Core/Compat/PSP clean
	$(MAKE) REBOOTEXDIR="$(CURDIR)/Libs/BootLoadEx" -C Core/Compat/PSP/rebootex clean
	# VitaCompat
	$(MAKE) -C Core/Compat/ePSP clean
	$(MAKE) REBOOTEXDIR="$(CURDIR)/Libs/BootLoadEx" -C Core/Compat/ePSP/rebootex clean
	# VitaPopsCompat
	$(MAKE) -C Core/Compat/ePSX clean
	$(MAKE) REBOOTEXDIR="$(CURDIR)/Libs/BootLoadEx" -C Core/Compat/ePSX/rebootex clean
	# Pentazemin
	$(MAKE) -C Core/Compat/vPSP clean
	$(MAKE) REBOOTEXDIR="$(CURDIR)/Libs/BootLoadEx" -C Core/Compat/vPSP/rebootex clean
	# BootLoadEx
	$(MAKE) -C Libs/BootLoadEx clean
	# Rest
	$(Q)rm -rf dist

fullclean: clean
	# Libs
	$(MAKE) -C Libs/ark-dev-sdk clean
