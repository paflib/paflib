## Process this file with automake to produce Makefile.in
AM_CFLAGS = -Wall -rdynamic
AM_CPPFLAGS = -I$(top_srcdir)/common -I$(top_srcdir)/ebb

check_SCRIPTS = 
check_PROGRAMS =

nobase_include_HEADERS = paf/ebb.h
LIBPPCEBB_PRIVATE_INCLUDE = ebb-callback.h \
                            ebb-common.h \
                            ebb-hwcap.h \
                            ebb-init.h \
                            ebb-priv.h \
                            ebb-sysdep.h
lib_LTLIBRARIES = libpaf-ebb.la
LIBPPCEBB_COMMON_SOURCES = ../common/paf-hwcap.c \
                           ../common/paf-interp.c \
                           ../common/paf-common.h \
                           ../common/paf-hwcap.h
libpaf_ebb_la_SOURCES = ebb.c \
                        ebb-init.c \
                        ebb-hwcap.c \
                        ebb-hook.c \
                        ebb-version.c \
                        $(include_HEADERS) \
                        $(LIBPPCEBB_PRIVATE_INCLUDE) \
                        $(LIBPPCEBB_COMMON_SOURCES)
if POWERPC64
libpaf_ebb_la_SOURCES += ebb-callback-ppc64.S
else
libpaf_ebb_la_SOURCES += ebb-callback-ppc32.S
endif
libpaf_ebb_la_CFLAGS = -Wl,-entry=__paflib_ebb_main -fpic
libpaf_ebb_la_LDFLAGS = -version-info $(LIBPAF_SO_VERSION) \
-e __paflib_ebb_main

man3_MANS = doc/libpaf-ebb.3

# Test programs invoked with 'make check'
TESTS =

TESTS            += test_ebb
test_ebb_SOURCES  = tests/test_ebb.c
test_ebb_LDADD    = libpaf-ebb.la
check_PROGRAMS  += test_ebb

TESTS                             += test_ebb_save_area_env.sh$(EXEEXT)
test_ebb_save_area_env_sh_SOURCES  = tests/test_ebb_save_area_env.sh
test_ebb_save_area_env.sh$(EXEEXT): $(top_srcdir)/ebb/$(test_ebb_save_area_env_sh_SOURCES)
	rm -rf $@
	cp -p  $(top_srcdir)/ebb/$(test_ebb_save_area_env_sh_SOURCES) $@
check_SCRIPTS                    += $(test_ebb_save_area_env_sh_SOURCES)

TESTS                += test_ebb_pmu
test_ebb_pmu_SOURCES  = tests/test_ebb_pmu.c \
                        tests/test_ebb_common.c
test_ebb_pmu_LDADD    = libpaf-ebb.la
check_PROGRAMS      += test_ebb_pmu

TESTS                      += test_ebb_pmu_reset
test_ebb_pmu_reset_SOURCES  = tests/test_ebb_pmu_reset.c \
                              tests/test_ebb_common.c
test_ebb_pmu_reset_LDADD    = libpaf-ebb.la
check_PROGRAMS            += test_ebb_pmu_reset

TESTS                      += test_ebb_backtrace
test_ebb_backtrace_SOURCES  = tests/test_ebb_backtrace.c \
                              tests/test_ebb_common.c
test_ebb_backtrace_LDADD    = libpaf-ebb.la
check_PROGRAMS            += test_ebb_backtrace

TESTS                      += test_ebb_pmu_multi
test_ebb_pmu_multi_SOURCES  = tests/test_ebb_pmu_multi.c \
                              tests/test_ebb_common.c
test_ebb_pmu_multi_LDADD    = libpaf-ebb.la
check_PROGRAMS            += test_ebb_pmu_multi

TESTS                        += test_ebb_pmu_clobber
test_ebb_pmu_clobber_SOURCES  = tests/test_ebb_pmu_clobber.c \
                                tests/test_ebb_common.c
test_ebb_pmu_clobber_LDADD    = libpaf-ebb.la
check_PROGRAMS              += test_ebb_pmu_clobber

