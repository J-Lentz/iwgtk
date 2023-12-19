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

#ifndef _IWGTK_DIAGNOSTIC_H
#define _IWGTK_DIAGNOSTIC_H

typedef struct Diagnostic_s Diagnostic;

struct Diagnostic_s {
    GDBusProxy *proxy;
    GDBusProxy *device_proxy;
    GtkWidget *button;
};

void diagnostic_launch(Diagnostic *diagnostic);
void diagnostic_results_cb(GDBusProxy *proxy, GAsyncResult *res, Diagnostic *diagnostic);
void diagnostic_table_insert(GtkWidget *table, GtkWidget *property, GtkWidget *value, int row);
void diagnostic_window_show(Diagnostic *diagnostic, GtkWidget *table);
gboolean diagnostic_key_press(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, GtkWindow *window);

Diagnostic* diagnostic_add(Window *window, GDBusObject *object, GDBusProxy *proxy);
void diagnostic_remove(Window *window, Diagnostic *diagnostic);
void bind_device_diagnostic(Device *device, Diagnostic *diagnostic);
void unbind_device_diagnostic(Device *device, Diagnostic *diagnostic);

#endif
