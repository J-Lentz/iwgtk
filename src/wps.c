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

const ErrorMessage detailed_errors_wps[] = {
    {IWD_ERROR_BUSY,                   "Busy"},
    {IWD_ERROR_ABORTED,                "Canceled"},
    {IWD_ERROR_FAILED,                 "Operation failed"},
    {IWD_ERROR_INVALID_FORMAT,         "Invalid PIN"},
    {IWD_ERROR_IN_PROGRESS,            "Operation already in progress"},
    {IWD_ERROR_WSC_SESSION_OVERLAP,    "Multiple access points found"},
    {IWD_ERROR_WSC_TIME_EXPIRED,       "No AP found in PIN mode"},
    {IWD_ERROR_WSC_WALK_TIME_EXPIRED,  "No AP found in push-button mode"},
    {0, NULL}
};

static const CallbackMessages wps_messages = {
    "WPS connection successful",
    "WPS connection failed",
    detailed_errors_wps
};

void wps_callback(GDBusProxy *proxy, GAsyncResult *res, WPS *wps) {
    gtk_widget_show(wps->menu);
    gtk_widget_hide(wps->cancel);

    validation_callback(proxy, res, (gpointer) &wps_messages);
}

void wps_connect_pin_dialog(WPS *wps) {
    WPSDialog *wps_dialog;

    GtkWidget *table, *buttons;

    wps_dialog = malloc(sizeof(WPSDialog));
    wps_dialog->wps = wps;

    wps_dialog->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(wps_dialog->window), "Connect via WPS");

    wps_dialog->pin = gtk_entry_new();

    buttons = dialog_buttons(wps_dialog, G_CALLBACK(wps_pin_dialog_submit), wps_dialog->window);

    table = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(wps_dialog->window), table);

    gtk_grid_attach(GTK_GRID(table), gtk_label_new("PIN: "), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(table), wps_dialog->pin,        1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(table), buttons,                1, 1, 1, 1);

    grid_column_set_alignment(table, 0, GTK_ALIGN_END);
    grid_column_set_alignment(table, 1, GTK_ALIGN_START);

    g_signal_connect_swapped(wps_dialog->window, "destroy", G_CALLBACK(free), wps_dialog);
    gtk_widget_show_all(wps_dialog->window);
}

void wps_pin_dialog_submit(WPSDialog *wps_dialog) {
    const gchar *pin;

    pin = gtk_entry_get_text(GTK_ENTRY(wps_dialog->pin));

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
    gtk_widget_hide(wps_dialog->wps->menu);

    gtk_widget_destroy(wps_dialog->window);
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

    gtk_widget_show(wps->menu);
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
    gtk_widget_hide(wps->menu);
}

WPS* wps_add(Window *window, GDBusObject *object, GDBusProxy *proxy) {
    GtkWidget *widget_pushbutton, *widget_pin;
    GtkWidget *menu;
    GtkWidget *cancel_button;
    WPS *wps;

    wps = malloc(sizeof(WPS));
    wps->proxy = proxy;

    widget_pushbutton = gtk_menu_item_new_with_label("Push button");
    g_signal_connect_swapped(widget_pushbutton, "activate", G_CALLBACK(wps_connect_pushbutton), (gpointer) wps);

    widget_pin = gtk_menu_item_new_with_label("PIN");
    g_signal_connect_swapped(widget_pin, "activate", G_CALLBACK(wps_connect_pin_dialog), (gpointer) wps);

    menu = gtk_menu_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), widget_pushbutton);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), widget_pin);
    gtk_widget_show_all(menu);

    wps->menu = gtk_menu_button_new();
    gtk_menu_button_set_popup(GTK_MENU_BUTTON(wps->menu), menu);
    gtk_button_set_label(GTK_BUTTON(wps->menu), "Connect via WPS");

    wps->cancel = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(wps->cancel), label_with_spinner("Cancel WPS Connection"));
    g_signal_connect_swapped(wps->cancel, "clicked", G_CALLBACK(wps_cancel), (gpointer) wps);

    gtk_widget_show_all(wps->menu);
    gtk_widget_show_all(wps->cancel);
    gtk_widget_hide(wps->cancel);

    {
	wps->hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	g_object_ref_sink(wps->hbox);
	gtk_box_pack_start(GTK_BOX(wps->hbox), wps->menu, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(wps->hbox), wps->cancel, TRUE, TRUE, 0);
	gtk_widget_show(wps->hbox);
    }

    couple_register(window, DEVICE_WPS, 1, wps, object);
    return wps;
}

void wps_remove(Window *window, WPS *wps) {
    couple_unregister(window, DEVICE_WPS, 1, wps);
    g_object_unref(wps->hbox);
    free(wps);
}

void bind_device_wps(Device *device, WPS *wps) {
    gtk_grid_attach(GTK_GRID(device->table), wps->hbox, 4, 1, 1, 1);
    gtk_widget_set_halign(wps->hbox, GTK_ALIGN_FILL);
}

void unbind_device_wps(Device *device, WPS *wps) {
    gtk_container_remove(GTK_CONTAINER(device->table), wps->hbox);
}
