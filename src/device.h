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

#ifndef _IWGTK_DEVICE_H
#define _IWGTK_DEVICE_H

typedef struct Device_s Device;
typedef struct Window_s Window;

struct Device_s {
    GDBusProxy *proxy;

    // Button for the tab switcher
    GtkWidget *button;

    // Widgets
    GtkWidget *master;
    GtkWidget *table;
    GtkWidget *power_switch;
    GtkWidget *mac_label;
    GtkWidget *mode_box;

    // Handlers
    gulong handler_update;
};

void device_show(GtkToggleButton *button, Device *device);
void device_set(Device *device);
void mode_box_changed(GtkComboBox *box, Device *device);
GtkWidget* mode_box_new(GDBusProxy *adapter_proxy);
Device* device_add(Window *window, GDBusObject *object, GDBusProxy *proxy);
void device_remove(Window *window, Device *device);

#endif
