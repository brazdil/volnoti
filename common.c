#include <stdlib.h>
#include <glib.h>

#include "common.h"

void handleError(const char* msg, const char* reason, gboolean fatal) {
    g_printerr("ERROR: %s (%s)\n", msg, reason);
    if (fatal)
        exit(EXIT_FAILURE);
}

void printDebug(const gchar *msg, int debug) {
    if (debug)
        g_print("%s", msg);
}

void printDebugOK(int debug) {
    if (debug)
        g_print(" OK\n");
}
