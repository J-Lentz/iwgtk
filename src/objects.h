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

#ifndef _IWGTK_OBJECTS_H
#define _IWGTK_OBJECTS_H

typedef gpointer (*ConstructorFunction) (GDBusObject *object, GDBusProxy *proxy);
typedef gpointer (*DestructorFunction) (gpointer data);

typedef void (*ObjectIterFunction) (GDBusObjectManager *manager, GDBusObject *object, GDBusProxy *proxy);

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

typedef struct ObjectList {
    GDBusObject *object;
    gpointer data;
    struct ObjectList *next;
} ObjectList;

typedef struct {
    const gchar *interface;
    ConstructorFunction constructor;
    DestructorFunction destructor;
    ObjectList *objects;
} ObjectType;

typedef void (*BindFunction) (gpointer A, gpointer B);
typedef void (*UnbindFunction) (gpointer A, gpointer B);

// The indices of this enum must correspond to the order of the rows in couple_table[].
typedef enum {
    ADAPTER_DEVICE,
    DEVICE_STATION,
    DEVICE_AP,
    DEVICE_ADHOC,
    DEVICE_WPS
} CoupleIndex;

typedef struct CoupleList {
    GDBusObject *object;
    gpointer data[2];
    struct CoupleList *next;
} CoupleList;

typedef struct {
    BindFunction bind;
    UnbindFunction unbind;
    CoupleList *couples;
} CoupleType;

extern ObjectType object_table[];
extern CoupleType couple_table[];

void object_add(GDBusObjectManager *manager, GDBusObject *object);
void object_remove(GDBusObjectManager *manager, GDBusObject *object);
void object_iterate_interfaces(GDBusObject *object, ObjectIterFunction method);

void interface_add(GDBusObjectManager *manager, GDBusObject *object, GDBusProxy *proxy);
void interface_remove(GDBusObjectManager *manager, GDBusObject *object, GDBusProxy *proxy);

void couple_register(CoupleIndex index, int this, gpointer data, GDBusObject *object);
void couple_unregister(CoupleIndex index, int this, gpointer data);

#endif
