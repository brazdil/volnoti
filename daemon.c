#include <stdlib.h>
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <unistd.h>

#include "common-defs.h"

typedef struct {
	GObject parent;
	gint volume;
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
}

static void volume_object_class_init(VolumeObjectClass* klass) {
	g_assert(klass != NULL);
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

	return TRUE;
}

static void handleError(const char* msg, const char* reason, gboolean fatal) {
    g_printerr("ERROR: %s (%s)\n", msg, reason);
    if (fatal)
        exit(EXIT_FAILURE);
}

static void printDebug(const gchar *msg, int debug) {
    if (debug)
        g_print("%s", msg);
}

static void printDebugOK(int debug) {
    if (debug)
        g_print(" OK\n");
}

int main(int argc, char* argv[]) {

#ifdef DEBUG
    int debug = TRUE;
#else
    int debug = FALSE;
#endif

    DBusGConnection *bus = NULL;
    DBusGProxy *busProxy = NULL;
    GMainLoop *mainLoop = NULL;
    GError *error = NULL;
    guint result;

    // initialize GObject
    g_type_init();

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

    // Run forever
    printDebug("Running the main loop...\n", debug);
    g_main_loop_run(mainLoop);
    return EXIT_FAILURE;
}
