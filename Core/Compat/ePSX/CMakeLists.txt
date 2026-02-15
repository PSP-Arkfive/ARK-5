add_compat_prx(vitapops vitapops
   SRC_FILES
    main.c
    src/syspatch.c
    src/popsdisplay.c
    src/vitaflash.c
    payload.h
   BTCNF_FILES
      psxbtcnf.bin
   LINK_LIBS
      pspkermit_driver
      pspkermitperipheral_driver
)
