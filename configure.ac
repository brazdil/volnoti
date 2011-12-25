#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([volnoti], [0.2], [])
AC_CONFIG_SRCDIR([src/daemon.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.10 -Wall no-define])
# AC_CONFIG_FILES([Makefile])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# Checks for libraries.
PKG_CHECK_MODULES([DBUS], [dbus-1])
PKG_CHECK_MODULES([DBUS_GLIB], [dbus-glib-1])
PKG_CHECK_MODULES([GTK], [gtk+-2.0])
PKG_CHECK_MODULES([CAIRO], [cairo])
PKG_CHECK_MODULES([GDK_PIXBUF], [gdk-pixbuf-2.0])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strchr strcspn])

AC_OUTPUT([
  Makefile
  src/Makefile
  res/Makefile
])
