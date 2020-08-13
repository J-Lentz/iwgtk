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

#ifndef _MAIN_H
#define _MAIN_H

typedef struct {
    GtkApplication *application;
    GDBusObjectManager *manager;
    GQuark iwd_error_domain;

    // Command line flags
    gboolean notifications_disable;
    gboolean signal_icon_disable;

    // Widgets
    GtkWidget *window;
    GtkWidget *master;
    GtkWidget *header;
    GtkWidget *main;

    GtkWidget *known_network_button;
    GtkWidget *known_network_table;
} GlobalData;

extern GlobalData global;
extern const ErrorMessage detailed_errors_standard[];

void known_network_table_show(GtkToggleButton *button);
void object_manager_callback(GDBusObjectManagerClient *manager, GAsyncResult *res);
void iwd_up_handler(GDBusConnection *conn, const gchar *name, const gchar *name_owner);
void iwd_down_handler(GDBusConnection *conn, const gchar *name);
static void activate(GtkApplication *app);
void print_version();

#endif
