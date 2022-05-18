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

#ifndef _IWGTK_ADAPTER_H
#define _IWGTK_ADAPTER_H

typedef struct Adapter {
    GDBusProxy *proxy;

    // Widgets
    GtkWidget *frame;
    GtkWidget *device_buttons;
    GtkWidget *name_label;
    GtkWidget *power_switch;

    // Handlers
    gulong handler_update;
} Adapter;

void adapter_set(Adapter *adapter);
Adapter* adapter_add(Window *window, GDBusObject *object, GDBusProxy *proxy);
void adapter_remove(Window *window, Adapter *adapter);
void bind_adapter_device(Adapter *adapter, Device *device);
void unbind_adapter_device(Adapter *adapter, Device *device);
guint adapter_list_position(GDBusProxy *proxy1, ObjectList *list);

#endif
