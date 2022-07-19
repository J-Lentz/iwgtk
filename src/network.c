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

const ErrorMessage detailed_errors_network[] = {
    {IWD_ERROR_FAILED,         N_("Incorrect passphrase")},
    {IWD_ERROR_INVALID_FORMAT, N_("Invalid passphrase")},
    {0, NULL}
};

const gchar* get_security_type(const gchar *type_raw) {
    if (strcmp(type_raw, "open") == 0) {
	return _("Open");
    }
    else if (strcmp(type_raw, "psk") == 0) {
	return _("PSK");
    }
    else if (strcmp(type_raw, "8021x") == 0) {
	return _("EAP");
    }
    else if (strcmp(type_raw, "wep") == 0) {
	// The "wep" type value is undocumented
	return _("WEP");
    }
    else {
	return type_raw;
    }
}

void connect_button_clicked(GtkButton *button, GDBusProxy *network_proxy) {
    CallbackMessages *messages;

    messages = g_malloc(sizeof(CallbackMessages));
    messages->error_table = detailed_errors_network;
    messages->free = TRUE;

    {
	GVariant *ssid_var;
	const gchar *ssid;

	ssid_var = g_dbus_proxy_get_cached_property(network_proxy, "Name");
	ssid = g_variant_get_string(ssid_var, NULL);

	messages->success = g_strdup_printf(_("Connected to %s"), ssid);
	messages->failure = g_strdup_printf(_("Failed to connect to %s"), ssid);

	g_variant_unref(ssid_var);
    }

    g_dbus_proxy_call(
	network_proxy,
	"Connect",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) method_call_notify,
	messages);
}

void disconnect_button_clicked(GtkButton *button, Network *network) {
    CallbackMessages *messages;

    messages = g_malloc(sizeof(CallbackMessages));
    messages->error_table = NULL;
    messages->free = TRUE;

    {
	GVariant *ssid_var;
	const gchar *ssid;

	ssid_var = g_dbus_proxy_get_cached_property(network->proxy, "Name");
	ssid = g_variant_get_string(ssid_var, NULL);

	messages->success = g_strdup_printf(_("Disconnected from %s"), ssid);
	messages->failure = g_strdup_printf(_("Failed to disconnect from %s"), ssid);

	g_variant_unref(ssid_var);
    }

    g_dbus_proxy_call(
	network->station->proxy,
	"Disconnect",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) method_call_notify,
	messages);
}

void network_set(Network *network) {
    NetworkStatus network_status;

    {
	GVariant *ssid_var;
	const gchar *ssid;

	ssid_var = g_dbus_proxy_get_cached_property(network->proxy, "Name");
	ssid = g_variant_get_string(ssid_var, NULL);
	gtk_label_set_text(GTK_LABEL(network->ssid_label), ssid);

	g_variant_unref(ssid_var);
    }

    {
	GVariant *type_var;
	const gchar *type_raw;

	type_var = g_dbus_proxy_get_cached_property(network->proxy, "Type");
	type_raw = g_variant_get_string(type_var, NULL);
	gtk_label_set_text(GTK_LABEL(network->security_label), get_security_type(type_raw));
	g_variant_unref(type_var);
    }

    if (network->button_handler_id != 0) {
	g_signal_handler_disconnect(network->connect_button, network->button_handler_id);
    }

    {
	GVariant *connected_var;
	gboolean connected;
	GtkButton *button;

	connected_var = g_dbus_proxy_get_cached_property(network->proxy, "Connected");
	connected = g_variant_get_boolean(connected_var);
	g_variant_unref(connected_var);

	button = GTK_BUTTON(network->connect_button);

	if (connected) {
	    if (network->station->network_connected != network) {
		network->station->network_connected = network;
	    }

	    if (network->station->state == STATION_CONNECTED) {
		network_status = NETWORK_CONNECTED;
		gtk_widget_set_tooltip_text(network->status_icon, _("Connected"));
	    }
	    else {
		network_status = NETWORK_CONNECTING;
		gtk_widget_set_tooltip_text(network->status_icon, _("Connecting"));
	    }

	    gtk_button_set_label(button, _("Disconnect"));
	    gtk_widget_set_tooltip_text(network->connect_button, _("Disconnect from network"));
	    network->button_handler_id = g_signal_connect(button, "clicked", G_CALLBACK(disconnect_button_clicked), network);
	}
	else {
	    GVariant *known_network_var;

	    if (network->station->network_connected == network) {
		network->station->network_connected = NULL;
	    }

	    known_network_var = g_dbus_proxy_get_cached_property(network->proxy, "KnownNetwork");
	    if (known_network_var) {
		g_variant_unref(known_network_var);
		network_status = NETWORK_KNOWN;
		gtk_widget_set_tooltip_text(network->status_icon, _("Known network"));
	    }
	    else {
		network_status = NETWORK_UNKNOWN;
		gtk_widget_set_tooltip_text(network->status_icon, _("Unknown network"));
	    }

	    gtk_button_set_label(button, _("Connect"));
	    gtk_widget_set_tooltip_text(network->connect_button, _("Connect to network"));
	    network->button_handler_id = g_signal_connect(button, "clicked", G_CALLBACK(connect_button_clicked), network->proxy);
	}
    }

    {
	const gchar *icon_name;

	icon_name = station_icons[network->level];
	symbolic_icon_set_image(icon_name, color_status[network_status], network->status_icon);
    }
}

void station_add_network(Station *station, GDBusProxy *network_proxy, gint16 signal_strength, int index) {
    Network *network;

    network = station->networks + index;

    network->proxy = network_proxy;
    network->station = station;
    network->level = get_signal_level(signal_strength);
    network->button_handler_id = 0;

    network->status_icon = gtk_picture_new();
    network->ssid_label = gtk_label_new(NULL);
    network->security_label = gtk_label_new(NULL);
    network->connect_button = gtk_button_new();

    gtk_widget_set_tooltip_text(network->ssid_label, _("SSID"));
    gtk_widget_set_tooltip_text(network->security_label, _("Network security type"));

    g_object_ref_sink(network->status_icon);
    g_object_ref_sink(network->ssid_label);
    g_object_ref_sink(network->security_label);
    g_object_ref_sink(network->connect_button);

    gtk_grid_attach(GTK_GRID(station->network_table), network->status_icon,    0, index, 1, 1);
    gtk_grid_attach(GTK_GRID(station->network_table), network->ssid_label,     1, index, 1, 1);
    gtk_grid_attach(GTK_GRID(station->network_table), network->security_label, 2, index, 1, 1);
    gtk_grid_attach(GTK_GRID(station->network_table), network->connect_button, 3, index, 1, 1);

    gtk_widget_set_halign(network->status_icon,    GTK_ALIGN_START);
    gtk_widget_set_halign(network->ssid_label,     GTK_ALIGN_START);
    gtk_widget_set_halign(network->security_label, GTK_ALIGN_START);
    gtk_widget_set_halign(network->connect_button, GTK_ALIGN_FILL);

    gtk_widget_set_hexpand(network->ssid_label, TRUE);
    gtk_widget_set_size_request(network->status_icon, 32, 32);
    gtk_widget_set_size_request(network->connect_button, 107, -1);

    network_set(network);
    network->handler_update = g_signal_connect_swapped(network_proxy, "g-properties-changed", G_CALLBACK(network_set), network);
}

void network_remove(Network *network) {
    g_object_unref(network->status_icon);
    g_object_unref(network->ssid_label);
    g_object_unref(network->security_label);
    g_object_unref(network->connect_button);

    g_signal_handler_disconnect(network->proxy, network->handler_update);
}
