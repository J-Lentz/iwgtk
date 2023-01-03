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

#include "iwgtk.h"

void diagnostic_launch(StationDiagnostic *diagnostic) {
    g_dbus_proxy_call(
	diagnostic->proxy,
	"GetDiagnostics",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) diagnostic_results_cb,
	diagnostic);
}

void diagnostic_results_cb(GDBusProxy *proxy, GAsyncResult *res, StationDiagnostic *diagnostic) {
    GVariant *diagnostics_data;
    GError *err;

    err = NULL;
    diagnostics_data = g_dbus_proxy_call_finish(proxy, res, &err);

    if (diagnostics_data) {
	GtkWidget *table;
	GtkWidget *property;
	GtkWidget *value;

	table = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(table), 10);

	gtk_widget_set_margin_start(table, 5);
	gtk_widget_set_margin_end(table, 5);
	gtk_widget_set_margin_top(table, 5);
	gtk_widget_set_margin_bottom(table, 5);

	property = new_label_bold(_("Property"));
	value = new_label_bold(_("Value"));
	diagnostic_table_insert(table, property, value, 0);

	{
	    GVariantIter *iter;
	    const gchar *property_str;
	    GVariant *value_var;
	    int i = 1;

	    g_variant_get(diagnostics_data, "(a{sv})", &iter);

	    while (g_variant_iter_next(iter, "{&sv}", &property_str, &value_var)) {
		property = gtk_label_new(property_str);

		if (g_variant_is_of_type(value_var, G_VARIANT_TYPE_STRING)) {
		    value = gtk_label_new(g_variant_get_string(value_var, NULL));
		}
		else {
		    gchar *value_str;

		    value_str = g_variant_print(value_var, FALSE);
		    value = gtk_label_new(value_str);
		    g_free(value_str);
		}

		diagnostic_table_insert(table, property, value, i ++);
		g_variant_unref(value_var);
	    }

	    g_variant_iter_free(iter);
	}

	g_variant_unref(diagnostics_data);
	diagnostic_window_show(diagnostic, table);
    }
    else {
	g_printerr("Failed to retrieve station diagnostics: %s\n", err->message);
	g_error_free(err);
    }
}

void diagnostic_table_insert(GtkWidget *table, GtkWidget *property, GtkWidget *value, int row) {
    gtk_widget_set_halign(property, GTK_ALIGN_END);
    gtk_widget_set_halign(value, GTK_ALIGN_START);

    gtk_grid_attach(GTK_GRID(table), property, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(table), value,    1, row, 1, 1);
}

void diagnostic_window_show(StationDiagnostic *diagnostic, GtkWidget *table) {
    GtkWidget *window;

    window = gtk_window_new();

    {
	GVariant *device_name_var;
	const gchar *device_name;
	gchar *window_title;

	device_name_var = g_dbus_proxy_get_cached_property(diagnostic->device_proxy, "Name");
	device_name = g_variant_get_string(device_name_var, NULL);

	window_title = g_strdup_printf(_("%s: Station diagnostics"), device_name);
	g_variant_unref(device_name_var);

	gtk_window_set_title(GTK_WINDOW(window), window_title);
	g_free(window_title);
    }

    {
	GtkEventController *controller;

	controller = gtk_event_controller_key_new();
	g_signal_connect(controller, "key-pressed", G_CALLBACK(diagnostic_key_press), window);
	gtk_widget_add_controller(window, controller);
    }

    gtk_window_set_child(GTK_WINDOW(window), table);
    gtk_widget_show(window);
}

gboolean diagnostic_key_press(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, GtkWindow *window) {
    if (keyval == GDK_KEY_Escape) {
	gtk_window_destroy(window);
	return TRUE;
    }
    return FALSE;
}

StationDiagnostic* diagnostic_add(Window *window, GDBusObject *object, GDBusProxy *proxy) {
    StationDiagnostic *diagnostic;

    diagnostic = g_malloc(sizeof(StationDiagnostic));
    diagnostic->proxy = proxy;

    diagnostic->button = gtk_button_new_with_label(_("Diagnostics"));
    g_object_ref_sink(diagnostic->button);

    g_signal_connect_swapped(diagnostic->button, "clicked", G_CALLBACK(diagnostic_launch), diagnostic);
    couple_register(window, DEVICE_DIAGNOSTIC, 1, diagnostic, object);

    return diagnostic;
}

void diagnostic_remove(Window *window, StationDiagnostic *diagnostic) {
    couple_unregister(window, DEVICE_DIAGNOSTIC, 1, diagnostic);
    g_object_unref(diagnostic->button);
    g_free(diagnostic);
}

void bind_device_diagnostic(Device *device, StationDiagnostic *diagnostic) {
    diagnostic->device_proxy = device->proxy;

    gtk_grid_attach(GTK_GRID(device->table), diagnostic->button, 0, 1, 1, 1);
}

void unbind_device_diagnostic(Device *device, StationDiagnostic *diagnostic) {
    gtk_grid_remove(GTK_GRID(device->table), diagnostic->button);
}
