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

    Window *windows;

    // Command line flags
    gboolean icon_disable;
    gboolean notifications_disable;
    gboolean signal_icon_disable;
} GlobalData;

extern GlobalData global;
extern const ErrorMessage detailed_errors_standard[];

void object_manager_callback(GDBusObjectManagerClient *manager, GAsyncResult *res);
void iwd_up_handler(GDBusConnection *conn, const gchar *name, const gchar *name_owner);
void iwd_down_handler(GDBusConnection *conn, const gchar *name);
void startup(GtkApplication *app);
void print_version();

#endif
