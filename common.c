#include <stdlib.h>
#include <glib.h>

#include "common.h"

void handle_error(const char* msg, const char* reason, gboolean fatal) {
    g_printerr("ERROR: %s (%s)\n", msg, reason);
    if (fatal)
        exit(EXIT_FAILURE);
}

void print_debug(const gchar *msg, int debug) {
    if (debug)
        g_print("%s", msg);
}

void print_debug_ok(int debug) {
    if (debug)
        g_print(" OK\n");
}
