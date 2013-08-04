libdir                           = $(exec_prefix)/components/tests.python.basic/Code/$(host_triplet)
pydir                            = $(libdir)/../python
AM_CPPFLAGS                      = -I $(abs_top_srcdir)/include
AM_CPPFLAGS                     += $(PYTHON_CPPFLAGS)
AM_CFLAGS                        = @CFLAG_VISIBILITY@
lib_LTLIBRARIES                  = libpython-tests.in_y.la
libpython_tests_in_y_la_SOURCES  = component.c
libpython_tests_in_y_la_LIBADD   = -lpthread -ldl
libpython_tests_in_y_la_LIBADD  += $(PYTHON_LDFLAGS)
libpython_tests_in_y_la_LDFLAGS  = $(abs_top_builddir)/src/pycom/libpycom.la -version-info 0:0:0
libpython_tests_in_y_la_LDFLAGS += $(PYTHON_EXTRA_LIBS) $(PYTHON_EXTRA_LDFLAGS)
MOSTLYCLEANFILES                 = component.c component.h

install-data-hook:
	$(MKDIR_P) $(pydir)/tests/python/basic/in_y
	$(INSTALL_DATA) tests/__init__.py $(pydir)/tests/	
	$(INSTALL_DATA) tests/python/__init__.py $(pydir)/tests/python/
	$(INSTALL_DATA) tests/python/basic/__init__.py $(pydir)/tests/python/basic
	$(INSTALL_DATA) tests/python/basic/in_y/__init__.py $(pydir)/tests/python/basic/in_y/
	$(INSTALL_DATA) tests/python/basic/in_y/Test.py $(pydir)/tests/python/basic/in_y/
	
component.c component.h: component.xml
	$(srcdir)/../../../src/codegen/xcom-codegen	\
		-I $(srcdir)/../../interfaces		\
		-I $(srcdir)/../../../interfaces	\
		-p python -n component $<

