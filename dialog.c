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

void dialog_cancel_callback(GtkWidget *cancel_button) {
    GtkWidget *window;

    window = gtk_widget_get_toplevel(cancel_button);
    gtk_widget_destroy(window);
}

/*
 * TODO:
 * This garbage GValue is an ugly hack.
 * Why does g_closure_invoke() fail when n_param_values is 0?
 */
gboolean dialog_key_press_callback(GtkWidget *window, GdkEventKey *event, GClosure *submit_closure) {
    GValue garbage = G_VALUE_INIT;
    g_value_init(&garbage, G_TYPE_POINTER);

    switch (event->keyval) {
	case GDK_KEY_Return:
	    g_closure_invoke(submit_closure, NULL, 1, &garbage, NULL);
	    return TRUE;
	case GDK_KEY_Escape:
	    gtk_widget_destroy(window);
	    return TRUE;
    }
    return FALSE;
}

GtkWidget* dialog_buttons(gpointer data, GCallback submit_callback, GtkWidget *window) {
    GtkWidget *row, *submit, *cancel;
    GClosure *submit_closure;

    row = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);

    submit = gtk_button_new_with_label("Submit");
    gtk_box_pack_start(GTK_BOX(row), submit, FALSE, FALSE, 0);

    submit_closure = g_cclosure_new_swap(submit_callback, data, NULL);
    g_signal_connect_closure(submit, "clicked", submit_closure, FALSE);

    cancel = gtk_button_new_with_label("Cancel");
    gtk_box_pack_start(GTK_BOX(row), cancel, FALSE, FALSE, 0);
    g_signal_connect(cancel, "clicked", G_CALLBACK(dialog_cancel_callback), NULL);

    g_signal_connect(window, "key-press-event", G_CALLBACK(dialog_key_press_callback), submit_closure);

    return row;
}
