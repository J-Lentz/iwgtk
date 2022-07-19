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

static int n_known_networks = 0;

void known_network_forget(GDBusProxy *proxy) {
    g_dbus_proxy_call(
	proxy,
	"Forget",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) method_call_log,
	"Failed to forget known network: %s\n");
}

void known_network_set(KnownNetwork *kn) {
    {
	GVariant *name_var;
	const gchar *name;

	name_var = g_dbus_proxy_get_cached_property(kn->proxy, "Name");
	name = g_variant_get_string(name_var, NULL);
	gtk_label_set_text(GTK_LABEL(kn->name_label), name);
	g_variant_unref(name_var);
    }

    {
	GVariant *type_var;
	const gchar *type_raw;

	type_var = g_dbus_proxy_get_cached_property(kn->proxy, "Type");
	type_raw = g_variant_get_string(type_var, NULL);
	gtk_label_set_text(GTK_LABEL(kn->security_label), get_security_type(type_raw));
	g_variant_unref(type_var);
    }

    {
	GVariant *hidden_var;
	gboolean hidden;

	hidden_var = g_dbus_proxy_get_cached_property(kn->proxy, "Hidden");
	hidden = g_variant_get_boolean(hidden_var);

	if (hidden) {
	    gtk_widget_show(kn->hidden_label);
	}
	else {
	    gtk_widget_hide(kn->hidden_label);
	}

	g_variant_unref(hidden_var);
    }

    {
	GVariant *last_connected_var;

	last_connected_var = g_dbus_proxy_get_cached_property(kn->proxy, "LastConnectedTime");
	if (last_connected_var) {
	    const gchar *last_connected_raw;

	    GDateTime *datetime_utc;
	    GDateTime *datetime_local;
	    gchar *datetime_formatted;

	    last_connected_raw = g_variant_get_string(last_connected_var, NULL);

	    datetime_utc = g_date_time_new_from_iso8601(last_connected_raw, NULL);
	    datetime_local = g_date_time_to_local(datetime_utc);
	    g_date_time_unref(datetime_utc);

	    datetime_formatted = g_date_time_format(datetime_local,
		    (global.last_connection_time_fmt ? global.last_connection_time_fmt : "%x\n%l:%M %p"));
	    g_date_time_unref(datetime_local);

	    gtk_label_set_text(GTK_LABEL(kn->last_connection_label), datetime_formatted);
	    g_free(datetime_formatted);

	    g_variant_unref(last_connected_var);
	}
	else {
	    gtk_label_set_text(GTK_LABEL(kn->last_connection_label), _("Never"));
	}
    }
}

KnownNetwork* known_network_add(Window *window, GDBusObject *object, GDBusProxy *proxy) {
    KnownNetwork *kn;
    GtkWidget *name_box, *autoconnect_switch, *forget_button;

    kn = g_malloc(sizeof(KnownNetwork));
    kn->proxy = proxy;

    kn->name_label = gtk_label_new(NULL);
    g_object_ref_sink(kn->name_label);
    gtk_widget_set_tooltip_text(kn->name_label, _("SSID"));

    kn->hidden_label = new_label_gray(_("(Hidden)"));
    g_object_ref_sink(kn->hidden_label);

    name_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(name_box), kn->name_label);
    gtk_box_append(GTK_BOX(name_box), kn->hidden_label);

    kn->security_label = gtk_label_new(NULL);
    g_object_ref_sink(kn->security_label);
    gtk_widget_set_tooltip_text(kn->security_label, _("Network security"));

    autoconnect_switch = switch_new(proxy, "AutoConnect");
    gtk_widget_set_tooltip_text(autoconnect_switch, _("Automatically connect to this network"));

    forget_button = gtk_button_new_with_label(_("Forget"));
    g_signal_connect_swapped(forget_button, "clicked", G_CALLBACK(known_network_forget), proxy);
    gtk_widget_set_tooltip_text(forget_button, _("Forget this network"));

    kn->last_connection_label = gtk_label_new(NULL);
    g_object_ref_sink(kn->last_connection_label);
    gtk_widget_set_tooltip_text(kn->last_connection_label, _("Most recent connection to this network"));

    gtk_widget_set_hexpand(name_box, TRUE);

    gtk_widget_set_halign(name_box,                  GTK_ALIGN_START);
    gtk_widget_set_halign(kn->name_label,            GTK_ALIGN_START);
    gtk_widget_set_halign(kn->hidden_label,          GTK_ALIGN_START);

    gtk_widget_set_halign(kn->security_label,        GTK_ALIGN_START);
    gtk_widget_set_halign(autoconnect_switch,        GTK_ALIGN_START);
    gtk_widget_set_halign(forget_button,             GTK_ALIGN_START);
    gtk_widget_set_halign(kn->last_connection_label, GTK_ALIGN_START);

    gtk_grid_attach(GTK_GRID(window->known_network_table), name_box,                  0, n_known_networks, 1, 1);
    gtk_grid_attach(GTK_GRID(window->known_network_table), kn->security_label,        1, n_known_networks, 1, 1);
    gtk_grid_attach(GTK_GRID(window->known_network_table), autoconnect_switch,        2, n_known_networks, 1, 1);
    gtk_grid_attach(GTK_GRID(window->known_network_table), forget_button,             3, n_known_networks, 1, 1);
    gtk_grid_attach(GTK_GRID(window->known_network_table), kn->last_connection_label, 4, n_known_networks, 1, 1);

    kn->handler_update = g_signal_connect_swapped(proxy, "g-properties-changed", G_CALLBACK(known_network_set), kn);
    known_network_set(kn);

    n_known_networks ++;

    return kn;
}

void known_network_remove(Window *window, KnownNetwork *kn) {
    g_signal_handler_disconnect(kn->proxy, kn->handler_update);

    {
	int n;

	gtk_grid_query_child(GTK_GRID(window->known_network_table), kn->security_label, NULL, &n, NULL, NULL);
	gtk_grid_remove_row(GTK_GRID(window->known_network_table), n);
    }

    g_object_unref(kn->name_label);
    g_object_unref(kn->hidden_label);
    g_object_unref(kn->security_label);
    g_object_unref(kn->last_connection_label);
    g_free(kn);

    n_known_networks --;
}
