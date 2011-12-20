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
