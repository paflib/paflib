## Process this file with automake to produce Makefile.in

# Make sure that when we re-make ./configure, we get the macros we need
ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = -Wall
AM_LDFLAGS = -rdynamic
AM_CPPFLAGS = -I$(top_srcdir)/ebb

include_HEADERS = ppcebb.h
LIBPPCEBB_PRIVATE_INCLUDE = ppcebb-callback.h \
			    ppcebb-common.h \
			    ppcebb-hwcap.h \
			    ppcebb-init.h \
			    ppcebb-priv.h \
			    ppcebb-sysdep.h
lib_LTLIBRARIES = libppcebb.la
libppcebb_la_SOURCES = ppcebb.c \
                       ppcebb-init.c \
                       ppcebb-hwcap.c \
                       ppcebb-hook.c \
                       $(include_HEADERS) \
		       $(LIBPPCEBB_PRIVATE_INCLUDE)
if POWERPC64
libppcebb_la_SOURCES += ppcebb-callback-ppc64.S
else
libppcebb_la_SOURCES += ppcebb-callback-ppc32.S
endif

man3_MANS = doc/libppcebb.3

# Test programs invoked with 'make check'
TESTS =
TESTS               += test_ppcebb
test_ppcebb_SOURCES  = tests/test_ppcebb.c
test_ppcebb_LDADD    = libppcebb.la

TESTS                   += test_ppcebb_pmu
test_ppcebb_pmu_SOURCES  = tests/test_ppcebb_pmu.c \
                           tests/test_ppcebb_common.c
test_ppcebb_pmu_LDADD    = libppcebb.la

TESTS                         += test_ppcebb_pmu_reset
test_ppcebb_pmu_reset_SOURCES  = tests/test_ppcebb_pmu_reset.c \
                                 tests/test_ppcebb_common.c
test_ppcebb_pmu_reset_LDADD    = libppcebb.la

TESTS                         += test_ppcebb_backtrace
test_ppcebb_backtrace_SOURCES  = tests/test_ppcebb_backtrace.c \
                                 tests/test_ppcebb_common.c
test_ppcebb_backtrace_LDADD    = libppcebb.la

TESTS                         += test_ppcebb_pmu_multi
test_ppcebb_pmu_multi_SOURCES  = tests/test_ppcebb_pmu_multi.c \
                                 tests/test_ppcebb_common.c
test_ppcebb_pmu_multi_LDADD    = libppcebb.la

noinst_PROGRAMS = $(TESTS)
