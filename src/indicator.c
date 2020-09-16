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

Indicator* indicator_new(GDBusProxy *proxy, IndicatorSetter indicator_set) {
    Indicator *indicator;
    GDBusObject *device_object;

    indicator = malloc(sizeof(Indicator));
    indicator->next = NULL;

    device_object = g_dbus_interface_get_object(G_DBUS_INTERFACE(proxy));
    indicator->sni = sni_new(device_object);
    indicator->sni->activate = (SNIActivateHandler) indicator_activate;
    indicator->sni->context_menu = (SNIActivateHandler) indicator_activate;

    indicator->proxy = proxy;
    indicator->update_handler = g_signal_connect_swapped(proxy, "g-properties-changed", G_CALLBACK(indicator_set), indicator);

    indicator_set(indicator);

    g_application_hold(G_APPLICATION(global.application));

    return indicator;
}

void indicator_rm(Indicator *indicator) {
    sni_rm(indicator->sni);
    g_application_release(G_APPLICATION(global.application));
    g_signal_handler_disconnect(indicator->proxy, indicator->update_handler);
    free(indicator);
}

void indicator_set_station(Indicator *indicator) {
    GVariant *state_var;
    const gchar *state;
    const gchar *icon_name;
    const gchar *icon_desc;

    state_var = g_dbus_proxy_get_cached_property(indicator->proxy, "State");
    state = g_variant_get_string(state_var, NULL);

    if (!strcmp(state, "connected")) {
	icon_name = APPLICATION_ID ".station-up";
	icon_desc = "Connected to wifi network";
    }
    else if (!strcmp(state, "connecting")) {
	icon_name = APPLICATION_ID ".station-connecting";
	icon_desc = "Connecting to wifi network";
    }
    else {
	icon_name = APPLICATION_ID ".station-down";
	icon_desc = "Not connected to a wifi network";
    }

    g_variant_unref(state_var);

    sni_icon_set(indicator->sni, icon_name);
    sni_title_set(indicator->sni, icon_desc);
}

void indicator_set_ap(Indicator *indicator) {
    gboolean started;
    const gchar *icon_name;
    const gchar *icon_desc;

    {
	GVariant *started_var;
	started_var = g_dbus_proxy_get_cached_property(indicator->proxy, "Started");
	started = g_variant_get_boolean(started_var);
	g_variant_unref(started_var);
    }

    if (started) {
	icon_name = APPLICATION_ID ".ap-up";
	icon_desc = "AP is up";
    }
    else {
	icon_name = APPLICATION_ID ".ap-down";
	icon_desc = "AP is down";
    }

    sni_icon_set(indicator->sni, icon_name);
    sni_title_set(indicator->sni, icon_desc);
}

/*
 * TODO:
 * Merge common code from indicator_set_ap() and indicator_set_adhoc() into a single
 * procedure.
 *
 * TODO:
 * Use a distinct icon for ad-hoc mode
 */
void indicator_set_adhoc(Indicator *indicator) {
    gboolean started;
    const gchar *icon_name;
    const gchar *icon_desc;

    {
	GVariant *started_var;
	started_var = g_dbus_proxy_get_cached_property(indicator->proxy, "Started");
	started = g_variant_get_boolean(started_var);
	g_variant_unref(started_var);
    }

    if (started) {
	icon_name = APPLICATION_ID ".ap-up";
	icon_desc = "Ad-hoc node is up";
    }
    else {
	icon_name = APPLICATION_ID ".ap-down";
	icon_desc = "Ad-hoc node is down";
    }

    sni_icon_set(indicator->sni, icon_name);
    sni_title_set(indicator->sni, icon_desc);
}

void indicator_activate(GDBusObject *device_object) {
    /*
     * If there is an active window, our objective is to destroy it.
     */
    {
	Window *window;

	window = global.windows;
        if (window != NULL) {
	    while (window != NULL) {
                gtk_widget_destroy(window->window);
	        window = window->next;
	    }
            return;
        }
    }

    /*
     * There is no active window, so our objective is to create one.
     */
    {
	ObjectList *list;

	/*
	 * window_new() prepends the global.windows list.
	 * If that behavior changes, this function will break!
	 */

	window_new(global.application);
	list = global.windows->objects[OBJECT_DEVICE];

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
