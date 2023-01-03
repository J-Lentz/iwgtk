/*
 *  Copyright 2020-2023 Jesse Lentz and contributors
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

#ifndef _IWGTK_NETWORK_H
#define _IWGTK_NETWORK_H

typedef struct Network_s Network;
typedef struct Station_s Station;

typedef enum {
    NETWORK_CONNECTED,
    NETWORK_CONNECTING,
    NETWORK_KNOWN,
    NETWORK_UNKNOWN
} NetworkStatus;

struct Network_s {
    GDBusProxy *proxy;
    Station *station;

    gint8 level;

    // Widgets
    GtkWidget *status_icon;
    GtkWidget *ssid_label;
    GtkWidget *security_label;
    GtkWidget *connect_button;

    // Handlers
    gulong handler_update;
    gulong button_handler_id;
};

const gchar* get_security_type(const gchar *type_raw);
void connect_button_clicked(GtkButton *button, GDBusProxy *network_proxy);
void disconnect_button_clicked(GtkButton *button, Network *network);
void network_set(Network *network);
void station_add_network(Station *station, GDBusProxy *network_proxy, gint16 signal_strength, int index);
void network_remove(Network *network);

#endif
