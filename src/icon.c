/*
 *  Copyright 2022 Jesse Lentz
 *
 *  This file is part of iwgtk.
 *
 *  iwgtk is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  iwgtk is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with iwgtk.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "iwgtk.h"

/*
 * Signal thresholds are in dBm.
 */

const gint16 signal_thresholds[] = {-60, -67, -74, -81};

const gchar* station_icons[] = {
    ICON_STATION_0,
    ICON_STATION_1,
    ICON_STATION_2,
    ICON_STATION_3,
    ICON_STATION_4
};

const GdkRGBA color_green =           {0.0, 1.0, 0.0, 1.0};
const GdkRGBA color_yellow =          {1.0, 1.0, 0.0, 1.0};
const GdkRGBA color_green_saturated = {0.25, 0.5, 0.25, 1.0};
const GdkRGBA color_gray =            {0.5, 0.5, 0.5, 1.0};

const GdkRGBA *color_status[] = {
    &color_green,
    &color_yellow,
    &color_green_saturated,
    &color_gray
};

gint8 get_signal_level(gint16 signal_strength) {
    gint8 i;

    for (i = 0; i < N_SIGNAL_THRESHOLDS; i ++) {
	if (signal_strength > 100*signal_thresholds[i]) {
	    return i;
	}
    }

    return N_SIGNAL_THRESHOLDS;
}

GtkSnapshot* symbolic_icon_get_snapshot(const gchar *icon_name, const GdkRGBA *icon_color) {
    GtkIconPaintable *icon;
    GtkSnapshot *snapshot;

    icon = gtk_icon_theme_lookup_icon(
	gtk_icon_theme_get_for_display(gdk_display_get_default()),
	icon_name,
	NULL,
	32,
	1,
	GTK_TEXT_DIR_NONE,
	GTK_ICON_LOOKUP_FORCE_SYMBOLIC
    );

    snapshot = gtk_snapshot_new();

    gtk_symbolic_paintable_snapshot_symbolic(
	GTK_SYMBOLIC_PAINTABLE(icon),
	GDK_SNAPSHOT(snapshot),
	32.0,
	32.0,
	icon_color,
	1
    );

    g_object_unref(icon);
    return snapshot;
}

void symbolic_icon_set_image(const gchar *icon_name, const GdkRGBA *icon_color, GtkWidget *image) {
    GtkSnapshot *snapshot;
    GdkPaintable *paintable;

    snapshot = symbolic_icon_get_snapshot(icon_name, icon_color);

    paintable = gtk_snapshot_free_to_paintable(snapshot, NULL);
    gtk_image_set_from_paintable(GTK_IMAGE(image), paintable);
    g_object_unref(paintable);
}

cairo_surface_t* symbolic_icon_get_surface(const gchar *icon_name, const GdkRGBA *icon_color) {
    GtkSnapshot *snapshot;
    cairo_surface_t *surface;

    snapshot = symbolic_icon_get_snapshot(icon_name, icon_color);
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 32, 32);

    {
	cairo_t *cr;
	GskRenderNode *node;

	cr = cairo_create(surface);
	node = gtk_snapshot_free_to_node(snapshot);
	gsk_render_node_draw(node, cr);
	//gsk_render_node_unref(node);
	cairo_destroy(cr);
    }

    return surface;
}
