## Process this file with autoconf to produce configure.
## In general, the safest way to proceed is to run ./autogen.sh

# These are m4 macros because autoconf won't allow non-statically determined
# variables in AC_INIT.   We put these here because libtool mixes up the order
# in the -version-info statement.
m4_define([MAJOR], 0)
m4_define([MINOR], 1)
m4_define([PATCH], 0)

AC_PREREQ([2.61])
AC_INIT([libpaf], [MAJOR.MINOR.PATCH], [azanella@br.ibm.com emachado@br.ibm.com])
AM_INIT_AUTOMAKE([1.10 no-define foreign -Wall -Werror subdir-objects])
AC_CANONICAL_HOST

AC_CONFIG_MACRO_DIR([m4])

# Update this value for every release: (A:B:C will map to foo.so.(A-C).C.B)
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
LIBPAF_SO_VERSION=MAJOR:MINOR:PATCH

AC_SUBST(LIBPAF_SO_VERSION)

AC_CONFIG_SRCDIR([ebb/ebb.c])

# We want to prevent building/configuration in the source directory
if test "`cd $srcdir; /bin/pwd`" = "`/bin/pwd`"; then
	AC_MSG_ERROR([you must configure in a separate build directory])
fi

# Check for supported platforms (currently only Linux on POWER)
case "$host_cpu-$host_os" in
  powerpc64*-linux* | ppc64*-linux* | powerpc32*-linux | ppc32*-linux* )
    # Supported platforms
  ;;
  *)
    AC_MSG_ERROR([Unsupported architecture/os])
  ;;
esac

AC_ARG_ENABLE([debug],
	      AS_HELP_STRING([--enable-debug],
			     [enable debug @<:@not default@:>@]),
	      [enable_debug=$enableval],
	      [enable_debug=no])
if test $enable_debug = yes; then
  AC_DEFINE([ENABLE_DEBUG], 1, [Enable debug])
fi

AC_ARG_ENABLE(dsc,
	      AS_HELP_STRING([--disable-dsc],
			     [disable DSC library build @<:@not default@:>@]),
	      [enable_dsc=$enableval],
	      [enable_dsc=yes]
)

AC_ARG_ENABLE(ebb,
	      AS_HELP_STRING([--disable-ebb],
			     [disable EBB library build @<:@not default@:>@]),
	      [enable_ebb=$enableval],
	      [enable_ebb=yes]
)

if test $enable_ebb != yes; then
  if test $enable_dsc != yes; then
    AC_MSG_ERROR([All the libraries are disabled. At least one library have to be enabled.])
  fi
fi

AM_CONDITIONAL([DSC], [test "$enable_dsc" = yes])
AM_CONDITIONAL([EBB], [test "$enable_ebb" = yes])

# Checkings for libpaf-ebb
AC_ARG_ENABLE([save-area],
	      AS_HELP_STRING([--enable-save-area=AREA],
			     [define the default EBB internal save area\
@<:@runtime default@:>@]),
              [save_area=$enableval],
              [save_area=runtime])
if test $save_area = tcb; then
   AC_DEFINE([USE_EBB_TCB], 1, [Define to 1 to force use of GLIBC TCB EBB])
elif test $save_area = tls; then
  AC_DEFINE([USE_EBB_TLS], 1, [Define to 1 to force use of __thread vars])
fi

AC_CHECK_PROG(AWK, gawk)

# Check if build is for 32 or 64 bits, Used in Makefile.am to select the
# callback implementation
AC_CACHE_CHECK([if GCC sets __powerpc64__], paflib_cv_powerpc64, [dnl
cat > conftest.c <<EOF
void foo () {}
EOF
if ${CC-cc} $CFLAGS $CPPFLAGS -E -dD conftest.c -o - 2>&1 | \
grep __powerpc64__ > /dev/null; then
  paflib_cv_powerpc64=yes
else
  paflib_cv_powerpc64=no
fi
rm -f conftest*])
if test $paflib_cv_powerpc64 = yes; then
  AM_CONDITIONAL(POWERPC64, [true])
else
  AM_CONDITIONAL(POWERPC64, [false])
fi

# Check which is the loader
paflib_cv_loader=`
cat > conftest.c <<EOF
int main () {}
EOF
${CC-cc} $CFLAGS $CPPFLAGS -o conftest conftest.c && ldd conftest | $AWK '
BEGIN {
  loader=$0 ~ "ld.*so"
}
{
  gsub (/[[[:space:]]]*/, "", $loader)
  split ($loader, fields, "[[=(]]")
}
END {
  print fields[[1]]
}'
rm -f conftest*`
AC_DEFINE_UNQUOTED([PAFLIB_RUNTIME_LOADER], ["$paflib_cv_loader"],
                   [The loader to use in direct DSO call])

# Checkings for libpaf-dsc
AC_LANG(C)

AC_CHECK_HEADER(pthread.h, [],
    AC_MSG_ERROR([<pthread.h> not found.  libppcdsc requires libpthread.]))
AC_CHECK_LIB([pthread], [pthread_create], [],
    AC_MSG_ERROR([pthread_create() not found.  libppsdsc requires libpthread.]))

# Checks for programs
AM_PROG_AS
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LN_S

# Fixes the warning for automake >= 1.12
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

AC_PROG_LIBTOOL
LT_INIT

# Checks for header files
AC_CONFIG_HEADERS([config.h])
AC_CHECK_HEADERS([signal.h errno.h dlfnc.h unistd.h link.h gnu/libc-version.h \
linux/perf_event.h])

# Checks for typedefs, structures, and compiler characteristics
AC_C_VOLATILE

AC_CONFIG_FILES([Makefile ebb/Makefile dsc/Makefile])
AC_OUTPUT
