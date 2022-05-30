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

const ErrorMessage detailed_errors_wps[] = {
    {IWD_ERROR_INVALID_FORMAT,        "Invalid PIN"},
    {IWD_ERROR_WSC_SESSION_OVERLAP,   "Multiple access points found"},
    {IWD_ERROR_WSC_TIME_EXPIRED,      "No AP found in PIN mode"},
    {IWD_ERROR_WSC_WALK_TIME_EXPIRED, "No AP found in push-button mode"},
    {0, NULL}
};

static const CallbackMessages wps_messages = {
    "WPS connection successful",
    "WPS connection failed",
    detailed_errors_wps
};

void wps_callback(GDBusProxy *proxy, GAsyncResult *res, WPS *wps) {
    gtk_widget_show(wps->connect);
    gtk_widget_hide(wps->cancel);

    validation_callback(proxy, res, (gpointer) &wps_messages);
}

void wps_connect_pin_dialog(WPS *wps) {
    WPSDialog *wps_dialog;

    GtkWidget *table, *buttons;

    wps_dialog = g_malloc(sizeof(WPSDialog));
    wps_dialog->wps = wps;

    wps_dialog->window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(wps_dialog->window), "Connect via WPS");

    wps_dialog->pin = gtk_password_entry_new();
    gtk_password_entry_set_show_peek_icon(GTK_PASSWORD_ENTRY(wps_dialog->pin), TRUE);

    buttons = dialog_buttons(wps_dialog, (SubmitCallback) wps_pin_dialog_submit, wps_dialog->window);

    table = gtk_grid_new();
    gtk_window_set_child(GTK_WINDOW(wps_dialog->window), table);

    gtk_grid_attach(GTK_GRID(table), gtk_label_new("PIN: "), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(table), wps_dialog->pin,        1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(table), buttons,                1, 1, 1, 1);

    grid_column_set_alignment(table, 0, GTK_ALIGN_END);
    grid_column_set_alignment(table, 1, GTK_ALIGN_START);

    g_signal_connect_swapped(wps_dialog->window, "destroy", G_CALLBACK(g_free), wps_dialog);
    gtk_widget_show(wps_dialog->window);
}

void wps_pin_dialog_submit(WPSDialog *wps_dialog) {
    const gchar *pin;

    pin = gtk_editable_get_text(GTK_EDITABLE(wps_dialog->pin));

    if (*pin == '\0') {
	return;
    }

    g_dbus_proxy_call(
	wps_dialog->wps->proxy,
	"StartPin",
	g_variant_new("(s)", pin),
	G_DBUS_CALL_FLAGS_NONE,
	G_MAXINT,
	NULL,
	(GAsyncReadyCallback) wps_callback,
	(gpointer) wps_dialog->wps);

    gtk_widget_show(wps_dialog->wps->cancel);
    gtk_widget_hide(wps_dialog->wps->connect);

    gtk_window_destroy(GTK_WINDOW(wps_dialog->window));
}

void wps_cancel(WPS *wps) {
    g_dbus_proxy_call(
	wps->proxy,
	"Cancel",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) validation_callback_log,
	(gpointer) "Error canceling WPS connection: %s\n");

    gtk_widget_show(wps->connect);
    gtk_widget_hide(wps->cancel);
}

void wps_connect_pushbutton(WPS *wps) {
    g_dbus_proxy_call(
	wps->proxy,
	"PushButton",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	G_MAXINT,
	NULL,
	(GAsyncReadyCallback) wps_callback,
	(gpointer) wps);

    gtk_widget_show(wps->cancel);
    gtk_widget_hide(wps->connect);
}

WPS* wps_add(Window *window, GDBusObject *object, GDBusProxy *proxy) {
    GtkWidget *menu;
    GtkWidget *cancel_button;
    WPS *wps;

    wps = g_malloc(sizeof(WPS));
    wps->proxy = proxy;

    {
	GtkWidget *pushbutton;
	GtkWidget *pin;
	GtkWidget *vbox;

	pushbutton = gtk_button_new_with_label("Push button");
	pin = gtk_button_new_with_label("PIN");

	g_signal_connect_swapped(pushbutton, "clicked", G_CALLBACK(wps_connect_pushbutton), (gpointer) wps);
	g_signal_connect_swapped(pin, "clicked", G_CALLBACK(wps_connect_pin_dialog), (gpointer) wps);

	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_append(GTK_BOX(vbox), pushbutton);
	gtk_box_append(GTK_BOX(vbox), pin);

	wps->menu = gtk_popover_new();
	gtk_popover_set_child(GTK_POPOVER(wps->menu), vbox);
	gtk_popover_set_has_arrow(GTK_POPOVER(wps->menu), FALSE);
    }

    wps->connect = gtk_button_new_with_label("Connect via WPS");
    g_signal_connect_swapped(wps->connect, "clicked", G_CALLBACK(gtk_widget_show), (gpointer) wps->menu);
    gtk_widget_set_parent(wps->menu, wps->connect);

    wps->cancel = gtk_button_new();
    gtk_button_set_child(GTK_BUTTON(wps->cancel), label_with_spinner("Cancel"));
    g_signal_connect_swapped(wps->cancel, "clicked", G_CALLBACK(wps_cancel), (gpointer) wps);

    gtk_widget_hide(wps->cancel);

    {
	wps->hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	g_object_ref_sink(wps->hbox);
	gtk_box_append(GTK_BOX(wps->hbox), wps->connect);
	gtk_box_append(GTK_BOX(wps->hbox), wps->cancel);
    }

    couple_register(window, DEVICE_WPS, 1, wps, object);
    return wps;
}

void wps_remove(Window *window, WPS *wps) {
    gtk_widget_unparent(wps->menu);
    couple_unregister(window, DEVICE_WPS, 1, wps);
    g_object_unref(wps->hbox);
    g_free(wps);
}

void bind_device_wps(Device *device, WPS *wps) {
    gtk_grid_attach(GTK_GRID(device->table), wps->hbox, 0, 1, 1, 1);
    gtk_widget_set_halign(wps->hbox, GTK_ALIGN_FILL);
    gtk_widget_set_valign(wps->hbox, GTK_ALIGN_CENTER);
}

void unbind_device_wps(Device *device, WPS *wps) {
    gtk_grid_remove(GTK_GRID(device->table), wps->hbox);
}
