AM_CPPFLAGS  = -I $(abs_top_srcdir)/include
AM_CPPFLAGS += -D __XCOM__ -D XC_HOST='"$(host_triplet)"'
AM_CFLAGS    = -Wall -Wextra -Werror @CFLAG_VISIBILITY@ $(GLIB_CFLAGS) $(EXPAT_CFLAGS)
lib_LTLIBRARIES = libxcom.la
libxcom_la_SOURCES = cache.c component.c handledir.c hashtable.c import.c init.c interfaces.c ports.c query.c semaphore.c trace.c
libxcom_la_LIBADD = $(GLIB_LIBS) $(EXPAT_LIBS) -lpthread -ldl
libxcom_la_LDFLAGS = -version-info 0:0:0
