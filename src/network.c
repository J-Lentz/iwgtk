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

/*
 * TODO
 * Customize error messages, e.g., "Connected to network _SSID_"
 */

static const CallbackMessages connect_messages = {
    "Connected to network",
    "Connection attempt failed",
    detailed_errors_standard
};

static const CallbackMessages disconnect_messages = {
    "Disconnected from network",
    "Disconnection attempt failed",
    detailed_errors_standard
};

const gchar* get_security_type(const gchar *type_raw) {
    if (strcmp(type_raw, "open") == 0) {
	return "Open";
    }
    else if (strcmp(type_raw, "psk") == 0) {
	return "PSK";
    }
    else if (strcmp(type_raw, "8021x") == 0) {
	return "EAP";
    }
    else if (strcmp(type_raw, "wep") == 0) {
	// The "wep" type value is undocumented
	return "WEP";
    }
    else {
	return type_raw;
    }
}

void connect_button_clicked(GtkButton *button, GDBusProxy *network_proxy) {
    g_dbus_proxy_call(
	network_proxy,
	"Connect",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) validation_callback,
	(gpointer) &connect_messages);
}

void disconnect_button_clicked(GtkButton *button, GDBusProxy *station_proxy) {
    g_dbus_proxy_call(
	station_proxy,
	"Disconnect",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) validation_callback,
	(gpointer) &disconnect_messages);
}

void network_set(Network *network) {
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
	GtkImage *status_icon;

	connected_var = g_dbus_proxy_get_cached_property(network->proxy, "Connected");
	connected = g_variant_get_boolean(connected_var);
	g_variant_unref(connected_var);

	button = GTK_BUTTON(network->connect_button);
	status_icon = GTK_IMAGE(network->status_icon);

	if (connected) {
	    GVariant *state_var;
	    const gchar *state;

	    state_var = g_dbus_proxy_get_cached_property(network->station_proxy, "State");
	    state = g_variant_get_string(state_var, NULL);

	    if (strcmp(state, "connected") == 0) {
		gtk_image_set_from_resource(status_icon, RESOURCE_CONNECTED);
	    }
	    else {
		gtk_image_set_from_resource(status_icon, RESOURCE_CONNECTING);
	    }

	    g_variant_unref(state_var);

	    gtk_button_set_label(button, "Disconnect");
	    network->button_handler_id = g_signal_connect(button, "clicked", G_CALLBACK(disconnect_button_clicked), (gpointer) network->station_proxy);
	}
	else {
	    GVariant *known_network_var;

	    known_network_var = g_dbus_proxy_get_cached_property(network->proxy, "KnownNetwork");
	    if (known_network_var) {
		g_variant_unref(known_network_var);
		gtk_image_set_from_resource(status_icon, RESOURCE_KNOWN);
	    }
	    else {
		gtk_image_set_from_resource(status_icon, RESOURCE_UNKNOWN);
	    }

	    gtk_button_set_label(button, "Connect");
	    network->button_handler_id = g_signal_connect(button, "clicked", G_CALLBACK(connect_button_clicked), (gpointer) network->proxy);
	}
    }
}

Network* network_add(GDBusObject *object, GDBusProxy *proxy) {
    GtkWidget *master;
    GtkWidget *ssid_label, *security_label, *connect_button;
    Network *network;

    network = malloc(sizeof(Network));
    network->proxy = proxy;
    network->button_handler_id = 0;

    {
	GVariant *device_path_var;
	const gchar *device_path;

	device_path_var = g_dbus_proxy_get_cached_property(proxy, "Device");
	device_path = g_variant_get_string(device_path_var, NULL);
	network->station_proxy = G_DBUS_PROXY(g_dbus_object_manager_get_interface(global.manager, device_path, IWD_IFACE_STATION));
	g_variant_unref(device_path_var);
    }

    network->status_icon = gtk_image_new();
    network->ssid_label = gtk_label_new(NULL);
    network->security_label = gtk_label_new(NULL);
    network->connect_button = gtk_button_new();

    g_object_ref_sink(network->status_icon);
    g_object_ref_sink(network->ssid_label);
    g_object_ref_sink(network->security_label);
    g_object_ref_sink(network->connect_button);

    g_signal_connect_swapped(proxy, "g-properties-changed", G_CALLBACK(network_set), (gpointer) network);
    network_set(network);

    return network;
}

void network_remove(Network *network) {
    g_object_unref(network->status_icon);
    g_object_unref(network->ssid_label);
    g_object_unref(network->security_label);
    g_object_unref(network->connect_button);

    free(network);
}

GtkWidget* signal_widget(gint16 signal_strength) {
    if (global.signal_icon_disable) {
	gchar signal_text[5];
	sprintf(signal_text, "%d", signal_strength / 100);
	return gtk_label_new(signal_text);
    }
    else {
	const gchar *signal_resource;

	if (signal_strength > -6000) {
	    signal_resource = RESOURCE_SIGNAL_4;
	}
	else if (signal_strength > -6700) {
	    signal_resource = RESOURCE_SIGNAL_3;
	}
	else if (signal_strength > -7400) {
	    signal_resource = RESOURCE_SIGNAL_2;
	}
	else if (signal_strength > -8100) {
	    signal_resource = RESOURCE_SIGNAL_1;
	}
	else {
	    signal_resource = RESOURCE_SIGNAL_0;
	}

	return gtk_image_new_from_resource(signal_resource);
    }
}

void bind_station_network(Station *station, Network *network, gint16 signal_strength, int index) {
    GtkWidget *signal;

    signal = signal_widget(signal_strength);

    gtk_grid_attach(GTK_GRID(station->networks), network->status_icon,    0, index, 1, 1);
    gtk_grid_attach(GTK_GRID(station->networks), network->ssid_label,     1, index, 1, 1);
    gtk_grid_attach(GTK_GRID(station->networks), network->security_label, 2, index, 1, 1);
    gtk_grid_attach(GTK_GRID(station->networks), signal,                  3, index, 1, 1);
    gtk_grid_attach(GTK_GRID(station->networks), network->connect_button, 4, index, 1, 1);

    gtk_widget_set_halign(network->status_icon,    GTK_ALIGN_START);
    gtk_widget_set_halign(network->ssid_label,     GTK_ALIGN_START);
    gtk_widget_set_halign(signal,                  GTK_ALIGN_START);
    gtk_widget_set_halign(network->security_label, GTK_ALIGN_START);
    gtk_widget_set_halign(network->connect_button, GTK_ALIGN_FILL);

    gtk_widget_set_hexpand(network->ssid_label, TRUE);
}

Network* network_lookup(const char *path) {
    GDBusObject *object;
    ObjectList *list;
    object = g_dbus_object_manager_get_object(global.manager, path);
    list = object_table[OBJECT_NETWORK].objects;

    while (list->object != object) {
	list = list->next;
    }

    g_object_unref(object);
    return (Network *) list->data;
}
