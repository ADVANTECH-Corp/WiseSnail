#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([WiseAgent GateWay Core], [0], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE([foreign -Wall])
AC_CONFIG_HEADERS([config.h])
AC_PREFIX_DEFAULT([/usr/])
AC_CONFIG_FILES([wisecore.pc])
AC_CONFIG_MACRO_DIR([m4])


# Checks for programs.
AC_PROG_CC
AM_PROG_AR
LT_INIT
# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
#AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNCS([memset strstr])

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
