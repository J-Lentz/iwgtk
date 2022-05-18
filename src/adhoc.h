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

#ifndef _IWGTK_ADHOC_H
#define _IWGTK_ADHOC_H

typedef struct {
    GDBusProxy *proxy;
    Device *device;

    // Widgets
    GtkWidget *button;
    GtkWidget *n_peers;
    GtkWidget *peer_list;

    // Handlers
    gulong handler_update;
} AdHoc;

typedef struct {
    AdHoc *adhoc;

    GtkWidget *window;
    GtkWidget *ssid;
    GtkWidget *psk;
    GtkWidget *psk_toggle;
} AdHocDialog;

void psk_toggle_changed(GtkToggleButton *psk_toggle, AdHocDialog *adhoc_dialog);
void adhoc_dialog_launch(AdHoc *adhoc);
void adhoc_dialog_submit(AdHocDialog *adhoc_dialog);
void adhoc_dialog_cancel(AdHocDialog *adhoc_dialog);
void adhoc_button_clicked(AdHoc *adhoc);
void adhoc_set(AdHoc *adhoc);
AdHoc* adhoc_add(Window *window, GDBusObject *object, GDBusProxy *proxy);
void adhoc_remove(Window *window, AdHoc *adhoc);
void bind_device_adhoc(Device *device, AdHoc *adhoc);
void unbind_device_adhoc(Device *device, AdHoc *adhoc);

#endif
