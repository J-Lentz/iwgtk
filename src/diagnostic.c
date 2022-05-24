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

void diagnostic_callback(GDBusProxy *proxy, GAsyncResult *res, GtkWidget *table) {
    GVariant *data;
    GError *err;

    err = NULL;
    data = g_dbus_proxy_call_finish(proxy, res, &err);

    if (data) {
	GVariantIter *iter;
	gchar *property;
	GVariant *value;

	g_variant_get(data, "(a{sv})", &iter);
	int i = 0;
	while (g_variant_iter_next(iter, "{sv}", &property, &value)) {
	    GtkWidget *property_label;
	    GtkWidget *value_label;

	    property_label = gtk_label_new(property);

	    if (g_variant_is_of_type(value, G_VARIANT_TYPE_STRING)) {
		value_label = gtk_label_new(g_variant_get_string(value, NULL));
	    }
	    else {
		gchar *value_str;
		value_str = g_variant_print(value, FALSE);
		value_label = gtk_label_new(value_str);
		g_free(value_str);
	    }

	    i ++;
	    gtk_grid_attach(GTK_GRID(table), property_label, 0, i, 1, 1);
	    gtk_grid_attach(GTK_GRID(table), value_label,    1, i, 1, 1);

	    gtk_widget_set_halign(property_label, GTK_ALIGN_END);
	    gtk_widget_set_halign(value_label, GTK_ALIGN_START);

	    g_free(property);
	    g_variant_unref(value);
	}

	g_variant_iter_free(iter);
	g_variant_unref(data);

	gtk_widget_show_all(table);
    }
    else {
	fprintf(stderr, "Error retrieving station diagnostics: %s\n", err->message);
	g_error_free(err);
    }
}

gboolean diagnostic_key_press_callback(GtkWidget *window, GdkEventKey *event) {
    if (event->keyval == GDK_KEY_Escape) {
	gtk_widget_destroy(window);
	return TRUE;
    }
    return FALSE;
}

void diagnostic_show(StationDiagnostic *diagnostic) {
    GtkWidget *window;
    GtkWidget *table;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    {
	GVariant *device_name_var;
	const gchar *device_name;
	gchar *window_title;

	device_name_var = g_dbus_proxy_get_cached_property(diagnostic->device_proxy, "Name");
	device_name = g_variant_get_string(device_name_var, NULL);

	window_title = g_strconcat(device_name, ": Station diagnostics", NULL);
	g_variant_unref(device_name_var);

	gtk_window_set_title(GTK_WINDOW(window), window_title);
	g_free(window_title);
    }

    table = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), table);
    gtk_grid_set_column_spacing(GTK_GRID(table), 10);
    gtk_container_set_border_width(GTK_CONTAINER(table), 10);

    g_dbus_proxy_call(
	diagnostic->proxy,
	"GetDiagnostics",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) diagnostic_callback,
	(gpointer) table);

    {
	GtkWidget *property;
	GtkWidget *value;

	property = new_label_bold("Property");
	value = new_label_bold("Value");

	gtk_widget_set_halign(property, GTK_ALIGN_END);
	gtk_widget_set_halign(value, GTK_ALIGN_START);

	gtk_grid_attach(GTK_GRID(table), property, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(table), value,    1, 0, 1, 1);
    }

    g_signal_connect(window, "key-press-event", G_CALLBACK(diagnostic_key_press_callback), NULL);
    gtk_widget_show_all(window);
}

StationDiagnostic* diagnostic_add(Window *window, GDBusObject *object, GDBusProxy *proxy) {
    StationDiagnostic *diagnostic;

    diagnostic = malloc(sizeof(StationDiagnostic));
    diagnostic->proxy = proxy;

    diagnostic->button = gtk_button_new_with_label("Diagnostics");
    g_object_ref_sink(diagnostic->button);

    g_signal_connect_swapped(diagnostic->button, "clicked", G_CALLBACK(diagnostic_show), (gpointer) diagnostic);
    couple_register(window, DEVICE_DIAGNOSTIC, 1, diagnostic, object);

    return diagnostic;
}

void diagnostic_remove(Window *window, StationDiagnostic *diagnostic) {
    couple_unregister(window, DEVICE_DIAGNOSTIC, 1, diagnostic);
    g_object_unref(diagnostic->button);
    free(diagnostic);
}

void bind_device_diagnostic(Device *device, StationDiagnostic *diagnostic) {
    diagnostic->device_proxy = device->proxy;

    gtk_grid_attach(GTK_GRID(device->table), diagnostic->button, 3, 1, 1, 1);
    gtk_widget_set_halign(diagnostic->button, GTK_ALIGN_START);
    gtk_widget_show_all(diagnostic->button);
}

void unbind_device_diagnostic(Device *device, StationDiagnostic *diagnostic) {
    gtk_container_remove(GTK_CONTAINER(device->table), diagnostic->button);
}
