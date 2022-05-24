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

Indicator* indicator_new(GDBusProxy *proxy, IndicatorSetter indicator_set) {
    Indicator *indicator;
    GDBusObject *device_object;

    indicator = malloc(sizeof(Indicator));
    indicator->next = NULL;

    device_object = g_dbus_interface_get_object(G_DBUS_INTERFACE(proxy));

    indicator->sni = sni_new(device_object);
    indicator->sni->context_menu_handler = (SNIActivateHandler) indicator_activate;
    indicator->sni->activate_handler = (SNIActivateHandler) indicator_activate;
    indicator->level = N_SIGNAL_THRESHOLDS + 1;

    sni_category_set(indicator->sni, "Hardware");
    sni_id_set(indicator->sni, APPLICATION_ID);
    sni_status_set(indicator->sni, "Active");

    indicator->proxy = proxy;
    indicator->update_handler = g_signal_connect_swapped(proxy, "g-properties-changed", G_CALLBACK(indicator_set), indicator);

    if (indicator_set == indicator_set_station) {
	GError *err;
	const gchar *path;
	GVariant *levels;

	path = g_dbus_proxy_get_object_path(proxy);

	err = NULL;
	indicator->signal_agent_id = g_dbus_connection_register_object(
	    g_dbus_proxy_get_connection(proxy),
	    path,
	    &signal_agent_interface_info,
	    &signal_agent_interface_vtable,
	    indicator,
	    NULL,
	    &err);

	if (err != NULL) {
	    fprintf(stderr, "Error registering signal level agent: %s\n", err->message);
	    g_error_free(err);
	}

	levels = g_variant_new_fixed_array(G_VARIANT_TYPE_INT16, signal_thresholds, N_SIGNAL_THRESHOLDS, sizeof(gint16));
	g_dbus_proxy_call(
	    indicator->proxy,
	    "RegisterSignalLevelAgent",
	    g_variant_new("(o@an)", path, levels),
	    G_DBUS_CALL_FLAGS_NONE,
	    -1,
	    NULL,
	    (GAsyncReadyCallback) validation_callback_log,
	    "Failed to register signal level agent: %s\n");
    }
    else {
	indicator->signal_agent_id = 0;
    }

    indicator_set(indicator);
    g_application_hold(G_APPLICATION(global.application));

    return indicator;
}

void indicator_rm(Indicator *indicator) {
    if (indicator->signal_agent_id != 0) {
	g_dbus_connection_unregister_object(g_dbus_proxy_get_connection(indicator->proxy), indicator->signal_agent_id);
    }

    sni_rm(indicator->sni);
    g_signal_handler_disconnect(indicator->proxy, indicator->update_handler);
    free(indicator);
    g_application_release(G_APPLICATION(global.application));
}

void indicator_set_station(Indicator *indicator) {
    GVariant *state_var;
    const gchar *state;

    state_var = g_dbus_proxy_get_cached_property(indicator->proxy, "State");
    state = g_variant_get_string(state_var, NULL);

    if (!strcmp(state, "connected")) {
	indicator->status = INDICATOR_STATION_CONNECTED;
	sni_title_set(indicator->sni, "Connected to wifi network");

	if (indicator->level <= N_SIGNAL_THRESHOLDS) {
	    indicator_set_connected(indicator);
	}
    }
    else if (!strcmp(state, "connecting")) {
	indicator->status = INDICATOR_STATION_CONNECTING;
	sni_title_set(indicator->sni, "Connecting to wifi network");

	if (indicator->level <= N_SIGNAL_THRESHOLDS) {
	    indicator_set_connected(indicator);
	}
    }
    else {
	indicator->status = INDICATOR_STATION_DISCONNECTED;
	sni_title_set(indicator->sni, "Not connected to any wifi network");
	icon_load(ICON_STATION_OFFLINE, &color_gray, (IconLoadCallback) sni_icon_pixmap_set, indicator->sni);
    }

    g_variant_unref(state_var);
}

void indicator_set_connected(Indicator *indicator) {
    const GdkRGBA *color;

    if (indicator->status == INDICATOR_STATION_CONNECTED) {
	color = &color_green;
    }
    else if (indicator->status == INDICATOR_STATION_CONNECTING) {
	color = &color_yellow;
    }
    else {
	fprintf(stderr, "Error: Signal level is set, but the station is neither connected nor connecting\n");
	return;
    }

    icon_load(station_icons[indicator->level], color, (IconLoadCallback) sni_icon_pixmap_set, indicator->sni);
}

void indicator_set_ap(Indicator *indicator) {
    GVariant *started_var;

    started_var = g_dbus_proxy_get_cached_property(indicator->proxy, "Started");

    if (g_variant_get_boolean(started_var)) {
	sni_title_set(indicator->sni, "AP is up");
	icon_load(ICON_AP, &color_green, (IconLoadCallback) sni_icon_pixmap_set, indicator->sni);
    }
    else {
	sni_title_set(indicator->sni, "AP is down");
	icon_load(ICON_AP, &color_gray, (IconLoadCallback) sni_icon_pixmap_set, indicator->sni);
    }

    g_variant_unref(started_var);
}

void indicator_set_adhoc(Indicator *indicator) {
    GVariant *started_var;

    started_var = g_dbus_proxy_get_cached_property(indicator->proxy, "Started");

    if (g_variant_get_boolean(started_var)) {
	sni_title_set(indicator->sni, "Ad-hoc node is up");
	icon_load(ICON_ADHOC, &color_green, (IconLoadCallback) sni_icon_pixmap_set, indicator->sni);
    }
    else {
	sni_title_set(indicator->sni, "Ad-hoc node is down");
	icon_load(ICON_ADHOC, &color_gray, (IconLoadCallback) sni_icon_pixmap_set, indicator->sni);
    }

    g_variant_unref(started_var);
}

void indicator_activate(GDBusObject *device_object) {
    if (global.window != NULL) {
	gtk_widget_destroy(global.window->window);
    }
    else {
	ObjectList *list;

	window_new();
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
	    indicator_set_connected(indicator);
	}
	else {
	    fprintf(stderr, "Error: SignalLevelAgent provided an invalid signal level\n");
	}
    }
    else if (strcmp(method_name, "Release") == 0) {
	/*
	 * NOOP
	 */
	g_dbus_method_invocation_return_value(invocation, NULL);
    }
    else {
	g_dbus_method_invocation_return_dbus_error(invocation, "org.freedesktop.DBus.Error.UnknownMethod", "Unknown method");
    }
}
