libdir = $(exec_prefix)/components/xcom.test/Code/$(host_triplet)
AM_CPPFLAGS  = -I $(srcdir)/../../include
AM_CFLAGS    = @CFLAG_VISIBILITY@
lib_LTLIBRARIES = libxcom.test.la
libxcom_test_la_SOURCES = test.c component.c component.h
libxcom_test_la_LDFLAGS = -version-info 0:0:0

component.c component.h: component.xml
	$(srcdir)/../codegen/xcom-codegen -I $(srcdir)/../../interfaces -n component $<

test.c: component.h

