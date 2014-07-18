AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}
AM_CONFIG_HEADER = ../config.h

EXTRA_DIST = specs.xml

AM_CPPFLAGS = \
              @DBUS_CFLAGS@ \
              @DBUS_GLIB_CFLAGS@ \
              @GTK_CFLAGS@ \
              @CAIRO_CFLAGS@ \
              @GDK_PIXBUF_CFLAGS@ \
              -DPREFIX="\"$(datarootdir)/pixmaps/@PACKAGE@/\""

COMMON = common.c common.h gopt.c gopt.h

bin_PROGRAMS = volnoti volnoti-show

volnoti_SOURCES = daemon.c notification.c notification.h \
                  value-daemon-stub.h $(COMMON)
volnoti_LDADD = \
                @DBUS_LIBS@ \
                @DBUS_GLIB_LIBS@ \
                @GTK_LIBS@ \
                @CAIRO_LIBS@ \
                @GDK_PIXBUF_LIBS@

volnoti_show_SOURCES = client.c value-client-stub.h $(COMMON)
volnoti_show_LDADD = \
                     @DBUS_LIBS@ \
                     @DBUS_GLIB_LIBS@

interface_xml = specs.xml

BUILT_SOURCES = value-daemon-stub.h value-client-stub.h 
CLEANFILES = $(BUILT_SOURCES)

value-daemon-stub.h: $(interface_xml)
	dbus-binding-tool --prefix=volume_object --mode=glib-server \
	  $< > $@

value-client-stub.h: $(interface_xml)
	dbus-binding-tool --prefix=volume_object --mode=glib-client \
	  $< > $@
