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

static const CallbackMessages connect_hidden_messages = {
    "Found hidden network",
    "Failed to connect to hidden network",
    detailed_errors_standard
};

void hidden_ssid_dialog(Station *station) {
    HiddenNetworkDialog *dialog;
    GtkWidget *table, *buttons;

    dialog = malloc(sizeof(HiddenNetworkDialog));
    dialog->station = station;

    dialog->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(dialog->window), "Connect to Hidden Network");

    dialog->ssid = gtk_entry_new();

    table = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(dialog->window), table);

    buttons = dialog_buttons(dialog, G_CALLBACK(hidden_ssid_submit), dialog->window);

    gtk_grid_attach(GTK_GRID(table), gtk_label_new("SSID: "), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(table), dialog->ssid,            1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(table), buttons,                 1, 1, 1, 1);

    grid_column_set_alignment(table, 0, GTK_ALIGN_END);
    grid_column_set_alignment(table, 1, GTK_ALIGN_START);

    g_signal_connect_swapped(dialog->window, "destroy", G_CALLBACK(free), dialog);
    gtk_widget_show_all(dialog->window);
}

void hidden_ssid_submit(HiddenNetworkDialog *dialog) {
    const gchar *ssid;

    ssid = gtk_entry_get_text(GTK_ENTRY(dialog->ssid));
    if (*ssid == '\0') {
	return;
    }

    g_dbus_proxy_call(
	dialog->station->proxy,
	"ConnectHiddenNetwork",
	g_variant_new("(s)", ssid),
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) validation_callback,
	(gpointer) &connect_hidden_messages);

    gtk_widget_destroy(dialog->window);
}

void bind_station_network_hidden(Station *station, const gchar *address, const gchar *type, gint16 signal_strength, int index) {
    GtkWidget *status_icon;
    GtkWidget *address_label;
    GtkWidget *security_label;
    GtkWidget *signal;

    status_icon = gtk_image_new_from_icon_name(RESOURCE_HIDDEN, GTK_ICON_SIZE_DND);
    address_label = gtk_label_new(address);
    security_label = gtk_label_new(get_security_type(type));
    signal = signal_widget(signal_strength);

    gtk_grid_attach(GTK_GRID(station->networks), status_icon,    0, index, 1, 1);
    gtk_grid_attach(GTK_GRID(station->networks), address_label,  1, index, 1, 1);
    gtk_grid_attach(GTK_GRID(station->networks), security_label, 2, index, 1, 1);
    gtk_grid_attach(GTK_GRID(station->networks), signal,         3, index, 1, 1);

    gtk_widget_set_halign(status_icon,    GTK_ALIGN_START);
    gtk_widget_set_halign(address_label,  GTK_ALIGN_START);
    gtk_widget_set_halign(security_label, GTK_ALIGN_START);
    gtk_widget_set_halign(signal,         GTK_ALIGN_START);

    gtk_widget_set_hexpand(address_label, TRUE);
}
