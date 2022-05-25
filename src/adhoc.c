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

static const CallbackMessages adhoc_start_messages = {
    "Ad-hoc node started",
    "Failed to start ad-hoc node",
    detailed_errors_standard
};

static const CallbackMessages adhoc_stop_messages = {
    "Ad-hoc node stopped",
    "Failed to stop ad-hoc node",
    detailed_errors_standard
};

void psk_toggle_changed(GtkToggleButton *psk_toggle, AdHocDialog *adhoc_dialog) {
    gboolean psk_state;
    psk_state = gtk_toggle_button_get_active(psk_toggle);
    gtk_widget_set_sensitive(adhoc_dialog->psk, psk_state);
}

void adhoc_dialog_launch(AdHoc *adhoc) {
    AdHocDialog *adhoc_dialog;
    GtkWidget *table, *buttons;

    adhoc_dialog = g_malloc(sizeof(AdHocDialog));
    adhoc_dialog->adhoc = adhoc;

    adhoc_dialog->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(adhoc_dialog->window), "Start Ad-Hoc Node");

    adhoc_dialog->ssid = gtk_entry_new();
    adhoc_dialog->psk = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(adhoc_dialog->psk), 0);


    buttons = dialog_buttons(adhoc_dialog, G_CALLBACK(adhoc_dialog_submit), adhoc_dialog->window);

    table = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(adhoc_dialog->window), table);

    adhoc_dialog->psk_toggle = gtk_check_button_new();
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(adhoc_dialog->psk_toggle), TRUE);
    g_signal_connect(adhoc_dialog->psk_toggle, "toggled", G_CALLBACK(psk_toggle_changed), (gpointer) adhoc_dialog);

    gtk_grid_attach(GTK_GRID(table), gtk_label_new("SSID: "),     0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(table), adhoc_dialog->ssid,          2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(table), gtk_label_new("Password: "), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(table), adhoc_dialog->psk_toggle,    1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(table), adhoc_dialog->psk,           2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(table), buttons,                     2, 2, 1, 1);

    grid_column_set_alignment(table, 0, GTK_ALIGN_END);
    grid_column_set_alignment(table, 2, GTK_ALIGN_START);

    g_signal_connect_swapped(adhoc_dialog->window, "destroy", G_CALLBACK(g_free), adhoc_dialog);
    gtk_widget_show_all(adhoc_dialog->window);
}

void adhoc_dialog_submit(AdHocDialog *adhoc_dialog) {
    const gchar *ssid, *psk;
    gboolean use_psk;
    const char *method;
    GVariant *parameters;

    use_psk = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adhoc_dialog->psk_toggle));
    ssid = gtk_entry_get_text(GTK_ENTRY(adhoc_dialog->ssid));
    psk = gtk_entry_get_text(GTK_ENTRY(adhoc_dialog->psk));

    if (*ssid == '\0') {
	return;
    }

    if (use_psk) {
	if (*psk == '\0') {
	    return;
	}

	method = "Start";
	parameters = g_variant_new("(ss)", ssid, psk);
    }
    else {
	method = "StartOpen";
	parameters = g_variant_new("(s)", ssid);
    }

    g_dbus_proxy_call(
	adhoc_dialog->adhoc->proxy,
	method,
	parameters,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) validation_callback,
	(gpointer) &adhoc_start_messages);

    gtk_widget_destroy(adhoc_dialog->window);
}

void adhoc_button_clicked(AdHoc *adhoc) {
    GVariant *started_var;
    gboolean started;

    started_var = g_dbus_proxy_get_cached_property(adhoc->proxy, "Started");
    started = g_variant_get_boolean(started_var);

    if (started) {
	g_dbus_proxy_call(
	    adhoc->proxy,
	    "Stop",
	    NULL,
	    G_DBUS_CALL_FLAGS_NONE,
	    -1,
	    NULL,
	    (GAsyncReadyCallback) validation_callback,
	    (gpointer) &adhoc_stop_messages);
    }
    else {
	adhoc_dialog_launch(adhoc);
    }

    g_variant_unref(started_var);
}

void adhoc_set(AdHoc *adhoc) {
    GVariant *started_var;
    gboolean started;

    started_var = g_dbus_proxy_get_cached_property(adhoc->proxy, "Started");
    started = g_variant_get_boolean(started_var);

    // Clear peer list
    gtk_container_foreach(GTK_CONTAINER(adhoc->peer_list), (GtkCallback) gtk_widget_destroy, NULL);

    if (started) {
	GVariant *peer_list_var;
	GVariantIter *iter;
	const gchar *peer_mac;
	int n;

	peer_list_var = g_dbus_proxy_get_cached_property(adhoc->proxy, "ConnectedPeers");
	g_variant_get(peer_list_var, "as", &iter);

	n = 0;
	while (g_variant_iter_next(iter, "s", &peer_mac)) {
	    GtkWidget *peer_label;

	    peer_label = gtk_label_new(peer_mac);
	    gtk_widget_show(peer_label);
	    gtk_box_pack_start(GTK_BOX(adhoc->peer_list), peer_label, FALSE, FALSE, 0);
	    gtk_widget_set_halign(peer_label, GTK_ALIGN_START);

	    n ++;
	}

	g_variant_iter_free(iter);
	g_variant_unref(peer_list_var);

	{
	    gchar *n_peers_text;

	    switch (n) {
		case 0:
		    n_peers_text = "No connected peers";
		    break;
		case 1:
		    n_peers_text = "1 connected peer:";
		    break;
		default:
		    n_peers_text = g_strdup_printf("%d connected peers:", n);
	    }

	    gtk_label_set_text(GTK_LABEL(adhoc->n_peers), n_peers_text);

	    if (n > 1) {
		g_free(n_peers_text);
	    }
	}

	gtk_button_set_label(GTK_BUTTON(adhoc->button), "Stop node");
    }
    else {
	gtk_label_set_text(GTK_LABEL(adhoc->n_peers), "");
	gtk_button_set_label(GTK_BUTTON(adhoc->button), "Start node");
    }

    g_variant_unref(started_var);
}

AdHoc* adhoc_add(Window *window, GDBusObject *object, GDBusProxy *proxy) {
    AdHoc *adhoc;
    GtkWidget *row1;

    adhoc = g_malloc(sizeof(AdHoc));
    adhoc->proxy = proxy;

    adhoc->button = gtk_button_new_with_label(NULL);
    g_object_ref_sink(adhoc->button);
    g_signal_connect_swapped(adhoc->button, "clicked", G_CALLBACK(adhoc_button_clicked), (gpointer) adhoc);

    adhoc->n_peers = gtk_label_new(NULL);
    adhoc->peer_list = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    gtk_widget_set_halign(adhoc->n_peers, GTK_ALIGN_START);
    gtk_widget_set_margin_start(adhoc->n_peers, 5);
    gtk_widget_set_margin_start(adhoc->peer_list, 5);

    gtk_widget_show(adhoc->n_peers);
    gtk_widget_show(adhoc->button);
    gtk_widget_show(adhoc->peer_list);

    couple_register(window, DEVICE_ADHOC, 1, adhoc, object);

    adhoc->handler_update = g_signal_connect_swapped(proxy, "g-properties-changed", G_CALLBACK(adhoc_set), (gpointer) adhoc);

    return adhoc;
}

void adhoc_remove(Window *window, AdHoc *adhoc) {
    couple_unregister(window, DEVICE_ADHOC, 1, adhoc);
    g_object_unref(adhoc->button);

    g_signal_handler_disconnect(adhoc->proxy, adhoc->handler_update);
    g_free(adhoc);
}

void bind_device_adhoc(Device *device, AdHoc *adhoc) {
    adhoc->device = device;
    gtk_grid_attach(GTK_GRID(device->table), adhoc->button, 3, 0, 1, 1);
    gtk_box_pack_start(GTK_BOX(device->master), adhoc->n_peers, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(device->master), adhoc->peer_list, TRUE, TRUE, 0);
    adhoc_set(adhoc);
}

void unbind_device_adhoc(Device *device, AdHoc *adhoc) {
    adhoc->device = NULL;
    gtk_container_remove(GTK_CONTAINER(device->table), adhoc->button);
    gtk_container_remove(GTK_CONTAINER(device->master), adhoc->n_peers);
    gtk_container_remove(GTK_CONTAINER(device->master), adhoc->peer_list);
}
