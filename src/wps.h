/*
 *  Copyright 2020-2025 Jesse Lentz and contributors
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

#ifndef _IWGTK_WPS_H
#define _IWGTK_WPS_H

typedef struct WPS_s WPS;
typedef struct WPSDialog_s WPSDialog;

struct WPS_s {
    GDBusProxy *proxy;
    Station *station;
    gulong handler_pushbutton;
    gulong handler_pin;

    GtkWidget *label;
    GtkWidget *pushbutton;
    GtkWidget *pin;
};

struct WPSDialog_s {
    WPS *wps;
    GtkWidget *window;
    GtkWidget *pin;
};

void wps_set_pushbutton(WPS *wps);
void wps_set_pin(WPS *wps);
void wps_set_pushbutton_cancel(WPS *wps);
void wps_set_pin_cancel(WPS *wps);
void wps_pushbutton_callback(GDBusProxy *proxy, GAsyncResult *res, WPS *wps);
void wps_pin_callback(GDBusProxy *proxy, GAsyncResult *res, WPS *wps);

void wps_connect_pin_dialog(WPS *wps);
void wps_pin_dialog_submit(WPSDialog *wps_dialog);
void wps_connect_pushbutton(WPS *wps);
void wps_cancel(WPS *wps, GtkWidget *button);

WPS* wps_add(Window *window, GDBusObject *object, GDBusProxy *proxy);
void wps_remove(Window *window, WPS *wps);
void bind_station_wps(Station *station, WPS *wps);
void unbind_station_wps(Station *station, WPS *wps);

#endif
