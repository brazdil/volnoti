# Interface XML name (used in multiple targets)
interface_xml := specs.xml

# Define a list of pkg-config packages we want to use
pkg_packages := dbus-1 dbus-glib-1 gtk+-2.0 cairo

PKG_CFLAGS  := $(shell pkg-config --cflags $(pkg_packages))
PKG_LDFLAGS := $(shell pkg-config --libs $(pkg_packages))
# Add additional flags:
# -g : add debugging symbols
# -Wall : enable most gcc warnings
# -DG_DISABLE_DEPRECATED : disable GLib functions marked as deprecated
ADD_CFLAGS := -g -Wall -DG_DISABLE_DEPRECATED -DDEBUG
# -DNO_DAEMON : do not daemonize the server (on a separate line so can
#               be disabled just by commenting the line)
ADD_CFLAGS += -DNO_DAEMON

# Combine flags
CFLAGS  := $(PKG_CFLAGS) $(ADD_CFLAGS) $(CFLAGS)
LDFLAGS := $(PKG_LDFLAGS) $(LDFLAGS)

# Define a list of generated files so that they can be cleaned as well
cleanfiles := value-client-stub.h \
              value-daemon-stub.h

targets = daemon client

.PHONY: all clean checkxml
all: $(targets)

# We don't use the implicit pattern rules built into GNU make, since
# they put the LDFLAGS in the wrong location (and linking consequently
# fails sometimes).
#
# NOTE: You could actually collapse the compilation and linking phases
#       together, but this arrangement is much more common.

daemon: daemon.o gopt.o common.o notification.o
	 $(CC) $^ -o $@ $(LDFLAGS)

client: client.o gopt.o common.o
	$(CC) $^ -o $@ $(LDFLAGS)

# The server and client depend on the respective implementation source
# files, but also on the common interface as well as the generated
# stub interfaces.
daemon.o: daemon.c common.h value-daemon-stub.h gopt.h notification.h
	$(CC) $(CFLAGS) -DPROGNAME=\"$(basename $@)\" -c $< -o $@

client.o: client.c common.h value-client-stub.h gopt.h
	$(CC) $(CFLAGS) -DPROGNAME=\"$(basename $@)\" -c $< -o $@

gopt.o: gopt.c gopt.h
	$(CC) $(CFLAGS) -c $< -o $@

common.o: common.c common.h
	$(CC) $(CFLAGS) -c $< -o $@

notification.o: notification.c notification.h
	$(CC) $(CFLAGS) -c $< -o $@

# If the interface XML changes, the respective stub interfaces will be
# automatically regenerated. Normally this would also mean that your
# builds would fail after this since you'd be missing implementation
# code.
value-daemon-stub.h: $(interface_xml)
	dbus-binding-tool --prefix=volume_object --mode=glib-server \
	  $< > $@

value-client-stub.h: $(interface_xml)
	dbus-binding-tool --prefix=volume_object --mode=glib-client \
	  $< > $@

# Special target to run DTD validation on the interface XML. Not run
# automatically (since xmllint isn't always available and also needs
# Internet connectivity).
checkxml: $(interface_xml)
	@xmllint --valid --noout $<
	@echo $< checks out ok

clean:
	$(RM) $(targets) $(cleanfiles) *.o

# In order to force a rebuild if this file is modified, we add the
# Makefile as a dependency to all low-level targets. Adding the same
# dependency to multiple files on the same line is allowed in GNU make
# syntax as follows. Just make sure that additinal dependencies are
# listed after explicit rules, or that no implicit pattern rules will
# match the dependency. Otherwise funny things happen. Placing the
# Makefile dependency at the very end is often the safest solution.
server.o client.o: Makefile
