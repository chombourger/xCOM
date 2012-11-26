libdir = $(exec_prefix)/components/examples.hello/Code/$(host_triplet)
AM_CPPFLAGS  = -I $(srcdir)/../../include -I $(srcdir)/../include
AM_CFLAGS    = @CFLAG_VISIBILITY@
lib_LTLIBRARIES = libhello.la
libhello_la_SOURCES = hello.c component.c
libhello_la_LIBADD = $(GIO_UNIX_LIBS) -lpthread -ldl
libhello_la_LDFLAGS = -version-info 0:0:0

component.c component.h: component.xml
	$(srcdir)/../../src/codegen/xcom-codegen -I $(srcdir)/../interfaces -n component $<

hello.c: component.h

