#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <dbus/dbus-glib.h>

#include "common.h"
#include "gopt.h"
#include "notification.h"

typedef struct {
    GObject parent;
    gint volume;
    GtkWindow *notification;
    GdkPixbuf *icon_high;
    GdkPixbuf *icon_medium;
    GdkPixbuf *icon_low;
    GdkPixbuf *icon_off;
    GdkPixbuf *icon_muted;
    gint time_left;
    gint timeout;
    gboolean debug;
} VolumeObject;

typedef struct {
  GObjectClass parent;
} VolumeObjectClass;

GType volume_object_get_type(void);
gboolean volume_object_notify(VolumeObject* obj,
                              gint value_in,
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
                              GError** error) {
    g_assert(obj != NULL);

    if (value > 100)
        value = 100;
    if (value < -1)
        value = -1;

    obj->volume = value;

    if (obj->notification == NULL) {
        print_debug("Creating new notification...", obj->debug);
    	obj->notification = create_notification();
        gtk_widget_realize(GTK_WIDGET(obj->notification));
        g_timeout_add(1000, (GSourceFunc) time_handler, (gpointer) obj);
        print_debug_ok(obj->debug);
    }

    if (obj->volume >= 75)
    	set_notification_icon(GTK_WINDOW(obj->notification), obj->icon_high);
    else if (obj->volume >= 50)
    	set_notification_icon(GTK_WINDOW(obj->notification), obj->icon_medium);
    else if (obj->volume >= 25)
    	set_notification_icon(GTK_WINDOW(obj->notification), obj->icon_low);
    else if (obj->volume >= 0)
    	set_notification_icon(GTK_WINDOW(obj->notification), obj->icon_off);
    else
    	set_notification_icon(GTK_WINDOW(obj->notification), obj->icon_muted);

	obj->time_left = obj->timeout;
    gtk_widget_show_all(GTK_WIDGET(obj->notification));

    return TRUE;
}

static void print_usage(const char* filename, int failure) {
    g_print("Usage: %s [-v] [-n] [-t <int>]\n"
    		" -h\t\t--help\t\t\thelp\n"
    		" -v\t\t--verbose\t\tverbose\n"
    		" -t <int>\t--timeout <int>\t\tnotification timeout in seconds\n"
    		" -n\t\t--no-daemon\t\tdo not daemonize\n", filename);
    if (failure)
    	exit(EXIT_FAILURE);
    else
    	exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    void *options = gopt_sort(&argc, (const char**) argv, gopt_start(
            gopt_option('h', 0, gopt_shorts('h', '?'), gopt_longs("help", "HELP")),
            gopt_option('n', 0, gopt_shorts('n'), gopt_longs("no-daemon")),
            gopt_option('t', GOPT_ARG, gopt_shorts('t'), gopt_longs("timeout")),
            gopt_option('v', GOPT_REPEAT, gopt_shorts('v'), gopt_longs("verbose"))));

    int help = gopt(options, 'h');
    int debug = gopt(options, 'v');
    int no_daemon = gopt(options, 'n');

    int timeout = 3;
    if (gopt(options, 't')) {
        if (sscanf(gopt_arg_i(options, 't', 0), "%d", &timeout) != 1)
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

    GtkIconTheme *theme = gtk_icon_theme_get_default();
    if (theme == NULL)
        handle_error("Couldn't get the GTK+ theme.", "Unknown(OOM?)", TRUE);
    status->icon_high = gtk_icon_theme_load_icon(theme, "audio-volume-high", 256, 0, &error);
    if (error != NULL)
        handle_error("Couldn't load audio-volume-high icon.", "Unknown(OOM?)", TRUE);
    status->icon_medium = gtk_icon_theme_load_icon(theme, "audio-volume-medium", 256, 0, &error);
    if (error != NULL)
        handle_error("Couldn't load audio-volume-medium icon.", "Unknown(OOM?)", TRUE);
    status->icon_low = gtk_icon_theme_load_icon(theme, "audio-volume-low", 256, 0, &error);
    if (error != NULL)
        handle_error("Couldn't load audio-volume-low icon.", "Unknown(OOM?)", TRUE);
    status->icon_off = gtk_icon_theme_load_icon(theme, "audio-volume-off", 256, 0, &error);
    if (error != NULL)
        handle_error("Couldn't load audio-volume-off icon.", "Unknown(OOM?)", TRUE);
    status->icon_muted = gtk_icon_theme_load_icon(theme, "audio-volume-muted", 256, 0, &error);
    if (error != NULL)
        handle_error("Couldn't load audio-volume-muted icon.", "Unknown(OOM?)", TRUE);

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
