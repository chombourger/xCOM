#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT(xCOM, 0.1, https://github.com/chombourger/xCOM)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SUBDIRS([ext/jansson])

AC_CANONICAL_HOST

# Libtool
LT_INIT

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Check for -fvisibility compiler switch
gl_VISIBILITY

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([assert.h dirent.h dlfcn.h errno.h fcntl.h pthread.h stdarg.h stddef.h stdio.h stdlib.h string.h sys/stat.h sys/time.h sys/types.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gettimeofday memset])

AX_LIB_EXPAT

# Python
AX_PYTHON
AM_CONDITIONAL(HAVE_PYTHON, [test "$ax_python_header" != "no"])

PKG_CHECK_MODULES(                                                          
   [GLIB],                                                                  
   [glib-2.0 >= 2.24.0],                                                    
   [AC_DEFINE([GLIB_ENABLED],[1],[Define to 1 if glib support is needed])], 
   [AC_MSG_NOTICE([Compiling without glib support])]                        
)

PKG_CHECK_MODULES(                                                          
   [GIO_UNIX],                                                                  
   [gio-unix-2.0 >= 2.28.0],
   xc_have_gio_unix=yes,
   xc_have_gio_unix=no
)

AM_CONDITIONAL(HAVE_GIO_UNIX, [test "$xc_have_gio_unix" = "yes"])
if test "$xc_have_gio_unix" = "yes"; then
   AC_DEFINE([GIO_UNIX_ENABLED],[1],[Define to 1 if gio-unix support is needed])
fi

AC_CONFIG_FILES([
   Makefile
   ext/Makefile
   src/Makefile
   src/lib/Makefile
   src/pycom/Makefile
   src/spawn/Makefile
   src/test/Makefile
   examples/Makefile
   examples/hello/Makefile
   examples/hello-dbus-server/Makefile
   examples/python/Makefile
   tests/Makefile
   tests/python.basic/Makefile
   tests/python.basic/tests.in_b/Makefile
   tests/python.basic/tests.in_s/Makefile
   tests/python.basic/tests.in_y/Makefile
   tests/python.basic/tests.out_s/Makefile
])

AC_OUTPUT
