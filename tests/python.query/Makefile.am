libdir                      = $(exec_prefix)/components/tests.python.query/Code/$(host_triplet)
pydir                       = $(libdir)/../python
AM_CPPFLAGS                 = $(PYTHON_CPPFLAGS) -I $(abs_top_srcdir)/include
AM_CFLAGS                   = @CFLAG_VISIBILITY@
lib_LTLIBRARIES             = libpython-query.la
libpython_query_la_SOURCES  = component.c
libpython_query_la_LIBADD   = $(PYTHON_EXTRA_LIBS) $(PYTHON_EXTRA_LDFLAGS)
libpython_query_la_LIBADD  += -lpthread -ldl
libpython_query_la_LDFLAGS  = $(abs_top_builddir)/src/pycom/libpycom.la -version-info 0:0:0
libpython_query_la_LDFLAGS += $(PYTHON_LDFLAGS)
MOSTLYCLEANFILES            = component.c component.h

install-data-hook:
	$(MKDIR_P) $(pydir)/tests/python/query
	$(INSTALL_DATA) tests/__init__.py $(pydir)/tests/	
	$(INSTALL_DATA) tests/python/__init__.py $(pydir)/tests/python/
	$(INSTALL_DATA) tests/python/query/__init__.py $(pydir)/tests/python/query/
	$(INSTALL_DATA) tests/python/query/Test.py $(pydir)/tests/python/query/
	
component.c component.h: component.xml
	$(abs_top_srcdir)/src/codegen/xcom-codegen	\
		-I $(abs_top_srcdir)/interfaces		\
		-p python -n component $<

