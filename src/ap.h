/*
 *  Copyright 2020-2022 Jesse Lentz and contributors
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

#ifndef _IWGTK_AP_H
#define _IWGTK_AP_H

typedef struct AP_s AP;
typedef struct APDialog_s APDialog;
typedef struct Device_s Device;

struct AP_s {
    GDBusProxy *proxy;
    Device *device;

    // Widgets
    GtkWidget *button;
    GtkWidget *ssid;

    // Handlers
    gulong handler_update;
};

struct APDialog_s {
    AP *ap;

    GtkWidget *window;
    GtkWidget *ssid;
    GtkWidget *psk;
};

void ap_dialog_launch(AP *ap);
void ap_dialog_submit(APDialog *data);
void ap_dialog_cancel(APDialog *data);
void ap_button_clicked(AP *data);
void ap_set(AP *data);

AP* ap_add(Window *window, GDBusObject *object, GDBusProxy *proxy);
void ap_remove(Window *window, AP *ap);
void bind_device_ap(Device *device, AP *ap);
void unbind_device_ap(Device *device, AP *ap);

#endif
