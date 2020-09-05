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

const ErrorMessage detailed_errors_standard[] = {
    {IWD_ERROR_BUSY,              "Busy"},
    {IWD_ERROR_INVALID_FORMAT,    "Invalid network parameters"},
    {IWD_ERROR_IN_PROGRESS,       "Already in progress"},
    {IWD_ERROR_ABORTED,           "Canceled"},
    {0, NULL}
};

static const GOptionEntry command_options[] = {
    {
	"no-icon",
	'I',
	G_OPTION_FLAG_NONE,
	G_OPTION_ARG_NONE,
	&global.icon_disable,
	"Disable indicator icon",
	NULL
    },
    {
	"no-notifications",
	'N',
	G_OPTION_FLAG_NONE,
	G_OPTION_ARG_NONE,
	&global.notifications_disable,
	"Disable desktop notifications",
	NULL
    },
    {
	"signal-strength",
	's',
	G_OPTION_FLAG_NONE,
	G_OPTION_ARG_NONE,
	&global.signal_icon_disable,
	"Display signal strengths numerically (in dBm)",
	NULL
    },
    {
	"version",
	'V',
	G_OPTION_FLAG_NO_ARG,
	G_OPTION_ARG_CALLBACK,
	(GOptionArgFunc) print_version,
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
	    fprintf(stderr, "Error creating GDBusObjectManager: %s\n", err->message);
	    g_error_free(err);
	    exit(1);
	}
    }

    {
	Window *window;

	window = global.windows;
	while (window != NULL) {
	    window_set(window);
	    window = window->next;
	}
    }

    {
	GDBusProxy *agent_manager;

	agent_manager = G_DBUS_PROXY(g_dbus_object_manager_get_interface(global.manager, IWD_PATH_AGENT_MANAGER, IWD_IFACE_AGENT_MANAGER));
	agent_register(agent_manager);
    }
}

void iwd_up_handler(GDBusConnection *conn, const gchar *name, const gchar *name_owner) {
    g_dbus_object_manager_client_new(
	conn,
	G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
	name,
	IWD_PATH_OBJECT_MANAGER,
	NULL,
	NULL,
	NULL,
	NULL,
	(GAsyncReadyCallback) object_manager_callback,
	NULL
    );
}

void iwd_down_handler(GDBusConnection *conn, const gchar *name) {
    if (global.manager != NULL) {
	g_object_unref(global.manager);
	global.manager = NULL;
    }

    {
	Window *window;

	window = global.windows;
	while (window != NULL) {
	    window_set(window);
	    window = window->next;
	}
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
	IWD_DBUS_NAME,
	G_BUS_NAME_WATCHER_FLAGS_NONE,
	(GBusNameAppearedCallback) iwd_up_handler,
	(GBusNameVanishedCallback) iwd_down_handler,
	NULL,
	NULL
    );
}

void print_version() {
    puts(VERSION_STRING);
    exit(0);
}

int main (int argc, char **argv) {
    {
	GOptionContext *context;
	GError *err;

	context = g_option_context_new(NULL);
	g_option_context_set_summary(context, "iwgtk is a graphical wifi management utility.");
	g_option_context_add_main_entries(context, command_options, NULL);

	err = NULL;
	g_option_context_parse(context, &argc, &argv, &err);
	g_option_context_free(context);

	if (err) {
	    fprintf(stderr, "Incorrect usage: %s\n", err->message);
	    g_error_free(err);
	    exit(1);
	}
    }

    global.application = gtk_application_new("application.iwgtk", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(global.application, "startup", G_CALLBACK(startup), NULL);
    g_signal_connect(global.application, "activate", G_CALLBACK(window_new), NULL);

    {
	int status;

	status = g_application_run(G_APPLICATION(global.application), argc, argv);
	g_object_unref(global.application);
	return status;
    }
}
