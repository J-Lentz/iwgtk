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

#include "iwgtk.h"

const gchar* get_error_detail(GError *err, const ErrorMessage *error_table) {
    gint i;

    if (error_table && err->domain == global.iwd_error_domain) {
	for (i = 0; error_table[i].code != 0; i ++) {
	    if (err->code == error_table[i].code) {
		return error_table[i].message;
	    }
	}
    }

    /*
     * Return err->message with its prefix removed. If we reach the end of the string for
     * some reason, then just return err->message in its entirety.
     */

    for (i = 0; err->message[i] != ' '; i ++) {
	if (err->message[i] == '\0') {
	    return err->message;
	}
    }

    return err->message + i + 1;
}

void method_call_notify(GDBusProxy *proxy, GAsyncResult *res, CallbackMessages *messages) {
    GVariant *ret;
    GError *err;

    err = NULL;
    ret = g_dbus_proxy_call_finish(proxy, res, &err);

    if (ret) {
	g_variant_unref(ret);

	if (messages->success) {
	    send_notification(messages->success);
	}
    }
    else {
	if (messages->failure) {
	    const gchar *err_detail;
	    gchar *message_detailed;

	    err_detail = get_error_detail(err, messages->error_table);
	    message_detailed = g_strconcat(messages->failure, ": ", err_detail, NULL);
	    send_notification(message_detailed);
	    g_free(message_detailed);
	}

	g_printerr("%s\n", err->message);
	g_error_free(err);
    }

    if (messages->free) {
	g_free((void *) messages->success);
	g_free((void *) messages->failure);
	g_free((void *) messages);
    }
}

void method_call_log(GDBusProxy *proxy, GAsyncResult *res, const gchar *message) {
    GVariant *ret;
    GError *err;

    err = NULL;
    ret = g_dbus_proxy_call_finish(proxy, res, &err);

    if (ret) {
	g_variant_unref(ret);
    }
    else {
	g_printerr(message, err->message);
	g_error_free(err);
    }
}

/*
 * Return true if adapter 0 < adapter 1 and false if adapter 1 < adapter 0.
 */
gboolean adapter_sort(GDBusProxy *proxy0, GDBusProxy *proxy1) {
    const gchar *path0, *path1;
    guint adapter0, adapter1;

    static const gchar *adapter_path_fmt = "%u";

    if (proxy0 == NULL || proxy1 == NULL) {
	return FALSE;
    }

    path0 = g_dbus_proxy_get_object_path(proxy0) + IWD_PATH_PREFIX_LENGTH;
    path1 = g_dbus_proxy_get_object_path(proxy1) + IWD_PATH_PREFIX_LENGTH;

    sscanf(path0, adapter_path_fmt, &adapter0);
    sscanf(path1, adapter_path_fmt, &adapter1);

    if (adapter0 < adapter1) {
	return TRUE;
    }

    return FALSE;
}

/*
 * Return true if device 0 < device 1 and false if device 1 < device 0.
 */
/*
 * TODO:
 * Use this to sort wlan* buttons within a device.
 */
gboolean device_sort(GDBusProxy *proxy0, GDBusProxy *proxy1) {
    const gchar *path0, *path1;
    guint adapter0, adapter1;
    guint dev0, dev1;

    static const gchar *device_path_fmt = "%u/%u";

    if (proxy0 == NULL || proxy1 == NULL) {
	return FALSE;
    }

    path0 = g_dbus_proxy_get_object_path(proxy0) + IWD_PATH_PREFIX_LENGTH;
    path1 = g_dbus_proxy_get_object_path(proxy1) + IWD_PATH_PREFIX_LENGTH;

    sscanf(path0, device_path_fmt, &adapter0, &dev0);
    sscanf(path1, device_path_fmt, &adapter1, &dev1);

    if (adapter0 < adapter1) {
	return TRUE;
    }

    if (adapter1 < adapter0) {
	return FALSE;
    }

    /*
     * We know now that adapter0 == adapter1.
     */

    if (dev0 < dev1) {
	return TRUE;
    }

    return FALSE;
}

void set_remote_property_callback(GDBusProxy *proxy, GAsyncResult *res, FailureClosure *failure) {
    GVariant *ret;
    GError *err;

    err = NULL;
    ret = g_dbus_proxy_call_finish(proxy, res, &err);

    if (ret) {
	g_variant_unref(ret);
    }
    else {
	g_printerr("Error setting remote property '%s': %s\n", failure->property, err->message);
	g_error_free(err);

	failure->callback(failure->data);
    }

    g_free(failure);
}

/*
 * When a property is updated remotely, iwgtk responds by updating a widget. This widget
 * state change triggers a signal which causes set_remote_property() to be called. The
 * equality check in this function prevents the property change from being volleyed back
 * to iwd. This is kind of a hack; it would be more elegant if set_remote_property() were
 * only called for user-initiated state changes.
 */
void set_remote_property(GDBusProxy *proxy, const gchar *property, GVariant *value, SetFunction failure_callback, gpointer failure_data) {
    GVariant *value_cached;

    value_cached = g_dbus_proxy_get_cached_property(proxy, property);
    if (!g_variant_equal(value, value_cached)) {
	FailureClosure *failure_closure;

	failure_closure = g_malloc(sizeof(FailureClosure));
	failure_closure->callback = failure_callback;
	failure_closure->data = failure_data;
	failure_closure->property = property;

	g_dbus_proxy_call(
	    proxy,
	    "org.freedesktop.DBus.Properties.Set",
	    g_variant_new("(ssv)", g_dbus_proxy_get_interface_name(proxy), property, value),
	    G_DBUS_CALL_FLAGS_NONE,
	    -1,
	    NULL,
	    (GAsyncReadyCallback) set_remote_property_callback,
	    failure_closure);
    }
    else {
	g_variant_unref(value);
    }
    g_variant_unref(value_cached);
}

GVariant* lookup_property(GVariant *dictionary, const gchar *property) {
    GVariantIter iter;
    gchar *key;
    GVariant *value;

    g_variant_iter_init(&iter, dictionary);
    while (g_variant_iter_next(&iter, "{sv}", &key, &value)) {
	if (strcmp(property, key) == 0) {
	    g_free(key);
	    return value;
	}

	g_free(key);
	g_variant_unref(value);
    }

    return NULL;
}

void send_notification(const gchar *text) {
    if (~global.state & NOTIFICATIONS_DISABLE) {
	GNotification *notification;

	notification = g_notification_new("iwgtk");
	g_notification_set_body(notification, text);
	g_notification_set_priority(notification, G_NOTIFICATION_PRIORITY_NORMAL);
	g_application_send_notification(G_APPLICATION(global.application), NULL, notification);
	g_object_unref(notification);
    }
}

void grid_column_set_alignment(GtkWidget *grid, int col, GtkAlign align) {
    GtkWidget *cell;
    int i;

    i = 0;
    while (cell = gtk_grid_get_child_at(GTK_GRID(grid), col, i)) {
	gtk_widget_set_halign(cell, align);
	i ++;
    }
}

GtkWidget* label_with_spinner(const gchar *text) {
    GtkWidget *box, *spinner;

    spinner = gtk_spinner_new();
    gtk_spinner_start(GTK_SPINNER(spinner));

    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_append(GTK_BOX(box), spinner);
    gtk_box_append(GTK_BOX(box), gtk_label_new(text));

    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    return box;
}

GtkWidget* new_label_bold(const gchar *text) {
    PangoAttrList *attr_list;
    GtkWidget *label;

    attr_list = pango_attr_list_new();
    pango_attr_list_insert(attr_list, pango_attr_weight_new(PANGO_WEIGHT_BOLD));

    label = gtk_label_new(text);
    gtk_label_set_attributes(GTK_LABEL(label), attr_list);
    pango_attr_list_unref(attr_list);
    return label;
}

GtkWidget* new_label_gray(const gchar *text) {
    PangoAttrList *attr_list;
    GtkWidget *label;

    attr_list = pango_attr_list_new();
    pango_attr_list_insert(attr_list, pango_attr_weight_new(PANGO_WEIGHT_SEMILIGHT));
    pango_attr_list_insert(attr_list, pango_attr_foreground_new(RGB_MAX/2, RGB_MAX/2, RGB_MAX/2));

    label = gtk_label_new(text);
    gtk_label_set_attributes(GTK_LABEL(label), attr_list);
    pango_attr_list_unref(attr_list);
    return label;
}
