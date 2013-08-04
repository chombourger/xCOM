AM_CPPFLAGS          = $(PYTHON_CPPFLAGS) -I $(abs_top_srcdir)/include -D__PYCOM__
AM_CFLAGS            = -Wall -Wextra -Werror @CFLAG_VISIBILITY@
lib_LTLIBRARIES      = libpycom.la
libpycom_la_SOURCES  = import.c module.c pycom.c query.c
libpycom_la_LIBADD   = $(abs_top_builddir)/src/lib/libxcom.la
libpycom_la_LIBADD  += $(PYTHON_LDFLAGS) $(PYTHON_EXTRA_LIBS) $(PYTHON_EXTRA_LDFLAGS)
libpycom_la_LDFLAGS  = -version-info 0:0:0
