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

void station_set(Station *station) {
    gboolean scanning;

    {
	GVariant *scanning_var;

	scanning_var = g_dbus_proxy_get_cached_property(station->proxy, "Scanning");
	scanning = g_variant_get_boolean(scanning_var);
	g_variant_unref(scanning_var);
    }

    gtk_widget_set_sensitive(station->scan_button, !scanning);
    gtk_button_set_child(GTK_BUTTON(station->scan_button),
	    scanning ? station->scan_widget_scanning : station->scan_widget_idle);

    if (scanning) {
	station->state = STATION_SCANNING;
	station->network_connected = NULL;
	station_network_table_clear(station);
    }
    else {
	if (station->state == STATION_SCANNING) {
	    // A scan has just completed
	    station_network_table_build(station);
	}

	{
	    GVariant *state_var;
	    const gchar *state;

	    state_var = g_dbus_proxy_get_cached_property(station->proxy, "State");
	    state = g_variant_get_string(state_var, NULL);

	    if (strcmp(state, "connected") == 0) {
		station->state = STATION_CONNECTED;
	    }
	    else if (strcmp(state, "connecting") == 0) {
		station->state = STATION_CONNECTING;
	    }
	    else {
		station->state = STATION_DISCONNECTED;
		station->network_connected = NULL;
	    }

	    g_variant_unref(state_var);
	}

	if (station->network_connected) {
	    network_set(station->network_connected);
	}
    }

    if (station->dpp) {
	dpp_set(station->dpp);
    }
}

Station* station_add(Window *window, GDBusObject *object, GDBusProxy *proxy) {
    Station *station;

    station = g_malloc(sizeof(Station));
    station->proxy = proxy;
    station->dpp = NULL;
    station->state = STATION_SCANNING;
    station->n_networks = 0;
    station->network_connected = NULL;

    station->scan_button = gtk_button_new();
    g_object_ref_sink(station->scan_button);
    g_signal_connect_swapped(station->scan_button, "clicked", G_CALLBACK(send_scan_request), station);

    station->scan_widget_idle = gtk_label_new(_("Scan"));
    g_object_ref_sink(station->scan_widget_idle);

    station->scan_widget_scanning = label_with_spinner(_("Scanning"));
    g_object_ref_sink(station->scan_widget_scanning);

    station->provision_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    g_object_ref_sink(station->provision_vbox);

    {
	GtkWidget *hidden_connect;

	hidden_connect = gtk_button_new_with_label(_("Hidden network"));
	gtk_widget_set_tooltip_text(hidden_connect, _("Connect to a hidden network"));
	gtk_widget_set_size_request(hidden_connect, PROVISION_MENU_WIDTH, -1);

	g_signal_connect_swapped(hidden_connect, "clicked", G_CALLBACK(hidden_ssid_dialog), station);
	gtk_box_append(GTK_BOX(station->provision_vbox), hidden_connect);
    }

    station->provision_menu = gtk_popover_new();
    g_object_ref_sink(station->provision_menu);

    gtk_popover_set_has_arrow(GTK_POPOVER(station->provision_menu), FALSE);
    gtk_popover_set_child(GTK_POPOVER(station->provision_menu), station->provision_vbox);

    station->provision_button = gtk_button_new_with_label(_("Provision"));
    g_object_ref_sink(station->provision_button);

    g_signal_connect_swapped(station->provision_button, "clicked", G_CALLBACK(gtk_widget_set_visible), station->provision_menu);
    gtk_widget_set_parent(station->provision_menu, station->provision_button);

    station->network_table = gtk_grid_new();
    g_object_ref_sink(station->network_table);

    gtk_widget_set_size_request(station->scan_button, 110, -1);
    gtk_widget_set_halign(station->scan_button, GTK_ALIGN_FILL);

    gtk_grid_set_column_spacing(GTK_GRID(station->network_table), 10);
    gtk_grid_set_row_spacing(GTK_GRID(station->network_table), 10);

    gtk_widget_set_margin_start(station->network_table, 5);
    gtk_widget_set_margin_end(station->network_table, 5);
    gtk_widget_set_margin_bottom(station->network_table, 5);

    couple_register(window, DEVICE_STATION, 1, station, object);
    couple_register(window, STATION_DPP,    0, station, object);
    couple_register(window, STATION_WPS,    0, station, object);

    station->handler_update = g_signal_connect_swapped(proxy, "g-properties-changed", G_CALLBACK(station_set), station);
    station_set(station);

    return station;
}

void station_remove(Window *window, Station *station) {
    g_signal_handler_disconnect(station->proxy, station->handler_update);

    couple_unregister(window, DEVICE_STATION, 1, station);
    couple_unregister(window, STATION_DPP,    0, station);
    couple_unregister(window, STATION_WPS,    0, station);

    gtk_widget_unparent(station->provision_menu);

    g_object_unref(station->scan_button);
    g_object_unref(station->scan_widget_idle);
    g_object_unref(station->scan_widget_scanning);

    g_object_unref(station->provision_button);
    g_object_unref(station->provision_menu);
    g_object_unref(station->provision_vbox);

    station_network_table_clear(station);
    g_object_unref(station->network_table);

    g_free(station);
}

void bind_device_station(Device *device, Station *station) {
    gtk_grid_attach(GTK_GRID(device->table), station->scan_button,      3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(device->table), station->provision_button, 3, 1, 1, 1);
    gtk_box_append(GTK_BOX(device->master), station->network_table);
}

void unbind_device_station(Device *device, Station *station) {
    gtk_grid_remove(GTK_GRID(device->table), station->scan_button);
    gtk_grid_remove(GTK_GRID(device->table), station->provision_button);
    gtk_box_remove(GTK_BOX(device->master), station->network_table);
}

void send_scan_request(Station *station) {
    g_dbus_proxy_call(
	station->proxy,
	"Scan",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) method_call_log,
	"Scan request failed: %s\n");
}

void insert_separator(Station *station, gint position) {
    GtkWidget *separator;

    separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(station->network_table), separator, 0, position, 5, 1);
}

void station_network_table_build(Station *station) {
    g_dbus_proxy_call(
	station->proxy,
	"GetOrderedNetworks",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) get_networks_callback,
	station);
}

void station_network_table_clear(Station *station) {
    {
	GtkWidget *child;

	while ((child = gtk_widget_get_first_child(station->network_table))) {
	    gtk_grid_remove(GTK_GRID(station->network_table), child);
	}
    }

    if (station->n_networks != 0) {
	for (int i = 0; i < station->n_networks; i ++) {
	    network_remove(station->networks + i);
	}

	station->n_networks = 0;
	g_free(station->networks);
    }
}

void get_networks_callback(GDBusProxy *proxy, GAsyncResult *res, Station *station) {
    GVariant *ordered_networks;
    GError *err;

    err = NULL;
    ordered_networks = g_dbus_proxy_call_finish(proxy, res, &err);

    if (ordered_networks) {
	GVariantIter *iter;
	const gchar *network_path;
	gint16 signal_strength;
	gint i;

	g_variant_get(ordered_networks, "(a(on))", &iter);

	station->n_networks = g_variant_iter_n_children(iter);
	station->networks = g_malloc(station->n_networks * sizeof(Network));
	i = 0;

	while (g_variant_iter_next(iter, "(&on)", &network_path, &signal_strength)) {
	    GDBusProxy *network_proxy;

	    network_proxy = G_DBUS_PROXY(g_dbus_object_manager_get_interface(global.manager, network_path, IWD_IFACE_NETWORK));

	    if (network_proxy) {
		station_add_network(station, network_proxy, signal_strength, i ++);
	    }
	    else {
		g_printerr("Failed to find network object '%s'\n", network_path);
	    }
	}

	if (station->n_networks > 0) {
	    insert_separator(station, station->n_networks);
	}

	g_variant_iter_free(iter);
	g_variant_unref(ordered_networks);
    }
    else {
	g_printerr("Failed to retrieve available network list: %s\n", err->message);
	g_error_free(err);
    }

    if (global.state & SHOW_HIDDEN_NETWORKS) {
	g_dbus_proxy_call(
	    proxy,
	    "GetHiddenAccessPoints",
	    NULL,
	    G_DBUS_CALL_FLAGS_NONE,
	    -1,
	    NULL,
	    (GAsyncReadyCallback) get_hidden_networks_callback,
	    station);
    }
}

void get_hidden_networks_callback(GDBusProxy *proxy, GAsyncResult *res, Station *station) {
    GVariant *ordered_networks;
    GError *err;

    err = NULL;
    ordered_networks = g_dbus_proxy_call_finish(proxy, res, &err);

    if (ordered_networks) {
	gint i;
	GVariantIter *iter;
	const gchar *address;
	gint16 signal_strength;
	const gchar *type;

	i = 0;
	g_variant_get(ordered_networks, "(a(sns))", &iter);

	while (g_variant_iter_next(iter, "(&sn&s)", &address, &signal_strength, &type)) {
	    i ++;
	    station_add_hidden_network(station, address, type, signal_strength, station->n_networks + i);
	}

	if (i > 0) {
	    insert_separator(station, station->n_networks + i + 1);
	}

	g_variant_iter_free(iter);
	g_variant_unref(ordered_networks);
    }
    else {
	g_printerr("Failed to retrieve hidden network list: %s\n", err->message);
	g_error_free(err);
    }
}
