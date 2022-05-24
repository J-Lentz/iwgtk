/*
 *  Copyright 2020-2022 Jesse Lentz
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

#ifndef _IWGTK_STATION_H
#define _IWGTK_STATION_H

typedef struct Station_s Station;
typedef struct Device_s Device;
typedef struct Network_s Network;

typedef enum {
    STATION_CONNECTED,
    STATION_CONNECTING,
    STATION_DISCONNECTED
} StationState;

struct Station_s {
    GDBusProxy *proxy;
    Device *device;
    StationState state;

    // Networks
    gsize n_networks;
    Network *networks;
    Network *network_connected;

    // Widgets
    GtkWidget *network_table;
    GtkWidget *scan_button;
    GtkWidget *scan_widget_idle;
    GtkWidget *scan_widget_scanning;

    // Handlers
    gulong handler_update;
    gulong handler_scan;
};

void scan_button_clicked(GtkButton *button, Station *station);
void network_remove_callback(GtkWidget *network, GtkWidget *network_table);
void scan_button_update(GDBusProxy *proxy, GVariant *properties, gchar **invalidated_properties, Station *station);
void scan_button_set_child(Station *station, gboolean scanning);
GtkWidget* scan_button_new(Station *station);
void station_set(Station *station);
Station* station_add(Window *window, GDBusObject *object, GDBusProxy *proxy);
void station_remove(Window *window, Station *station);
void bind_device_station(Device *device, Station *station);
void unbind_device_station(Device *device, Station *station);
void insert_separator(Station *station);
void populate_network_list(Station *station);
void get_networks_callback(GDBusProxy *proxy, GAsyncResult *res, Station *station);
void get_hidden_networks_callback(GDBusProxy *proxy, GAsyncResult *res, Station *station);
void station_remove_all_networks(Station *station);

#endif
