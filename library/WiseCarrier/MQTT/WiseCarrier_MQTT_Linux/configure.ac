#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([Wise MQTT Carrier for Linux], [VERSION], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE([foreign -Wall])
AC_CONFIG_HEADERS([config.h])
AC_PREFIX_DEFAULT([/usr/])
AC_CONFIG_MACRO_DIR([m4])


# Checks for programs.
AC_PROG_CC
AM_PROG_AR
LT_INIT
# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
#AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday memset strchr strstr])

AC_CONFIG_FILES([Makefile src/Makefile])

red=$(tput setaf 1)
clr=$(tput sgr0)

AC_ARG_ENABLE([paho],[AC_HELP_STRING([--disable-paho],[Disable Paho support(MQTT)])],[enable_carrier_paho="$enableval"],[enable_carrier_paho="yes"])
AM_CONDITIONAL([ENABLE_CARRIER_PAHO], [test x$enable_carrier_paho = "xyes"])

AC_ARG_ENABLE([mosquitto],[AC_HELP_STRING([--disable-mosquitto],[Disable Mosquitto support(MQTT)])],[enable_carrier_mosquitto="$enableval"],[enable_carrier_mosquitto="yes"])
AM_CONDITIONAL([ENABLE_CARRIER_MOSQUITTO], [test x$enable_carrier_mosquitto = "xyes"])

AS_IF([test "x$enable_paho" = "xno"],
      [AS_IF([test "x$enable_mosquitto" = "xno"],
             [AC_MSG_ERROR([${red}one of these MQTT support must be chosen.(paho , mosquitto)${clr}])
      ])
])


AC_OUTPUT
