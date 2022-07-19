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

void adapter_set(Adapter *adapter) {
    {
	GVariant *name_var;
	const gchar *name;

	name_var = g_dbus_proxy_get_cached_property(adapter->proxy, "Name");
	name = g_variant_get_string(name_var, NULL);
	gtk_label_set_text(GTK_LABEL(adapter->name_label), name);
	g_variant_unref(name_var);
    }

    {
	GVariant *vendor_var, *model_var;
	const gchar *vendor, *model;
	gchar *card_full_name;

	vendor_var = g_dbus_proxy_get_cached_property(adapter->proxy, "Vendor");
	model_var = g_dbus_proxy_get_cached_property(adapter->proxy, "Model");

	vendor = g_variant_get_string(vendor_var, NULL);
	model = g_variant_get_string(model_var, NULL);

	card_full_name = g_strconcat(vendor, " ", model, NULL);
	g_variant_unref(vendor_var);
	g_variant_unref(model_var);

	gtk_widget_set_tooltip_text(GTK_WIDGET(adapter->frame), card_full_name);
	g_free(card_full_name);
    }
}

Adapter* adapter_add(Window *window, GDBusObject *object, GDBusProxy *proxy) {
    Adapter *adapter;
    GtkWidget *vbox;
    GtkWidget *row1;

    adapter = g_malloc(sizeof(Adapter));
    adapter->proxy = proxy;

    adapter->frame = gtk_frame_new(NULL);
    g_object_ref_sink(adapter->frame);

    gtk_widget_set_hexpand(adapter->frame, FALSE);
    gtk_box_append(GTK_BOX(window->adapter_hbox), adapter->frame);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_frame_set_child(GTK_FRAME(adapter->frame), vbox);

    row1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    adapter->device_buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    g_object_ref_sink(adapter->device_buttons);

    gtk_box_append(GTK_BOX(vbox), row1);
    gtk_box_append(GTK_BOX(vbox), adapter->device_buttons);

    adapter->name_label = gtk_label_new(NULL);
    g_object_ref_sink(adapter->name_label);
    gtk_box_append(GTK_BOX(row1), adapter->name_label);

    adapter->power_switch = switch_new(proxy, "Powered");
    g_object_ref_sink(adapter->power_switch);
    gtk_box_append(GTK_BOX(row1), adapter->power_switch);
    gtk_widget_set_tooltip_text(GTK_WIDGET(adapter->power_switch), _("Enable/disable wireless adapter (RF kill)"));

    adapter->handler_update = g_signal_connect_swapped(proxy, "g-properties-changed", G_CALLBACK(adapter_set), adapter);
    adapter_set(adapter);

    couple_register(window, ADAPTER_DEVICE, 0, adapter, object);
    return adapter;
}

void adapter_remove(Window *window, Adapter *adapter) {
    gtk_box_remove(GTK_BOX(window->adapter_hbox), adapter->frame);
    couple_unregister(window, ADAPTER_DEVICE, 0, adapter);

    g_object_unref(adapter->frame);
    g_object_unref(adapter->device_buttons);
    g_object_unref(adapter->name_label);
    g_object_unref(adapter->power_switch);

    g_signal_handler_disconnect(adapter->proxy, adapter->handler_update);
    g_free(adapter);
}

void bind_adapter_device(Adapter *adapter, Device *device) {
    gtk_box_append(GTK_BOX(adapter->device_buttons), device->button);
}

void unbind_adapter_device(Adapter *adapter, Device *device) {
    gtk_box_remove(GTK_BOX(adapter->device_buttons), device->button);
}
