libdir = $(exec_prefix)/components/examples.hello/Code/$(host_triplet)
AM_CPPFLAGS  = -I $(srcdir)/../../include -I $(srcdir)/../include
AM_CFLAGS    = @CFLAG_VISIBILITY@ $(GIO_UNIX_CFLAGS)
lib_LTLIBRARIES = libhello-dbus-server.la
libhello_dbus_server_la_SOURCES = server.c hello-dbus.c
libhello_dbus_server_la_LIBADD = $(GIO_UNIX_LIBS) -lpthread -ldl
libhello_dbus_server_la_LDFLAGS = -version-info 0:0:0

hello-dbus.c hello-dbus.h: hello.xml Makefile
	gdbus-codegen					\
		--interface-prefix xcom.examples.	\
		--generate-c-code hello-dbus		\
		$<
