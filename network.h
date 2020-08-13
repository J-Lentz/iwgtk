/*
 *  Copyright 2020 Jesse Lentz
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

typedef struct {
    GDBusProxy *proxy;
    GDBusProxy *station_proxy;

    GtkWidget *status_icon;
    GtkWidget *ssid_label;
    GtkWidget *security_label;
    GtkWidget *connect_button;

    gulong button_handler_id;
} Network;

void security_type_set(GDBusProxy *proxy, GtkWidget *label);
void connect_button_clicked(GtkButton *button, GDBusProxy *network_proxy);
void disconnect_button_clicked(GtkButton *button, GDBusProxy *station_proxy);
void connect_button_set(Network *network);
void network_update(GDBusProxy *network_proxy, GVariant *properties, gchar **invalidated_properties, Network *network);
Network* network_add(GDBusObject *object, GDBusProxy *proxy);
void network_remove(Network *network);
void bind_station_network(Station *station, Network *network, gint16 signal_strength, int index);
Network* network_lookup(const char *path);

#endif
