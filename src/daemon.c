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
#include <unistd.h>
#include <glib.h>
#include <dbus/dbus-glib.h>

#include "common.h"
#include "gopt.h"
#include "notification.h"

#define IMAGE_PATH   PREFIX

typedef struct {
    GObject parent;

    gint volume;
    gboolean muted;
    gboolean brightness;

    GtkWindow *notification;

    GdkPixbuf *icon_high;
    GdkPixbuf *icon_medium;
    GdkPixbuf *icon_low;
    GdkPixbuf *icon_off;
    GdkPixbuf *icon_muted;
    GdkPixbuf *icon_brightness;

    GdkPixbuf *image_progressbar_empty;
    GdkPixbuf *image_progressbar_full;
    GdkPixbuf *image_progressbar;
    gint width_progressbar;
    gint height_progressbar;

    gint time_left;
    gint timeout;
    gboolean debug;
    Settings settings;
} VolumeObject;

typedef struct {
  GObjectClass parent;
} VolumeObjectClass;

GType volume_object_get_type(void);
gboolean volume_object_notify(VolumeObject* obj,
                              gint value_in,
                              gboolean muted,
                              gboolean brightness,
                              GError** error);

#define VOLUME_TYPE_OBJECT \
        (volume_object_get_type())
#define VOLUME_OBJECT(object) \
        (G_TYPE_CHECK_INSTANCE_CAST ((object), \
         VOLUME_TYPE_OBJECT, VolumeObject))
#define VOLUME_OBJECT_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), \
         VOLUME_TYPE_OBJECT, VolumeObjectClass))
#define VOLUME_IS_OBJECT(object) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((object), \
         VOLUME_TYPE_OBJECT))
#define VOLUME_IS_OBJECT_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
         VOLUME_TYPE_OBJECT))
#define VOLUME_OBJECT_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
         VOLUME_TYPE_OBJECT, VolumeObjectClass))

G_DEFINE_TYPE(VolumeObject, volume_object, G_TYPE_OBJECT)

#include "value-daemon-stub.h"

static void volume_object_init(VolumeObject* obj) {
    g_assert(obj != NULL);
    obj->notification = NULL;
}

static void volume_object_class_init(VolumeObjectClass* klass) {
    g_assert(klass != NULL);

    dbus_g_object_type_install_info(VOLUME_TYPE_OBJECT,
                                    &dbus_glib_volume_object_object_info);
}

static gboolean
time_handler(VolumeObject *obj)
{
    g_assert(obj != NULL);

    obj->time_left--;

    if (obj->time_left <= 0) {
        print_debug("Destroying notification...", obj->debug);
        destroy_notification(obj->notification);
        obj->notification = NULL;
        print_debug_ok(obj->debug);
        return FALSE;
    }

    return TRUE;
}

gboolean volume_object_notify(VolumeObject* obj,
                              gint value,
                              gboolean muted,
                              gboolean brightness,
                              GError** error) {
    g_assert(obj != NULL);

    if (muted) {
        obj->muted = TRUE;
    } else {
        obj->muted = FALSE;
    }
    obj->brightness = brightness ? TRUE : FALSE;
    obj->volume = (value > 100) ? 100 : value;

    if (obj->notification == NULL) {
        print_debug("Creating new notification...", obj->debug);
        obj->notification = create_notification(obj->settings);
        gtk_widget_realize(GTK_WIDGET(obj->notification));
        g_timeout_add(1000, (GSourceFunc) time_handler, (gpointer) obj);
        print_debug_ok(obj->debug);
    }

    // choose icon
    if (obj->brightness)
        set_notification_icon(GTK_WINDOW(obj->notification), obj->icon_brightness);
    else if (obj->muted)
        set_notification_icon(GTK_WINDOW(obj->notification), obj->icon_muted);
    else if (obj->volume >= 75)
        set_notification_icon(GTK_WINDOW(obj->notification), obj->icon_high);
    else if (obj->volume >= 50)
        set_notification_icon(GTK_WINDOW(obj->notification), obj->icon_medium);
    else if (obj->volume >= 25)
        set_notification_icon(GTK_WINDOW(obj->notification), obj->icon_low);
    else
        set_notification_icon(GTK_WINDOW(obj->notification), obj->icon_off);

    // prepare and set progress bar
    gint width_full = obj->width_progressbar * obj->volume / 100;
    gdk_pixbuf_copy_area(obj->image_progressbar_full, 0, 0, width_full, obj->height_progressbar,
                         obj->image_progressbar, 0, 0);
    gdk_pixbuf_copy_area(obj->image_progressbar_empty, width_full, 0, obj->width_progressbar - width_full, obj->height_progressbar,
                         obj->image_progressbar, width_full, 0);
    set_progressbar_image(GTK_WINDOW(obj->notification), obj->image_progressbar);

    obj->time_left = obj->timeout;
    gtk_widget_show_all(GTK_WIDGET(obj->notification));

    return TRUE;
}

static void print_usage(const char* filename, int failure) {
    Settings settings = get_default_settings();
    g_print("Usage: %s [arguments]\n"
            " -h\t\t--help\t\t\thelp\n"
            " -v\t\t--verbose\t\tverbose\n"
            " -n\t\t--no-daemon\t\tdo not daemonize\n"
            "\n"
            "Configuration:\n"
            " -t <int>\t--timeout <int>\t\tnotification timeout in seconds\n"
            " -a <float>\t--alpha <float>\t\ttransparency level (0.0 - 1.0, default %.2f)\n"
            " -r <int>\t--corner-radius <int>\tradius of the round corners in pixels (default %d)\n"
            , filename, settings.alpha, settings.corner_radius);
    if (failure)
        exit(EXIT_FAILURE);
    else
        exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    Settings settings = get_default_settings();
    int timeout = 3;
    
    void *options = gopt_sort(&argc, (const char**) argv, gopt_start(
            gopt_option('h', 0, gopt_shorts('h', '?'), gopt_longs("help", "HELP")),
            gopt_option('n', 0, gopt_shorts('n'), gopt_longs("no-daemon")),
            gopt_option('t', GOPT_ARG, gopt_shorts('t'), gopt_longs("timeout")),
            gopt_option('a', GOPT_ARG, gopt_shorts('a'), gopt_longs("alpha")),
            gopt_option('r', GOPT_ARG, gopt_shorts('r'), gopt_longs("corner-radius")),
            gopt_option('v', GOPT_REPEAT, gopt_shorts('v'), gopt_longs("verbose"))));

    int help = gopt(options, 'h');
    int debug = gopt(options, 'v');
    int no_daemon = gopt(options, 'n');

    if (gopt(options, 't')) {
        if (sscanf(gopt_arg_i(options, 't', 0), "%d", &timeout) != 1)
            print_usage(argv[0], TRUE);
    }
    
    if (gopt(options, 'a')) {
        if (sscanf(gopt_arg_i(options, 'a', 0), "%f", &settings.alpha) != 1 || settings.alpha < 0.0f || settings.alpha > 1.0f)
            print_usage(argv[0], TRUE);
    }

    if (gopt(options, 'r')) {
        if (sscanf(gopt_arg_i(options, 'r', 0), "%d", &settings.corner_radius) != 1)
            print_usage(argv[0], TRUE);
    }

    gopt_free(options);

    if (help)
        print_usage(argv[0], FALSE);

    DBusGConnection *bus = NULL;
    DBusGProxy *bus_proxy = NULL;
    VolumeObject *status = NULL;
    GMainLoop *main_loop = NULL;
    GError *error = NULL;
    guint result;

    // initialize GObject and GTK
    g_type_init();
    g_log_set_always_fatal(G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL);
    gtk_init(&argc, &argv);

    // create main loop
    main_loop = g_main_loop_new(NULL, FALSE);
    if (main_loop == NULL)
        handle_error("Couldn't create GMainLoop", "Unknown(OOM?)", TRUE);
    
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
    bus_proxy = dbus_g_proxy_new_for_name(bus,
                                         DBUS_SERVICE_DBUS,
                                         DBUS_PATH_DBUS,
                                         DBUS_INTERFACE_DBUS);
    if (bus_proxy == NULL)
        handle_error("Couldn't get a proxy for D-Bus",
                    "Unknown(dbus_g_proxy_new_for_name)",
                    TRUE);
    print_debug_ok(debug);

    // register the service
    print_debug("Registering the service...", debug);
    if (!dbus_g_proxy_call(bus_proxy,
                           "RequestName",
                           &error,

                           G_TYPE_STRING,
                           VALUE_SERVICE_NAME,
                           G_TYPE_UINT,
                           0,
                           G_TYPE_INVALID,

                           G_TYPE_UINT,
                           &result,
                           G_TYPE_INVALID))
        handle_error("D-Bus.RequestName RPC failed",
                  error->message,
                  TRUE);
    if (result != 1)
        handle_error("Failed to get the primary well-known name.",
                    "RequestName result != 1", TRUE);
    print_debug_ok(debug);

    // create the Volume object
    print_debug("Preparing data...", debug);
    status = g_object_new(VOLUME_TYPE_OBJECT, NULL);
    if (status == NULL)
        handle_error("Failed to create one VolumeObject instance.",
                    "Unknown(OOM?)", TRUE);

    status->debug = debug;
    status->timeout = timeout;
    status->settings = settings;

    // volume icons
    status->icon_high = gdk_pixbuf_new_from_file(IMAGE_PATH "volume_high.svg", &error);
    if (error != NULL)
        handle_error("Couldn't load volume_high.svg.", error->message, TRUE);
    status->icon_medium = gdk_pixbuf_new_from_file(IMAGE_PATH "volume_medium.svg", &error);
    if (error != NULL)
        handle_error("Couldn't load volume_medium.svg.", error->message, TRUE);
    status->icon_low = gdk_pixbuf_new_from_file(IMAGE_PATH "volume_low.svg", &error);
    if (error != NULL)
        handle_error("Couldn't load volume_low.svg.", error->message, TRUE);
    status->icon_off = gdk_pixbuf_new_from_file(IMAGE_PATH "volume_off.svg", &error);
    if (error != NULL)
        handle_error("Couldn't load volume_off.svg.", error->message, TRUE);
    status->icon_muted = gdk_pixbuf_new_from_file(IMAGE_PATH "volume_muted.svg", &error);
    if (error != NULL)
        handle_error("Couldn't load volume_muted.svg.", error->message, TRUE);
    status->icon_brightness = gdk_pixbuf_new_from_file(IMAGE_PATH "brightness.svg", &error);
    if (error != NULL)
        handle_error("Couldn't load brightness.svg.", error->message, TRUE);

    // progress bar
    status->image_progressbar_empty = gdk_pixbuf_new_from_file(IMAGE_PATH "progressbar_empty.png", &error);
    if (error != NULL)
        handle_error("Couldn't load progressbar_empty.png.", error->message, TRUE);
    status->image_progressbar_full = gdk_pixbuf_new_from_file(IMAGE_PATH "progressbar_full.png", &error);
    if (error != NULL)
        handle_error("Couldn't load progressbar_full.png.", error->message, TRUE);

    // check that the images are of the same size
    if (gdk_pixbuf_get_width(status->image_progressbar_empty) != gdk_pixbuf_get_width(status->image_progressbar_full) ||
        gdk_pixbuf_get_height(status->image_progressbar_empty) != gdk_pixbuf_get_height(status->image_progressbar_full) ||
        gdk_pixbuf_get_bits_per_sample(status->image_progressbar_empty) != gdk_pixbuf_get_bits_per_sample(status->image_progressbar_full))
        handle_error("Progress bar images aren't of the same size or don't have the same number of bits per sample.", "Unknown(OOM?)", TRUE);

    // create pixbuf for combined image
    status->width_progressbar = gdk_pixbuf_get_width(status->image_progressbar_empty);
    status->height_progressbar = gdk_pixbuf_get_height(status->image_progressbar_empty);
    status->image_progressbar = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
                                               TRUE,
                                               gdk_pixbuf_get_bits_per_sample(status->image_progressbar_empty),
                                               status->width_progressbar,
                                               status->height_progressbar);

    print_debug_ok(debug);

    // register the Volume object
    print_debug("Registering volume object...", debug);
    dbus_g_connection_register_g_object(bus,
                                        VALUE_SERVICE_OBJECT_PATH,
                                        G_OBJECT(status));
    print_debug_ok(debug);

    // daemonize
    if (!no_daemon) {
        print_debug("Daemonizing...\n", debug);
        if (daemon(0, 0) != 0)
            handle_error("failed to daemonize", "unknown", FALSE);
    }

    // Run forever
    print_debug("Running the main loop...\n", debug);
    g_main_loop_run(main_loop);
    return EXIT_FAILURE;
}
