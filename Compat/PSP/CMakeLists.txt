add_compat_prx(pspcompat psp
   SRC_FILES
      main.c
      src/cwpatch.c
      src/high_mem.c
      src/libertas.c
      src/overclock.c
      src/pops4tool.c
      src/region_free.c
      src/syspatch.c
      src/usb_charge.c
      src/wpa2.c
      imports.S
      payload.h
   BTCNF_FILES
      pstbtcnf_dt.bin
      pstbtcnf_tt.bin
)
