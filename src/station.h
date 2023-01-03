/*
 *  Copyright 2020-2022 Jesse Lentz and contributors
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

#define PROVISION_MENU_WIDTH 180

typedef struct Station_s Station;
typedef struct Device_s Device;
typedef struct Network_s Network;
typedef struct DPP_s DPP;

typedef enum {
    STATION_CONNECTED,
    STATION_CONNECTING,
    STATION_DISCONNECTED,
    STATION_SCANNING
} StationState;

struct Station_s {
    GDBusProxy *proxy;
    DPP *dpp;
    StationState state;
    gulong handler_update;

    // Networks
    gsize n_networks;
    Network *networks;
    Network *network_connected;

    // Widgets
    GtkWidget *scan_button;
    GtkWidget *scan_widget_idle;
    GtkWidget *scan_widget_scanning;

    GtkWidget *provision_button;
    GtkWidget *provision_menu;
    GtkWidget *provision_vbox;

    GtkWidget *network_table;
};

void station_set(Station *station);
Station* station_add(Window *window, GDBusObject *object, GDBusProxy *proxy);
void station_remove(Window *window, Station *station);
void bind_device_station(Device *device, Station *station);
void unbind_device_station(Device *device, Station *station);

void send_scan_request(Station *station);
void insert_separator(Station *station, gint position);
void station_network_table_build(Station *station);
void station_network_table_clear(Station *station);
void get_networks_callback(GDBusProxy *proxy, GAsyncResult *res, Station *station);
void get_hidden_networks_callback(GDBusProxy *proxy, GAsyncResult *res, Station *station);

#endif
