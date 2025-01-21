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

#include <locale.h>
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
	N_("Start indicator (tray) icon daemon"),
	NULL
    },
    {
	"notifications",
	'n',
	G_OPTION_FLAG_NONE,
	G_OPTION_ARG_NONE,
	NULL,
	N_("Enable desktop notifications (default)"),
	NULL
    },
    {
	"no-notifications",
	'N',
	G_OPTION_FLAG_NONE,
	G_OPTION_ARG_NONE,
	NULL,
	N_("Disable desktop notifications"),
	NULL
    },
    {
	"version",
	'V',
	G_OPTION_FLAG_NONE,
	G_OPTION_ARG_NONE,
	NULL,
	N_("Print version number"),
	NULL
    },
    {NULL}
};

void object_manager_callback(GDBusObjectManagerClient *manager, GAsyncResult *res) {
    gboolean launch_window;

    if (global.state & WINDOW_LAUNCH_PENDING) {
	global.state &= ~WINDOW_LAUNCH_PENDING;
	g_application_release(G_APPLICATION(global.application));
	launch_window = TRUE;
    }
    else {
	launch_window = FALSE;
    }

    {
	GError *err;

	err = NULL;
	global.manager = g_dbus_object_manager_client_new_finish(res, &err);

	if (err) {
	    if (err->domain == G_DBUS_ERROR && err->code == G_DBUS_ERROR_ACCESS_DENIED) {
		g_printerr("Access denied: User must be a member of either the netdev or wheel group to control iwd\n");
	    }
	    else {
		g_printerr("Error creating GDBusObjectManager: %s\n", err->message);
	    }

	    g_error_free(err);
	    return;
	}
    }

    if (launch_window) {
	window_launch();
    }

    if (global.state & INDICATOR_DAEMON) {
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
    global.state &= ~IWD_DOWN;

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
    global.state |= IWD_DOWN;
    send_notification(_("iwd is down"));

    if (global.window != NULL) {
	gtk_window_destroy(GTK_WINDOW(global.window->window));
	g_printerr("Destroying iwgtk window: iwd has stopped running\n");
    }

    if (global.manager != NULL) {
	g_object_unref(global.manager);
	global.manager = NULL;
    }

    if (global.state & WINDOW_LAUNCH_PENDING) {
	global.state &= ~WINDOW_LAUNCH_PENDING;
	g_application_release(G_APPLICATION(global.application));
	g_printerr("Cannot launch iwgtk window: iwd is not running\n");
    }
}

static void config_set_color(GKeyFile *conf, const gchar *group, const gchar *key, GdkRGBA *color) {
    gchar *value;

    value = g_key_file_get_string(conf, group, key, NULL);

    if (value) {
	if (!gdk_rgba_parse(color, value)) {
	    g_printerr("Configuration file contains invalid color value: %s.%s=%s\n", group, key, value);
	}

	g_free(value);
    }
}

static gint config_get_int(GKeyFile *conf, const gchar *group, const gchar *key, gint value_default) {
    gint value;
    GError *err;

    err = NULL;
    value = g_key_file_get_integer(conf, group, key, &err);

    if (err) {
	g_error_free(err);
	return value_default;
    }

    return value;
}

static gint config_get_bool(GKeyFile *conf, const gchar *group, const gchar *key, gboolean value_default) {
    gboolean value;
    GError *err;

    err = NULL;
    value = g_key_file_get_boolean(conf, group, key, &err);

    if (err) {
	g_error_free(err);
	return value_default;
    }

    return value;
}

static void config_set_values(GKeyFile *conf) {
    config_set_color(conf, "indicator.colors.station", "connected", &colors.station_connected);
    config_set_color(conf, "indicator.colors.station", "connecting", &colors.station_connecting);
    config_set_color(conf, "indicator.colors.station", "disconnected", &colors.station_disconnected);

    config_set_color(conf, "indicator.colors.ap", "up", &colors.ap_up);
    config_set_color(conf, "indicator.colors.ap", "down", &colors.ap_down);

    config_set_color(conf, "indicator.colors.adhoc", "up", &colors.adhoc_up);
    config_set_color(conf, "indicator.colors.adhoc", "down", &colors.adhoc_down);

    config_set_color(conf, "indicator.colors.disabled", "device", &colors.disabled_device);
    config_set_color(conf, "indicator.colors.disabled", "adapter", &colors.disabled_adapter);

    config_set_color(conf, "network.colors", "connected", &colors.network_connected);
    config_set_color(conf, "network.colors", "connecting", &colors.network_connecting);
    config_set_color(conf, "network.colors", "known", &colors.network_known);
    config_set_color(conf, "network.colors", "unknown", &colors.network_unknown);
    config_set_color(conf, "network.colors", "hidden", &colors.network_hidden);

    if (global.last_connection_time_fmt) {
	g_free(global.last_connection_time_fmt);
    }

    global.last_connection_time_fmt = g_key_file_get_string(conf, "known-network", "last-connection-time.format", NULL);

    global.width = config_get_int(conf, "window", "width", 440);
    global.height = config_get_int(conf, "window", "height", 600);

    if (config_get_bool(conf, "window", "dark", FALSE)) {
	g_object_set(gtk_settings_get_default(), "gtk-application-prefer-dark-theme", TRUE, NULL);
    }

    if (config_get_bool(conf, "window", "show-hidden-networks", FALSE)) {
	global.state |= SHOW_HIDDEN_NETWORKS;
    }
}

static gboolean config_read_file(const gchar *path, GKeyFile *key_file) {
    GError *err;
    gboolean conf_loaded;

    err = NULL;
    conf_loaded = g_key_file_load_from_file(key_file, path, G_KEY_FILE_NONE, &err);

    if (err) {
	/*
	 * Only show parser errors (G_KEY_FILE_ERROR_*). Silently ignore "file not
	 * found" and other OS errors (G_FILE_ERROR_*).
	 */

	if (err->domain == G_KEY_FILE_ERROR) {
	    g_printerr("Failed to parse configuration file '%s': %s\n", path, err->message);
	}

	g_error_free(err);
    }

    return conf_loaded;
}

static void config_load_attempt() {
    GKeyFile *key_file;
    gchar *user_conf;

    key_file = g_key_file_new();
    user_conf = g_strconcat(g_get_user_config_dir(), "/iwgtk.conf", NULL);

    if (config_read_file(user_conf, key_file) || config_read_file(SYSCONFDIR "/iwgtk.conf", key_file)) {
	config_set_values(key_file);
    }

    g_free(user_conf);
    g_key_file_free(key_file);
}

void startup(GtkApplication *app) {
    {
	volatile gsize error_domain_volatile;

	g_dbus_error_register_error_domain("iwd-error-quark", &error_domain_volatile, iwd_error_codes, G_N_ELEMENTS(iwd_error_codes));
	global.iwd_error_domain = (GQuark) error_domain_volatile;
    }

    icon_theme_set();
    config_load_attempt();

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
	puts(PACKAGE " " VERSION);
	return 0;
    }

    return -1;
}

gint command_line(GApplication *application, GApplicationCommandLine *command_line) {
    GVariantDict *options;

    options = g_application_command_line_get_options_dict(command_line);

    if (g_variant_dict_contains(options, "notifications")) {
	global.state &= ~NOTIFICATIONS_DISABLE;
    }

    if (g_variant_dict_contains(options, "no-notifications")) {
	global.state |= NOTIFICATIONS_DISABLE;
    }

    if (g_variant_dict_contains(options, "indicators")) {
	if (global.state & INDICATOR_DAEMON) {
	    g_printerr("Indicator daemon is already running\n");
	}
	else {
	    global.state |= INDICATOR_DAEMON;
	    g_application_hold(application);

	    if (global.manager) {
		add_all_dbus_objects(NULL);
	    }
	}
    }
    else {
	window_launch();
    }

    return 0;
}

int main (int argc, char **argv) {
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(PACKAGE, "UTF-8");
    textdomain(PACKAGE);

    global.application = gtk_application_new(APPLICATION_ID, G_APPLICATION_HANDLES_COMMAND_LINE);

    g_application_set_option_context_summary(G_APPLICATION(global.application), _("iwgtk is a wireless networking GUI."));
    g_application_add_main_option_entries(G_APPLICATION(global.application), command_options);

    g_signal_connect(global.application, "startup", G_CALLBACK(startup), NULL);
    g_signal_connect(global.application, "handle-local-options", G_CALLBACK(handle_local_options), NULL);
    g_signal_connect(global.application, "command-line", G_CALLBACK(command_line), NULL);

    {
	int status;

	status = g_application_run(G_APPLICATION(global.application), argc, argv);
	g_object_unref(global.application);
	return status;
    }
}
