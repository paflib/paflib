ACLOCAL_AMFLAGS =  -I m4
AM_CFLAGS = -Wall
AM_CPPFLAGS = -I$(top_srcdir)/dsc

include_HEADERS = ppcdsc.h
INTERNAL_INCLUDES = hwcap.h \
			common.h

lib_LTLIBRARIES = libppcdsc.la
libppcdsc_la_SOURCES = init.c \
			ppcdsc.c \
			hwcap.c \
			$(include_HEADERS) \
			$(INTERNAL_INCLUDES)
libppcdsc_la_LIBADD = $(LIBAUXV)

TESTS = test_ppcdsc
test_ppcdsc_SOURCES = tests/test_ppcdsc.c
test_ppcdsc_LDADD = libppcdsc.la

noinst_PROGRAMS = $(TESTS)
