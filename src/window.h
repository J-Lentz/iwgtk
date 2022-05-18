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

#ifndef _WINDOW_H
#define _WINDOW_H

#define n_object_types 9
typedef enum {
    OBJECT_KNOWN_NETWORK,
    OBJECT_ADAPTER,
    OBJECT_DEVICE,
    OBJECT_STATION,
    OBJECT_ACCESS_POINT,
    OBJECT_ADHOC,
    OBJECT_WPS,
    OBJECT_DIAGNOSTIC,
    OBJECT_NETWORK
} ObjectType;

#define n_couple_types 6
typedef enum {
    ADAPTER_DEVICE,
    DEVICE_STATION,
    DEVICE_AP,
    DEVICE_ADHOC,
    DEVICE_WPS,
    DEVICE_DIAGNOSTIC
} CoupleType;

typedef struct ObjectList {
    GDBusObject *object;
    gpointer data;
    struct ObjectList *next;
} ObjectList;

typedef struct CoupleList {
    GDBusObject *object;
    gpointer data[2];
    struct CoupleList *next;
} CoupleList;

typedef struct {
    GtkWidget *window;

    GtkWidget *master;
    GtkWidget *header;
    GtkWidget *main;

    GtkWidget *known_network_button;
    GtkWidget *known_network_table;

    ObjectList *objects[n_object_types];
    CoupleList *couples[n_couple_types];
} Window;

typedef gpointer (*ConstructorFunction) (Window *window, GDBusObject *object, GDBusProxy *proxy);
typedef void (*DestructorFunction) (Window *window, gpointer data);

typedef struct {
    const gchar *interface;
    ConstructorFunction new;
    DestructorFunction rm;
    IndicatorSetter indicator_set;
} ObjectMethods;

typedef void (*BindFunction) (gpointer A, gpointer B);
typedef void (*UnbindFunction) (gpointer A, gpointer B);

typedef struct {
    BindFunction bind;
    UnbindFunction unbind;
} CoupleMethods;

extern ObjectMethods object_methods[];
extern CoupleMethods couple_methods[];

typedef void (*ObjectIterFunction) (GDBusObjectManager *manager, GDBusObject *object, GDBusProxy *proxy, Window *window);

void window_new();
void window_set();
void window_rm(Window *window);
void known_network_table_show(GtkToggleButton *button, Window *window);

void add_all_dbus_objects(Window *window);
void object_add(GDBusObjectManager *manager, GDBusObject *object, Window *window);
void object_rm(GDBusObjectManager *manager, GDBusObject *object, Window *window);
void object_iterate_interfaces(GDBusObjectManager *manager, GDBusObject *object, Window *window, ObjectIterFunction method);
void interface_add(GDBusObjectManager *manager, GDBusObject *object, GDBusProxy *proxy, Window *window);
void object_list_append(ObjectList **list, GDBusObject *object, gpointer data);
void window_add_object(GDBusObject *object, GDBusProxy *proxy, Window *window, int type);
void interface_rm(GDBusObjectManager *manager, GDBusObject *object, GDBusProxy *proxy, Window *window);

void couple_register(Window *window, CoupleType couple_type, int this, gpointer data, GDBusObject *object);
void couple_unregister(Window *window, CoupleType couple_type, int this, gpointer data);

#endif
