#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <gtk/gtk.h>
#include <cairo.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

GtkWindow* create_notification();
void move_notification(GtkWindow *win, int x, int y);
void set_notification_icon(GtkWindow *nw, GdkPixbuf *pixbuf);
void destroy_notification(GtkWindow *win);

#endif /* NOTIFICATION_H */
