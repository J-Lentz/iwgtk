/*
 *  Copyright 2022-2025 Jesse Lentz and contributors
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

#ifndef _IWGTK_ICON_H
#define _IWGTK_ICON_H

typedef struct ColorTable_s ColorTable;

#define ICON_STATION_0       "network-wireless-signal-excellent-symbolic"
#define ICON_STATION_1       "network-wireless-signal-good-symbolic"
#define ICON_STATION_2       "network-wireless-signal-ok-symbolic"
#define ICON_STATION_3       "network-wireless-signal-weak-symbolic"
#define ICON_STATION_4       "network-wireless-signal-none-symbolic"

/*
 * TODO: Use a different icon for ad-hoc mode.
 */

#define ICON_STATION_OFFLINE  "network-wireless-offline-symbolic"
#define ICON_AP               "network-wireless-hotspot-symbolic"
#define ICON_ADHOC            "network-wireless-hotspot-symbolic"
#define ICON_DEVICE_DISABLED  "network-wireless-disabled-symbolic"
#define ICON_ADAPTER_DISABLED "network-wireless-hardware-disabled-symbolic"

#define N_SIGNAL_THRESHOLDS 4

struct ColorTable_s {
    GdkRGBA station_connected;
    GdkRGBA station_connecting;
    GdkRGBA station_disconnected;

    GdkRGBA ap_up;
    GdkRGBA ap_down;

    GdkRGBA adhoc_up;
    GdkRGBA adhoc_down;

    GdkRGBA disabled_device;
    GdkRGBA disabled_adapter;

    GdkRGBA network_connected;
    GdkRGBA network_connecting;
    GdkRGBA network_known;
    GdkRGBA network_unknown;
    GdkRGBA network_hidden;
};

extern ColorTable colors;
extern const gint16 signal_thresholds[];
extern const gchar* station_icons[];

extern const GdkRGBA *color_status[];

void icon_theme_set();
gint8 get_signal_level(gint16 signal_strength);

GtkSnapshot* symbolic_icon_get_snapshot(const gchar *icon_name, const GdkRGBA *icon_color);
void symbolic_icon_set_image(const gchar *icon_name, const GdkRGBA *icon_color, GtkWidget *image);
cairo_surface_t* symbolic_icon_get_surface(const gchar *icon_name, const GdkRGBA *icon_color);

#endif
