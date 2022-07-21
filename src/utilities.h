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

#ifndef _IWGTK_UTILITIES_H
#define _IWGTK_UTILITIES_H

#define RGB_MAX 65535

typedef void (*SetFunction) (gpointer data);

typedef struct FailureClosure_s FailureClosure;

struct FailureClosure_s {
    SetFunction callback;
    gpointer data;
    const gchar *property;
};

const gchar* get_error_detail(GError *err, const ErrorMessage *error_table);
void method_call_notify(GDBusProxy *proxy, GAsyncResult *res, CallbackMessages *data);
void method_call_log(GDBusProxy *proxy, GAsyncResult *res, const gchar *message);

gboolean adapter_sort(GDBusProxy *proxy0, GDBusProxy *proxy1);
gboolean device_sort(GDBusProxy *proxy0, GDBusProxy *proxy1);
void set_remote_property_callback(GDBusProxy *proxy, GAsyncResult *res, FailureClosure *failure);
void set_remote_property(GDBusProxy *proxy, const gchar *property, GVariant *value, SetFunction failure_callback, gpointer failure_data);
GVariant* lookup_property(GVariant *dictionary, const gchar *property);
void send_notification(const gchar *text);
void grid_column_set_alignment(GtkWidget *grid, int col, GtkAlign align);
GtkWidget* label_with_spinner(const gchar *text);
GtkWidget* new_label_bold(const gchar *text);
GtkWidget* new_label_gray(const gchar *text);

#endif
