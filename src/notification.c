/**
 *  Volnoti - Lightweight Volume Notification

 *  Copyright (C) 2006-2007 Christian Hammond <chipx86@chipx86.com>
 *  Copyright (C) 2009 Red Hat, Inc.
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
 
#include "notification.h"

#define USE_COMPOSITE

#define WIDTH                   400
#define DEFAULT_X0              0
#define DEFAULT_Y0              0
#define DEFAULT_RADIUS          30
#define DEFAULT_BORDER          (DEFAULT_RADIUS * 3 / 2)

#define IMAGE_SIZE              110
#define IMAGE_PADDING           (IMAGE_SIZE / 3)
#define BODY_X_OFFSET           (IMAGE_SIZE + 8)
#define MAX_ICON_SIZE           IMAGE_SIZE
#define MAX_PROGRESSBAR_SIZE    (IMAGE_SIZE * 18 / 10)

typedef struct {
    GtkWidget *win;
    GtkWidget *main_vbox;
    GtkWidget *iconbox;
    GtkWidget *icon;
    GtkWidget *progressbarbox;
    GtkWidget *progressbar;

    int width;
    int height;
    int last_width;
    int last_height;

    gboolean composited;
    NotificationProperties properties;
} WindowData;

static void
color_reverse(const GdkColor *a, GdkColor *b) {
    gdouble red;
    gdouble green;
    gdouble blue;
    gdouble h;
    gdouble s;
    gdouble v;

    red = (gdouble) a->red / 65535.0;
    green = (gdouble) a->green / 65535.0;
    blue = (gdouble) a->blue / 65535.0;

    gtk_rgb_to_hsv (red, green, blue, &h, &s, &v);

    /* pivot brightness around the center */
    v = 0.5 + (0.5 - v);
    if (v > 1.0)
        v = 1.0;
    else if (v < 0.0)
        v = 0.0;

    /* reduce saturation by 50% */
    s *= 0.5;

    gtk_hsv_to_rgb (h, s, v, &red, &green, &blue);

    b->red = red * 65535.0;
    b->green = green * 65535.0;
    b->blue = blue * 65535.0;
}

static GdkPixbuf *
scale_pixbuf (GdkPixbuf *pixbuf,
              int        max_width,
              int        max_height,
              gboolean   no_stretch_hint)
{
        int        pw;
        int        ph;
        float      scale_factor_x = 1.0;
        float      scale_factor_y = 1.0;
        float      scale_factor = 1.0;

        pw = gdk_pixbuf_get_width (pixbuf);
        ph = gdk_pixbuf_get_height (pixbuf);

        /* Determine which dimension requires the smallest scale. */
        scale_factor_x = (float) max_width / (float) pw;
        scale_factor_y = (float) max_height / (float) ph;

        if (scale_factor_x > scale_factor_y) {
                scale_factor = scale_factor_y;
        } else {
                scale_factor = scale_factor_x;
        }

        /* always scale down, allow to disable scaling up */
        if (scale_factor < 1.0 || !no_stretch_hint) {
                int scale_x;
                int scale_y;

                scale_x = (int) (pw * scale_factor);
                scale_y = (int) (ph * scale_factor);
                return gdk_pixbuf_scale_simple (pixbuf,
                                                scale_x,
                                                scale_y,
                                                GDK_INTERP_BILINEAR);
        } else {
                return g_object_ref (pixbuf);
        }
}

static void
draw_round_rect(cairo_t* cr,
                gdouble  aspect,
                gdouble  x,
                gdouble  y,
                gdouble  corner_radius,
                gdouble  width,
                gdouble  height) {
    gdouble radius = corner_radius / aspect;

    cairo_move_to (cr, x + radius, y);

    // top-right, left of the corner
    cairo_line_to (cr,
                   x + width - radius,
                   y);

    // top-right, below the corner
    cairo_arc (cr,
               x + width - radius,
               y + radius,
               radius,
               -90.0f * G_PI / 180.0f,
               0.0f * G_PI / 180.0f);

    // bottom-right, above the corner
    cairo_line_to (cr,
                   x + width,
                   y + height - radius);

    // bottom-right, left of the corner
    cairo_arc (cr,
               x + width - radius,
               y + height - radius,
               radius,
               0.0f * G_PI / 180.0f,
               90.0f * G_PI / 180.0f);

    // bottom-left, right of the corner
    cairo_line_to (cr,
                   x + radius,
                   y + height);

    // bottom-left, above the corner
    cairo_arc (cr,
               x + radius,
               y + height - radius,
               radius,
               90.0f * G_PI / 180.0f,
               180.0f * G_PI / 180.0f);

    // top-left, below the corner
    cairo_line_to (cr,
                   x,
                   y + radius);

    // top-left, right of the corner
    cairo_arc (cr,
               x + radius,
               y + radius,
               radius,
               180.0f * G_PI / 180.0f,
               270.0f * G_PI / 180.0f);
}

static void
fill_background(GtkWidget *widget, WindowData *windata, cairo_t *cr) {
    GdkColor  color;
    double    r, g, b;

    draw_round_rect (cr,
                     1.0f,
                     DEFAULT_X0 + 1,
                     DEFAULT_Y0 + 1,
                     DEFAULT_RADIUS,
                     widget->allocation.width - 2,
                     widget->allocation.height - 2);

    color = widget->style->bg [GTK_STATE_NORMAL];
    r = (float)color.red / 65535.0;
    g = (float)color.green / 65535.0;
    b = (float)color.blue / 65535.0;
    cairo_set_source_rgba (cr, r, g, b, windata->properties.alpha);
    cairo_fill_preserve (cr);

    // border
//  color = widget->style->text_aa [GTK_STATE_NORMAL];
//  r = (float) color.red / 65535.0;
//  g = (float) color.green / 65535.0;
//  b = (float) color.blue / 65535.0;
//  cairo_set_source_rgba (cr, r, g, b, BACKGROUND_ALPHA / 2);
//  cairo_set_line_width (cr, 1);
//  cairo_stroke (cr);
}

static void
update_shape(WindowData *windata) {
    GdkBitmap *mask;
    cairo_t   *cr;

    if (windata->width == windata->last_width
        && windata->height == windata->last_height) {
            return;
    }

    if (windata->width == 0 || windata->height == 0) {
            windata->width = MAX(windata->win->allocation.width, 1);
            windata->height = MAX(windata->win->allocation.height, 1);
    }

    if (windata->composited) {
            gtk_widget_shape_combine_mask (windata->win, NULL, 0, 0);
            return;
    }

    windata->last_width = windata->width;
    windata->last_height = windata->height;
    mask = (GdkBitmap *) gdk_pixmap_new (NULL,
                                         windata->width,
                                         windata->height,
                                         1);
    if (mask == NULL) {
            return;
    }

    cr = gdk_cairo_create (mask);
    if (cairo_status (cr) == CAIRO_STATUS_SUCCESS) {
            cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
            cairo_paint (cr);

            cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
            cairo_set_source_rgb (cr, 1.0f, 1.0f, 1.0f);
            draw_round_rect (cr,
                             1.0f,
                             DEFAULT_X0,
                             DEFAULT_Y0,
                             DEFAULT_RADIUS,
                             windata->width,
                             windata->height);
            cairo_fill (cr);

            gtk_widget_shape_combine_mask (windata->win, mask, 0, 0);
    }
    cairo_destroy (cr);

    g_object_unref (mask);
}


static void
paint_window(GtkWidget *widget, WindowData *windata) {
    cairo_t         *context;
    cairo_surface_t *surface;
    cairo_t         *cr;

    if (windata->width == 0 || windata->height == 0) {
            windata->width = MAX (windata->win->allocation.width, 1);
            windata->height = MAX (windata->win->allocation.height, 1);
    }

    context = gdk_cairo_create (widget->window);

    cairo_set_operator (context, CAIRO_OPERATOR_SOURCE);
    surface = cairo_surface_create_similar (cairo_get_target (context),
                                            CAIRO_CONTENT_COLOR_ALPHA,
                                            widget->allocation.width,
                                            widget->allocation.height);
    cr = cairo_create (surface);

    fill_background (widget, windata, cr);

    cairo_destroy (cr);
    cairo_set_source_surface (context, surface, 0, 0);
    cairo_paint (context);
    cairo_surface_destroy (surface);
    cairo_destroy (context);

    update_shape (windata);
}

static void
override_style(GtkWidget *widget, GtkStyle  *previous_style) {
    GtkStateType state;
    GtkStyle    *style;
    GdkColor     fg;
    GdkColor     bg;

    style = gtk_style_copy (widget->style);
    if (previous_style == NULL
        || (previous_style != NULL
            && (previous_style->bg[GTK_STATE_NORMAL].red != style->bg[GTK_STATE_NORMAL].red
                || previous_style->bg[GTK_STATE_NORMAL].green != style->bg[GTK_STATE_NORMAL].green
                || previous_style->bg[GTK_STATE_NORMAL].blue != style->bg[GTK_STATE_NORMAL].blue))) {

            state = (GtkStateType) 0;
            while (state < (GtkStateType) G_N_ELEMENTS (widget->style->bg))  {
                color_reverse (&style->bg[state], &bg);
                gtk_widget_modify_bg (widget, state, &bg);
                state++;
            }

    }

    if (previous_style == NULL
        || (previous_style != NULL
            && (previous_style->fg[GTK_STATE_NORMAL].red != style->fg[GTK_STATE_NORMAL].red
                || previous_style->fg[GTK_STATE_NORMAL].green != style->fg[GTK_STATE_NORMAL].green
                || previous_style->fg[GTK_STATE_NORMAL].blue != style->fg[GTK_STATE_NORMAL].blue))) {

            state = (GtkStateType) 0;
            while (state < (GtkStateType) G_N_ELEMENTS (widget->style->fg)) {
                color_reverse (&style->fg[state], &fg);
                gtk_widget_modify_fg (widget, state, &fg);
                state++;
            }
    }

    g_object_unref (style);
}

static void
destroy_windata(WindowData *windata) {
    g_free (windata);
}

static gboolean
on_configure_event(GtkWidget *widget, GdkEventConfigure *event, WindowData *windata) {
    windata->width = event->width;
    windata->height = event->height;

    gtk_widget_queue_draw(widget);

    return FALSE;
}

static void
on_style_set(GtkWidget *widget, GtkStyle *previous_style, WindowData *windata) {
    g_signal_handlers_block_by_func (G_OBJECT(widget), on_style_set, windata);
    override_style (widget, previous_style);

    gtk_widget_queue_draw (widget);

    g_signal_handlers_unblock_by_func (G_OBJECT(widget), on_style_set, windata);
}

static void
on_composited_changed(GtkWidget  *window, WindowData *windata) {
    windata->composited = gdk_screen_is_composited (gtk_widget_get_screen (window));
    update_shape(windata);
}

static void
on_window_realize(GtkWidget  *widget, WindowData *windata) {
}

static gboolean
on_window_expose (GtkWidget *widget, GdkEventExpose *event, WindowData *windata) {
    paint_window (widget, windata);
    return FALSE;
}

static gboolean
on_window_map (GtkWidget  *widget, GdkEvent   *event, WindowData *windata) {
    return FALSE;
}

GtkWindow* create_notification(NotificationProperties properties) {
    WindowData *windata;

    GtkWidget   *win;

#ifdef USE_COMPOSITE
    GdkColormap *colormap;
    GdkScreen   *screen;
#endif

    // create WindowData object
    windata = g_new0(WindowData, 1);

    // create GTK window
    win = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_window_set_resizable(GTK_WINDOW (win), FALSE);
    gtk_widget_set_app_paintable(win, TRUE);
    windata->win = win;
    windata->properties = properties;

    // connect signals
    g_signal_connect (G_OBJECT (win),
                      "style-set",
                      G_CALLBACK (on_style_set),
                      windata);
    g_signal_connect (G_OBJECT (win),
                      "map-event",
                      G_CALLBACK (on_window_map),
                      windata);
    g_signal_connect (G_OBJECT (win),
                      "expose-event",
                      G_CALLBACK (on_window_expose),
                      windata);
    g_signal_connect (G_OBJECT (win),
                      "realize",
                      G_CALLBACK (on_window_realize),
                      windata);

    // prepare composite
    windata->composited = FALSE;
#ifdef USE_COMPOSITE
    screen = gtk_window_get_screen (GTK_WINDOW (win));
    colormap = gdk_screen_get_rgba_colormap (screen);
    if (colormap != NULL) {
        gtk_widget_set_colormap (win, colormap);
        if (gdk_screen_is_composited (screen)) {
            windata->composited = TRUE;
        }
    }
    g_signal_connect(win,
                     "composited-changed",
                     G_CALLBACK (on_composited_changed),
                     windata);
#endif

    gtk_window_set_title(GTK_WINDOW (win), "Notification");
    gtk_window_set_type_hint(GTK_WINDOW (win),
                             GDK_WINDOW_TYPE_HINT_NOTIFICATION);
    gtk_window_set_default_size(GTK_WINDOW(win), 400, 400);
    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);

    g_object_set_data_full (G_OBJECT (win),
                            "windata", windata,
                            (GDestroyNotify)destroy_windata);

    g_signal_connect (G_OBJECT (win),
                      "configure-event",
                      G_CALLBACK (on_configure_event),
                      windata);

    windata->main_vbox = gtk_vbox_new(FALSE, 0);
    g_signal_connect(G_OBJECT(windata->main_vbox),
                     "style-set",
                     G_CALLBACK(on_style_set),
                     windata);
    gtk_widget_show(windata->main_vbox);
    gtk_container_add(GTK_CONTAINER(win), windata->main_vbox);
    gtk_container_set_border_width(GTK_CONTAINER(windata->main_vbox), DEFAULT_BORDER);

//    windata->main_hbox = gtk_hbox_new (FALSE, 0);
//    gtk_widget_show (windata->main_hbox);
//    gtk_box_pack_start (GTK_BOX (main_vbox),
//                        windata->main_hbox,
//                        FALSE, FALSE, 0);

    // icon box
    windata->iconbox = gtk_alignment_new (0.5f, 0, 0, 0);
    gtk_widget_show (windata->iconbox);
    gtk_alignment_set_padding (GTK_ALIGNMENT (windata->iconbox),
                               0, IMAGE_PADDING, 0, 0);
    gtk_box_pack_start (GTK_BOX (windata->main_vbox),
                        windata->iconbox,
                        FALSE, FALSE, 0);
    gtk_widget_set_size_request (windata->iconbox, BODY_X_OFFSET, -1);

    // icon
    windata->icon = gtk_image_new ();
    gtk_widget_show (windata->icon);
    gtk_container_add (GTK_CONTAINER (windata->iconbox), windata->icon);

    // progress bar box
    windata->progressbarbox = gtk_alignment_new (0.5f, 0, 0, 0);
    gtk_widget_show (windata->progressbarbox);
//    gtk_alignment_set_padding (GTK_ALIGNMENT (windata->iconbox),
//                               5, 0, 0, 0);
    gtk_box_pack_start (GTK_BOX (windata->main_vbox),
                        windata->progressbarbox,
                        FALSE, FALSE, 0);
    gtk_widget_set_size_request (windata->progressbarbox, BODY_X_OFFSET, -1);

    // progress bar
    windata->progressbar = gtk_image_new ();
    gtk_widget_show (windata->progressbar);
    gtk_container_add (GTK_CONTAINER (windata->progressbarbox), windata->progressbar);

    return GTK_WINDOW(win);
}

void
move_notification (GtkWindow *win, int x, int y) {
    gtk_window_move(GTK_WINDOW(win), x, y);
}

void
destroy_notification (GtkWindow *win) {
    gtk_widget_destroy(GTK_WIDGET(win));
}

void
set_notification_icon (GtkWindow *nw, GdkPixbuf *pixbuf) {
        WindowData *windata;
        GdkPixbuf  *scaled;

        windata = g_object_get_data (G_OBJECT (nw), "windata");

        g_assert (windata != NULL);

        scaled = NULL;
        if (pixbuf != NULL) {
                scaled = scale_pixbuf (pixbuf,
                                       MAX_ICON_SIZE,
                                       MAX_ICON_SIZE,
                                       TRUE);
        }

        gtk_image_set_from_pixbuf (GTK_IMAGE (windata->icon), scaled);

        if (scaled != NULL) {
                int pixbuf_width = gdk_pixbuf_get_width (scaled);

                gtk_widget_show (windata->icon);
                gtk_widget_set_size_request (windata->iconbox,
                                             MAX (BODY_X_OFFSET, pixbuf_width), -1);
                g_object_unref (scaled);
        } else {
                gtk_widget_hide (windata->icon);
                gtk_widget_set_size_request (windata->iconbox,
                                             BODY_X_OFFSET,
                                             -1);
        }
}

void
set_progressbar_image (GtkWindow *nw, GdkPixbuf *pixbuf) {
        WindowData *windata;
        GdkPixbuf  *scaled;

        windata = g_object_get_data (G_OBJECT (nw), "windata");

        g_assert (windata != NULL);

        scaled = NULL;
        if (pixbuf != NULL) {
                scaled = scale_pixbuf (pixbuf,
                                       MAX_PROGRESSBAR_SIZE,
                                       MAX_PROGRESSBAR_SIZE,
                                       TRUE);
        }

        gtk_image_set_from_pixbuf (GTK_IMAGE (windata->progressbar), scaled);

        if (scaled != NULL) {
                int pixbuf_width = gdk_pixbuf_get_width (scaled);

                gtk_widget_show (windata->icon);
                gtk_widget_set_size_request (windata->progressbarbox,
                                             MAX (BODY_X_OFFSET, pixbuf_width), -1);
                g_object_unref (scaled);
        } else {
                gtk_widget_hide (windata->icon);
                gtk_widget_set_size_request (windata->progressbarbox,
                                             BODY_X_OFFSET,
                                             -1);
        }
}
