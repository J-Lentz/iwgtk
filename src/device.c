/*
 *  Copyright 2020-2022 Jesse Lentz and contributors
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

void device_show(GtkToggleButton *button, Device *device) {
    if (gtk_toggle_button_get_active(button)) {
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(global.window->main), device->master);
    }
}

void device_set(Device *device) {
    {
	GVariant *mac_var;
	const gchar *mac;

	mac_var = g_dbus_proxy_get_cached_property(device->proxy, "Address");
	mac = g_variant_get_string(mac_var, NULL);
	gtk_label_set_text(GTK_LABEL(device->mac_label), mac);
	g_variant_unref(mac_var);
    }

    {
	GVariant *mode_var;
	const gchar *mode;

	mode_var = g_dbus_proxy_get_cached_property(device->proxy, "Mode");
	mode = g_variant_get_string(mode_var, NULL);
	gtk_combo_box_set_active_id(GTK_COMBO_BOX(device->mode_box), mode);
	g_variant_unref(mode_var);
    }
}

void mode_box_changed(GtkComboBox *box, Device *device) {
    const gchar *mode;

    mode = gtk_combo_box_get_active_id(box);
    set_remote_property(device->proxy, "Mode", g_variant_new_string(mode), (SetFunction) device_set, device);
}

GtkWidget* mode_box_new(GDBusProxy *adapter_proxy) {
    GtkWidget *box;
    GtkListStore *list_store;
    GtkTreeIter list_store_iter;
    GtkCellRenderer *cell_renderer;

    GVariant *supported_modes_var;
    GVariantIter supported_modes_iter;
    gchar *supported_mode;

    list_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

    supported_modes_var = g_dbus_proxy_get_cached_property(adapter_proxy, "SupportedModes");

    g_variant_iter_init(&supported_modes_iter, supported_modes_var);
    while (g_variant_iter_next(&supported_modes_iter, "s", &supported_mode)) {
	const gchar *supported_mode_display;

	if (strcmp(supported_mode, "station") == 0) {
	    supported_mode_display = _("Station");
	}
	else if (strcmp(supported_mode, "ap") == 0) {
	    supported_mode_display = _("AP");
	}
	else if (strcmp(supported_mode, "ad-hoc") == 0) {
	    supported_mode_display = _("Ad-hoc");
	}
	else {
	    supported_mode_display = supported_mode;
	}

	gtk_list_store_append(list_store, &list_store_iter);
	gtk_list_store_set(list_store, &list_store_iter, 0, supported_mode, 1, supported_mode_display, -1);
    }

    box = gtk_combo_box_new_with_model(GTK_TREE_MODEL(list_store));
    g_object_unref(list_store);

    cell_renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(box), cell_renderer, TRUE);
    gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(box), cell_renderer, "text", 1);

    gtk_combo_box_set_id_column(GTK_COMBO_BOX(box), 0);

    return box;
}

Device* device_add(Window *window, GDBusObject *object, GDBusProxy *proxy) {
    Device *device;

    device = g_malloc(sizeof(Device));
    device->proxy = proxy;

    {
	GVariant *name_var;
	const gchar *name;

	name_var = g_dbus_proxy_get_cached_property(proxy, "Name");
	name = g_variant_get_string(name_var, NULL);

	device->button = gtk_toggle_button_new_with_label(name);
	g_object_ref_sink(device->button);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(device->button), FALSE);
	gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(device->button), GTK_TOGGLE_BUTTON(window->known_network_button));
	gtk_widget_set_hexpand(device->button, TRUE);
	g_signal_connect(device->button, "toggled", G_CALLBACK(device_show), device);

	g_variant_unref(name_var);
    }

    {
	GVariant *adapter_path_var;
	const gchar *adapter_path;
	GDBusObject *adapter_object;
	GDBusProxy *adapter_proxy;

	adapter_path_var = g_dbus_proxy_get_cached_property(proxy, "Adapter");
	adapter_path = g_variant_get_string(adapter_path_var, NULL);
	adapter_object = g_dbus_object_manager_get_object(global.manager, adapter_path);
	g_variant_unref(adapter_path_var);

	couple_register(window, ADAPTER_DEVICE, 1, device, adapter_object);

	adapter_proxy = G_DBUS_PROXY(g_dbus_object_get_interface(adapter_object, IWD_IFACE_ADAPTER));
	device->mode_box = mode_box_new(adapter_proxy);
	g_object_unref(adapter_proxy);
	g_object_ref_sink(device->mode_box);
    }

    device->master = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    g_object_ref_sink(device->master);

    device->table = gtk_grid_new();
    g_object_ref_sink(device->table);

    gtk_box_append(GTK_BOX(device->master), device->table);
    gtk_box_append(GTK_BOX(device->master), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    gtk_widget_set_size_request(device->table, 400, -1);
    gtk_widget_set_halign(device->table, GTK_ALIGN_CENTER);

    device->mac_label = gtk_label_new(NULL);
    g_object_ref_sink(device->mac_label);
    gtk_widget_set_tooltip_text(GTK_WIDGET(device->mac_label), _("MAC address"));

    {
	GtkWidget *up_label, *mode_label, *power_switch;

	power_switch = switch_new(proxy, "Powered");
	gtk_widget_set_tooltip_text(GTK_WIDGET(power_switch), _("Enable/disable wireless interface"));

	up_label = gtk_label_new(_("Enabled: "));
	mode_label = gtk_label_new(_("Mode: "));

	gtk_grid_attach(GTK_GRID(device->table), device->mac_label, 0, 0, 1, 1);

	gtk_grid_attach(GTK_GRID(device->table), up_label,          1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(device->table), power_switch,      2, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(device->table), mode_label,        1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(device->table), device->mode_box,  2, 1, 1, 1);

	gtk_widget_set_margin_start(up_label, 10);
	gtk_widget_set_margin_start(mode_label, 10);
	gtk_widget_set_margin_end(power_switch, 10);
	gtk_widget_set_margin_end(device->mode_box, 10);

	gtk_widget_set_halign(device->mac_label, GTK_ALIGN_CENTER);
	gtk_widget_set_size_request(device->mac_label, -1, 34);

	gtk_widget_set_halign(up_label, GTK_ALIGN_END);
	gtk_widget_set_halign(mode_label, GTK_ALIGN_END);

	gtk_widget_set_halign(power_switch, GTK_ALIGN_START);
	gtk_widget_set_halign(device->mode_box, GTK_ALIGN_START);

	gtk_widget_set_valign(device->mac_label, GTK_ALIGN_CENTER);

	gtk_widget_set_valign(up_label, GTK_ALIGN_CENTER);
	gtk_widget_set_valign(mode_label, GTK_ALIGN_CENTER);

	gtk_widget_set_valign(power_switch, GTK_ALIGN_CENTER);
	gtk_widget_set_valign(device->mode_box, GTK_ALIGN_CENTER);

	gtk_grid_set_row_spacing(GTK_GRID(device->table), 3);
	gtk_grid_set_column_spacing(GTK_GRID(device->table), 3);
    }

    device->handler_update = g_signal_connect_swapped(proxy, "g-properties-changed", G_CALLBACK(device_set), device);
    device_set(device);

    g_signal_connect(device->mode_box, "changed", G_CALLBACK(mode_box_changed), device);

    couple_register(window, DEVICE_STATION,    0, device, object);
    couple_register(window, DEVICE_AP,         0, device, object);
    couple_register(window, DEVICE_ADHOC,      0, device, object);
    couple_register(window, DEVICE_DIAGNOSTIC, 0, device, object);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(device->button), TRUE);
    return device;
}

void device_remove(Window *window, Device *device) {
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(device->button))) {
	ObjectList *device_list;
	GtkWidget *button_alt;

	button_alt = NULL;
	device_list = window->objects[OBJECT_DEVICE];

	/*
	 * TODO: Select the next-in-line device more intelligently.
	 * e.g., prefer to switch to a device associated with the same adapter, if
	 * possible.
	 */
	while (device_list != NULL) {
	    Device *device_alt;
	    device_alt = (Device *) device_list->data;
	    if (device_alt != device) {
		button_alt = device_alt->button;
		break;
	    }
	    device_list = device_list->next;
	}

	if (!button_alt) {
	    button_alt = window->known_network_button;
	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_alt), TRUE);
    }

    couple_unregister(window, DEVICE_STATION,    0, device);
    couple_unregister(window, DEVICE_AP,         0, device);
    couple_unregister(window, DEVICE_ADHOC,      0, device);
    couple_unregister(window, DEVICE_DIAGNOSTIC, 0, device);

    couple_unregister(window, ADAPTER_DEVICE, 1, device);

    g_object_unref(device->button);
    g_object_unref(device->mode_box);
    g_object_unref(device->master);
    g_object_unref(device->table);
    g_object_unref(device->mac_label);

    g_signal_handler_disconnect(device->proxy, device->handler_update);
    g_free(device);
}
