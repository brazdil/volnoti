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
 
#ifndef INCLUDE_COMMON_DEFS_H
#define INCLUDE_COMMON_DEFS_H

#define VALUE_SERVICE_NAME        "uk.ac.cam.db538.volume-notification"
#define VALUE_SERVICE_OBJECT_PATH "/VolumeNotification"
/* And we're interested in using it through this interface.
   This must match the entry in the interface definition XML. */
#define VALUE_SERVICE_INTERFACE   "uk.ac.cam.db538.VolumeNotification"
#define VOL_MUTED 1
#define MIC_MUTED 2
#define MIC_UNMUTED 3

void handle_error(const char* msg, const char* reason, gboolean fatal);
void print_debug(const gchar *msg, int debug);
void print_debug_ok(int debug);

#endif
