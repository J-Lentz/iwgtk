/*
 *  Copyright 2022 Jesse Lentz and contributors
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

#define FLOAT_00 0.0
#define FLOAT_64 (float) 0x64 / (float) 0xff
#define FLOAT_80 (float) 0x80 / (float) 0xff
#define FLOAT_ff 1.0

#define COLOR_LIME       {FLOAT_00, FLOAT_ff, FLOAT_00, 1.0}
#define COLOR_YELLOW     {FLOAT_ff, FLOAT_ff, FLOAT_00, 1.0}
#define COLOR_DARK_GREEN {FLOAT_00, FLOAT_64, FLOAT_00, 1.0}
#define COLOR_GRAY       {FLOAT_80, FLOAT_80, FLOAT_80, 1.0}

ColorTable colors = {
    COLOR_LIME,       // station_connected
    COLOR_YELLOW,     // station_connecting
    COLOR_GRAY,       // station_disconnected

    COLOR_LIME,       // ap_up
    COLOR_GRAY,       // ap_down

    COLOR_LIME,       // adhoc_up
    COLOR_GRAY,       // adhoc_down

    COLOR_GRAY,       // disabled_device
    COLOR_GRAY,       // disabled_adapter

    COLOR_LIME,       // network_connected
    COLOR_YELLOW,     // network_connecting
    COLOR_DARK_GREEN, // network_known
    COLOR_GRAY,       // network_unknown
    COLOR_GRAY        // network_hidden
};

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

const GdkRGBA *color_status[] = {
    &colors.network_connected,
    &colors.network_connecting,
    &colors.network_known,
    &colors.network_unknown
};

void icon_theme_set() {
    GdkDisplay *display;

    const gchar* icons[] = {
	ICON_STATION_0,
	ICON_STATION_1,
	ICON_STATION_2,
	ICON_STATION_3,
	ICON_STATION_4,
	ICON_STATION_OFFLINE,
	ICON_AP,
	ICON_ADHOC,
	ICON_DEVICE_DISABLED,
	ICON_ADAPTER_DISABLED,
	NULL
    };

    display = gdk_display_get_default();
    global.theme = gtk_icon_theme_get_for_display(display);

    for (int i = 0; icons[i] != NULL; i ++) {
	if (!gtk_icon_theme_has_icon(global.theme, icons[i])) {
	    g_printerr("Icon theme '%s' is missing icon '%s': Overriding theme to Adwaita\n",
		    gtk_icon_theme_get_theme_name(global.theme), icons[i]);

	    global.theme = gtk_icon_theme_new();
	    gtk_icon_theme_set_theme_name(global.theme, "Adwaita");
	    break;
	}
    }
}

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
	global.theme,
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
    gtk_picture_set_paintable(GTK_PICTURE(image), paintable);
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
