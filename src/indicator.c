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

GDBusArgInfo arg_device = {-1, "device", "o", NULL};
GDBusArgInfo arg_level = {-1, "level", "y", NULL};

GDBusInterfaceInfo signal_agent_interface_info = {
    -1,
    IWD_IFACE_SIGNAL_LEVEL_AGENT,
    (GDBusMethodInfo *[]) {
	&(GDBusMethodInfo) {
	    -1,
	    "Release",
	    (GDBusArgInfo *[]) {&arg_device, NULL},
	    NULL,
	    NULL
	},
	&(GDBusMethodInfo) {
	    -1,
	    "Changed",
	    (GDBusArgInfo *[]) {&arg_device, &arg_level, NULL},
	    NULL,
	    NULL
	},
	NULL
    },
    NULL, // Signal info
    NULL, // Property info
    NULL  // Annotation info
};

GDBusInterfaceVTable signal_agent_interface_vtable = {
    (GDBusInterfaceMethodCallFunc) signal_agent_method_call_handler,
    NULL,
    NULL
};

Indicator* indicator_new(GDBusProxy *device_proxy) {
    Indicator *indicator;
    GDBusObject *device_object;
    GDBusProxy *adapter_proxy;

    indicator = g_malloc(sizeof(Indicator));
    indicator->next = NULL;
    indicator->update_mode_handler = 0;
    indicator->signal_agent_id = 0;

    device_object = g_dbus_interface_get_object(G_DBUS_INTERFACE(device_proxy));

    indicator->sni = sni_new(device_object);
    indicator->sni->context_menu_handler = (SNIActivateHandler) indicator_activate;
    indicator->sni->activate_handler = (SNIActivateHandler) indicator_activate;
    indicator->level = N_SIGNAL_THRESHOLDS + 1;

    sni_category_set(indicator->sni, "Hardware");
    sni_id_set(indicator->sni, APPLICATION_ID);
    sni_status_set(indicator->sni, "Active");

    {
	GVariant *adapter_path_var;
	const gchar *adapter_path;

	adapter_path_var = g_dbus_proxy_get_cached_property(device_proxy, "Adapter");
	adapter_path = g_variant_get_string(adapter_path_var, NULL);
	adapter_proxy = G_DBUS_PROXY(g_dbus_object_manager_get_interface(global.manager, adapter_path, IWD_IFACE_ADAPTER));
	g_variant_unref(adapter_path_var);
    }

    indicator->device_proxy = device_proxy;
    indicator->adapter_proxy = adapter_proxy;

    indicator->update_device_handler = g_signal_connect_swapped(device_proxy, "g-properties-changed", G_CALLBACK(indicator_set_device), indicator);
    indicator->update_adapter_handler = g_signal_connect_swapped(adapter_proxy, "g-properties-changed", G_CALLBACK(indicator_set_device), indicator);

    indicator_set_device(indicator);

    return indicator;
}

void indicator_rm(Indicator *indicator) {
    if (indicator->signal_agent_id != 0) {
	g_dbus_connection_unregister_object(g_dbus_proxy_get_connection(indicator->proxy), indicator->signal_agent_id);
    }

    sni_rm(indicator->sni);

    g_signal_handler_disconnect(indicator->device_proxy, indicator->update_device_handler);
    g_signal_handler_disconnect(indicator->adapter_proxy, indicator->update_adapter_handler);

    if (indicator->update_mode_handler != 0) {
	g_signal_handler_disconnect(indicator->proxy, indicator->update_mode_handler);
    }

    g_free(indicator);
}

void indicator_station_init_signal_agent(Indicator *indicator, GDBusProxy *station_proxy) {
    GError *err;
    const gchar *path;
    GVariant *levels;
    static const gchar *agent_error_msg = "Failed to register signal level agent: %s\n";

    path = g_dbus_proxy_get_object_path(station_proxy);

    err = NULL;
    indicator->signal_agent_id = g_dbus_connection_register_object(
	g_dbus_proxy_get_connection(station_proxy),
	path,
	&signal_agent_interface_info,
	&signal_agent_interface_vtable,
	indicator,
	NULL,
	&err);

    if (err != NULL) {
	g_printerr(agent_error_msg, err->message);
	g_error_free(err);
	return;
    }

    levels = g_variant_new_fixed_array(G_VARIANT_TYPE_INT16, signal_thresholds, N_SIGNAL_THRESHOLDS, sizeof(gint16));
    g_dbus_proxy_call(
	station_proxy,
	"RegisterSignalLevelAgent",
	g_variant_new("(o@an)", path, levels),
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) method_call_log,
	(gpointer) agent_error_msg);
}

void indicator_set_device(Indicator *indicator) {
    GVariant *powered_var;
    gboolean powered;

    powered_var = g_dbus_proxy_get_cached_property(indicator->adapter_proxy, "Powered");
    powered = g_variant_get_boolean(powered_var);
    g_variant_unref(powered_var);

    if (!powered) {
	sni_title_set(indicator->sni, _("Wireless hardware is disabled"));
	sni_icon_pixmap_set(indicator->sni,
		symbolic_icon_get_surface(ICON_ADAPTER_DISABLED, &colors.disabled_adapter));
	return;
    }

    powered_var = g_dbus_proxy_get_cached_property(indicator->device_proxy, "Powered");
    powered = g_variant_get_boolean(powered_var);
    g_variant_unref(powered_var);

    if (!powered) {
	sni_title_set(indicator->sni, _("Wireless interface is disabled"));
	sni_icon_pixmap_set(indicator->sni,
		symbolic_icon_get_surface(ICON_DEVICE_DISABLED, &colors.disabled_device));
    }
}

void indicator_set_station(Indicator *indicator) {
    GVariant *state_var;
    const gchar *state;

    state_var = g_dbus_proxy_get_cached_property(indicator->proxy, "State");
    state = g_variant_get_string(state_var, NULL);

    if (!strcmp(state, "connected")) {
	indicator->status = INDICATOR_STATION_CONNECTED;
	indicator_set_station_connected_title(indicator, _("Connected to %s"));

	if (indicator->level <= N_SIGNAL_THRESHOLDS) {
	    indicator_set_station_connected(indicator);
	}
    }
    else if (!strcmp(state, "connecting")) {
	indicator->status = INDICATOR_STATION_CONNECTING;
	indicator_set_station_connected_title(indicator, _("Connecting to %s"));

	if (indicator->level <= N_SIGNAL_THRESHOLDS) {
	    indicator_set_station_connected(indicator);
	}
    }
    else {
	indicator->status = INDICATOR_STATION_DISCONNECTED;
	sni_title_set(indicator->sni, _("Not connected to any wireless network"));
	sni_icon_pixmap_set(indicator->sni,
		symbolic_icon_get_surface(ICON_STATION_OFFLINE, &colors.station_disconnected));
    }

    g_variant_unref(state_var);
}

void indicator_set_station_connected(Indicator *indicator) {
    const GdkRGBA *color;

    if (indicator->status == INDICATOR_STATION_CONNECTED) {
	color = &colors.station_connected;
    }
    else if (indicator->status == INDICATOR_STATION_CONNECTING) {
	color = &colors.station_connecting;
    }
    else {
	g_printerr("Signal level update received, but the station is neither connected nor connecting\n");
	return;
    }

    sni_icon_pixmap_set(indicator->sni,
	    symbolic_icon_get_surface(station_icons[indicator->level], color));
}

void indicator_set_station_connected_title(Indicator *indicator, const gchar *title_template) {
    GVariant *connected_network_var;

    connected_network_var = g_dbus_proxy_get_cached_property(indicator->proxy, "ConnectedNetwork");

    if (connected_network_var) {
	const gchar *connected_network;
	GDBusProxy *network_proxy;

	connected_network = g_variant_get_string(connected_network_var, NULL);
	network_proxy = G_DBUS_PROXY(g_dbus_object_manager_get_interface(global.manager, connected_network, IWD_IFACE_NETWORK));

	if (network_proxy) {
	    sni_network_set_title(indicator->sni, network_proxy, title_template);
	    g_object_unref(network_proxy);
	    g_variant_unref(connected_network_var);
	}
	else {
	    SNITitleDelayed *data;

	    data = g_malloc(sizeof(SNITitleDelayed));
	    data->sni = indicator->sni;
	    data->title_template = title_template;
	    data->connected_network_var = connected_network_var;
	    data->handler = g_signal_connect(global.manager, "object-added", G_CALLBACK(sni_set_title_delayed), data);
	}
    }
    else {
	g_printerr("ConnectedNetwork property was expected, but not found\n");
    }
}

void sni_network_set_title(StatusNotifierItem *sni, GDBusProxy *network_proxy, const gchar *title_template) {
    GVariant *ssid_var;
    const gchar *ssid;
    gchar *title;

    ssid_var = g_dbus_proxy_get_cached_property(network_proxy, "Name");
    ssid = g_variant_get_string(ssid_var, NULL);

    title = g_strdup_printf(title_template, ssid);
    sni_title_set(sni, title);
    g_free(title);

    g_variant_unref(ssid_var);
}

void sni_set_title_delayed(GDBusObjectManager *manager, GDBusObject *object, SNITitleDelayed *data) {
    const gchar *network_path;
    GDBusProxy *network_proxy;

    network_path = g_variant_get_string(data->connected_network_var, NULL);

    if (strcmp(g_dbus_object_get_object_path(object), network_path)) {
	return;
    }

    network_proxy = G_DBUS_PROXY(g_dbus_object_get_interface(object, IWD_IFACE_NETWORK));

    if (network_proxy) {
	sni_network_set_title(data->sni, network_proxy, data->title_template);
	g_object_unref(network_proxy);
    }
    else {
	g_printerr("Failed to find interface of type " IWD_IFACE_NETWORK " on object %s\n", network_path);
    }

    g_signal_handler_disconnect(manager, data->handler);
    g_variant_unref(data->connected_network_var);
    g_free(data);
}

void indicator_set_ap(Indicator *indicator) {
    GVariant *started_var;

    started_var = g_dbus_proxy_get_cached_property(indicator->proxy, "Started");

    if (g_variant_get_boolean(started_var)) {
	sni_title_set(indicator->sni, _("Access point is up"));
	sni_icon_pixmap_set(indicator->sni,
		symbolic_icon_get_surface(ICON_AP, &colors.ap_up));
    }
    else {
	sni_title_set(indicator->sni, _("Access point is down"));
	sni_icon_pixmap_set(indicator->sni,
		symbolic_icon_get_surface(ICON_AP, &colors.ap_down));
    }

    g_variant_unref(started_var);
}

void indicator_set_adhoc(Indicator *indicator) {
    GVariant *started_var;

    started_var = g_dbus_proxy_get_cached_property(indicator->proxy, "Started");

    if (g_variant_get_boolean(started_var)) {
	sni_title_set(indicator->sni, _("Ad-hoc node is up"));
	sni_icon_pixmap_set(indicator->sni,
		symbolic_icon_get_surface(ICON_ADHOC, &colors.adhoc_up));
    }
    else {
	sni_title_set(indicator->sni, _("Ad-hoc node is down"));
	sni_icon_pixmap_set(indicator->sni,
		symbolic_icon_get_surface(ICON_ADHOC, &colors.adhoc_down));
    }

    g_variant_unref(started_var);
}

void indicator_activate(GDBusObject *device_object) {
    if (global.window != NULL) {
	gtk_window_destroy(GTK_WINDOW(global.window->window));
    }
    else {
	ObjectList *list;

	window_launch();
	list = global.window->objects[OBJECT_DEVICE];

	while (list != NULL) {
	    if (list->object == device_object) {
		Device *device;

		device = (Device *) list->data;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(device->button), TRUE);
		break;
	    }
	    list = list->next;
	}
    }
}

void signal_agent_method_call_handler(GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *method_name, GVariant *parameters, GDBusMethodInvocation *invocation, Indicator *indicator) {
    if (strcmp(method_name, "Changed") == 0) {
	const gchar *device;

	g_variant_get(parameters, "(oy)", &device, &indicator->level);
	g_dbus_method_invocation_return_value(invocation, NULL);

	if (indicator->level <= N_SIGNAL_THRESHOLDS) {
	    indicator_set_station_connected(indicator);
	}
	else {
	    g_printerr("Invalid signal level received\n");
	}
    }
    else if (strcmp(method_name, "Release") == 0) {
	g_dbus_method_invocation_return_value(invocation, NULL);
	g_dbus_connection_unregister_object(connection, indicator->signal_agent_id);
	indicator->signal_agent_id = 0;
    }
    else {
	g_dbus_method_invocation_return_dbus_error(invocation, "org.freedesktop.DBus.Error.UnknownMethod", "Unknown method");
    }
}
