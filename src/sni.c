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
	    "IconPixmap",
	    "a(iiay)",
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
	    "OverlayIconPixmap",
	    "a(iiay)",
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
	    "AttentionIconPixmap",
	    "a(iiay)",
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
	    "ToolTip",
	    "(sa(iiay)ss)",
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
	&(GDBusPropertyInfo) {
	    -1,
	    "Menu",
	    "o",
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

    sni = g_malloc0(sizeof(StatusNotifierItem));
    sni->user_data = user_data;

    {
	static int sni_id = 0;

	sni->bus_name = g_strdup_printf("%s-%d-%d", STATUS_NOTIFIER_ITEM_BUS_NAME_PREFIX, getpid(), ++ sni_id);
	sni->owner_id = g_bus_own_name(
	    G_BUS_TYPE_SESSION,
	    sni->bus_name,
	    G_BUS_NAME_OWNER_FLAGS_DO_NOT_QUEUE,
	    (GBusAcquiredCallback) sni_bus_acquired,
	    (GBusNameAcquiredCallback) sni_bus_name_acquired,
	    (GBusNameLostCallback) sni_bus_name_lost,
	    sni,
	    NULL);
    }

    return sni;
}

void sni_rm(StatusNotifierItem *sni) {
    g_bus_unown_name(sni->owner_id);
    g_dbus_connection_unregister_object(sni->connection, sni->registration_id);

    if (sni->category != NULL) {
	g_variant_unref(sni->category);
    }
    if (sni->id != NULL) {
	g_variant_unref(sni->id);
    }
    if (sni->title != NULL) {
	g_variant_unref(sni->title);
    }
    if (sni->status != NULL) {
	g_variant_unref(sni->status);
    }
    if (sni->icon_name != NULL) {
	g_variant_unref(sni->icon_name);
    }
    if (sni->icon_pixmap != NULL) {
	g_variant_unref(sni->icon_pixmap);
    }
    if (sni->overlay_icon_name != NULL) {
	g_variant_unref(sni->overlay_icon_name);
    }
    if (sni->overlay_icon_pixmap != NULL) {
	g_variant_unref(sni->overlay_icon_pixmap);
    }
    if (sni->attention_icon_name != NULL) {
	g_variant_unref(sni->attention_icon_name);
    }
    if (sni->attention_icon_pixmap != NULL) {
	g_variant_unref(sni->attention_icon_pixmap);
    }
    if (sni->attention_movie_name != NULL) {
	g_variant_unref(sni->attention_movie_name);
    }
    if (sni->tooltip != NULL) {
	g_variant_unref(sni->tooltip);
    }
    if (sni->menu != NULL) {
	g_variant_unref(sni->menu);
    }

    g_free(sni->bus_name);
    g_free(sni);
}

void sni_bus_acquired(GDBusConnection *connection, const gchar *name, StatusNotifierItem *sni) {
    GError *err;

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

void sni_bus_name_lost(GDBusConnection *connection, const gchar *name, StatusNotifierItem *sni) {
    fprintf(stderr, "Bus name '%s' lost\n", name);
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

void sni_method_call(GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *method_name, GVariant *parameters, GDBusMethodInvocation *invocation, StatusNotifierItem *sni) {
    if (strcmp(method_name, "ContextMenu") == 0) {
	if (sni->context_menu_handler != NULL) {
	    sni->context_menu_handler(sni->user_data);
	}
	g_dbus_method_invocation_return_value(invocation, NULL);
    }
    else if (strcmp(method_name, "Activate") == 0) {
	if (sni->activate_handler != NULL) {
	    sni->activate_handler(sni->user_data);
	}
	g_dbus_method_invocation_return_value(invocation, NULL);
    }
    else if (strcmp(method_name, "SecondaryActivate") == 0) {
	if (sni->secondary_activate_handler != NULL) {
	    sni->secondary_activate_handler(sni->user_data);
	}
	g_dbus_method_invocation_return_value(invocation, NULL);
    }
    else if (strcmp(method_name, "Scroll") == 0) {
	if (sni->scroll_handler != NULL) {
	    int delta;
	    const gchar *orientation;

	    g_variant_get(parameters, "(is)", &delta, &orientation);
	    sni->scroll_handler(delta, orientation, sni->user_data);
	}
	g_dbus_method_invocation_return_value(invocation, NULL);
    }
    else {
	g_dbus_method_invocation_return_dbus_error(invocation, "org.freedesktop.DBus.Error.UnknownMethod", "Unknown method");
    }
}

GVariant* sni_get_property(GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *property_name, GError **error, StatusNotifierItem *sni) {
    if (strcmp(property_name, "Category") == 0) {
	if (sni->category) {
	    return g_variant_ref(sni->category);
	}
    }
    else if (strcmp(property_name, "Id") == 0) {
	if (sni->id) {
	    return g_variant_ref(sni->id);
	}
    }
    else if (strcmp(property_name, "Title") == 0) {
	if (sni->title) {
	    return g_variant_ref(sni->title);
	}
    }
    else if (strcmp(property_name, "Status") == 0) {
	if (sni->status) {
	    return g_variant_ref(sni->status);
	}
    }
    else if (strcmp(property_name, "WindowId") == 0) {
	return g_variant_new_uint32(sni->window_id);
    }
    else if (strcmp(property_name, "IconName") == 0) {
	if (sni->icon_name) {
	    return g_variant_ref(sni->icon_name);
	}
    }
    else if (strcmp(property_name, "IconPixmap") == 0) {
	if (sni->icon_pixmap) {
	    return g_variant_ref(sni->icon_pixmap);
	}
    }
    else if (strcmp(property_name, "OverlayIconName") == 0) {
	if (sni->overlay_icon_name) {
	    return g_variant_ref(sni->overlay_icon_name);
	}
    }
    else if (strcmp(property_name, "OverlayIconPixmap") == 0) {
	if (sni->overlay_icon_pixmap) {
	    return g_variant_ref(sni->overlay_icon_pixmap);
	}
    }
    else if (strcmp(property_name, "AttentionIconName") == 0) {
	if (sni->attention_icon_name) {
	    return g_variant_ref(sni->attention_icon_name);
	}
    }
    else if (strcmp(property_name, "AttentionIconPixmap") == 0) {
	if (sni->attention_icon_pixmap) {
	    return g_variant_ref(sni->attention_icon_pixmap);
	}
    }
    else if (strcmp(property_name, "AttentionMovieName") == 0) {
	if (sni->attention_movie_name) {
	    return g_variant_ref(sni->attention_movie_name);
	}
    }
    else if (strcmp(property_name, "ToolTip") == 0) {
	if (sni->tooltip) {
	    return g_variant_ref(sni->tooltip);
	}
    }
    else if (strcmp(property_name, "ItemIsMenu") == 0) {
	return g_variant_new_boolean(sni->item_is_menu);
    }
    else if (strcmp(property_name, "Menu") == 0) {
	if (sni->menu) {
	    return g_variant_ref(sni->menu);
	}
    }

    g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_PROPERTY, "Property '%s' is not implemented", property_name);
    return NULL;
}

void sni_emit_signal(StatusNotifierItem *sni, const gchar *signal_name, GVariant *parameters) {
    if (sni->connection != NULL) {
	GError *err;

	err = NULL;
	g_dbus_connection_emit_signal(
	    sni->connection,
	    NULL,
	    STATUS_NOTIFIER_ITEM_OBJECT_PATH,
	    STATUS_NOTIFIER_ITEM_INTERFACE,
	    signal_name,
	    parameters,
	    &err);

	if (err != NULL) {
	    fprintf(stderr, "Error emitting %s signal: %s\n", signal_name, err->message);
	    g_error_free(err);
	}
    }
}

gboolean sni_abstract_icon_pixmap_set(GVariant **sni_icon_pixmap, const GdkPixbuf *pixbuf) {
    GVariant *tuple[3];

    // Verify that the GdkPixbuf meets our assumptions about its format
    if (gdk_pixbuf_get_bits_per_sample(pixbuf) != 8 || gdk_pixbuf_get_n_channels(pixbuf) != 4) {
	fprintf(stderr, "Could not update indicator icon: Invalid GdkPixbuf format\n");
	return FALSE;
    }

    {
	const guint8 *img_rgba;
	guint8 *img_argb;
	gint32 img_width, img_height;
	gsize img_nbytes;

	img_rgba = gdk_pixbuf_read_pixels(pixbuf);

	img_width = gdk_pixbuf_get_width(pixbuf);
	img_height = gdk_pixbuf_get_height(pixbuf);
	img_nbytes = img_width*img_height*4;

	img_argb = g_malloc(img_nbytes);
	for (int i = 0; i < img_nbytes; i += 4) {
	    img_argb[i+0] = img_rgba[i+3];
	    img_argb[i+1] = img_rgba[i+0];
	    img_argb[i+2] = img_rgba[i+1];
	    img_argb[i+3] = img_rgba[i+2];
	}

	tuple[0] = g_variant_new_int32(img_width);
	tuple[1] = g_variant_new_int32(img_height);
	tuple[2] = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, img_argb, img_nbytes, 1);

	g_free(img_argb);
    }

    {
	GVariant *tuple_var;

	tuple_var = g_variant_new_tuple(tuple, 3);
	if (*sni_icon_pixmap != NULL) {
	    g_variant_unref(*sni_icon_pixmap);
	}
	*sni_icon_pixmap = g_variant_new_array(G_VARIANT_TYPE("(iiay)"), &tuple_var, 1);
	g_variant_ref_sink(*sni_icon_pixmap);
    }

    return TRUE;
}

void sni_category_set(StatusNotifierItem *sni, const gchar *category) {
    if (sni->category != NULL) {
	g_variant_unref(sni->category);
    }
    sni->category = g_variant_new_string(category);
    g_variant_ref_sink(sni->category);
}

void sni_id_set(StatusNotifierItem *sni, const gchar *id) {
    if (sni->id != NULL) {
	g_variant_unref(sni->id);
    }
    sni->id = g_variant_new_string(id);
    g_variant_ref_sink(sni->id);
}

void sni_title_set(StatusNotifierItem *sni, const gchar *title) {
    if (sni->title != NULL) {
	g_variant_unref(sni->title);
    }
    sni->title = g_variant_new_string(title);
    g_variant_ref_sink(sni->title);

    sni_emit_signal(sni, "NewTitle", NULL);
}

void sni_status_set(StatusNotifierItem *sni, const gchar *status) {
    if (sni->status != NULL) {
	g_variant_unref(sni->status);
    }
    sni->status = g_variant_new_string(status);
    g_variant_ref_sink(sni->status);

    sni_emit_signal(sni, "NewStatus", g_variant_new("(s)", status));
}

void sni_icon_name_set(StatusNotifierItem *sni, const gchar *icon_name) {
    if (sni->icon_name != NULL) {
	g_variant_unref(sni->icon_name);
    }
    sni->icon_name = g_variant_new_string(icon_name);
    g_variant_ref_sink(sni->icon_name);

    sni_emit_signal(sni, "NewIcon", NULL);
}

void sni_icon_pixmap_set(StatusNotifierItem *sni, const GdkPixbuf *pixbuf) {
    if (sni_abstract_icon_pixmap_set(&sni->icon_pixmap, pixbuf)) {
	sni_emit_signal(sni, "NewIcon", NULL);
    }
}

void sni_overlay_icon_name_set(StatusNotifierItem *sni, const gchar *overlay_icon_name) {
    if (sni->overlay_icon_name != NULL) {
	g_variant_unref(sni->overlay_icon_name);
    }
    sni->overlay_icon_name = g_variant_new_string(overlay_icon_name);
    g_variant_ref_sink(sni->overlay_icon_name);

    sni_emit_signal(sni, "NewOverlayIcon", NULL);
}

void sni_overlay_icon_pixmap_set(StatusNotifierItem *sni, const GdkPixbuf *pixbuf) {
    if (sni_abstract_icon_pixmap_set(&sni->overlay_icon_pixmap, pixbuf)) {
	sni_emit_signal(sni, "NewOverlayIcon", NULL);
    }
}

void sni_attention_icon_name_set(StatusNotifierItem *sni, const gchar *attention_icon_name) {
    if (sni->attention_icon_name != NULL) {
	g_variant_unref(sni->attention_icon_name);
    }
    sni->attention_icon_name = g_variant_new_string(attention_icon_name);
    g_variant_ref_sink(sni->attention_icon_name);

    sni_emit_signal(sni, "NewAttentionIcon", NULL);
}

void sni_attention_icon_pixmap_set(StatusNotifierItem *sni, const GdkPixbuf *pixbuf) {
    if (sni_abstract_icon_pixmap_set(&sni->attention_icon_pixmap, pixbuf)) {
	sni_emit_signal(sni, "NewAttentionIcon", NULL);
    }
}

void sni_attention_movie_name_set(StatusNotifierItem *sni, const gchar *attention_movie_name) {
    if (sni->attention_movie_name != NULL) {
	g_variant_unref(sni->attention_movie_name);
    }
    sni->attention_movie_name = g_variant_new_string(attention_movie_name);
    g_variant_ref_sink(sni->attention_movie_name);
}
