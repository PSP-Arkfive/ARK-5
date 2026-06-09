add_compat_prx(vitaplus pentazemin
   SRC_FILES
      main.c
      src/syspatch.c
      imports.S
      payload.h
   BTCNF_FILES
      psvbtjnf.bin
      psvbtknf.bin
   LINK_LIBS
      pspkermit_driver
      pspkermitperipheral_driver
)
