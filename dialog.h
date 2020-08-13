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

#ifndef _IWGTK_DIALOG_H
#define _IWGTK_DIALOG_H

void dialog_cancel_callback(GtkWidget *cancel_button);
gboolean dialog_key_press_callback(GtkWidget *window, GdkEventKey *event, GClosure *submit_closure);
GtkWidget* dialog_buttons(gpointer data, GCallback submit_callback, GtkWidget *window);

#endif
