/*
 *  Copyright 2020-2025 Jesse Lentz and contributors
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

ObjectMethods object_methods[] = {
    {IWD_IFACE_KNOWN_NETWORK,      (ConstructorFunction) known_network_add, (DestructorFunction) known_network_remove},
    {IWD_IFACE_ADAPTER,            (ConstructorFunction) adapter_add,       (DestructorFunction) adapter_remove},
    {IWD_IFACE_DEVICE,             (ConstructorFunction) device_add,        (DestructorFunction) device_remove},
    {IWD_IFACE_STATION,            (ConstructorFunction) station_add,       (DestructorFunction) station_remove},
    {IWD_IFACE_AP,                 (ConstructorFunction) ap_add,            (DestructorFunction) ap_remove},
    {IWD_IFACE_AD_HOC,             (ConstructorFunction) adhoc_add,         (DestructorFunction) adhoc_remove},
    {IWD_IFACE_DPP,                (ConstructorFunction) dpp_add,           (DestructorFunction) dpp_remove},
    {IWD_IFACE_WPS,                (ConstructorFunction) wps_add,           (DestructorFunction) wps_remove},
    {IWD_IFACE_STATION_DIAGNOSTIC, (ConstructorFunction) diagnostic_add,    (DestructorFunction) diagnostic_remove}
};

CoupleMethods couple_methods[] = {
    {(BindFunction) bind_adapter_device,    (UnbindFunction) unbind_adapter_device},
    {(BindFunction) bind_device_station,    (UnbindFunction) unbind_device_station},
    {(BindFunction) bind_device_ap,         (UnbindFunction) unbind_device_ap},
    {(BindFunction) bind_device_adhoc,      (UnbindFunction) unbind_device_adhoc},
    {(BindFunction) bind_station_dpp,       (UnbindFunction) unbind_station_dpp},
    {(BindFunction) bind_station_wps,       (UnbindFunction) unbind_station_wps},
    {(BindFunction) bind_device_diagnostic, (UnbindFunction) unbind_device_diagnostic}
};

void window_launch() {
    Window *window;

    if (global.window) {
	gtk_window_present(GTK_WINDOW(global.window->window));
	return;
    }

    if (!global.manager) {
	if (global.state & IWD_DOWN) {
	    g_printerr("Cannot launch iwgtk window: iwd is not running\n");
	}
	else {
	    global.state |= WINDOW_LAUNCH_PENDING;
	    g_application_hold(G_APPLICATION(global.application));
	}

	return;
    }

    window = g_malloc(sizeof(Window));
    global.window = window;

    memset(window->objects, 0, sizeof(void *) * n_object_types);
    memset(window->couples, 0, sizeof(void *) * n_couple_types);

    window->window = gtk_application_window_new(global.application);
    gtk_window_set_title(GTK_WINDOW(window->window), PACKAGE);
    gtk_window_set_default_size(GTK_WINDOW(window->window), global.width, global.height);
    gtk_window_set_icon_name(GTK_WINDOW(window->window), PACKAGE);

    window->master = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    g_object_ref_sink(window->master);
    gtk_widget_set_size_request(window->master, 440, 300);
    gtk_window_set_child(GTK_WINDOW(window->window), window->master);

    window->known_network_button = gtk_toggle_button_new_with_label(_("Known networks"));
    g_object_ref_sink(window->known_network_button);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(window->known_network_button), TRUE);
    g_signal_connect(window->known_network_button, "toggled", G_CALLBACK(known_network_table_show), window);

    {
	GtkWidget *header;
	GtkWidget *known_network_button_vbox;

	header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_box_append(GTK_BOX(window->master), header);

	gtk_widget_set_margin_start(header, 5);
	gtk_widget_set_margin_end(header, 5);
	gtk_widget_set_margin_top(header, 5);

	window->adapter_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	g_object_ref_sink(window->adapter_hbox);
	gtk_widget_set_hexpand(window->adapter_hbox, TRUE);
	gtk_box_append(GTK_BOX(header), window->adapter_hbox);

	known_network_button_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_append(GTK_BOX(header), known_network_button_vbox);

	{
	    GtkWidget *close_button;

	    close_button = gtk_button_new_with_label(_("Close"));
	    g_signal_connect_swapped(close_button, "clicked", G_CALLBACK(gtk_window_destroy), window->window);
	    gtk_box_append(GTK_BOX(known_network_button_vbox), close_button);
	}

	gtk_box_append(GTK_BOX(known_network_button_vbox), window->known_network_button);
    }

    gtk_box_append(GTK_BOX(window->master), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    window->main = gtk_scrolled_window_new();
    g_object_ref_sink(window->main);
    gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(window->main), TRUE);
    gtk_box_append(GTK_BOX(window->master), window->main);

    window->known_network_table = gtk_grid_new();
    g_object_ref_sink(window->known_network_table);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(window->main), window->known_network_table);

    gtk_widget_set_margin_start(window->known_network_table, 5);
    gtk_widget_set_margin_end(window->known_network_table, 5);
    gtk_widget_set_margin_bottom(window->known_network_table, 5);

    gtk_grid_set_column_spacing(GTK_GRID(window->known_network_table), 10);
    gtk_grid_set_row_spacing(GTK_GRID(window->known_network_table), 10);

    add_all_dbus_objects(window);
    g_signal_connect_swapped(window->window, "destroy", G_CALLBACK(window_rm), window);
    gtk_widget_set_visible(window->window, true);
}

void window_rm(Window *window) {
    for (int i = 0; i < n_object_types; i ++) {
	while (window->objects[i] != NULL) {
	    object_methods[i].rm(window, window->objects[i]->data);

	    {
		ObjectList *rm;

		rm = window->objects[i];
		window->objects[i] = window->objects[i]->next;
		g_free(rm);
	    }
	}
    }

    g_object_unref(window->master);
    g_object_unref(window->adapter_hbox);
    g_object_unref(window->main);
    g_object_unref(window->known_network_button);
    g_object_unref(window->known_network_table);

    g_free(window);
    global.window = NULL;
}

void known_network_table_show(GtkToggleButton *button, Window *window) {
    if (gtk_toggle_button_get_active(button)) {
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(window->main), window->known_network_table);
    }
}

/*
 * window=NULL implies that all interfaces are to be added to the indicator list, rather
 * than to a window.
 */
void add_all_dbus_objects(Window *window) {
    GList *object_list, *i;

    object_list = g_dbus_object_manager_get_objects(global.manager);
    for (i = object_list; i != NULL; i = i->next) {
	GDBusObject *object;

	object = (GDBusObject *) i->data;
	object_iterate_interfaces(NULL, object, window, interface_add);
	g_object_unref(object);
    }
    g_list_free(object_list);
}

void object_add(GDBusObjectManager *manager, GDBusObject *object, Window *window) {
    object_iterate_interfaces(manager, object, window, interface_add);
}

void object_rm(GDBusObjectManager *manager, GDBusObject *object, Window *window) {
    object_iterate_interfaces(manager, object, window, interface_rm);
}

void object_iterate_interfaces(GDBusObjectManager *manager, GDBusObject *object, Window *window, ObjectIterFunction method) {
	GList *interface_list, *j;

	interface_list = g_dbus_object_get_interfaces(object);
	for (j = interface_list; j != NULL; j = j->next) {
	    GDBusProxy *proxy;

	    proxy = (GDBusProxy *) j->data;
	    method(manager, object, proxy, window);
	    g_object_unref(proxy);
	}
	g_list_free(interface_list);
}

void interface_add(GDBusObjectManager *manager, GDBusObject *object, GDBusProxy *proxy, Window *window) {
    const gchar *name;

    name = g_dbus_proxy_get_interface_name(proxy);

    for (int i = 0; i < n_object_types; i ++) {
	if (strcmp(name, object_methods[i].interface) == 0) {
	    if (window != NULL) {
		// This function has been invoked for a particular window.
		window_add_object(object, proxy, window, i);
	    }
	    else {
		/*
		 * If window=NULL and manager=NULL, then we are only interested in
		 * initializing the indicators.
		 */
		if (manager != NULL && global.window != NULL) {
		    window_add_object(object, proxy, global.window, i);
		}

		/*
		 * There are two scenarios when we want to run this:
		 * (1) due to a new object/interface being added (window=NULL, manager!=NULL)
		 * (2) right after the manager is initialized (window=NULL, manager=NULL
		 */
		if (global.state & INDICATOR_DAEMON) {
		    Indicator **indicator;

		    indicator = &global.indicators;
		    while (*indicator != NULL) {
			GDBusObject *device_object;

			device_object = g_dbus_interface_get_object(G_DBUS_INTERFACE((*indicator)->device_proxy));

			if (object == device_object) {
			    IndicatorSetter indicator_set_mode;

			    indicator_set_mode = NULL;

			    switch (i) {
				case OBJECT_STATION:
				    indicator_station_init_signal_agent(*indicator, proxy);
				    indicator_set_mode = indicator_set_station;
				    break;
				case OBJECT_ACCESS_POINT:
				    indicator_set_mode = indicator_set_ap;
				    break;
				case OBJECT_ADHOC:
				    indicator_set_mode = indicator_set_adhoc;
				    break;
			    }

			    if (indicator_set_mode != NULL) {
				(*indicator)->proxy = proxy;
				(*indicator)->update_mode_handler = g_signal_connect_swapped(proxy, "g-properties-changed", G_CALLBACK(indicator_set_mode), *indicator);
				indicator_set_mode(*indicator);
			    }

			    return;
			}

			indicator = &(*indicator)->next;
		    }

		    if (i == OBJECT_DEVICE) {
			*indicator = indicator_new(proxy);
		    }
		}
	    }

	    break;
	}
    }
}

void object_list_append(ObjectList **list, GDBusObject *object, gpointer data) {
    while (*list != NULL) {
	list = &(*list)->next;
    }

    *list = g_malloc(sizeof(ObjectList));
    (*list)->object = object;
    (*list)->data = data;
    (*list)->next = NULL;
}

void window_add_object(GDBusObject *object, GDBusProxy *proxy, Window *window, int type) {
    gpointer data;

    data = object_methods[type].new(window, object, proxy);
    object_list_append(&window->objects[type], object, data);
}

void interface_rm(GDBusObjectManager *manager, GDBusObject *object, GDBusProxy *proxy, Window *window) {
    const gchar *name;

    name = g_dbus_proxy_get_interface_name(proxy);

    for (int i = 0; i < n_object_types; i ++) {
	if (!strcmp(name, object_methods[i].interface)) {
	    if (global.window != NULL) {
		ObjectList **list;

		list = &global.window->objects[i];
		while ((*list)->object != object) {
		    list = &(*list)->next;
		}

		object_methods[i].rm(global.window, (*list)->data);

		{
		    ObjectList *rm;

		    rm = *list;
		    *list = (*list)->next;
		    g_free(rm);
		}
	    }

	    if (global.state & INDICATOR_DAEMON) {
		Indicator **indicator;

		indicator = &global.indicators;

		while (*indicator != NULL) {
		    GDBusObject *device_object;

		    device_object = g_dbus_interface_get_object(G_DBUS_INTERFACE((*indicator)->device_proxy));

		    if (device_object == object) {
			if (proxy == (*indicator)->device_proxy) {
			    // The indicator's device has been taken down; delete the indicator

			    Indicator *rm;

			    rm = *indicator;
			    *indicator = (*indicator)->next;
			    indicator_rm(rm);
			}
			else if (proxy == (*indicator)->proxy) {
			    // The indicator's mode has changed, or it has been powered down

			    g_signal_handler_disconnect(proxy, (*indicator)->update_mode_handler);
			    (*indicator)->update_mode_handler = 0;
			}

			return;
		    }

		    indicator = &(*indicator)->next;
		}
	    }

	    break;
	}
    }
}

void couple_register(Window *window, CoupleType couple_type, int this, gpointer data, GDBusObject *object) {
    CoupleList **list;

    list = &window->couples[couple_type];
    while (*list != NULL) {
	if ( (*list)->object == object) {
	    if ( (*list)->data[this] == NULL) {
		(*list)->data[this] = data;
	    }
	    else {
		CoupleList *new_entry;
		new_entry = g_malloc(sizeof(CoupleList));
		new_entry->object = object;
		new_entry->data[this] = data;
		new_entry->data[1 - this] = (*list)->data[1 - this];

		new_entry->next = (*list)->next;
		*list = new_entry;
	    }
	    couple_methods[couple_type].bind( (*list)->data[0], (*list)->data[1] );
	    return;
	}
	else {
	    list = &(*list)->next;
	}
    }

    {
	CoupleList *new_entry;
	new_entry = g_malloc(sizeof(CoupleList));
	new_entry->object = object;
	new_entry->data[this] = data;
	new_entry->data[1 - this] = NULL;

	new_entry->next = NULL;
	*list = new_entry;
    }
}

void couple_unregister(Window *window, CoupleType couple_type, int this, gpointer data) {
    CoupleList **list;

    list = &window->couples[couple_type];
    while (*list != NULL) {
	if ( (*list)->data[this] == data ) {
	    if ( (*list)->data[1 - this] == NULL) {
		// Couple node is empty - delete it
		CoupleList *rm;

		rm = *list;
		*list = (*list)->next;
		g_free(rm);
	    }
	    else {
		// Unbind the couple
		couple_methods[couple_type].unbind( (*list)->data[0], (*list)->data[1] );
		(*list)->data[this] = NULL;
		list = &(*list)->next;
	    }
	}
	else {
	    list = &(*list)->next;
	}
    }
}
