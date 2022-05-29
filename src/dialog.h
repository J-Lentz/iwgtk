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

#ifndef _IWGTK_DIALOG_H
#define _IWGTK_DIALOG_H

typedef struct Dialog_s Dialog;

typedef void (*SubmitCallback) (gpointer data);

struct Dialog_s {
    GtkWidget *window;
    SubmitCallback submit_callback;
    gpointer user_data;
};

gboolean dialog_key_press(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, Dialog *dialog);
GtkWidget* dialog_buttons(gpointer user_data, SubmitCallback submit_callback, GtkWidget *window);

#endif
