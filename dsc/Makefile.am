ACLOCAL_AMFLAGS =  -I m4
AM_CFLAGS = -Wall
AM_CPPFLAGS = -I$(top_srcdir)/dsc

nobase_include_HEADERS = paf/dsc.h
INTERNAL_INCLUDES = hwcap.h \
			common.h

lib_LTLIBRARIES = libpaf-dsc.la
libpaf_dsc_la_SOURCES = init.c \
			dsc.c \
			hwcap.c \
			$(include_HEADERS) \
			$(INTERNAL_INCLUDES)
libpaf_dsc_la_LIBADD = $(LIBAUXV)

TESTS = test_dsc
test_dsc_SOURCES = tests/test_dsc.c
test_dsc_LDADD = libpaf-dsc.la

noinst_PROGRAMS = $(TESTS)
