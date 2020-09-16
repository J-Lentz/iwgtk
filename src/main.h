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

#ifndef _IWGTK_MAIN_H
#define _IWGTK_MAIN_H

typedef struct {
    GtkApplication *application;
    GDBusObjectManager *manager;
    GQuark iwd_error_domain;
    const gchar *session_bus_address;

    Window *window;
    Indicator *indicators;

    // Command line flags
    gboolean indicators_enable;
    gboolean notifications_disable;
    gboolean signal_icon_disable;
} GlobalData;

extern GlobalData global;
extern const ErrorMessage detailed_errors_standard[];

void object_manager_callback(GDBusObjectManagerClient *manager, GAsyncResult *res);
void iwd_up(GDBusConnection *connection);
void iwd_down(GDBusConnection *connection);
void startup(GtkApplication *app);
gint handle_local_options(GApplication *application, GVariantDict *options);
gint command_line(GApplication *application, GApplicationCommandLine *command_line);
void iwgtk_quit();

#endif
