/**
 *  Volnoti - Lightweight Volume Notification
 *  Copyright (C) 2011  David Brazdil <db538@cam.ac.uk>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <unistd.h>

#include "common.h"
#include "gopt.h"

#include "value-client-stub.h"

static void print_usage(const char* filename, int failure) {
    g_print("Usage: %s [-v] [-m] <value>\n"
            " -h\t--help\t\thelp\n"
            " -v\t--verbose\tverbose\n"
            " -m\t--mute\t\tmuted\n"
            " -b\t--brightness\t\tdisplay brightness\n"
            " <value>\t\tint 0-100\n", filename);
    if (failure)
        exit(EXIT_FAILURE);
    else
        exit(EXIT_SUCCESS);
}

int main(int argc, const char* argv[]) {
    void *options = gopt_sort(&argc, argv, gopt_start(
            gopt_option('h', 0, gopt_shorts('h', '?'), gopt_longs("help", "HELP")),
            gopt_option('m', 0, gopt_shorts('m'), gopt_longs("mute")),
            gopt_option('b', 0, gopt_shorts('b'), gopt_longs("brightness")),
            gopt_option('v', GOPT_REPEAT, gopt_shorts('v'), gopt_longs("verbose"))));
    int help = gopt(options, 'h');
    int debug = gopt(options, 'v');
    int muted = gopt(options, 'm');
    int brightness = gopt(options, 'b');
    gopt_free(options);

    if (help)
        print_usage(argv[0], FALSE);

    gint volume;
    if (muted) {
        if (argc > 2) {
            print_usage(argv[0], TRUE);
        } else if (argc == 2) {
            if (sscanf(argv[1], "%d", &volume) != 1)
                print_usage(argv[0], TRUE);

            if (volume > 100 || volume < 0)
                print_usage(argv[0], TRUE);
        } else {
            volume = 0;
        }
    } else {
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
    uk_ac_cam_db538_VolumeNotification_notify(proxy, volume, muted, brightness, &error);
    if (error !=  NULL) {
        handle_error("Failed to send notification", error->message, FALSE);
        g_clear_error(&error);
        return EXIT_FAILURE;
    }
    print_debug_ok(debug);

    return EXIT_SUCCESS;
}
