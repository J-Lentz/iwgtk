/*
 *  Copyright 2020 Jesse Lentz
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

static const CallbackMessages scan_messages = {
    NULL,
    "Error scanning",
    detailed_errors_standard
};

void scan_button_clicked(GtkButton *button, Station *station) {
    g_dbus_proxy_call(
	station->proxy,
	"Scan",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) validation_callback,
	(gpointer) &scan_messages);
}

void network_remove_callback(GtkWidget *network, GtkWidget *network_table) {
    gtk_container_remove(GTK_CONTAINER(network_table), network);
}

void scan_button_update(GDBusProxy *proxy, GVariant *properties, gchar **invalidated_properties, Station *station) {
    GVariant *scanning_var;

    scanning_var = lookup_property(properties, "Scanning");
    if (scanning_var) {
	gboolean scanning;
	scanning = g_variant_get_boolean(scanning_var);

	gtk_container_remove(
		GTK_CONTAINER(station->scan_button),
		gtk_bin_get_child(GTK_BIN(station->scan_button)));

	scan_button_set_child(station, scanning);

	if (!scanning) {
	    /*
	     * The "Scanning" property has just changed from true to false.
	     * Therefore, a scan has been completed and the network list must be updated.
	     */

	    gtk_container_foreach(GTK_CONTAINER(station->networks), (GtkCallback) network_remove_callback, (gpointer) station->networks);
	    populate_network_list(station);
	}

	g_variant_unref(scanning_var);
    }
}

void scan_button_set_child(Station *station, gboolean scanning) {
    gtk_widget_set_sensitive(station->scan_button, !scanning);

    if (scanning) {
	gtk_container_add(GTK_CONTAINER(station->scan_button), station->scan_widget_scanning);
    }
    else {
	gtk_container_add(GTK_CONTAINER(station->scan_button), station->scan_widget_idle);
    }
}

GtkWidget* scan_button_new(Station *station) {
    GtkWidget *spinner;

    station->scan_button = gtk_button_new();
    station->scan_widget_idle = gtk_label_new("Scan");
    gtk_widget_show(station->scan_widget_idle);
    g_object_ref(station->scan_widget_idle);

    station->scan_widget_scanning = label_with_spinner("Scanning");
    g_object_ref(station->scan_widget_scanning);

    {
	GVariant *scanning_var;
	gboolean scanning;

	scanning_var = g_dbus_proxy_get_cached_property(station->proxy, "Scanning");
	scanning = g_variant_get_boolean(scanning_var);
	scan_button_set_child(station, scanning);
	g_variant_unref(scanning_var);
    }

    g_signal_connect(station->proxy, "g-properties-changed", G_CALLBACK(scan_button_update), (gpointer) station);
    g_signal_connect(station->scan_button, "clicked", G_CALLBACK(scan_button_clicked), (gpointer) station);

    return station->scan_button;
}

void station_set(Station *station) {
    GVariant *state_var;
    const gchar *state;

    state_var = g_dbus_proxy_get_cached_property(station->proxy, "State");
    state = g_variant_get_string(state_var, NULL);
    gtk_label_set_text(GTK_LABEL(station->device->status), state);
    g_variant_unref(state_var);
}

Station* station_add(GDBusObject *object, GDBusProxy *proxy) {
    Station *station;

    station = malloc(sizeof(Station));
    station->proxy = proxy;

    station->scan_button = scan_button_new(station);
    gtk_widget_show_all(station->scan_button);

    station->networks = gtk_grid_new();
    g_object_ref_sink(station->networks);
    gtk_grid_set_column_spacing(GTK_GRID(station->networks), 10);
    gtk_grid_set_row_spacing(GTK_GRID(station->networks), 10);

    gtk_widget_set_margin_start(station->networks, 5);
    gtk_widget_set_margin_end(station->networks, 5);
    gtk_widget_set_margin_bottom(station->networks, 5);

    populate_network_list(station);
    gtk_widget_show_all(station->networks);

    g_signal_connect_swapped(proxy, "g-properties-changed", G_CALLBACK(station_set), (gpointer) station);

    couple_register(DEVICE_STATION, 1, station, object);
    return station;
}

void station_remove(Station *station) {
    g_object_unref(station->scan_widget_idle);
    g_object_unref(station->scan_widget_scanning);
    g_object_unref(station->networks);
    couple_unregister(DEVICE_STATION, 1, station);
    free(station);
}

void bind_device_station(Device *device, Station *station) {
    station->device = device;

    gtk_grid_attach(GTK_GRID(device->table), station->scan_button, 4, 0, 1, 1);
    gtk_widget_set_halign(station->scan_button, GTK_ALIGN_FILL);

    gtk_box_pack_start(GTK_BOX(device->master), station->networks, TRUE, TRUE, 0);

    station_set(station);
}

void unbind_device_station(Device *device, Station *station) {
    station->device = NULL;
    gtk_container_remove(GTK_CONTAINER(device->table), station->scan_button);
    gtk_container_remove(GTK_CONTAINER(device->master), station->networks);
}

void get_ordered_networks_callback(GDBusProxy *proxy, GAsyncResult *res, Station *station) {
    GVariant *ordered_networks;
    GError *err;

    err = NULL;
    ordered_networks = g_dbus_proxy_call_finish(proxy, res, &err);

    if (ordered_networks) {
	GVariantIter *iter;
	gchar *network_path;
	gint16 signal_strength;

	g_variant_get(ordered_networks, "(a(on))", &iter);
	int i = 0;
	while (g_variant_iter_next(iter, "(on)", &network_path, &signal_strength)) {
	    Network *network;
	    network = network_lookup(network_path);
	    bind_station_network(station, network, signal_strength, i ++);
	    g_free(network_path);
	}

	g_variant_iter_free(iter);
	g_variant_unref(ordered_networks);

	gtk_widget_show_all(station->networks);
    }
    else {
	fprintf(stderr, "Error retrieving scan results: %s\n", err->message);
	g_error_free(err);
    }
}

void populate_network_list(Station *station) {
    g_dbus_proxy_call(
	station->proxy,
	"GetOrderedNetworks",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) get_ordered_networks_callback,
	(gpointer) station);
}
