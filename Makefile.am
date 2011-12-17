AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}
AM_CONFIG_HEADER = config.h

AM_CPPFLAGS = \
              @DBUS_CFLAGS@ \
              @DBUS_GLIB_CFLAGS@ \
              @GTK_CFLAGS@ \
              @CAIRO_CFLAGS@ \
              @GDK_PIXBUF_CFLAGS@

bin_PROGRAMS = volnoti volnoti-show

volnoti_SOURCES = src/daemon.c src/notification.c src/notification.h \
                  src/common.c src/common.h src/gopt.c src/gopt.h
volnoti_LDADD = \
                @DBUS_LIBS@ \
                @DBUS_GLIB_LIBS@ \
                @GTK_LIBS@ \
                @CAIRO_LIBS@ \
                @GDK_PIXBUF_LIBS@

volnoti_show_SOURCES = src/client.c \
                       src/common.c src/common.h src/gopt.c src/gopt.h
volnoti_show_LDADD = \
                     @DBUS_LIBS@ \
                     @DBUS_GLIB_LIBS@
