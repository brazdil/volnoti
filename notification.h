#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <gtk/gtk.h>
#include <cairo.h>

GtkWindow* create_notification();
void move_notification (GtkWindow *win, int x, int y);
void destroy_notification (GtkWindow *win);

#endif /* NOTIFICATION_H */
