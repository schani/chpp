dnl Process this file with autoconf to produce a configure script.
AC_INIT([chpp], [0.3.6])
AC_CONFIG_SRCDIR([main.c])
AM_INIT_AUTOMAKE

AM_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_GCC_TRADITIONAL
AC_PROG_YACC
AM_PROG_LEX
dnl AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_INSTALL

dnl Checks for libraries.
AM_PATH_MYSQL
AM_PATH_MSQL
AC_CHECK_LIB(crypt,crypt)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SIGNAL
AC_STRUCT_ST_BLKSIZE
AC_STRUCT_ST_BLOCKS
dnl AC_STRUCT_TM
AC_C_BIGENDIAN
AC_CHECK_SIZEOF(int,4)
AC_CHECK_SIZEOF(long,4)

dnl Checks for library functions.
dnl AC_FUNC_ALLOCA
dnl AC_FUNC_MMAP
dnl AC_FUNC_SETVBUF_REVERSED
dnl AC_FUNC_VPRINTF
AC_FUNC_MEMCMP
AC_REPLACE_FUNCS(strerror getcwd)

chppincludedir=$datarootdir/chpp/include
AC_SUBST(chppincludedir)
dnl AC_DEFINE_UNQUOTED(CHPP_INCLUDEDIR, "$prefix/lib/chpp/include")

chpp_cv_fcntl=yes
AC_ARG_ENABLE(fcntl, [  --disable-fcntl         Use flock() instead of fcntl to lock files ],
	[if test $enableval = no; then chpp_cv_fcntl=no; fi])
if test $chpp_cv_fcntl = yes; then
	AC_DEFINE(USE_FCNTL)
else
	AC_DEFINE(USE_FLOCK)
fi

AC_OUTPUT(Makefile builtins/Makefile builtins/database/Makefile include/Makefile doc/Makefile)
