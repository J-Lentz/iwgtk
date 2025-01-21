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

#ifndef _IWGTK_HIDDEN_H
#define _IWGTK_HIDDEN_H

typedef struct HiddenNetworkDialog_s HiddenNetworkDialog;
typedef struct Station_s Station;

struct HiddenNetworkDialog_s {
    Station *station;

    GtkWidget *window;
    GtkWidget *ssid;
};

void hidden_ssid_dialog(Station *station);
void hidden_ssid_submit(HiddenNetworkDialog *dialog);
void station_add_hidden_network(Station *station, const gchar *address, const gchar *type, gint16 signal_strength, int index);

#endif
