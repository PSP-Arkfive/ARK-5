add_prx_module(systemctrl exports.exp)

target_sources(systemctrl PRIVATE 
   main.c
	src/configparser.c
	src/controller.c
	src/cpuclock.c
	src/cryptography.c
	src/dummy.c
	src/elf.c
	src/exitgame.c
	src/flushcache.c
	src/gameinfo.c
	src/hooknids.c
	src/init.c
	src/interruptman.c
	src/kubridge.c
	src/leda.c
	src/loadercore.c
	src/loadexec.c
	src/mediasync.c
	src/missingfunc.c
	src/modulemanager.c
	src/msstor_cache.c
	src/nid_660_data.c
	src/nidresolver.c
	src/np9660.c
	src/oe_malloc.c
	src/patches.c
	src/pgd.c
	src/plugin.c
	src/psnfix.c
	src/rebootex.c
	src/sctrl.c
	src/sctrl_hen.c
	src/sctrl_se.c
	src/setlongjmp.S
	src/settings.c
	src/sysmem.c
	src/syspatch.c
	src/threadman.c
	src/utility.c
)

target_compile_options(systemctrl PRIVATE ${COMPILE_OPS})
target_include_directories(systemctrl PRIVATE include)
target_link_libraries(systemctrl PRIVATE
   -nostartfiles
	microlz
   pspsemaphore_660
   pspinit_kernel_660
   pspsysmem_kernel_660
   pspsysclib_kernel_660
   psploadcore_kernel_660
   pspmodulemgr_kernel_660
   pspthreadman_kernel_660
   pspinterruptmanager_kernel_660
   pspsdk
   pspmodinfo
   pspkernel
)

