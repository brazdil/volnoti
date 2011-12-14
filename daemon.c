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
    obj->volume = 100;
    obj->notification = NULL;
}

static void volume_object_class_init(VolumeObjectClass* klass) {
    g_assert(klass != NULL);

    dbus_g_object_type_install_info(VOLUME_TYPE_OBJECT,
                                    &dbus_glib_volume_object_object_info);
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

    g_print("Notify %d\n", obj->volume);

    if (obj->notification == NULL) {
    	obj->notification = create_notification();
        gtk_widget_realize(GTK_WIDGET(obj->notification));
//        move_notification(GTK_WINDOW(obj->notification), 32, 32);
    }

    gtk_widget_show(GTK_WIDGET(obj->notification));

    return TRUE;
}

static void printUsage(const char* filename, int failure) {
    g_print("Usage: %s [-v] [-n]\n"
    		" -h\t--help\t\thelp\n"
    		" -v\t--verbose\tverbose\n"
    		" -n\t--no-daemon\tdo not daemonize\n");
    if (failure)
    	exit(EXIT_FAILURE);
    else
    	exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    void *options = gopt_sort(&argc, (const char**) argv, gopt_start(
            gopt_option('h', 0, gopt_shorts('h', '?'), gopt_longs("help", "HELP")),
            gopt_option('n', 0, gopt_shorts('n'), gopt_longs("no-daemon")),
            gopt_option('v', GOPT_REPEAT, gopt_shorts('v'), gopt_longs("verbose"))));
    int help = gopt(options, 'h');
    int debug = gopt(options, 'v');
    int no_daemon = gopt(options, 'n');
    gopt_free(options);

    if (help)
    	printUsage(argv[0], FALSE);

    DBusGConnection *bus = NULL;
    DBusGProxy *busProxy = NULL;
    VolumeObject *status = NULL;
    GMainLoop *mainLoop = NULL;
    GError *error = NULL;
    guint result;

    // initialize GObject and GTK
    g_type_init();
    g_log_set_always_fatal(G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL);
    gtk_init(&argc, &argv);

    // create main loop
    mainLoop = g_main_loop_new(NULL, FALSE);
    if (mainLoop == NULL)
        handleError("Couldn't create GMainLoop", "Unknown(OOM?)", TRUE);
    
    // connect to D-Bus
    printDebug("Connecting to D-Bus...", debug);
    bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    if (error != NULL)
        handleError("Couldn't connect to D-Bus",
                    error->message,
                    TRUE);
    printDebugOK(debug);

    // get the proxy
    printDebug("Getting proxy...", debug);
    busProxy = dbus_g_proxy_new_for_name(bus,
                                         DBUS_SERVICE_DBUS,
                                         DBUS_PATH_DBUS,
                                         DBUS_INTERFACE_DBUS);
    if (busProxy == NULL)
        handleError("Couldn't get a proxy for D-Bus",
                    "Unknown(dbus_g_proxy_new_for_name)",
                    TRUE);
    printDebugOK(debug);

    // register the service
    printDebug("Registering the service...", debug);
    if (!dbus_g_proxy_call(busProxy,
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
        handleError("D-Bus.RequestName RPC failed",
                  error->message,
                  TRUE);
    if (result != 1)
        handleError("Failed to get the primary well-known name.",
                    "RequestName result != 1", TRUE);
    printDebugOK(debug);

    // create the Volume object
    printDebug("Creating volume object...", debug);
    status = g_object_new(VOLUME_TYPE_OBJECT, NULL);
    if (status == NULL)
        handleError("Failed to create one VolumeObject instance.",
                    "Unknown(OOM?)", TRUE);
    printDebugOK(debug);

    // register the Volume object
    printDebug("Registering volume object...", debug);
    dbus_g_connection_register_g_object(bus,
                                        VALUE_SERVICE_OBJECT_PATH,
                                        G_OBJECT(status));
    printDebugOK(debug);

    // daemonize
    if (!no_daemon) {
        printDebug("Daemonizing...\n", debug);
		if (daemon(0, 0) != 0)
			handleError("failed to daemonize", "unknown", FALSE);
    }

    // Run forever
    printDebug("Running the main loop...\n", debug);
    g_main_loop_run(mainLoop);
    return EXIT_FAILURE;
}
