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

#ifndef _IWGTK_KNOWN_NETWORKS_H
#define _IWGTK_KNOWN_NETWORKS_H

typedef struct KnownNetwork_s KnownNetwork;

struct KnownNetwork_s {
    GDBusProxy *proxy;
    gulong handler_update;

    // Widgets
    GtkWidget *name_label;
    GtkWidget *hidden_label;
    GtkWidget *security_label;
    GtkWidget *last_connection_label;
};

void known_network_forget(GDBusProxy *proxy);
KnownNetwork* known_network_add(Window *window, GDBusObject *object, GDBusProxy *proxy);
void known_network_remove(Window *window, KnownNetwork *known_network);

#endif
