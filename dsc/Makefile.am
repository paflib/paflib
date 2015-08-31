AM_CFLAGS = -Wall -Werror
AM_LDFLAGS = -rdynamic
AM_CPPFLAGS = -I$(top_srcdir)/common -I$(top_srcdir)/dsc

nobase_include_HEADERS = paf/dsc.h
INTERNAL_INCLUDES = hwcap.h

lib_LTLIBRARIES = libpaf-dsc.la
libpaf_dsc_la_SOURCES = init.c \
                        dsc.c \
                        hwcap.c \
                        dsc-version.c \
                        $(include_HEADERS) \
                        $(INTERNAL_INCLUDES)
libpaf_dsc_la_CFLAGS = -Wl,-entry=__paflib_dsc_main
libpaf_dsc_la_LIBADD = $(LIBAUXV)
libpaf_dsc_la_LDFLAGS = -version-info $(LIBPAF_SO_VERSION) \
  -Wl,--whole-archive,../common/libpaf-common.a,--no-whole-archive

dist_man3_MANS = doc/libpaf-dsc.3

TESTS = test_dsc
test_dsc_SOURCES = tests/test_dsc.c
test_dsc_LDADD = libpaf-dsc.la

check_PROGRAMS = $(TESTS)
