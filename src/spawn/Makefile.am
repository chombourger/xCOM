AM_CPPFLAGS        = -I $(srcdir)/../../include
AM_CFLAGS          = -Wall -Wextra -Werror
bin_PROGRAMS       = xcom-spawn
xcom_spawn_SOURCES = spawn.c component.c
xcom_spawn_LDADD   = ../lib/libxcom.la
MOSTLYCLEANFILES   = component.c component.h

component.c component.h: component.xml
	$(srcdir)/../codegen/xcom-codegen -I $(srcdir)/../../interfaces -n component $<

spawn.c: component.h

