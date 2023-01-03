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

#ifndef _IWGTK_DPP_H
#define _IWGTK_DPP_H

typedef struct DPP_s DPP;

struct DPP_s {
    GDBusProxy *proxy;
    Station *station;
    const gchar *button_method;
    gulong handler_update;

    GtkWidget *label;
    GtkWidget *button;
    GtkWidget *qrcode;
};

void qrcode_draw(GtkDrawingArea *area, cairo_t *cr, int width, int height, cairo_surface_t *qr_surface);
GtkWidget* qrcode_widget_new(const gchar *uri);
void dpp_qrcode_add(DPP *dpp);

void dpp_button_press(DPP *dpp);

void dpp_set(DPP *dpp);
DPP* dpp_add(Window *window, GDBusObject *object, GDBusProxy *proxy);
void dpp_remove(Window *window, DPP *dpp);
void bind_station_dpp(Station *station, DPP *dpp);
void unbind_station_dpp(Station *station, DPP *dpp);

#endif
