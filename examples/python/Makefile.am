libdir = $(exec_prefix)/components/examples.python/Code/$(host_triplet)
pydir=$(libdir)/../python
AM_CPPFLAGS  = -I $(srcdir)/../../include -I $(srcdir)/../include -I $(PYTHON_INCLUDE_DIR)
AM_CFLAGS    = @CFLAG_VISIBILITY@
lib_LTLIBRARIES = libpython-example.la
libpython_example_la_SOURCES = component.c
libpython_example_la_LIBADD = -l$(PYTHON_LIB) -lpthread -ldl
libpython_example_la_LDFLAGS = -version-info 0:0:0
MOSTLYCLEANFILES = component.c component.h

install-data-hook:
	$(MKDIR_P) $(pydir)/examples/python
	$(INSTALL_DATA) examples/__init__.py $(pydir)/examples/	
	$(INSTALL_DATA) examples/python/__init__.py $(pydir)/examples/python/
	$(INSTALL_DATA) examples/python/App.py $(pydir)/examples/python/
	
component.c component.h: component.xml
	$(srcdir)/../../src/codegen/xcom-codegen	\
		-I $(srcdir)/../interfaces		\
		-I $(srcdir)/../../interfaces		\
		-p python -n component $<

