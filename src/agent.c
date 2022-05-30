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

GDBusArgInfo arg_network =    {-1, "network", "o", NULL};
GDBusArgInfo arg_passphrase = {-1, "passphrase", "s", NULL};
GDBusArgInfo arg_username =   {-1, "user", "s", NULL};
GDBusArgInfo arg_reason =     {-1, "reason", "s", NULL};

GDBusInterfaceInfo agent_interface_info = {
    -1,
    IWD_IFACE_AGENT,
    (GDBusMethodInfo *[]) {
	&(GDBusMethodInfo) {
	    -1,
	    "Release",
	    NULL,
	    NULL,
	    NULL
	},
	&(GDBusMethodInfo) {
	    -1,
	    "RequestPassphrase",
	    (GDBusArgInfo *[]) {&arg_network, NULL},
	    (GDBusArgInfo *[]) {&arg_passphrase, NULL},
	    NULL
	},
	&(GDBusMethodInfo) {
	    -1,
	    "RequestPrivateKeyPassphrase",
	    (GDBusArgInfo *[]) {&arg_network, NULL},
	    (GDBusArgInfo *[]) {&arg_passphrase, NULL},
	    NULL
	},
	&(GDBusMethodInfo) {
	    -1,
	    "RequestUserNameAndPassword",
	    (GDBusArgInfo *[]) {&arg_network, NULL},
	    (GDBusArgInfo *[]) {&arg_username, &arg_passphrase, NULL},
	    NULL
	},
	&(GDBusMethodInfo) {
	    -1,
	    "RequestUserPassword",
	    (GDBusArgInfo *[]) {&arg_network, &arg_username, NULL},
	    (GDBusArgInfo *[]) {&arg_passphrase, NULL},
	    NULL
	},
	&(GDBusMethodInfo) {
	    -1,
	    "Cancel",
	    (GDBusArgInfo *[]) {&arg_reason, NULL},
	    NULL,
	    NULL
	},
	NULL
    },
    NULL, // Signal info
    NULL, // Property info
    NULL  // Annotation info
};

GDBusInterfaceVTable agent_interface_vtable = {
    (GDBusInterfaceMethodCallFunc) agent_method_call_handler,
    NULL,
    NULL
};

void agent_register(GDBusProxy *proxy) {
    GError *err;
    Agent *agent;

    agent = g_malloc(sizeof(Agent));

    agent->proxy = proxy;
    agent->invocation = NULL;
    agent->pass_widget = NULL;
    agent->window = NULL;

    err = NULL;
    agent->registration_id = g_dbus_connection_register_object(
	    g_dbus_proxy_get_connection(proxy),
	    IWGTK_PATH_AGENT,
	    &agent_interface_info,
	    &agent_interface_vtable,
	    (gpointer) agent,
	    (GDestroyNotify) agent_remove,
	    &err);

    if (agent->registration_id == 0) {
	g_printerr("Error registering agent: %s\n", err->message);
	g_error_free(err);
    }

    g_dbus_proxy_call(
	proxy,
	"RegisterAgent",
	g_variant_new("(o)", IWGTK_PATH_AGENT),
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) method_call_log,
	(gpointer) "Error registering agent: %s\n");
}

void agent_remove(Agent *agent) {
    g_free(agent);
}

void agent_method_call_handler(GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *method_name, GVariant *parameters, GDBusMethodInvocation *invocation, Agent *agent) {
    request_cancel(agent);
    agent->invocation = invocation;

    if (strcmp(method_name, "Release") == 0) {
	g_dbus_connection_unregister_object(connection, agent->registration_id);
    }
    else if (strcmp(method_name, "RequestPassphrase") == 0) {
	request_dialog(agent, USERNAME_NONE);
    }
    else if (strcmp(method_name, "RequestPrivateKeyPassphrase") == 0) {
	request_dialog(agent, USERNAME_NONE);
    }
    else if (strcmp(method_name, "RequestUserNameAndPassword") == 0) {
	request_dialog(agent, USERNAME_ASK);
    }
    else if (strcmp(method_name, "RequestUserPassword") == 0) {
	request_dialog(agent, USERNAME_TELL);
    }
    else if (strcmp(method_name, "Cancel") == 0) {
	const gchar *reason;
	gchar *message;

	g_variant_get(parameters, "(s)", &reason);
	message = g_strconcat("Connection attempt canceled: ", reason, NULL);
	send_notification(message);
	g_free(message);
	g_dbus_method_invocation_return_value(invocation, NULL);
    }
    else {
	g_dbus_method_invocation_return_dbus_error(invocation, "org.freedesktop.DBus.Error.UnknownMethod", "Unknown method");
    }
}

void request_dialog(Agent *agent, guint8 request_type) {
    GVariant *parameters;
    GtkWidget *table, *user_widget;
    const gchar *network_path;
    int i;

    parameters = g_dbus_method_invocation_get_parameters(agent->invocation);
    agent->window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(agent->window), "Network Password");

    table = gtk_grid_new();
    gtk_window_set_child(GTK_WINDOW(agent->window), table);

    if (request_type == USERNAME_NONE) {
	g_variant_get(parameters, "(o)", &network_path);
	user_widget = NULL;
	agent->user_widget = NULL;
    }
    else {
	if (request_type == USERNAME_ASK) {
	    g_variant_get(parameters, "(o)", &network_path);
	    user_widget = gtk_entry_new();
	    agent->user_widget = user_widget;
	}
	else if (request_type == USERNAME_TELL) {
	    const gchar *username;

	    g_variant_get(parameters, "(os)", &network_path, &username);
	    user_widget = gtk_label_new(username);
	    agent->user_widget = NULL;
	}
    }

    {
	GDBusProxy *proxy;
	GVariant *ssid_var;
	const gchar *ssid;

	proxy = G_DBUS_PROXY(g_dbus_object_manager_get_interface(global.manager, network_path, IWD_IFACE_NETWORK));
	ssid_var = g_dbus_proxy_get_cached_property(proxy, "Name");
	ssid = g_variant_get_string(ssid_var, NULL);

	gtk_grid_attach(GTK_GRID(table), gtk_label_new("SSID: "), 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(table), gtk_label_new(ssid),     1, 0, 1, 1);

	g_variant_unref(ssid_var);
	g_object_unref(proxy);
    }

    i = 1;

    if (user_widget) {
	gtk_grid_attach(GTK_GRID(table), gtk_label_new("Username: "), 0, i, 1, 1);
	gtk_grid_attach(GTK_GRID(table), user_widget,                 1, i, 1, 1);
	i ++;
    }

    agent->pass_widget = gtk_password_entry_new();
    gtk_password_entry_set_show_peek_icon(GTK_PASSWORD_ENTRY(agent->pass_widget), TRUE);

    gtk_grid_attach(GTK_GRID(table), gtk_label_new("Password: "), 0, i, 1, 1);
    gtk_grid_attach(GTK_GRID(table), agent->pass_widget,           1, i, 1, 1);
    i ++;

    {
	GtkWidget *buttons;
	buttons = dialog_buttons(agent, (SubmitCallback) request_submit, agent->window);
	gtk_grid_attach(GTK_GRID(table), buttons, 1, i, 1, 1);
    }

    grid_column_set_alignment(table, 0, GTK_ALIGN_END);
    grid_column_set_alignment(table, 1, GTK_ALIGN_START);

    g_signal_connect_swapped(agent->window, "destroy", G_CALLBACK(request_cancel), agent);
    gtk_widget_show(agent->window);
}

void request_submit(Agent *agent) {
    const gchar *password;
    password = gtk_editable_get_text(GTK_EDITABLE(agent->pass_widget));

    if (*password == '\0') {
	return;
    }

    if (agent->user_widget) {
	const gchar *username;
	username = gtk_editable_get_text(GTK_EDITABLE(agent->user_widget));
	g_dbus_method_invocation_return_value(agent->invocation, g_variant_new("(ss)", username, password));
    }
    else {
	g_dbus_method_invocation_return_value(agent->invocation, g_variant_new("(s)", password));
    }

    agent->invocation = NULL;
    gtk_window_destroy(GTK_WINDOW(agent->window));
}

void request_cancel(Agent *agent) {
    if (agent->invocation != NULL) {
	g_dbus_method_invocation_return_dbus_error(agent->invocation, "net.connman.iwd.Agent.Error.Canceled", "Connection attempt canceled");
	agent->invocation = NULL;
    }
}
