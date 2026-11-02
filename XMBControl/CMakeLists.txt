add_prx_module(xmbcontrol exports.exp)

target_sources(xmbcontrol PRIVATE 
   main.c
   stub.S
   src/battery.c
   src/configparser.c
   src/list.c
   src/plugins.c
   src/settings.c
   src/utils.c
   src/vshcube.c
   src/vshgu.c
   src/vshmenu.c
   src/xmbpatch.c
)

target_compile_options(xmbcontrol PRIVATE ${COMPILE_OPS} -Oz -fshort-wchar -mno-check-zero-division)
target_include_directories(xmbcontrol PRIVATE include)
target_link_libraries(xmbcontrol PRIVATE
   -nostartfiles
   pspgum
   pspgu
   pspmath
   pspvram
   pspcpaf
   arkctrl
   pspsystemctrl_user
   pspkubridge
   pspvshctrl
   pspreg
   psppower
   pspdebug
   pspdisplay
   pspge
   pspctrl
   pspnet
   pspnet_apctl
)
