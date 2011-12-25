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
 
#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <gtk/gtk.h>
#include <cairo.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

typedef struct {
    gfloat alpha;
    gint corner_radius;
} Settings;

Settings get_default_settings();
GtkWindow* create_notification(Settings settings);
void move_notification(GtkWindow *win, int x, int y);
void set_notification_icon(GtkWindow *nw, GdkPixbuf *pixbuf);
void set_progressbar_image (GtkWindow *nw, GdkPixbuf *pixbuf);
void destroy_notification(GtkWindow *win);

#endif /* NOTIFICATION_H */
