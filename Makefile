PY = $(shell which python3)
PSPDEV = $(shell psp-config --pspdev-path)
BUILDTOOLS = $(PSPDEV)/share/psp-cfw-sdk/build-tools

.PHONY : Core dist


all: dist
	$(Q)echo "Build Done"


Core: 
	# SystemControl
	$(MAKE) -C SystemControl
	# VSHControl
	$(MAKE) -C VSHControl
	# XMBCOntrol
	$(MAKE) -C XMBControl
	# Inferno
	$(MAKE) -C Inferno
	# PopCorn
	$(MAKE) -C PopCorn
	# Stargate
	$(MAKE) -C Stargate
	# PSP Compat
	$(MAKE) -C Compat/PSP/rebootex
	$(MAKE) -C Compat/PSP/
	# ePSP Compat
	$(MAKE) -C Compat/ePSP/rebootex
	$(MAKE) -C Compat/ePSP/
	$(PY) $(BUILDTOOLS)/btcnf.py build Compat/ePSP/btcnf/psvbtcnf.txt
	$(PY) $(BUILDTOOLS)/btcnf.py build Compat/ePSP/btcnf/psvbtinf.txt
	# ePSX Compat
	$(MAKE) -C Compat/ePSX/rebootex
	$(MAKE) -C Compat/ePSX/
	$(PY) $(BUILDTOOLS)/btcnf.py build Compat/ePSX/btcnf/psxbtcnf.txt
	# vPSP Compat
	$(MAKE) -C Compat/vPSP/rebootex
	$(MAKE) -C Compat/vPSP/
	$(PY) $(BUILDTOOLS)/btcnf.py build Compat/vPSP/btcnf/psvbtjnf.txt
	$(PY) $(BUILDTOOLS)/btcnf.py build Compat/vPSP/btcnf/psvbtknf.txt


dist: Core
	$(Q)mkdir -p dist
	$(Q)mkdir -p dist/flash0
	$(Q)mv Compat/ePSP/btcnf/*.bin dist/flash0/
	$(Q)mv Compat/ePSX/btcnf/*.bin dist/flash0/
	$(Q)mv Compat/vPSP/btcnf/*.bin dist/flash0/
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_systemctrl.prx $(BUILDTOOLS)/gz/SystemControl.hdr SystemControl/systemctrl.prx SystemControl 0x3007
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_vshctrl.prx $(BUILDTOOLS)/gz/SystemControl.hdr VSHControl/vshctrl.prx VshControl 0x3007
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_xmbctrl.prx $(BUILDTOOLS)/gz/UserModule.hdr XMBControl/xmbctrl.prx XmbControl 0x0000
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_inferno.prx $(BUILDTOOLS)/gz/SystemControl.hdr Inferno/inferno.prx PRO_Inferno_Driver 0x3007
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_popcorn.prx $(BUILDTOOLS)/gz/SystemControl.hdr PopCorn/popcorn.prx PROPopcornManager 0x3007
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_stargate.prx $(BUILDTOOLS)/gz/SystemControl.hdr Stargate/stargate.prx Stargate 0x3007
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_pspcompat.prx $(BUILDTOOLS)/gz/SystemControl.hdr Compat/PSP/pspcompat.prx PSPCompat 0x3007
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_vitacompat.prx $(BUILDTOOLS)/gz/SystemControl.hdr Compat/ePSP/vitacompat.prx VitaCompat 0x3007
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_vitapops.prx $(BUILDTOOLS)/gz/SystemControl.hdr Compat/ePSX/vitapops.prx VitaPopsCompat 0x3007
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/flash0/ark_vitaplus.prx $(BUILDTOOLS)/gz/SystemControl.hdr Compat/vPSP/vitaplus.prx VitaPlusCompat 0x3007
	$(PY) $(BUILDTOOLS)/pack/pack.py -p dist/FLASH0.ARK flash0.txt -s


clean:
	# SystemControl
	$(MAKE) -C SystemControl clean
	# VSHControl
	$(MAKE) -C VSHControl clean
	# XMBControl
	$(MAKE) -C XMBControl clean
	# Inferno
	$(MAKE) -C Inferno clean
	# PopCorn
	$(MAKE) -C PopCorn clean
	# Stargate
	$(MAKE) -C Stargate clean
	# PSPCompat
	$(MAKE) -C Compat/PSP clean
	$(MAKE) -C Compat/PSP/rebootex clean
	# VitaCompat
	$(MAKE) -C Compat/ePSP clean
	$(MAKE) -C Compat/ePSP/rebootex clean
	# VitaPopsCompat
	$(MAKE) -C Compat/ePSX clean
	$(MAKE) -C Compat/ePSX/rebootex clean
	# VitaPlusCompat
	$(MAKE) -C Compat/vPSP clean
	$(MAKE) -C Compat/vPSP/rebootex clean
	# Rest
	$(Q)rm -rf dist
