libdir                        = $(exec_prefix)/components/examples.python/Code/$(host_triplet)
pydir                         = $(libdir)/../python
AM_CPPFLAGS                   = -I $(abs_top_srcdir)/include
AM_CPPFLAGS                  += $(PYTHON_CPPFLAGS)
AM_CFLAGS                     = @CFLAG_VISIBILITY@
lib_LTLIBRARIES               = libpython-example.la
libpython_example_la_SOURCES  = component.c
libpython_example_la_LIBADD   = -lpthread -ldl
libpython_example_la_LIBADD  += $(PYTHON_EXTRA_LIBS) $(PYTHON_EXTRA_LDFLAGS)
libpython_example_la_LDFLAGS  = $(abs_top_builddir)/src/pycom/libpycom.la -version-info 0:0:0
libpython_example_la_LDFLAGS += $(PYTHON_LDFLAGS)
MOSTLYCLEANFILES              = component.c component.h

install-data-hook:
	$(MKDIR_P) $(pydir)/examples/python
	$(INSTALL_DATA) examples/__init__.py $(pydir)/examples/	
	$(INSTALL_DATA) examples/python/__init__.py $(pydir)/examples/python/
	$(INSTALL_DATA) examples/python/App.py $(pydir)/examples/python/
	
component.c component.h: component.xml
	$(abs_top_srcdir)/src/codegen/xcom-codegen	\
		-I $(srcdir)/../interfaces		\
		-I $(abs_top_srcdir)/interfaces		\
		-p python -n component $<

