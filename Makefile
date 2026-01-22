PY = $(shell which python3)
PSPDEV = $(shell psp-config --pspdev-path)
BUILDTOOLS = $(PSPDEV)/share/psp-cfw-sdk/build-tools

.PHONY : mkdist FlashPackage \
	SystemControl VSHControl XMBControl Inferno PopCorn Stargate \
	PSPCompat VitaCompat VitaPopsCompat VitaPlusCompat


all: mkdist FlashPackage
	$(Q)echo "Build Done"


mkdist:
	$(Q)mkdir -p dist
	$(Q)mkdir -p dist/flash0

SystemControl: 
	$(MAKE) -C Core/SystemControl

VSHControl: 
	$(MAKE) -C Core/VSHControl

XMBControl: 
	$(MAKE) -C Core/XMBControl

Inferno: 
	$(MAKE) -C Core/Inferno

PopCorn: 
	$(MAKE) -C Core/PopCorn

Stargate: 
	$(MAKE) -C Core/Stargate

PSPCompat: 
	$(MAKE) -C Core/Compat/PSP/rebootex
	$(MAKE) -C Core/Compat/PSP/

VitaCompat: 
	$(MAKE) -C Core/Compat/ePSP/rebootex
	$(MAKE) -C Core/Compat/ePSP/
	$(PY) $(BUILDTOOLS)/btcnf.py build Core/Compat/ePSP/btcnf/psvbtcnf.txt
	$(PY) $(BUILDTOOLS)/btcnf.py build Core/Compat/ePSP/btcnf/psvbtinf.txt
	$(Q)mv Core/Compat/ePSP/btcnf/*.bin dist/flash0/

VitaPopsCompat: 
	$(MAKE) -C Core/Compat/ePSX/rebootex
	$(MAKE) -C Core/Compat/ePSX/
	$(PY) $(BUILDTOOLS)/btcnf.py build Core/Compat/ePSX/btcnf/psxbtcnf.txt
	$(Q)mv Core/Compat/ePSX/btcnf/*.bin dist/flash0/

VitaPlusCompat: 
	$(MAKE) -C Core/Compat/vPSP/rebootex
	$(MAKE) -C Core/Compat/vPSP/
	$(PY) $(BUILDTOOLS)/btcnf.py build Core/Compat/vPSP/btcnf/psvbtjnf.txt
	$(PY) $(BUILDTOOLS)/btcnf.py build Core/Compat/vPSP/btcnf/psvbtknf.txt
	$(Q)mv Core/Compat/vPSP/btcnf/*.bin dist/flash0/

FlashPackage: mkdist \
	SystemControl VSHControl XMBControl Inferno PopCorn Stargate \
	PSPCompat VitaCompat VitaPopsCompat VitaPlusCompat
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_systemctrl.prx $(BUILDTOOLS)/gz/SystemControl.hdr Core/SystemControl/systemctrl.prx SystemControl 0x3007
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_vshctrl.prx $(BUILDTOOLS)/gz/SystemControl.hdr Core/VSHControl/vshctrl.prx VshControl 0x3007
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_xmbctrl.prx $(BUILDTOOLS)/gz/UserModule.hdr Core/XMBControl/xmbctrl.prx XmbControl 0x0000
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_inferno.prx $(BUILDTOOLS)/gz/SystemControl.hdr Core/Inferno/inferno.prx PRO_Inferno_Driver 0x3007
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_popcorn.prx $(BUILDTOOLS)/gz/SystemControl.hdr Core/PopCorn/popcorn.prx PROPopcornManager 0x3007
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_stargate.prx $(BUILDTOOLS)/gz/SystemControl.hdr Core/Stargate/stargate.prx Stargate 0x3007
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_pspcompat.prx $(BUILDTOOLS)/gz/SystemControl.hdr Core/Compat/PSP/pspcompat.prx PSPCompat 0x3007
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_vitacompat.prx $(BUILDTOOLS)/gz/SystemControl.hdr Core/Compat/ePSP/vitacompat.prx VitaCompat 0x3007
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_vitapops.prx $(BUILDTOOLS)/gz/SystemControl.hdr Core/Compat/ePSX/vitapops.prx VitaPopsCompat 0x3007
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_vitaplus.prx $(BUILDTOOLS)/gz/SystemControl.hdr Core/Compat/vPSP/vitaplus.prx VitaPlusCompat 0x3007
	$(PY) $(BUILDTOOLS)/pack/pack.py -p dist/FLASH0.ARK flash0.txt -s


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
	$(MAKE) -C Core/Compat/PSP/rebootex clean
	# VitaCompat
	$(MAKE) -C Core/Compat/ePSP clean
	$(MAKE) -C Core/Compat/ePSP/rebootex clean
	# VitaPopsCompat
	$(MAKE) -C Core/Compat/ePSX clean
	$(MAKE) -C Core/Compat/ePSX/rebootex clean
	# VitaPlusCompat
	$(MAKE) -C Core/Compat/vPSP clean
	$(MAKE) -C Core/Compat/vPSP/rebootex clean
	# Rest
	$(Q)rm -rf dist
