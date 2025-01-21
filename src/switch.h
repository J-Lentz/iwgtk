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

#ifndef _IWGTK_SWITCH_H
#define _IWGTK_SWITCH_H

typedef struct SwitchData_s SwitchData;

struct SwitchData_s {
    GDBusProxy *proxy;
    GtkWidget *widget;
    const gchar *property;
    gulong handler;
};

gboolean switch_handler(GtkSwitch *widget, gboolean state, SwitchData *switch_data);
void switch_set(SwitchData *switch_data);
void switch_rm(GtkWidget *widget, SwitchData *switch_data);
GtkWidget* switch_new(GDBusProxy *proxy, const gchar *property);

#endif
