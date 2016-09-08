#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Wise Snail Library], [1.0.00], [fred.chang@advantech.com.tw])
AM_INIT_AUTOMAKE([foreign -Wall])
AC_PREFIX_DEFAULT([/usr/])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([WiseSnail.pc])
AC_CONFIG_MACRO_DIR([m4])

red=$(tput setaf 1)
clr=$(tput sgr0)

AC_ARG_ENABLE([paho],AS_HELP_STRING([--disable-paho], [Disable Paho support(MQTT)]))
AC_ARG_ENABLE([mosquitto],AS_HELP_STRING([--disable-mosquitto], [Disable Mosquitto support(MQTT)]))
AS_IF([test "x$enable_paho" = "xno"],
      [AS_IF([test "x$enable_mosquitto" = "xno"],
             [AC_MSG_ERROR([${red}one of these MQTT support must be chosen.(paho , mosquitto)${clr}])
      ])
])

AC_ARG_ENABLE([advlog],AS_HELP_STRING([--disable-advlog], [Disable feature AdvLog support]))
AM_CONDITIONAL([ENABLE_ADVLOG], [test "x$enable_advlog" = "xyes"])



# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AM_PROG_AR
LT_INIT

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])
# FIXME: Replace `main' with a function in `-lrt':
AC_CHECK_LIB([rt], [main])

# Checks for header files.
AC_CHECK_HEADERS([memory.h stdlib.h string.h sys/ioctl.h sys/socket.h unistd.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gettimeofday memset socket strchr strdup strstr])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_CONFIG_SUBDIRS([library/AdvJSON library/AdvLog library/WiseCore])
AC_OUTPUT
