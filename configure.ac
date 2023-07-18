AC_PREREQ(2.50)

AC_INIT(filtergen, 0.12.8, jaq@spacepants.org)

AC_CONFIG_SRCDIR(filtergen.c)
AM_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE([foreign dist-bzip2 parallel-tests color-tests])

AM_MAINTAINER_MODE

AC_SUBST(PROGRAM)
AC_SUBST(VERSION)

dnl --------------------------
dnl configuration success flag
dnl --------------------------

filtergen_config_ok=yes

dnl -------------------
dnl checks for programs
dnl -------------------

AC_PROG_CC
AM_PROG_LEX
AC_PROG_YACC

dnl -------------------
dnl check for libraries
dnl -------------------

AC_CHECK_HEADERS([getopt.h])

HAVE_GETOPT=no
AC_CHECK_LIB(getopt, getopt,
	HAVE_GETOPT=yes
	HAVE_GETOPT=no
)
if test "x$HAVE_GETOPT" = xyes ; then
	GETOPT_LIBS="-lgetopt"
	AC_SUBST(GETOPT_LIBS)
fi

AC_ARG_ENABLE(gcov,
	      AS_HELP_STRING([--enable-gcov],
                             [enable test coverage with gcov @<:@default=no@:>@]),
              [case "${enableval}" in
                 yes) enable_gcov=true ;;
                 no)  enable_gcov=false ;;
                 *)   AC_MSG_ERROR([bad value ${enableval} for --enable-gcov]) ;;
               esac],
              [enable_gcov=false])

if test "x$enable_gcov" = "xtrue"; then
  GCOV_CFLAGS="-fprofile-arcs -ftest-coverage"
  AC_SUBST(GCOV_CFLAGS)

  dnl libtool 1.5.22 and lower strip -fprofile-arcs from the flags
  dnl passed to the linker, which is a bug; -fprofile-arcs implicitly
  dnl links in -lgcov, so we do it explicitly here for the same effect
  GCOV_LIBS=-lgcov
  AC_SUBST(GCOV_LIBS)
fi

AM_CONDITIONAL(ENABLE_GCOV, test x"$gcov" = "xtrue")


# Google profiler
AC_MSG_CHECKING([whether to enable profiler])
AC_ARG_WITH([profiler],
  [AS_HELP_STRING([--with-profiler],[enable support for profiler [default=no]])],
  [with_profiler=$withval],
  [with_profiler=no]
)
AC_MSG_RESULT([$with_profiler])

if test "x${with_profiler}" = "xyes"; then
  AC_CHECK_LIB([profiler], [ProfilerStart],
    [AC_SUBST([LIBPROFILER], ["-lprofiler"])],
    [AC_MSG_FAILURE([check for profiler failed. Have you installed google-perftools-devel?])],
  )
fi

dnl -----------------
dnl set warning level
dnl -----------------

if test "x$ac_cv_c_compiler_gnu" = xyes ; then
	CFLAGS="$CFLAGS -W -Wall -Werror -Waggregate-return"
	CFLAGS="$CFLAGS -Wcast-align -Wcast-qual -Wnested-externs"
	CFLAGS="$CFLAGS -Wshadow -Wbad-function-cast -Wwrite-strings"
	CFLAGS="$CFLAGS -Winit-self -Wformat=2 -Wuninitialized"
	CFLAGS="$CFLAGS -Wpointer-arith"
	CFLAGS="$CFLAGS -Wstrict-aliasing -fstrict-aliasing"
fi

dnl ---------------
dnl set debug level
dnl ---------------

AC_MSG_CHECKING([whether to enable debugging compiler options])
AC_ARG_ENABLE(debug,
	[  --enable-debug            enable debugging compiler options],
	AC_MSG_RESULT(yes)
	tmp_CFLAGS=`echo $CFLAGS | sed 's/O2/O0/g'`
	CFLAGS="$tmp_CFLAGS",
	AC_MSG_RESULT(no)
)

AC_CHECK_PROGS(GCOV, gcov, false)
AC_CHECK_PROGS(LCOV, lcov, false)

dnl ------------------------------
dnl fill in path variable expandos
dnl ------------------------------

dnl $libdir usually gets set to ${exec_prefix}/lib,
dnl $prefix and $exec_prefix is likely to be "NONE"
dnl Autoconf usually sets pkglibdir correctly in the Makefile, but not in
dnl the configure script :(

test "$prefix" = "NONE" && prefix=/usr/local
test "$exec_prefix" = "NONE" && exec_prefix=$prefix

dnl Executable files
eval BINDIR="$bindir"
AC_SUBST(BINDIR)
eval SBINDIR="$sbindir"
AC_SUBST(SBINDIR)

dnl Configuration files
eval SYSCONFDIR="$sysconfdir"
AC_SUBST(SYSCONFDIR)

dnl Documentation
eval pkgdocdir="$datadir/doc/$PACKAGE"
AC_SUBST(pkgdocdir)

dnl Example scripts
eval pkgexdir="$pkgdocdir/examples"
AC_SUBST(pkgexdir)

dnl ----------------------------------------------------------
dnl configuration tests complete, provide a summary of results
dnl ----------------------------------------------------------

if test "x$filtergen_config_ok" = xno ; then
	echo "nothing yet"
else

dnl Dump it out
AC_CONFIG_FILES([
	Makefile
	t/Makefile
	filtergen.spec
	fgadm
	fgadm.conf
	rules.filter
])
AC_OUTPUT

AC_MSG_RESULT([
filtergen $VERSION: automatic configuration OK.

Type 'make' to compile filtergen.

Type 'make install' to install filtergen.
])

fi
