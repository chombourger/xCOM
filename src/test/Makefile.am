libdir = $(exec_prefix)/components/xcom.test/Code/$(host_triplet)
AM_CPPFLAGS  = -I $(abs_top_srcdir)/include
AM_CFLAGS    = @CFLAG_VISIBILITY@
lib_LTLIBRARIES = libxcom.test.la
libxcom_test_la_SOURCES = test.c component.c component.h
libxcom_test_la_LDFLAGS = -version-info 0:0:0
MOSTLYCLEANFILES = component.c component.h

component.c component.h: component.xml
	$(abs_top_srcdir)/src/codegen/xcom-codegen	\
		-I $(abs_top_srcdir)/interfaces		\
		-n component $<

test.c: component.h

