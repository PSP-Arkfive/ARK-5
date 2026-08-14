add_compat_prx(vitapops vitapops
   SRC_FILES
    main.c
    src/syspatch.c
    src/vitaflash.c
    payload.h
   BTCNF_FILES
      psxbtcnf.bin
   LINK_LIBS
      popsdisplay
      pspkermit_driver
      pspkermitperipheral_driver
)
