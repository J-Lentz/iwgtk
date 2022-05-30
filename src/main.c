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

GlobalData global;

static const GDBusErrorEntry iwd_error_codes[] = {
    {IWD_ERROR_BUSY,                  "net.connman.iwd.Busy"},
    {IWD_ERROR_FAILED,                "net.connman.iwd.Failed"},
    {IWD_ERROR_INVALID_ARGUMENTS,     "net.connman.iwd.InvalidArguments"},
    {IWD_ERROR_ALREADY_EXISTS,        "net.connman.iwd.AlreadyExists"},
    {IWD_ERROR_NOT_CONNECTED,         "net.connman.iwd.NotConnected"},
    {IWD_ERROR_NOT_CONFIGURED,        "net.connman.iwd.NotConfigured"},
    {IWD_ERROR_NOT_FOUND,             "net.connman.iwd.NotFound"},
    {IWD_ERROR_SERVICE_SET_OVERLAP,   "net.connman.iwd.ServiceSetOverlap"},
    {IWD_ERROR_ALREADY_PROVISIONED,   "net.connman.iwd.AlreadyProvisioned"},
    {IWD_ERROR_NOT_HIDDEN,            "net.connman.iwd.NotHidden"},
    {IWD_ERROR_ABORTED,               "net.connman.iwd.Aborted"},
    {IWD_ERROR_NO_AGENT,              "net.connman.iwd.NoAgent"},
    {IWD_ERROR_NOT_SUPPORTED,         "net.connman.iwd.NotSupported"},
    {IWD_ERROR_TIMEOUT,               "net.connman.iwd.Timeout"},
    {IWD_ERROR_IN_PROGRESS,           "net.connman.iwd.InProgress"},
    {IWD_ERROR_WSC_SESSION_OVERLAP,   "net.connman.iwd.SimpleConfiguration.SessionOverlap"},
    {IWD_ERROR_WSC_NO_CREDENTIALS,    "net.connman.iwd.SimpleConfiguration.NoCredentials"},
    {IWD_ERROR_WSC_TIME_EXPIRED,      "net.connman.iwd.SimpleConfiguration.TimeExpired"},
    {IWD_ERROR_WSC_WALK_TIME_EXPIRED, "net.connman.iwd.SimpleConfiguration.WalkTimeExpired"},
    {IWD_ERROR_WSC_NOT_REACHABLE,     "net.connman.iwd.SimpleConfiguration.NotReachable"},
    {IWD_ERROR_INVALID_FORMAT,        "net.connman.iwd.InvalidFormat"},
    {IWD_ERROR_NOT_AVAILABLE,         "net.connman.iwd.NotAvailable"},
    {IWD_ERROR_AGENT_CANCELED,        "net.connman.iwd.Agent.Error.Canceled"}
};

static const GOptionEntry command_options[] = {
    {
	"indicators",
	'i',
	G_OPTION_FLAG_NONE,
	G_OPTION_ARG_NONE,
	NULL,
	"Enable indicator (tray) icons and run iwgtk in the background",
	NULL
    },
    {
	"notifications",
	'n',
	G_OPTION_FLAG_NONE,
	G_OPTION_ARG_NONE,
	NULL,
	"Enable desktop notifications (default)",
	NULL
    },
    {
	"no-notifications",
	'N',
	G_OPTION_FLAG_NONE,
	G_OPTION_ARG_NONE,
	NULL,
	"Disable desktop notifications",
	NULL
    },
    {
	"version",
	'V',
	G_OPTION_FLAG_NONE,
	G_OPTION_ARG_NONE,
	NULL,
	"Print version",
	NULL
    },
    NULL
};

void object_manager_callback(GDBusObjectManagerClient *manager, GAsyncResult *res) {
    {
	GError *err;

	global.manager = g_dbus_object_manager_client_new_finish(res, &err);
	if (!global.manager) {
	    g_printerr("Error creating GDBusObjectManager: %s\n", err->message);
	    g_error_free(err);
	    exit(1);
	}
    }

    if (global.window != NULL) {
	window_set();
    }

    if (global.indicators_enable) {
	g_application_release(G_APPLICATION(global.application));
	add_all_dbus_objects(NULL);
    }

    g_signal_connect(global.manager, "interface-added",   G_CALLBACK(interface_add), NULL);
    g_signal_connect(global.manager, "interface-removed", G_CALLBACK(interface_rm),  NULL);
    g_signal_connect(global.manager, "object-added",      G_CALLBACK(object_add),    NULL);
    g_signal_connect(global.manager, "object-removed",    G_CALLBACK(object_rm),     NULL);

    {
	GDBusProxy *agent_manager;

	agent_manager = G_DBUS_PROXY(g_dbus_object_manager_get_interface(global.manager, IWD_PATH_AGENT_MANAGER, IWD_IFACE_AGENT_MANAGER));
	agent_register(agent_manager);
    }
}

void iwd_up(GDBusConnection *connection) {
    g_dbus_object_manager_client_new(
	connection,
	G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
	IWD_BUS_NAME,
	IWD_PATH_OBJECT_MANAGER,
	NULL,
	NULL,
	NULL,
	NULL,
	(GAsyncReadyCallback) object_manager_callback,
	NULL
    );
}

void iwd_down(GDBusConnection *connection) {
    if (global.manager != NULL) {
	g_object_unref(global.manager);
	global.manager = NULL;
    }

    if (global.window != NULL) {
	window_set();
    }
}

void startup(GtkApplication *app) {
    {
	volatile gsize error_domain_volatile;

	g_dbus_error_register_error_domain("iwd-error-quark", &error_domain_volatile, iwd_error_codes, G_N_ELEMENTS(iwd_error_codes));
	global.iwd_error_domain = (GQuark) error_domain_volatile;
    }

    g_bus_watch_name(
	G_BUS_TYPE_SYSTEM,
	IWD_BUS_NAME,
	G_BUS_NAME_WATCHER_FLAGS_NONE,
	(GBusNameAppearedCallback) iwd_up,
	(GBusNameVanishedCallback) iwd_down,
	NULL,
	NULL
    );
}

gint handle_local_options(GApplication *application, GVariantDict *options) {
    if (g_variant_dict_contains(options, "version")) {
	puts(VERSION_STRING);
	return 0;
    }

    return -1;
}

gint command_line(GApplication *application, GApplicationCommandLine *command_line) {
    GVariantDict *options;

    options = g_application_command_line_get_options_dict(command_line);

    if (g_variant_dict_contains(options, "notifications")) {
	global.notifications_disable = FALSE;
    }

    if (g_variant_dict_contains(options, "no-notifications")) {
	global.notifications_disable = TRUE;
    }

    if (g_variant_dict_contains(options, "indicators")) {
	global.indicators_enable = TRUE;
	g_application_hold(application);
    }
    else {
	window_launch();
    }

    return 0;
}

void iwgtk_quit() {
    if (global.window != NULL) {
	gtk_window_destroy(GTK_WINDOW(global.window->window));
    }

    while (global.indicators != NULL) {
	Indicator *rm;

	rm = global.indicators;
	global.indicators = global.indicators->next;
	indicator_rm(rm);
    }
}

int main (int argc, char **argv) {
    global.application = gtk_application_new(APPLICATION_ID, G_APPLICATION_HANDLES_COMMAND_LINE);

    g_application_set_option_context_summary(G_APPLICATION(global.application), "iwgtk is a graphical wifi management utility.");
    g_application_add_main_option_entries(G_APPLICATION(global.application), command_options);

    g_signal_connect(global.application, "startup", G_CALLBACK(startup), NULL);
    g_signal_connect(global.application, "handle-local-options", G_CALLBACK(handle_local_options), NULL);
    g_signal_connect(global.application, "command-line", G_CALLBACK(command_line), NULL);
    g_signal_connect(global.application, "activate", G_CALLBACK(window_launch), NULL);

    {
	int status;

	status = g_application_run(G_APPLICATION(global.application), argc, argv);
	g_object_unref(global.application);
	return status;
    }
}
