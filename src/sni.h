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

    // Methods to be exported via DBus
    SNIActivateHandler context_menu_handler;
    SNIActivateHandler activate_handler;
    SNIActivateHandler secondary_activate_handler;
    SNIScrollHandler scroll_handler;

    // Properties to be exported via DBus
    GVariant *category;
    GVariant *id;
    GVariant *title;
    GVariant *status;
    guint32 window_id;
    GVariant *icon_name;
    GVariant *icon_pixmap;
    GVariant *overlay_icon_name;
    GVariant *overlay_icon_pixmap;
    GVariant *attention_icon_name;
    GVariant *attention_icon_pixmap;
    GVariant *attention_movie_name;
    GVariant *tooltip;
    gboolean item_is_menu;
    GVariant *menu;
};

StatusNotifierItem* sni_new(gpointer user_data);
void sni_rm(StatusNotifierItem *sni);
void sni_bus_acquired(GDBusConnection *connection, const gchar *name, StatusNotifierItem *sni);
void sni_bus_name_acquired(GDBusConnection *connection, const gchar *name, StatusNotifierItem *sni);
void sni_bus_name_lost(GDBusConnection *connection, const gchar *name, StatusNotifierItem *sni);
void sni_watcher_up(GDBusConnection *connection, const gchar *name, const gchar *name_owner, StatusNotifierItem *sni);
void validate_method_call(GDBusConnection *connection, GAsyncResult *res, const gchar *message);
void sni_method_call(GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *method_name, GVariant *parameters, GDBusMethodInvocation *invocation, StatusNotifierItem *sni);
GVariant* sni_get_property(GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *property_name, GError **error, StatusNotifierItem *sni);
void sni_emit_signal(StatusNotifierItem *sni, const gchar *signal_name, GVariant *parameters);

gboolean sni_abstract_icon_pixmap_set(GVariant **sni_icon_pixmap, cairo_surface_t *surface);
void sni_category_set(StatusNotifierItem *sni, const gchar *category);
void sni_id_set(StatusNotifierItem *sni, const gchar *id);
void sni_title_set(StatusNotifierItem *sni, const gchar *title);
void sni_status_set(StatusNotifierItem *sni, const gchar *status);
void sni_icon_name_set(StatusNotifierItem *sni, const gchar *icon_name);
void sni_icon_pixmap_set(StatusNotifierItem *sni, cairo_surface_t *surface);
void sni_overlay_icon_name_set(StatusNotifierItem *sni, const gchar *overlay_icon_name);
void sni_overlay_icon_pixmap_set(StatusNotifierItem *sni, cairo_surface_t *surface);
void sni_attention_icon_name_set(StatusNotifierItem *sni, const gchar *attention_icon_name);
void sni_attention_icon_pixmap_set(StatusNotifierItem *sni, cairo_surface_t *surface);
void sni_attention_movie_name_set(StatusNotifierItem *sni, const gchar *attention_movie_name);

#endif
