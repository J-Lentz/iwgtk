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

void icon_load(const gchar *icon_name, const GdkRGBA *icon_color, IconLoadCallback user_callback, gpointer user_object) {
    GtkIconInfo *icon_info;
    IconLoadClosure *closure;

    closure = g_malloc(sizeof(IconLoadClosure));
    closure->callback = user_callback;
    closure->object = user_object;

    icon_info = gtk_icon_theme_lookup_icon(
	gtk_icon_theme_get_default(),
	icon_name,
	32,
	GTK_ICON_LOOKUP_FORCE_SYMBOLIC
    );

    gtk_icon_info_load_symbolic_async(
	icon_info,
	icon_color,
	NULL, NULL, NULL, NULL,
	(GAsyncReadyCallback) icon_load_finish,
	closure
    );
}

void icon_load_finish(GtkIconInfo *icon_info, GAsyncResult *res, IconLoadClosure *closure) {
    GdkPixbuf *pixbuf;
    gboolean was_symbolic;
    GError *err;

    err = NULL;
    pixbuf = gtk_icon_info_load_symbolic_finish(icon_info, res, &was_symbolic, &err);

    g_object_unref(icon_info);

    if (err != NULL) {
	fprintf(stderr, "Error loading icon data: %s\n", err->message);
	g_error_free(err);
    }

    if (!was_symbolic) {
	fprintf(stderr, "Failed to find symbolic icon; using non-symbolic icon instead\n");
    }

    closure->callback(closure->object, pixbuf);

    g_object_unref(pixbuf);
    g_free(closure);
}
