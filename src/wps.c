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
    detailed_errors_wps,
    FALSE
};

void wps_set_pushbutton(WPS *wps) {
    if (wps->handler_pushbutton != 0) {
	g_signal_handler_disconnect(wps->pushbutton, wps->handler_pushbutton);
    }

    gtk_button_set_label(GTK_BUTTON(wps->pushbutton), "Push button");
    wps->handler_pushbutton = g_signal_connect_swapped(wps->pushbutton, "clicked", G_CALLBACK(wps_connect_pushbutton), wps);
}

void wps_set_pin(WPS *wps) {
    if (wps->handler_pin != 0) {
	g_signal_handler_disconnect(wps->pin, wps->handler_pin);
    }

    gtk_button_set_label(GTK_BUTTON(wps->pin), "PIN");
    wps->handler_pin = g_signal_connect_swapped(wps->pin, "clicked", G_CALLBACK(wps_connect_pin_dialog), wps);
}

void wps_set_pushbutton_cancel(WPS *wps) {
    g_signal_handler_disconnect(wps->pushbutton, wps->handler_pushbutton);
    gtk_button_set_child(GTK_BUTTON(wps->pushbutton), label_with_spinner("Cancel"));
    wps->handler_pushbutton = g_signal_connect_swapped(wps->pushbutton, "clicked", G_CALLBACK(wps_cancel), wps);
}

void wps_set_pin_cancel(WPS *wps) {
    g_signal_handler_disconnect(wps->pin, wps->handler_pin);
    gtk_button_set_child(GTK_BUTTON(wps->pin), label_with_spinner("Cancel"));
    wps->handler_pin = g_signal_connect_swapped(wps->pin, "clicked", G_CALLBACK(wps_cancel), wps);
}

void wps_pushbutton_callback(GDBusProxy *proxy, GAsyncResult *res, WPS *wps) {
    wps_set_pushbutton(wps);
    method_call_notify(proxy, res, (CallbackMessages *) &wps_messages);
}

void wps_pin_callback(GDBusProxy *proxy, GAsyncResult *res, WPS *wps) {
    wps_set_pin(wps);
    method_call_notify(proxy, res, (CallbackMessages *) &wps_messages);
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
	(GAsyncReadyCallback) wps_pin_callback,
	wps_dialog->wps);

    wps_set_pin_cancel(wps_dialog->wps);
    gtk_window_destroy(GTK_WINDOW(wps_dialog->window));
}

void wps_connect_pushbutton(WPS *wps) {
    g_dbus_proxy_call(
	wps->proxy,
	"PushButton",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	G_MAXINT,
	NULL,
	(GAsyncReadyCallback) wps_pushbutton_callback,
	wps);

    wps_set_pushbutton_cancel(wps);
}

void wps_cancel(WPS *wps, GtkWidget *button) {
    g_dbus_proxy_call(
	wps->proxy,
	"Cancel",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) method_call_log,
	"Error canceling WPS connection: %s\n");

    if (button == wps->pushbutton) {
	wps_set_pushbutton(wps);
    }
    else {
	wps_set_pin(wps);
    }
}

WPS* wps_add(Window *window, GDBusObject *object, GDBusProxy *proxy) {
    WPS *wps;

    wps = g_malloc(sizeof(WPS));
    wps->proxy = proxy;
    wps->handler_pushbutton = 0;
    wps->handler_pin = 0;

    wps->label = new_label_bold("WPS");
    g_object_ref_sink(wps->label);
    gtk_widget_set_margin_top(wps->label, 10);

    wps->pushbutton = gtk_button_new();
    g_object_ref_sink(wps->pushbutton);
    wps_set_pushbutton(wps);

    wps->pin = gtk_button_new();
    g_object_ref_sink(wps->pin);
    wps_set_pin(wps);

    couple_register(window, STATION_WPS, 1, wps, object);
    return wps;
}

void wps_remove(Window *window, WPS *wps) {
    couple_unregister(window, STATION_WPS, 1, wps);

    g_object_unref(wps->label);
    g_object_unref(wps->pushbutton);
    g_object_unref(wps->pin);

    g_free(wps);
}

void bind_station_wps(Station *station, WPS *wps) {
    wps->station = station;

    gtk_box_append(GTK_BOX(station->provision_vbox), wps->label);
    gtk_box_append(GTK_BOX(station->provision_vbox), wps->pushbutton);
    gtk_box_append(GTK_BOX(station->provision_vbox), wps->pin);
}

void unbind_station_wps(Station *station, WPS *wps) {
    gtk_box_remove(GTK_BOX(station->provision_vbox), wps->label);
    gtk_box_remove(GTK_BOX(station->provision_vbox), wps->pushbutton);
    gtk_box_remove(GTK_BOX(station->provision_vbox), wps->pin);
}
