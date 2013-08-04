AM_CPPFLAGS        = -I $(abs_top_srcdir)/include
AM_CFLAGS          = -Wall -Wextra -Werror
bin_PROGRAMS       = xcom-spawn
xcom_spawn_SOURCES = spawn.c component.c
xcom_spawn_LDADD   = $(abs_top_builddir)/src/lib/libxcom.la -lpthread
MOSTLYCLEANFILES   = component.c component.h

component.c component.h: component.xml
	$(abs_top_srcdir)/src/codegen/xcom-codegen	\
		-I $(abs_top_srcdir)/interfaces		\
		-n component $<

spawn.c: component.h

