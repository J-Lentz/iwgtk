/*
 *  Copyright 2020-2025 Jesse Lentz and contributors
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

#ifndef _IWGTK_AGENT_H
#define _IWGTK_AGENT_H

#define USERNAME_NONE 0
#define USERNAME_ASK  1
#define USERNAME_TELL 2

typedef struct Agent_s Agent;

struct Agent_s {
    guint registration_id;
    GDBusProxy *proxy;
    GDBusMethodInvocation *invocation;

    GtkWidget *window;
    GtkWidget *user_widget;
    GtkWidget *pass_widget;
};

void agent_register(GDBusProxy *proxy);
void agent_remove(Agent *agent);
void agent_method_call_handler(GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *method_name, GVariant *parameters, GDBusMethodInvocation *invocation, Agent *data);
void request_dialog(Agent *data, guint8 request_type);
void request_submit(Agent *data);
void agent_window_destroy(Agent *data);

#endif
