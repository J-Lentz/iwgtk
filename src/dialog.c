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

gboolean dialog_key_press(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, Dialog *dialog) {
    switch (keyval) {
	case GDK_KEY_Return:
	    dialog->submit_callback(dialog->user_data);
	    return TRUE;
	case GDK_KEY_Escape:
	    gtk_window_destroy(GTK_WINDOW(dialog->window));
	    return TRUE;
    }

    return FALSE;
}

GtkWidget* dialog_buttons(gpointer user_data, SubmitCallback submit_callback, GtkWidget *window) {
    GtkWidget *row, *submit_button, *cancel_button;
    Dialog *dialog;

    row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    submit_button = gtk_button_new_with_label(_("Submit"));
    gtk_box_append(GTK_BOX(row), submit_button);

    cancel_button = gtk_button_new_with_label(_("Cancel"));
    gtk_box_append(GTK_BOX(row), cancel_button);

    g_signal_connect_swapped(submit_button, "clicked", G_CALLBACK(submit_callback), user_data);
    g_signal_connect_swapped(cancel_button, "clicked", G_CALLBACK(gtk_window_destroy), window);

    dialog = g_malloc(sizeof(Dialog));
    g_signal_connect_swapped(window, "destroy", G_CALLBACK(g_free), dialog);

    dialog->window = window;
    dialog->submit_callback = submit_callback;
    dialog->user_data = user_data;

    {
	GtkEventController *controller;

	controller = gtk_event_controller_key_new();
	gtk_event_controller_set_propagation_phase(controller, GTK_PHASE_CAPTURE);
	g_signal_connect(controller, "key-pressed", G_CALLBACK(dialog_key_press), dialog);
	gtk_widget_add_controller(window, controller);
    }

    return row;
}
