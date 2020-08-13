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

static const int n_object_types = 8;

ObjectType object_table[] = {
    {IWD_IFACE_KNOWN_NETWORK, (ConstructorFunction) known_network_add, (DestructorFunction) known_network_remove, NULL},
    {IWD_IFACE_ADAPTER,       (ConstructorFunction) adapter_add,       (DestructorFunction) adapter_remove,       NULL},
    {IWD_IFACE_DEVICE,        (ConstructorFunction) device_add,        (DestructorFunction) device_remove,        NULL},
    {IWD_IFACE_STATION,       (ConstructorFunction) station_add,       (DestructorFunction) station_remove,       NULL},
    {IWD_IFACE_AP,            (ConstructorFunction) ap_add,            (DestructorFunction) ap_remove,            NULL},
    {IWD_IFACE_AD_HOC,        (ConstructorFunction) adhoc_add,         (DestructorFunction) adhoc_remove,         NULL},
    {IWD_IFACE_WPS,           (ConstructorFunction) wps_add,           (DestructorFunction) wps_remove,           NULL},
    {IWD_IFACE_NETWORK,       (ConstructorFunction) network_add,       (DestructorFunction) network_remove,       NULL}
};

CoupleType couple_table[] = {
    {(BindFunction) bind_adapter_device, (UnbindFunction) unbind_adapter_device, NULL},
    {(BindFunction) bind_device_station, (UnbindFunction) unbind_device_station, NULL},
    {(BindFunction) bind_device_ap,      (UnbindFunction) unbind_device_ap,      NULL},
    {(BindFunction) bind_device_adhoc,   (UnbindFunction) unbind_device_adhoc,   NULL},
    {(BindFunction) bind_device_wps,     (UnbindFunction) unbind_device_wps,     NULL}
};

void object_add(GDBusObjectManager *manager, GDBusObject *object) {
    const gchar *name;
    name = g_dbus_object_get_object_path(object);
    
    object_iterate_interfaces(object, interface_add);
}

void object_remove(GDBusObjectManager *manager, GDBusObject *object) {
    const gchar *name;
    name = g_dbus_object_get_object_path(object);
    object_iterate_interfaces(object, interface_remove);
}

void object_iterate_interfaces(GDBusObject *object, ObjectIterFunction method) {
	GList *interface_list, *j;

	interface_list = g_dbus_object_get_interfaces(object);
	for (j = interface_list; j != NULL; j = j->next) {
	    GDBusProxy *proxy;

	    proxy = (GDBusProxy *) j->data;
	    method(global.manager, object, proxy);
	    g_object_unref(proxy);
	}
	g_list_free(interface_list);
}

void interface_add(GDBusObjectManager *manager, GDBusObject *object, GDBusProxy *proxy) {
    const gchar *name;

    name = g_dbus_proxy_get_interface_name(proxy);

    for (int i = 0; i < n_object_types; i ++) {
	if (strcmp(name, object_table[i].interface) == 0) {
	    ObjectList *object_list_entry;

	    object_list_entry = malloc(sizeof(ObjectList));
	    object_list_entry->object = object;
	    object_list_entry->data = object_table[i].constructor(object, proxy);
	    object_list_entry->next = object_table[i].objects;
	    object_table[i].objects = object_list_entry;

	    break;
	}
    }
}

void interface_remove(GDBusObjectManager *manager, GDBusObject *object, GDBusProxy *proxy) {
    const gchar *name;

    name = g_dbus_proxy_get_interface_name(proxy);

    for (int i = 0; i < n_object_types; i ++) {
	if (!strcmp(name, object_table[i].interface)) {
	    ObjectList **iter;

	    iter = &object_table[i].objects;
	    while ( (*iter)->object != object ) {
		iter = &(*iter)->next;
	    }

	    object_table[i].destructor( (*iter)->data );

	    {
		ObjectList *rm;
		rm = *iter;
		*iter = (*iter)->next;
		free(rm);
	    }

	    break;
	}
    }
}

void couple_register(CoupleIndex index, int this, gpointer data, GDBusObject *object) {
    CoupleList **iter;

    iter = &couple_table[index].couples;

    while (*iter != NULL) {
	if ( (*iter)->object == object) {
	    if ( (*iter)->data[this] == NULL) {
		(*iter)->data[this] = data;
	    }
	    else {
		CoupleList *new_entry;
		new_entry = malloc(sizeof(CoupleList));
		new_entry->object = object;
		new_entry->data[this] = data;
		new_entry->data[1 - this] = (*iter)->data[1 - this];

		new_entry->next = (*iter)->next;
		*iter = new_entry;
	    }
	    couple_table[index].bind( (*iter)->data[0], (*iter)->data[1] );
	    return;
	}
	else {
	    iter = &(*iter)->next;
	}
    }

    {
	CoupleList *new_entry;
	new_entry = malloc(sizeof(CoupleList));
	new_entry->object = object;
	new_entry->data[this] = data;
	new_entry->data[1 - this] = NULL;

	new_entry->next = NULL;
	*iter = new_entry;
    }
}

void couple_unregister(CoupleIndex index, int this, gpointer data) {
    CoupleList **iter;

    iter = &couple_table[index].couples;

    while (*iter != NULL) {
	if ( (*iter)->data[this] == data ) {
	    if ( (*iter)->data[1 - this] == NULL) {
		// Couple node is empty - delete it
		CoupleList *rm;
		rm = *iter;
		*iter = (*iter)->next;
		free(rm);
	    }
	    else {
		// Unbind the couple
		couple_table[index].unbind( (*iter)->data[0], (*iter)->data[1] );
		(*iter)->data[this] = NULL;
		iter = &(*iter)->next;
	    }
	}
	else {
	    iter = &(*iter)->next;
	}
    }
}
