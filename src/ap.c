/*
 *  Copyright 2020-2023 Jesse Lentz and contributors
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

static const CallbackMessages ap_start_messages = {
    N_("AP has been started"),
    N_("Failed to start AP"),
    NULL,
    FALSE
};

static const CallbackMessages ap_stop_messages = {
    N_("AP has been stopped"),
    N_("Failed to stop AP"),
    NULL,
    FALSE
};

void ap_dialog_launch(AP *ap) {
    APDialog *dialog;
    GtkWidget *table, *buttons;

    dialog = g_malloc(sizeof(APDialog));
    dialog->ap = ap;

    dialog->window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog->window), _("Start wireless access point"));

    dialog->ssid = gtk_entry_new();
    dialog->psk = gtk_password_entry_new();
    gtk_password_entry_set_show_peek_icon(GTK_PASSWORD_ENTRY(dialog->psk), TRUE);

    buttons = dialog_buttons(dialog, (SubmitCallback) ap_dialog_submit, dialog->window);

    table = gtk_grid_new();
    gtk_window_set_child(GTK_WINDOW(dialog->window), table);

    gtk_grid_attach(GTK_GRID(table), gtk_label_new(_("SSID: ")),     0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(table), dialog->ssid,                   1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(table), gtk_label_new(_("Password: ")), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(table), dialog->psk,                    1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(table), buttons,                        1, 2, 1, 1);

    grid_column_set_alignment(table, 0, GTK_ALIGN_END);
    grid_column_set_alignment(table, 1, GTK_ALIGN_START);

    g_signal_connect_swapped(dialog->window, "destroy", G_CALLBACK(g_free), dialog);
    gtk_widget_set_visible(dialog->window, true);
}

void ap_dialog_submit(APDialog *dialog) {
    const gchar *ssid, *psk;

    ssid = gtk_editable_get_text(GTK_EDITABLE(dialog->ssid));
    psk = gtk_editable_get_text(GTK_EDITABLE(dialog->psk));

    if (*ssid == '\0' || *psk == '\0') {
	return;
    }

    g_dbus_proxy_call(
	dialog->ap->proxy,
	"Start",
	g_variant_new("(ss)", ssid, psk),
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) method_call_notify,
	(gpointer) &ap_start_messages);

    gtk_window_destroy(GTK_WINDOW(dialog->window));
}

void ap_button_clicked(AP *ap) {
    GVariant *started_var;
    gboolean started;

    started_var = g_dbus_proxy_get_cached_property(ap->proxy, "Started");
    started = g_variant_get_boolean(started_var);

    if (started) {
	g_dbus_proxy_call(
	    ap->proxy,
	    "Stop",
	    NULL,
	    G_DBUS_CALL_FLAGS_NONE,
	    -1,
	    NULL,
	    (GAsyncReadyCallback) method_call_notify,
	    (gpointer) &ap_stop_messages);
    }
    else {
	ap_dialog_launch(ap);
    }

    g_variant_unref(started_var);
}

void ap_set(AP *ap) {
    {
	GVariant *started_var;
	gboolean started;

	started_var = g_dbus_proxy_get_cached_property(ap->proxy, "Started");
	started = g_variant_get_boolean(started_var);

	if (started) {
	    gtk_button_set_label(GTK_BUTTON(ap->button), _("Stop AP"));
	}
	else {
	    gtk_button_set_label(GTK_BUTTON(ap->button), _("Start AP"));
	}

	g_variant_unref(started_var);
    }

    {
	GVariant *name_var;
	name_var = g_dbus_proxy_get_cached_property(ap->proxy, "Name");

	if (name_var) {
	    gchar *ssid_label;

	    ssid_label = g_strdup_printf(_("SSID: %s"), g_variant_get_string(name_var, NULL));
	    g_variant_unref(name_var);

	    gtk_label_set_text(GTK_LABEL(ap->ssid), ssid_label);
	    g_free(ssid_label);

	    gtk_widget_set_visible(ap->ssid, true);
	}
	else {
	    gtk_widget_set_visible(ap->ssid, false);
	}
    }
}

AP* ap_add(Window *window, GDBusObject *object, GDBusProxy *proxy) {
    AP *ap;

    ap = g_malloc(sizeof(AP));
    ap->proxy = proxy;

    ap->button = gtk_button_new_with_label(NULL);
    g_object_ref_sink(ap->button);
    g_signal_connect_swapped(ap->button, "clicked", G_CALLBACK(ap_button_clicked), ap);

    ap->ssid = gtk_label_new(NULL);
    g_object_ref_sink(ap->ssid);
    gtk_widget_set_visible(ap->ssid, false);
    gtk_widget_set_halign(ap->ssid, GTK_ALIGN_START);

    couple_register(window, DEVICE_AP, 1, ap, object);

    ap->handler_update = g_signal_connect_swapped(proxy, "g-properties-changed", G_CALLBACK(ap_set), ap);

    return ap;
}

void ap_remove(Window *window, AP *ap) {
    couple_unregister(window, DEVICE_AP, 1, ap);
    g_object_unref(ap->button);
    g_object_unref(ap->ssid);

    g_signal_handler_disconnect(ap->proxy, ap->handler_update);
    g_free(ap);
}

void bind_device_ap(Device *device, AP *ap) {
    ap->device = device;
    gtk_grid_attach(GTK_GRID(device->table), ap->button, 3, 0, 1, 1);
    gtk_box_append(GTK_BOX(device->master), ap->ssid);
    ap_set(ap);
}

void unbind_device_ap(Device *device, AP *ap) {
    ap->device = NULL;
    gtk_grid_remove(GTK_GRID(device->table), ap->button);
    gtk_box_remove(GTK_BOX(device->master), ap->ssid);
}
