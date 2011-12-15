#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <unistd.h>

#include "common.h"
#include "gopt.h"

#include "value-client-stub.h"

static void print_usage(const char* filename, int failure) {
    g_print("Usage: %s [-v] [-m] <volume>\n"
    		" -h\t--help\t\thelp\n"
    		" -v\t--verbose\tverbose\n"
    		" -m\t--mute\t\tmuted\n"
    		" <volume>\t\tint 0-100\n", filename);
    if (failure)
    	exit(EXIT_FAILURE);
    else
    	exit(EXIT_SUCCESS);
}

int main(int argc, const char* argv[]) {
    void *options = gopt_sort(&argc, argv, gopt_start(
            gopt_option('h', 0, gopt_shorts('h', '?'), gopt_longs("help", "HELP")),
            gopt_option('m', 0, gopt_shorts('m'), gopt_longs("mute")),
            gopt_option('v', GOPT_REPEAT, gopt_shorts('v'), gopt_longs("verbose"))));
    int help = gopt(options, 'h');
    int debug = gopt(options, 'v');
    int muted = gopt(options, 'm');
    gopt_free(options);

    if (help)
    	print_usage(argv[0], FALSE);

    gint volume = -1;
    if (!muted) {
        if (argc != 2)
        	print_usage(argv[0], TRUE);

        if (sscanf(argv[1], "%d", &volume) != 1)
        	print_usage(argv[0], TRUE);

        if (volume > 100 || volume < 0)
        	print_usage(argv[0], TRUE);
    }

    DBusGConnection *bus = NULL;
    DBusGProxy *proxy = NULL;
    GError *error = NULL;

    // initialize GObject
    g_type_init();

    // connect to D-Bus
    print_debug("Connecting to D-Bus...", debug);
    bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    if (error != NULL)
        handle_error("Couldn't connect to D-Bus",
                    error->message,
                    TRUE);
    print_debug_ok(debug);

    // get the proxy
    print_debug("Getting proxy...", debug);
    proxy = dbus_g_proxy_new_for_name(bus,
                                      VALUE_SERVICE_NAME,
                                      VALUE_SERVICE_OBJECT_PATH,
                                      VALUE_SERVICE_INTERFACE);
    if (proxy == NULL)
    	handle_error("Couldn't get a proxy for D-Bus",
                    "Unknown(dbus_g_proxy_new_for_name)",
                    TRUE);
    print_debug_ok(debug);

    print_debug("Sending volume...", debug);
    uk_ac_cam_db538_VolumeNotification_notify(proxy, volume, &error);
    if (error !=  NULL) {
    	handle_error("Failed to send notification", error->message, FALSE);
        g_clear_error(&error);
        return EXIT_FAILURE;
    }
    print_debug_ok(debug);

    return EXIT_SUCCESS;
}
