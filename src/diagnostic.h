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

#ifndef _IWGTK_DIAGNOSTIC_H
#define _IWGTK_DIAGNOSTIC_H

typedef struct StationDiagnostic_s StationDiagnostic;

struct StationDiagnostic_s {
    GDBusProxy *proxy;
    GDBusProxy *device_proxy;
    GtkWidget *button;
};

void diagnostic_callback(GDBusProxy *proxy, GAsyncResult *res, GtkWidget *table);
gboolean diagnostic_key_press_callback(GtkWidget *window, GdkEventKey *event);
void diagnostic_show(StationDiagnostic *diagnostic);
StationDiagnostic* diagnostic_add(Window *window, GDBusObject *object, GDBusProxy *proxy);
void diagnostic_remove(Window *window, StationDiagnostic *diagnostic);
void bind_device_diagnostic(Device *device, StationDiagnostic *diagnostic);
void unbind_device_diagnostic(Device *device, StationDiagnostic *diagnostic);

#endif
