libdir = $(exec_prefix)/components/tests.python.basic/Code/$(host_triplet)
AM_CPPFLAGS  = -I $(abs_top_srcdir)/include
AM_CFLAGS    = @CFLAG_VISIBILITY@
lib_LTLIBRARIES = libpython.basic.la
libpython_basic_la_SOURCES = test.c component.c
libpython_basic_la_LIBADD = -lpthread -ldl
libpython_basic_la_LDFLAGS = -version-info 0:0:0
MOSTLYCLEANFILES = component.c component.h

component.c component.h: component.xml
	$(abs_top_srcdir)/src/codegen/xcom-codegen	\
		-I $(srcdir)/../interfaces		\
		-I $(abs_top_srcdir)/interfaces		\
		-n component $<

test.c: component.h

