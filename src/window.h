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

#ifndef _WINDOW_H
#define _WINDOW_H

#define n_object_types 8
typedef enum {
    OBJECT_KNOWN_NETWORK,
    OBJECT_ADAPTER,
    OBJECT_DEVICE,
    OBJECT_STATION,
    OBJECT_ACCESS_POINT,
    OBJECT_ADHOC,
    OBJECT_WPS,
    OBJECT_NETWORK
} ObjectIndex;

#define n_couple_types 5
typedef enum {
    ADAPTER_DEVICE,
    DEVICE_STATION,
    DEVICE_AP,
    DEVICE_ADHOC,
    DEVICE_WPS
} CoupleIndex;

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

typedef struct Window_s {
    GtkWidget *window;

    gulong handler_interface_add;
    gulong handler_interface_rm;
    gulong handler_object_add;
    gulong handler_object_rm;

    GtkWidget *master;
    GtkWidget *header;
    GtkWidget *main;

    GtkWidget *known_network_button;
    GtkWidget *known_network_table;

    ObjectList *objects[n_object_types];
    CoupleList *couples[n_couple_types];
    struct Window_s *next;
} Window;

typedef gpointer (*ConstructorFunction) (Window *window, GDBusObject *object, GDBusProxy *proxy);
typedef gpointer (*DestructorFunction) (Window *window, gpointer data);

typedef struct {
    const gchar *interface;
    ConstructorFunction constructor;
    DestructorFunction destructor;
} ObjectType;

typedef void (*BindFunction) (gpointer A, gpointer B);
typedef void (*UnbindFunction) (gpointer A, gpointer B);

typedef struct {
    BindFunction bind;
    UnbindFunction unbind;
} CoupleType;

extern ObjectType object_table[];
extern CoupleType couple_table[];

typedef void (*ObjectIterFunction) (Window *window, GDBusObject *object, GDBusProxy *proxy);

void window_new(GtkApplication *app);
void window_rm(Window *window);
void window_set(Window *win);
void known_network_table_show(GtkToggleButton *button, Window *win);

void object_add(Window *window, GDBusObject *object);
void object_rm(Window *window, GDBusObject *object);
void object_iterate_interfaces(Window *window, GDBusObject *object, ObjectIterFunction method);

void interface_add(Window *window, GDBusObject *object, GDBusProxy *proxy);
void interface_rm(Window *window, GDBusObject *object, GDBusProxy *proxy);

void couple_register(Window *window, CoupleIndex couple_type, int this, gpointer data, GDBusObject *object);
void couple_unregister(Window *window, CoupleIndex couple_type, int this, gpointer data);

#endif
