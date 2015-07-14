## Process this file with autoconf to produce configure.
## In general, the safest way to proceed is to run ./autogen.sh

# These are m4 macros because autoconf won't allow non-statically determined
# variables in AC_INIT.   We put these here because libtool mixes up the order
# in the -version-info statement.
m4_define([MAJOR], 0)
m4_define([MINOR], 2)
m4_define([PATCH], 0)

AC_PREREQ([2.61])
AC_INIT([libpaf], [MAJOR.MINOR.PATCH], [tulioqm@br.ibm.com emachado@br.ibm.com])
AM_INIT_AUTOMAKE([1.10 no-define foreign -Wall -Werror subdir-objects])
AC_CANONICAL_HOST
AM_MAINTAINER_MODE

AC_CONFIG_MACRO_DIR([m4])

# Update this value for every release: (A:B:C will map to foo.so.(A-C).C.B)
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
LIBPAF_SO_VERSION=MAJOR:MINOR:PATCH

AC_SUBST(LIBPAF_SO_VERSION)

AC_CONFIG_SRCDIR([ebb/ebb.c])

# Check for supported platforms (currently only Linux on POWER)
case "$host_cpu-$host_os" in
  powerpc64*-linux* | ppc64*-linux* | powerpc-linux* | ppc-linux* )
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
	      [AC_COMPILE_IFELSE(
		[AC_LANG_SOURCE([[
		  #ifdef _ARCH_PWR8
		  # define SUCCESS 1
		  #else
		  # error "Support for EBB facility isn't available"
		  #endif
		]])],
		[enable_ebb=yes],
		[enable_ebb=no])]
)
AC_ARG_ENABLE(tb,
              AS_HELP_STRING([--disable-tb],
                             [disable TIMEBASE build @<:@not default@:>@]),
              [enable_tb=$enableval],
              [enable_tb=yes]
)

if test $enable_ebb != yes; then
  if test $enable_dsc != yes; then
    AC_MSG_ERROR([All the libraries are disabled. At least one library have to be enabled.])
  fi
fi

AM_CONDITIONAL([DSC], [test "$enable_dsc" = yes])
AM_CONDITIONAL([EBB], [test "$enable_ebb" = yes])
AM_CONDITIONAL([TB], [test "$enable_tb" = yes])

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
int foo () {}
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
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Fixes the warning for automake >= 1.12
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

AC_PROG_LIBTOOL
LT_INIT

# Checks for header files
AC_CONFIG_HEADERS([config.h])
AC_CHECK_HEADERS([fcntl.h signal.h errno.h dlfnc.h unistd.h link.h \
gnu/libc-version.h \
linux/perf_event.h \
sys/ioctl.h])

# Checks for typedefs, structures, and compiler characteristics
AC_C_INLINE
AC_C_VOLATILE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([alarm getpagesize memset strchr strerror strncasecmp strstr \
strtol])

AC_CHECK_DECL([__NR_perf_event_open], [],
    AC_MSG_ERROR([__NR_perf_event_open not found.  Are you using Linux >= \
2.6.32?]),
    [[#include <sys/syscall.h>]])

AC_CONFIG_FILES([Makefile
		 common/Makefile
		 ebb/Makefile
		 dsc/Makefile
		 tb/Makefile])
AC_OUTPUT
