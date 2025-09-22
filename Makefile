PY = $(shell which python3)
ARKSDK = $(CURDIR)/Libs/ark-dev-sdk
BOOTLOADEX = $(CURDIR)/Libs/BootLoadEx

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
	$(MAKE) ARKSDK="$(ARKSDK)" -C Core/SystemControl

VSHControl: libraries
	$(MAKE) ARKSDK="$(ARKSDK)" -C Core/VSHControl

XMBControl: libraries
	$(MAKE) ARKSDK="$(ARKSDK)" -C Core/XMBControl

Inferno: libraries
	$(MAKE) ARKSDK="$(ARKSDK)" -C Core/Inferno

PopCorn: libraries
	$(MAKE) ARKSDK="$(ARKSDK)" -C Core/PopCorn

Stargate: libraries
	$(MAKE) ARKSDK="$(ARKSDK)" -C Core/Stargate

PSPCompat: libraries
	$(MAKE) ARKSDK="$(ARKSDK)" BOOTLOADEX="$(BOOTLOADEX)" -C Core/Compat/PSP/rebootex
	$(MAKE) ARKSDK="$(ARKSDK)" -C Core/Compat/PSP/

VitaCompat: libraries
	$(MAKE) ARKSDK="$(ARKSDK)" BOOTLOADEX="$(BOOTLOADEX)" -C Core/Compat/ePSP/rebootex
	$(MAKE) ARKSDK="$(ARKSDK)" -C Core/Compat/ePSP/
	$(PY) $(ARKSDK)/build-tools/btcnf.py build Core/Compat/ePSP/btcnf/psvbtcnf.txt
	$(PY) $(ARKSDK)/build-tools/btcnf.py build Core/Compat/ePSP/btcnf/psvbtinf.txt
	$(Q)mv Core/Compat/ePSP/btcnf/*.bin dist/flash0/

VitaPopsCompat: libraries
	$(MAKE) ARKSDK="$(ARKSDK)" BOOTLOADEX="$(BOOTLOADEX)" -C Core/Compat/ePSX/rebootex
	$(MAKE) ARKSDK="$(ARKSDK)" -C Core/Compat/ePSX/
	$(PY) $(ARKSDK)/build-tools/btcnf.py build Core/Compat/ePSX/btcnf/psxbtcnf.txt
	$(Q)mv Core/Compat/ePSX/btcnf/*.bin dist/flash0/

Pentazemin: libraries
	$(MAKE) ARKSDK="$(ARKSDK)" BOOTLOADEX="$(BOOTLOADEX)" -C Core/Compat/vPSP/rebootex
	$(MAKE) ARKSDK="$(ARKSDK)" -C Core/Compat/vPSP/
	$(PY) $(ARKSDK)/build-tools/btcnf.py build Core/Compat/vPSP/btcnf/psvbtjnf.txt
	$(PY) $(ARKSDK)/build-tools/btcnf.py build Core/Compat/vPSP/btcnf/psvbtknf.txt
	$(Q)mv Core/Compat/vPSP/btcnf/*.bin dist/flash0/

FlashPackage: mkdist \
	SystemControl VSHControl XMBControl Inferno PopCorn Stargate \
	PSPCompat VitaCompat VitaPopsCompat Pentazemin
	$(PY) $(ARKSDK)/build-tools/gz/pspgz.py dist/flash0/ark_systemctrl.prx $(ARKSDK)/build-tools/gz/SystemControl.hdr Core/SystemControl/systemctrl.prx SystemControl 0x3007
	$(PY) $(ARKSDK)/build-tools/gz/pspgz.py dist/flash0/ark_vshctrl.prx $(ARKSDK)/build-tools/gz/SystemControl.hdr Core/VSHControl/vshctrl.prx VshControl 0x3007
	$(PY) $(ARKSDK)/build-tools/gz/pspgz.py dist/flash0/ark_xmbctrl.prx $(ARKSDK)/build-tools/gz/UserModule.hdr Core/XMBControl/xmbctrl.prx XmbControl 0x0000
	$(PY) $(ARKSDK)/build-tools/gz/pspgz.py dist/flash0/ark_inferno.prx $(ARKSDK)/build-tools/gz/SystemControl.hdr Core/Inferno/inferno.prx PRO_Inferno_Driver 0x3007
	$(PY) $(ARKSDK)/build-tools/gz/pspgz.py dist/flash0/ark_popcorn.prx $(ARKSDK)/build-tools/gz/SystemControl.hdr Core/PopCorn/popcorn.prx PROPopcornManager 0x3007
	$(PY) $(ARKSDK)/build-tools/gz/pspgz.py dist/flash0/ark_stargate.prx $(ARKSDK)/build-tools/gz/SystemControl.hdr Core/Stargate/stargate.prx Stargate 0x3007
	$(PY) $(ARKSDK)/build-tools/gz/pspgz.py dist/flash0/ark_pspcompat.prx $(ARKSDK)/build-tools/gz/SystemControl.hdr Core/Compat/PSP/pspcompat.prx PSPCompat 0x3007
	$(PY) $(ARKSDK)/build-tools/gz/pspgz.py dist/flash0/ark_vitacompat.prx $(ARKSDK)/build-tools/gz/SystemControl.hdr Core/Compat/ePSP/vitacompat.prx VitaCompat 0x3007
	$(PY) $(ARKSDK)/build-tools/gz/pspgz.py dist/flash0/ark_vitapops.prx $(ARKSDK)/build-tools/gz/SystemControl.hdr Core/Compat/ePSX/vitapops.prx VitaPopsCompat 0x3007
	$(PY) $(ARKSDK)/build-tools/gz/pspgz.py dist/flash0/ark_vitaplus.prx $(ARKSDK)/build-tools/gz/SystemControl.hdr Core/Compat/vPSP/pentazemin.prx Pentazemin 0x3007
	$(PY) $(ARKSDK)/build-tools/pack/pack.py -p dist/FLASH0.ARK $(ARKSDK)/build-tools/pack/flash0.txt -s


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
	$(MAKE) BOOTLOADEX="$(BOOTLOADEX)" -C Core/Compat/PSP/rebootex clean
	# VitaCompat
	$(MAKE) -C Core/Compat/ePSP clean
	$(MAKE) BOOTLOADEX="$(BOOTLOADEX)" -C Core/Compat/ePSP/rebootex clean
	# VitaPopsCompat
	$(MAKE) -C Core/Compat/ePSX clean
	$(MAKE) BOOTLOADEX="$(BOOTLOADEX)" -C Core/Compat/ePSX/rebootex clean
	# Pentazemin
	$(MAKE) -C Core/Compat/vPSP clean
	$(MAKE) BOOTLOADEX="$(BOOTLOADEX)" -C Core/Compat/vPSP/rebootex clean
	# BootLoadEx
	$(MAKE) -C Libs/BootLoadEx clean
	# Rest
	$(Q)rm -rf dist

fullclean: clean
	# Libs
	$(MAKE) -C Libs/ark-dev-sdk clean
