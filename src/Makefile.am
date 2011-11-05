BINDING_XML = netnotify-daemon.xml
BUILT_SOURCES = netnotify-daemon-glue.h

AM_CFLAGS = -Wall $(NETNOTIFY_DAEMON_CFLAGS)
bin_PROGRAMS = netnotify-daemon
noinst_HEADERS = netnotify-daemon.h

netnotify_daemon_SOURCES = netnotify-daemon.c
netnotify_daemon_LDFLAGS = $(NETNOTIFY_DAEMON_LIBS)

EXTRA_DIST = $(BINDING_XML)
CLEANFILES = $(BUILT_SOURCES)

netnotify-daemon-glue.h: $(BINDING_XML)
	dbus-binding-tool --prefix=netnotify_daemon --mode=glib-server $<> $@
