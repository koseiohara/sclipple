
AC_INIT([sclipple], [0.2.0])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE([disable])

AC_PROG_CC
AC_USE_SYSTEM_EXTENSIONS

AC_CHECK_DECLS(
  [asprintf],
  [],
  [AC_MSG_ERROR([asprintf is required but not declared])],
  [[#include <stdio.h>]]
)

AC_CONFIG_FILES([
Makefile
src/Makefile
])

AC_OUTPUT
