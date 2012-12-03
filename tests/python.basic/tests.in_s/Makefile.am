libdir = $(exec_prefix)/components/tests.python.basic/Code/$(host_triplet)
pydir=$(libdir)/../python
AM_CPPFLAGS  = -I $(srcdir)/../../../include -I $(PYTHON_INCLUDE_DIR)
AM_CFLAGS    = @CFLAG_VISIBILITY@
lib_LTLIBRARIES = libpython-tests.in_s.la
libpython_tests_in_s_la_SOURCES = component.c
libpython_tests_in_s_la_LIBADD = -l$(PYTHON_LIB) -lpthread -ldl
libpython_tests_in_s_la_LDFLAGS = -version-info 0:0:0
MOSTLYCLEANFILES = component.c component.h

install-data-hook:
	$(MKDIR_P) $(pydir)/tests/python/basic/in_s
	$(INSTALL_DATA) tests/__init__.py $(pydir)/tests/	
	$(INSTALL_DATA) tests/python/__init__.py $(pydir)/tests/python/
	$(INSTALL_DATA) tests/python/basic/__init__.py $(pydir)/tests/python/basic
	$(INSTALL_DATA) tests/python/basic/in_s/__init__.py $(pydir)/tests/python/basic/in_s/
	$(INSTALL_DATA) tests/python/basic/in_s/Test.py $(pydir)/tests/python/basic/in_s/
	
component.c component.h: component.xml
	$(srcdir)/../../../src/codegen/xcom-codegen	\
		-I $(srcdir)/../../interfaces		\
		-I $(srcdir)/../../../interfaces	\
		-p python -n component $<

