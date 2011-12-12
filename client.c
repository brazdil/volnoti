#include <stdlib.h>
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <unistd.h>

#include "common-defs.h"
#include "gopt.h"

int main(int argc, char* argv[]) {

	void *options = gopt_sort( & argc, argv, gopt_start(
			gopt_option( 'h', 0, gopt_shorts( 'h', '?' ), gopt_longs( "help", "HELP" )),
			gopt_option( 'z', 0, gopt_shorts( 0 ), gopt_longs( "version" )),
			gopt_option( 'v', GOPT_REPEAT, gopt_shorts( 'v' ), gopt_longs( "verbose" ))));

	int debug = gopt(options, 'v');

    DBusGConnection *bus = NULL;
    DBusGProxy *busProxy = NULL;
    GMainLoop *mainLoop = NULL;
    GError *error = NULL;
    guint result;

}
