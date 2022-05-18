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

#include "iwgtk.h"
#include <unistd.h>

GDBusArgInfo arg_x           = {-1, "x",           "i", NULL};
GDBusArgInfo arg_y           = {-1, "y",           "i", NULL};
GDBusArgInfo arg_delta       = {-1, "delta",       "i", NULL};
GDBusArgInfo arg_orientation = {-1, "orientation", "s", NULL};
GDBusArgInfo arg_status      = {-1, "status",      "s", NULL};

GDBusInterfaceInfo sni_interface_info = {
    -1,
    STATUS_NOTIFIER_ITEM_INTERFACE,
    (GDBusMethodInfo *[]) {
	&(GDBusMethodInfo) {
	    -1,
	    "ContextMenu",
	    (GDBusArgInfo *[]) {&arg_x, &arg_y, NULL},
	    NULL,
	    NULL
	},
	&(GDBusMethodInfo) {
	    -1,
	    "Activate",
	    (GDBusArgInfo *[]) {&arg_x, &arg_y, NULL},
	    NULL,
	    NULL
	},
	&(GDBusMethodInfo) {
	    -1,
	    "SecondaryActivate",
	    (GDBusArgInfo *[]) {&arg_x, &arg_y, NULL},
	    NULL,
	    NULL
	},
	&(GDBusMethodInfo) {
	    -1,
	    "Scroll",
	    (GDBusArgInfo *[]) {&arg_delta, &arg_orientation, NULL},
	    NULL,
	    NULL
	},
	NULL
    },
    (GDBusSignalInfo *[]) {
	&(GDBusSignalInfo) {
	    -1,
	    "NewTitle",
	    NULL,
	    NULL
	},
	&(GDBusSignalInfo) {
	    -1,
	    "NewIcon",
	    NULL,
	    NULL
	},
	&(GDBusSignalInfo) {
	    -1,
	    "NewAttentionIcon",
	    NULL,
	    NULL
	},
	&(GDBusSignalInfo) {
	    -1,
	    "NewOverlayIcon",
	    NULL,
	    NULL
	},
	&(GDBusSignalInfo) {
	    -1,
	    "NewToolTip",
	    NULL,
	    NULL
	},
	&(GDBusSignalInfo) {
	    -1,
	    "NewStatus",
	    (GDBusArgInfo *[]) {&arg_status, NULL},
	    NULL
	},
	NULL
    },
    (GDBusPropertyInfo *[]) {
	&(GDBusPropertyInfo) {
	    -1,
	    "Category",
	    "s",
	    G_DBUS_PROPERTY_INFO_FLAGS_READABLE,
	    NULL
	},
	&(GDBusPropertyInfo) {
	    -1,
	    "Id",
	    "s",
	    G_DBUS_PROPERTY_INFO_FLAGS_READABLE,
	    NULL
	},
	&(GDBusPropertyInfo) {
	    -1,
	    "Title",
	    "s",
	    G_DBUS_PROPERTY_INFO_FLAGS_READABLE,
	    NULL
	},
	&(GDBusPropertyInfo) {
	    -1,
	    "Status",
	    "s",
	    G_DBUS_PROPERTY_INFO_FLAGS_READABLE,
	    NULL
	},
	&(GDBusPropertyInfo) {
	    -1,
	    "WindowId",
	    "u",
	    G_DBUS_PROPERTY_INFO_FLAGS_READABLE,
	    NULL
	},
	&(GDBusPropertyInfo) {
	    -1,
	    "IconName",
	    "s",
	    G_DBUS_PROPERTY_INFO_FLAGS_READABLE,
	    NULL
	},
	&(GDBusPropertyInfo) {
	    -1,
	    "OverlayIconName",
	    "s",
	    G_DBUS_PROPERTY_INFO_FLAGS_READABLE,
	    NULL
	},
	&(GDBusPropertyInfo) {
	    -1,
	    "AttentionIconName",
	    "s",
	    G_DBUS_PROPERTY_INFO_FLAGS_READABLE,
	    NULL
	},
	&(GDBusPropertyInfo) {
	    -1,
	    "AttentionMovieName",
	    "s",
	    G_DBUS_PROPERTY_INFO_FLAGS_READABLE,
	    NULL
	},
	&(GDBusPropertyInfo) {
	    -1,
	    "ItemIsMenu",
	    "b",
	    G_DBUS_PROPERTY_INFO_FLAGS_READABLE,
	    NULL
	},
	NULL
    },
    NULL  // Annotation info
};

GDBusInterfaceVTable sni_interface_vtable = {
    (GDBusInterfaceMethodCallFunc) sni_method_call,
    (GDBusInterfaceGetPropertyFunc) sni_get_property,
    NULL
};

StatusNotifierItem* sni_new(gpointer user_data) {
    StatusNotifierItem *sni;

    sni = malloc(sizeof(StatusNotifierItem));

    sni->connection = NULL;
    sni->user_data = user_data;

    sni->context_menu = NULL;
    sni->activate = NULL;
    sni->secondary_activate = NULL;
    sni->scroll = NULL;

    sni->category = "Hardware";
    sni->id = "iwgtk";
    sni->title = "Wifi management utility";
    sni->status = "Active";
    sni->window_id = 0;
    sni->icon_name = "";
    sni->overlay_icon_name = "";
    sni->attention_icon_name = "";
    sni->attention_movie_name = "";
    sni->item_is_menu = FALSE;

    g_dbus_connection_new_for_address(
	global.session_bus_address,
	G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT | G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION,
	NULL,
	NULL,
	(GAsyncReadyCallback) sni_connection_acquired,
	sni);

    return sni;
}

void sni_rm(StatusNotifierItem *sni) {
    g_bus_unown_name(sni->owner_id);
    g_dbus_connection_unregister_object(sni->connection, sni->registration_id);
    g_dbus_connection_close(
	sni->connection,
	NULL,
	(GAsyncReadyCallback) sni_connection_closed_callback,
	sni);
}

void sni_connection_closed_callback(GDBusConnection *connection, GAsyncResult *res, StatusNotifierItem *sni) {
    GError *err;

    err = NULL;
    g_dbus_connection_close_finish(connection, res, &err);

    if (err != NULL) {
	fprintf(stderr, "Error closing D-Bus connection: %s\n", err->message);
	g_error_free(err);
    }

    g_free(sni->bus_name);
    free(sni);
}

void sni_connection_acquired(GDBusConnection *connection, GAsyncResult *res, StatusNotifierItem *sni) {
    GError *err;

    err = NULL;
    g_dbus_connection_new_for_address_finish(res, &err);

    if (err != NULL) {
	fprintf(stderr, "Error connecting to D-Bus session bus: %s\n", err->message);
	g_error_free(err);
	return;
    }

    sni->connection = connection;

    err = NULL;
    sni->registration_id = g_dbus_connection_register_object(
	    connection,
	    STATUS_NOTIFIER_ITEM_OBJECT_PATH,
	    &sni_interface_info,
	    &sni_interface_vtable,
	    sni,
	    NULL,
	    &err);

    if (err != NULL) {
	fprintf(stderr, "Error registering StatusNotifierItem object: %s\n", err->message);
	g_error_free(err);
    }

    {
	static int sni_id = 0;

	sni->bus_name = g_strdup_printf("%s-%d-%d", STATUS_NOTIFIER_ITEM_BUS_NAME_PREFIX, getpid(), ++ sni_id);
	sni->owner_id = g_bus_own_name_on_connection(
	    connection,
	    sni->bus_name,
	    G_BUS_NAME_OWNER_FLAGS_DO_NOT_QUEUE,
	    (GBusNameAcquiredCallback) sni_bus_name_acquired,
	    (GBusNameLostCallback) sni_bus_name_lost,
	    sni,
	    NULL);
    }
}

void sni_bus_name_acquired(GDBusConnection *connection, const gchar *name, StatusNotifierItem *sni) {
    g_bus_watch_name_on_connection(
	connection,
	STATUS_NOTIFIER_WATCHER_BUS_NAME,
	G_BUS_NAME_WATCHER_FLAGS_NONE,
	(GBusNameAppearedCallback) sni_watcher_up,
	NULL,
	sni,
	NULL);
}

void sni_watcher_up(GDBusConnection *connection, const gchar *name, const gchar *name_owner, StatusNotifierItem *sni) {
    g_dbus_connection_call(
	connection,
	STATUS_NOTIFIER_WATCHER_BUS_NAME,
	STATUS_NOTIFIER_WATCHER_OBJECT_PATH,
	STATUS_NOTIFIER_WATCHER_INTERFACE,
	"RegisterStatusNotifierItem",
	g_variant_new("(s)", sni->bus_name),
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) validate_method_call,
	"Failed to register StatusNotifierItem: %s\n");
}

void validate_method_call(GDBusConnection *connection, GAsyncResult *res, const gchar *message) {
    GVariant *ret;
    GError *err;

    err = NULL;
    ret = g_dbus_connection_call_finish(connection, res, &err);

    if (ret) {
	g_variant_unref(ret);
    }
    else {
	fprintf(stderr, message, err->message);
	g_error_free(err);
    }
}

void sni_bus_name_lost(GDBusConnection *connection, const gchar *name, StatusNotifierItem *sni) {
    fprintf(stderr, "Error: Lost bus name %s\n", name);
}

void sni_icon_set(StatusNotifierItem *sni, const gchar *icon_name) {
    sni->icon_name = icon_name;
    sni_emit_signal(sni, "NewIcon");
}

void sni_title_set(StatusNotifierItem *sni, const gchar *title) {
    sni->title = title;
    sni_emit_signal(sni, "NewTitle");
}

void sni_method_call(GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *method_name, GVariant *parameters, GDBusMethodInvocation *invocation, StatusNotifierItem *sni) {
    if (strcmp(method_name, "ContextMenu") == 0) {
	if (sni->context_menu != NULL) {
	    sni->context_menu(sni->user_data);
	}
	g_dbus_method_invocation_return_value(invocation, NULL);
    }
    else if (strcmp(method_name, "Activate") == 0) {
	if (sni->activate != NULL) {
	    sni->activate(sni->user_data);
	}
	g_dbus_method_invocation_return_value(invocation, NULL);
    }
    else if (strcmp(method_name, "SecondaryActivate") == 0) {
	if (sni->secondary_activate != NULL) {
	    sni->secondary_activate(sni->user_data);
	}
	g_dbus_method_invocation_return_value(invocation, NULL);
    }
    else if (strcmp(method_name, "Scroll") == 0) {
	if (sni->scroll != NULL) {
	    int delta;
	    const gchar *orientation;

	    g_variant_get(parameters, "(is)", &delta, &orientation);
	    sni->scroll(delta, orientation, sni->user_data);
	}
	g_dbus_method_invocation_return_value(invocation, NULL);
    }
    else {
	g_dbus_method_invocation_return_dbus_error(invocation, "org.freedesktop.DBus.Error.UnknownMethod", "Unknown method");
    }
}

GVariant* sni_get_property(GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *property_name, GError **error, StatusNotifierItem *sni) {
    if (strcmp(property_name, "Category") == 0) {
	return g_variant_new_string(sni->category);
    }
    else if (strcmp(property_name, "Id") == 0) {
	return g_variant_new_string(sni->id);
    }
    else if (strcmp(property_name, "Title") == 0) {
	return g_variant_new_string(sni->title);
    }
    else if (strcmp(property_name, "Status") == 0) {
	return g_variant_new_string(sni->status);
    }
    else if (strcmp(property_name, "WindowId") == 0) {
	return g_variant_new_uint32(sni->window_id);
    }
    else if (strcmp(property_name, "IconName") == 0) {
	return g_variant_new_string(sni->icon_name);
    }
    else if (strcmp(property_name, "OverlayIconName") == 0) {
	return g_variant_new_string(sni->overlay_icon_name);
    }
    else if (strcmp(property_name, "AttentionIconName") == 0) {
	return g_variant_new_string(sni->attention_icon_name);
    }
    else if (strcmp(property_name, "AttentionMovieName") == 0) {
	return g_variant_new_string(sni->attention_movie_name);
    }
    else if (strcmp(property_name, "ItemIsMenu") == 0) {
	return g_variant_new_boolean(sni->item_is_menu);
    }
    else {
	return NULL;
    }
}

void sni_emit_signal(StatusNotifierItem *sni, const gchar *signal_name) {
    if (sni->connection != NULL) {
	GError *err;

	err = NULL;
	g_dbus_connection_emit_signal(
	    sni->connection,
	    NULL,
	    STATUS_NOTIFIER_ITEM_OBJECT_PATH,
	    STATUS_NOTIFIER_ITEM_INTERFACE,
	    signal_name,
	    NULL,
	    &err);

	if (err != NULL) {
	    fprintf(stderr, "Error emitting %s signal: %s\n", signal_name, err->message);
	    g_error_free(err);
	}
    }
}
