add_compat_prx(pspcompat psp
   SRC_FILES
      main.c
      src/syspatch.c
      src/wpa2.c
      src/cwpatch.c
      src/high_mem.c
      src/libertas.c
      src/usb_charge.c
      src/region_free.c
      src/pops4tool.c
      imports.S
      payload.h
   BTCNF_FILES
      pstbtcnf_dt.bin
      pstbtcnf_tt.bin
)
