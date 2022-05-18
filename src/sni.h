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

#ifndef _IWGTK_SNI_H
#define _IWGTK_SNI_H

#define STATUS_NOTIFIER_ITEM_OBJECT_PATH     "/StatusNotifierItem"
#define STATUS_NOTIFIER_ITEM_INTERFACE       "org.kde.StatusNotifierItem"
#define STATUS_NOTIFIER_ITEM_BUS_NAME_PREFIX "org.kde.StatusNotifierItem"

#define STATUS_NOTIFIER_WATCHER_OBJECT_PATH  "/StatusNotifierWatcher"
#define STATUS_NOTIFIER_WATCHER_INTERFACE    "org.kde.StatusNotifierWatcher"
#define STATUS_NOTIFIER_WATCHER_BUS_NAME     "org.kde.StatusNotifierWatcher"

typedef void (*SNIActivateHandler) (gpointer user_data);
typedef void (*SNIScrollHandler) (int delta, const gchar *orientation, gpointer user_data);

typedef struct StatusNotifierItem_s StatusNotifierItem;

struct StatusNotifierItem_s {
    GDBusConnection *connection;
    gpointer user_data;
    guint owner_id;
    guint registration_id;
    gchar *bus_name;

    SNIActivateHandler context_menu;
    SNIActivateHandler activate;
    SNIActivateHandler secondary_activate;
    SNIScrollHandler scroll;

    const gchar *category;
    const gchar *id;
    const gchar *title;
    const gchar *status;
    guint32 window_id;
    const gchar *icon_name;
    const gchar *overlay_icon_name;
    const gchar *attention_icon_name;
    const gchar *attention_movie_name;
    gboolean item_is_menu;
};

StatusNotifierItem* sni_new(gpointer user_data);
void sni_rm(StatusNotifierItem *sni);
void sni_connection_closed_callback(GDBusConnection *connection, GAsyncResult *res, StatusNotifierItem *sni);
void sni_connection_acquired(GDBusConnection *connection, GAsyncResult *res, StatusNotifierItem *sni);
void sni_bus_name_acquired(GDBusConnection *connection, const gchar *name, StatusNotifierItem *sni);
void sni_watcher_up(GDBusConnection *connection, const gchar *name, const gchar *name_owner, StatusNotifierItem *sni);
void validate_method_call(GDBusConnection *connection, GAsyncResult *res, const gchar *message);
void sni_bus_name_lost(GDBusConnection *connection, const gchar *name, StatusNotifierItem *sni);
void sni_icon_set(StatusNotifierItem *sni, const gchar *icon_name);
void sni_title_set(StatusNotifierItem *sni, const gchar *title);
void sni_method_call(GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *method_name, GVariant *parameters, GDBusMethodInvocation *invocation, StatusNotifierItem *sni);
GVariant* sni_get_property(GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *property_name, GError **error, StatusNotifierItem *sni);
void sni_emit_signal(StatusNotifierItem *sni, const gchar *signal_name);

#endif
