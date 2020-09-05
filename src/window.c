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

ObjectType object_table[] = {
    {IWD_IFACE_KNOWN_NETWORK, (ConstructorFunction) known_network_add, (DestructorFunction) known_network_remove},
    {IWD_IFACE_ADAPTER,       (ConstructorFunction) adapter_add,       (DestructorFunction) adapter_remove},
    {IWD_IFACE_DEVICE,        (ConstructorFunction) device_add,        (DestructorFunction) device_remove},
    {IWD_IFACE_STATION,       (ConstructorFunction) station_add,       (DestructorFunction) station_remove},
    {IWD_IFACE_AP,            (ConstructorFunction) ap_add,            (DestructorFunction) ap_remove},
    {IWD_IFACE_AD_HOC,        (ConstructorFunction) adhoc_add,         (DestructorFunction) adhoc_remove},
    {IWD_IFACE_WPS,           (ConstructorFunction) wps_add,           (DestructorFunction) wps_remove},
    {IWD_IFACE_NETWORK,       (ConstructorFunction) network_add,       (DestructorFunction) network_remove}
};

CoupleType couple_table[] = {
    {(BindFunction) bind_adapter_device, (UnbindFunction) unbind_adapter_device},
    {(BindFunction) bind_device_station, (UnbindFunction) unbind_device_station},
    {(BindFunction) bind_device_ap,      (UnbindFunction) unbind_device_ap},
    {(BindFunction) bind_device_adhoc,   (UnbindFunction) unbind_device_adhoc},
    {(BindFunction) bind_device_wps,     (UnbindFunction) unbind_device_wps}
};

void window_new(GtkApplication *app) {
    Window *window;

    window = malloc(sizeof(Window));

    window->next = global.windows;
    global.windows = window;

    memset(window->objects, 0, sizeof(void *) * n_object_types);
    memset(window->couples, 0, sizeof(void *) * n_couple_types);

    window->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window->window), "iwgtk");
    gtk_window_set_default_size(GTK_WINDOW(window->window), 600, 700);
    gtk_window_set_position(GTK_WINDOW(window->window), GTK_WIN_POS_CENTER);

    window_set(window);
}

void window_set(Window *window) {
    bin_empty(GTK_BIN(window->window));

    if (!global.manager) {
	GtkWidget *label;

	/*
	 * TODO:
	 * Replace this with a more informative message.
	 */
	label = gtk_label_new("iwd is not running");

	gtk_container_add(GTK_CONTAINER(window->window), label);
	gtk_widget_show_all(window->window);
	return;
    }

    window->master = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    g_object_ref_sink(window->master);
    gtk_container_add(GTK_CONTAINER(window->window), window->master);

    window->header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    g_object_ref_sink(window->header);
    gtk_box_pack_start(GTK_BOX(window->master), window->header, FALSE, FALSE, 0);

    gtk_widget_set_margin_start(window->header, 5);
    gtk_widget_set_margin_end(window->header, 5);
    gtk_widget_set_margin_top(window->header, 5);

    gtk_box_pack_start(GTK_BOX(window->master), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 0);

    window->main = gtk_scrolled_window_new(NULL, NULL);
    g_object_ref_sink(window->main);
    gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(window->main), TRUE);
    gtk_box_pack_start(GTK_BOX(window->master), window->main, FALSE, FALSE, 0);

    window->known_network_button = gtk_radio_button_new_with_label(NULL, "Known Networks");
    g_object_ref_sink(window->known_network_button);
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(window->known_network_button), FALSE);

    {
	GtkWidget *known_network_button_vbox;

	known_network_button_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_end(GTK_BOX(window->header), known_network_button_vbox, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(known_network_button_vbox), window->known_network_button, FALSE, FALSE, 0);
    }

    g_signal_connect(window->known_network_button, "toggled", G_CALLBACK(known_network_table_show), window);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(window->known_network_button), TRUE);

    window->known_network_table = gtk_grid_new();
    g_object_ref_sink(window->known_network_table);
    gtk_container_add(GTK_CONTAINER(window->main), window->known_network_table);

    gtk_widget_set_margin_start(window->known_network_table, 5);
    gtk_widget_set_margin_end(window->known_network_table, 5);
    gtk_widget_set_margin_bottom(window->known_network_table, 5);

    gtk_grid_attach(GTK_GRID(window->known_network_table), new_label_bold("SSID"),            0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(window->known_network_table), new_label_bold("Security"),        1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(window->known_network_table), new_label_bold("Autoconnect"),     2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(window->known_network_table), new_label_bold("Forget"),          3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(window->known_network_table), new_label_bold("Last connection"), 4, 0, 1, 1);

    gtk_grid_set_column_spacing(GTK_GRID(window->known_network_table), 10);
    gtk_grid_set_row_spacing(GTK_GRID(window->known_network_table), 10);

    gtk_widget_show_all(window->window);

    window->handler_interface_add = g_signal_connect_swapped(global.manager, "interface-added",   G_CALLBACK(interface_add), window);
    window->handler_interface_rm  = g_signal_connect_swapped(global.manager, "interface-removed", G_CALLBACK(interface_rm),  window);
    window->handler_object_add    = g_signal_connect_swapped(global.manager, "object-added",      G_CALLBACK(object_add),    window);
    window->handler_object_rm     = g_signal_connect_swapped(global.manager, "object-removed",    G_CALLBACK(object_rm),     window);

    {
	GList *object_list, *i;

	object_list = g_dbus_object_manager_get_objects(global.manager);
	for (i = object_list; i != NULL; i = i->next) {
	    GDBusObject *object;

	    object = (GDBusObject *) i->data;
	    object_iterate_interfaces(window, object, interface_add);
	    g_object_unref(object);
	}
	g_list_free(object_list);
    }

    g_signal_connect_swapped(window->window, "destroy", G_CALLBACK(window_rm), window);
}

void window_rm(Window *window) {
    int i;

    for (i = 0; i < n_object_types; i ++) {
	while (window->objects[i] != NULL) {
	    object_table[i].destructor(window, window->objects[i]->data);

	    {
		ObjectList *rm;

		rm = window->objects[i];
		window->objects[i] = window->objects[i]->next;
		free(rm);
	    }
	}
    }

    /*
     * All couples should be unregistered at this point.
     * Let's check just to be sure.
     * TODO: Delete this eventually
     */
    for (i = 0; i < n_couple_types; i ++) {
	if (window->couples[i] != NULL) {
	    fprintf(stderr, "Error: A window is being destroyed with an unregistered couple\n");
	}
    }

    g_signal_handler_disconnect(global.manager, window->handler_interface_add);
    g_signal_handler_disconnect(global.manager, window->handler_interface_rm);
    g_signal_handler_disconnect(global.manager, window->handler_object_add);
    g_signal_handler_disconnect(global.manager, window->handler_object_rm);
}

void known_network_table_show(GtkToggleButton *button, Window *window) {
    if (gtk_toggle_button_get_active(button)) {
	bin_empty(GTK_BIN(window->main));
	gtk_container_add(GTK_CONTAINER(window->main), window->known_network_table);
    }
}

void object_add(Window *window, GDBusObject *object) {
    object_iterate_interfaces(window, object, interface_add);
}

void object_rm(Window *window, GDBusObject *object) {
    object_iterate_interfaces(window, object, interface_rm);
}

void object_iterate_interfaces(Window *window, GDBusObject *object, ObjectIterFunction method) {
	GList *interface_list, *j;

	interface_list = g_dbus_object_get_interfaces(object);
	for (j = interface_list; j != NULL; j = j->next) {
	    GDBusProxy *proxy;

	    proxy = (GDBusProxy *) j->data;
	    method(window, object, proxy);
	    g_object_unref(proxy);
	}
	g_list_free(interface_list);
}

void interface_add(Window *window, GDBusObject *object, GDBusProxy *proxy) {
    const gchar *name;

    name = g_dbus_proxy_get_interface_name(proxy);

    for (int i = 0; i < n_object_types; i ++) {
	if (strcmp(name, object_table[i].interface) == 0) {
	    ObjectList *object_list_entry;

	    object_list_entry = malloc(sizeof(ObjectList));
	    object_list_entry->object = object;
	    object_list_entry->data = object_table[i].constructor(window, object, proxy);
	    object_list_entry->next = window->objects[i];
	    window->objects[i] = object_list_entry;

	    break;
	}
    }
}

void interface_rm(Window *window, GDBusObject *object, GDBusProxy *proxy) {
    const gchar *name;

    name = g_dbus_proxy_get_interface_name(proxy);

    for (int i = 0; i < n_object_types; i ++) {
	if (!strcmp(name, object_table[i].interface)) {
	    ObjectList **list;

	    list = &window->objects[i];
	    while ((*list)->object != object) {
		list = &(*list)->next;
	    }

	    object_table[i].destructor(window, (*list)->data);

	    {
		ObjectList *rm;

		rm = *list;
		*list = (*list)->next;
		free(rm);
	    }

	    break;
	}
    }
}

void couple_register(Window *window, CoupleIndex couple_type, int this, gpointer data, GDBusObject *object) {
    CoupleList **list;

    list = &window->couples[couple_type];
    while (*list != NULL) {
	if ( (*list)->object == object) {
	    if ( (*list)->data[this] == NULL) {
		(*list)->data[this] = data;
	    }
	    else {
		CoupleList *new_entry;
		new_entry = malloc(sizeof(CoupleList));
		new_entry->object = object;
		new_entry->data[this] = data;
		new_entry->data[1 - this] = (*list)->data[1 - this];

		new_entry->next = (*list)->next;
		*list = new_entry;
	    }
	    couple_table[couple_type].bind( (*list)->data[0], (*list)->data[1] );
	    return;
	}
	else {
	    list = &(*list)->next;
	}
    }

    {
	CoupleList *new_entry;
	new_entry = malloc(sizeof(CoupleList));
	new_entry->object = object;
	new_entry->data[this] = data;
	new_entry->data[1 - this] = NULL;

	new_entry->next = NULL;
	*list = new_entry;
    }
}

void couple_unregister(Window *window, CoupleIndex couple_type, int this, gpointer data) {
    CoupleList **list;

    list = &window->couples[couple_type];
    while (*list != NULL) {
	if ( (*list)->data[this] == data ) {
	    if ( (*list)->data[1 - this] == NULL) {
		// Couple node is empty - delete it
		CoupleList *rm;

		rm = *list;
		*list = (*list)->next;
		free(rm);
	    }
	    else {
		// Unbind the couple
		couple_table[couple_type].unbind( (*list)->data[0], (*list)->data[1] );
		(*list)->data[this] = NULL;
		list = &(*list)->next;
	    }
	}
	else {
	    list = &(*list)->next;
	}
    }
}
