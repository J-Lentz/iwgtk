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
 * TODO:
 * Use vendor and model names for something, or don't retrieve them at all.
 */
void adapter_set(Adapter *adapter) {
    GVariant *model_var, *vendor_var, *name_var;
    const gchar *model, *vendor, *name;
    gchar *card;

    model_var = g_dbus_proxy_get_cached_property(adapter->proxy, "Model");
    vendor_var = g_dbus_proxy_get_cached_property(adapter->proxy, "Vendor");
    name_var = g_dbus_proxy_get_cached_property(adapter->proxy, "Name");

    model = g_variant_get_string(model_var, NULL);
    vendor = g_variant_get_string(vendor_var, NULL);
    name = g_variant_get_string(name_var, NULL);

    gtk_label_set_text(GTK_LABEL(adapter->name_label), name);

    g_variant_unref(model_var);
    g_variant_unref(vendor_var);
    g_variant_unref(name_var);
}

Adapter* adapter_add(GDBusObject *object, GDBusProxy *proxy) {
    Adapter *adapter;
    GtkWidget *vbox;
    GtkWidget *row1;

    adapter = malloc(sizeof(Adapter));
    adapter->proxy = proxy;

    adapter->frame = gtk_frame_new(NULL);
    gtk_box_pack_start(GTK_BOX(global.header), adapter->frame, FALSE, FALSE, 0);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(adapter->frame), vbox);

    row1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    adapter->device_buttons = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(adapter->device_buttons), GTK_BUTTONBOX_SPREAD);

    gtk_box_pack_start(GTK_BOX(vbox), row1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), adapter->device_buttons, FALSE, FALSE, 0);

    adapter->name_label = gtk_label_new(NULL);
    gtk_box_pack_start(GTK_BOX(row1), adapter->name_label, FALSE, FALSE, 0);

    adapter->power_switch = switch_new(proxy, "Powered");
    gtk_box_pack_start(GTK_BOX(row1), adapter->power_switch, FALSE, FALSE, 0);

    gtk_widget_show_all(adapter->frame);

    g_signal_connect_swapped(proxy, "g-properties-changed", G_CALLBACK(adapter_set), (gpointer) adapter);
    adapter_set(adapter);

    couple_register(ADAPTER_DEVICE, 0, adapter, object);
    return adapter;
}

void adapter_remove(Adapter *adapter) {
    gtk_container_remove(GTK_CONTAINER(global.header), adapter->frame);
    couple_unregister(ADAPTER_DEVICE, 0, adapter);
    free(adapter);
}

void bind_adapter_device(Adapter *adapter, Device *device) {
    gtk_box_pack_start(GTK_BOX(adapter->device_buttons), device->button, FALSE, FALSE, 0);
}

void unbind_adapter_device(Adapter *adapter, Device *device) {
    gtk_container_remove(GTK_CONTAINER(adapter->device_buttons), device->button);
}
