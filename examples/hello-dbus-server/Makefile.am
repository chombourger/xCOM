libdir = $(exec_prefix)/components/examples.hello/Code/$(host_triplet)
AM_CPPFLAGS  = -I $(abs_top_srcdir)/include
AM_CFLAGS    = @CFLAG_VISIBILITY@ $(GIO_UNIX_CFLAGS)
lib_LTLIBRARIES = libhello-dbus-server.la
libhello_dbus_server_la_SOURCES = component.c server.c hello-dbus.c
libhello_dbus_server_la_LIBADD = $(GIO_UNIX_LIBS) -lpthread -ldl
libhello_dbus_server_la_LDFLAGS = -version-info 0:0:0
MOSTLYCLEANFILES = component.c component.h hello-dbus.c hello-dbus.h

hello-dbus.c hello-dbus.h: hello.xml Makefile
	gdbus-codegen					\
		--interface-prefix xcom.examples.	\
		--generate-c-code hello-dbus		\
		$<

component.c component.h: component.xml
	$(abs_top_srcdir)/src/codegen/xcom-codegen	\
		-I $(srcdir)/../interfaces		\
		-I $(abs_top_srcdir)/interfaces		\
		-n component $<

server.c: component.h hello-dbus.h

