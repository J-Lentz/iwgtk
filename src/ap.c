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

static const CallbackMessages ap_start_messages = {
    "AP started",
    "AP start-up failed",
    detailed_errors_standard
};

static const CallbackMessages ap_stop_messages = {
    "AP stopped",
    "Error stopping AP",
    detailed_errors_standard
};

void ap_dialog_launch(AP *ap) {
    APDialog *dialog;
    GtkWidget *table, *buttons;

    dialog = malloc(sizeof(APDialog));
    dialog->ap = ap;

    dialog->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(dialog->window), "Create Access Point");

    dialog->ssid = gtk_entry_new();
    dialog->psk = gtk_entry_new();

    buttons = dialog_buttons(dialog, G_CALLBACK(ap_dialog_submit), dialog->window);

    table = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(dialog->window), table);

    gtk_grid_attach(GTK_GRID(table), gtk_label_new("SSID: "),     0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(table), dialog->ssid,                1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(table), gtk_label_new("Password: "), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(table), dialog->psk,                 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(table), buttons,                     1, 2, 1, 1);

    grid_column_set_alignment(table, 0, GTK_ALIGN_END);
    grid_column_set_alignment(table, 1, GTK_ALIGN_START);

    g_signal_connect_swapped(dialog->window, "destroy", G_CALLBACK(free), dialog);
    gtk_widget_show_all(dialog->window);
}

void ap_dialog_submit(APDialog *dialog) {
    const gchar *ssid, *psk;

    ssid = gtk_entry_get_text(GTK_ENTRY(dialog->ssid));
    psk = gtk_entry_get_text(GTK_ENTRY(dialog->psk));

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
	(GAsyncReadyCallback) validation_callback,
	(gpointer) &ap_start_messages);

    gtk_widget_destroy(dialog->window);
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
	    (GAsyncReadyCallback) validation_callback,
	    (gpointer) &ap_stop_messages);
    }
    else {
	ap_dialog_launch(ap);
    }

    g_variant_unref(started_var);
}

void ap_set(AP *ap) {
    GVariant *started_var;
    gboolean started;

    started_var = g_dbus_proxy_get_cached_property(ap->proxy, "Started");
    started = g_variant_get_boolean(started_var);

    if (started) {
	gtk_label_set_text(GTK_LABEL(ap->device->status), "AP running");
	gtk_button_set_label(GTK_BUTTON(ap->button), "Stop AP");
    }
    else {
	gtk_label_set_text(GTK_LABEL(ap->device->status), "AP down");
	gtk_button_set_label(GTK_BUTTON(ap->button), "Start AP");
    }

    g_variant_unref(started_var);
}

AP* ap_add(GDBusObject *object, GDBusProxy *proxy) {
    AP *ap;

    ap = malloc(sizeof(AP));
    ap->proxy = proxy;

    ap->button = gtk_button_new_with_label(NULL);
    g_object_ref_sink(ap->button);
    g_signal_connect_swapped(ap->button, "clicked", G_CALLBACK(ap_button_clicked), (gpointer) ap);

    gtk_widget_show_all(ap->button);

    couple_register(DEVICE_AP, 1, ap, object);

    g_signal_connect_swapped(proxy, "g-properties-changed", G_CALLBACK(ap_set), (gpointer) ap);

    return ap;
}

void ap_remove(AP *ap) {
    couple_unregister(DEVICE_AP, 1, ap);
    g_object_unref(ap->button);
    free(ap);
}

void bind_device_ap(Device *device, AP *ap) {
    ap->device = device;
    gtk_grid_attach(GTK_GRID(device->table), ap->button, 4, 0, 1, 1);
    ap_set(ap);
}

void unbind_device_ap(Device *device, AP *ap) {
    ap->device = NULL;
    gtk_container_remove(GTK_CONTAINER(device->table), ap->button);
}
