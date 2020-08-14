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
 * At the time this handler is called, "active" is updated but "state" is not.
 * Returning FALSE causes "state" to be updated by the default handler.
 * (returning TRUE would cause "state" not to be updated).
 */
gboolean switch_handler(GtkSwitch *widget, gboolean state, SwitchData *switch_data) {
    set_remote_property(switch_data->proxy, switch_data->property, g_variant_new_boolean(state));
    return FALSE;
}

void switch_update(GDBusProxy *proxy, GVariant *properties, gchar **invalidated_properties, SwitchData *switch_data) {
    GVariant *state_var;
    gboolean state;

    state_var = g_dbus_proxy_get_cached_property(proxy, switch_data->property);
    state = g_variant_get_boolean(state_var);
    gtk_switch_set_active(GTK_SWITCH(switch_data->widget), state);
    g_variant_unref(state_var);
}

void switch_destroy(GtkWidget *widget, SwitchData *switch_data) {
    free(switch_data);
}

GtkWidget* switch_new(GDBusProxy *proxy, const gchar *property) {
    SwitchData *switch_data;
    GVariant *state_var;
    gboolean state;

    switch_data = malloc(sizeof(SwitchData));
    switch_data->proxy = proxy;
    switch_data->widget = gtk_switch_new();
    switch_data->property = property;

    state_var = g_dbus_proxy_get_cached_property(proxy, property);
    state = g_variant_get_boolean(state_var);
    g_variant_unref(state_var);

    gtk_switch_set_active(GTK_SWITCH(switch_data->widget), state);
    g_signal_connect(proxy, "g-properties-changed", G_CALLBACK(switch_update), (gpointer) switch_data);
    g_signal_connect(switch_data->widget, "state-set", G_CALLBACK(switch_handler), (gpointer) switch_data);
    g_signal_connect(switch_data->widget, "destroy", G_CALLBACK(switch_destroy), (gpointer) switch_data);

    gtk_widget_set_halign(switch_data->widget, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(switch_data->widget, GTK_ALIGN_CENTER);

    gtk_widget_show(switch_data->widget);
    return switch_data->widget;
}
