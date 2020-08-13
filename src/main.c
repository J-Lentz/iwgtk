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
	"disable-notifications",
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

static const char *version_string = "iwgtk 0.1";

void known_network_table_show(GtkToggleButton *button) {
    if (gtk_toggle_button_get_active(button)) {
	bin_empty(GTK_BIN(global.main));
	gtk_container_add(GTK_CONTAINER(global.main), global.known_network_table);
    }
}

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

    /*
     * Create window
     */

    global.master = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    g_object_ref_sink(global.master);
    gtk_container_add(GTK_CONTAINER(global.window), global.master);

    global.header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    g_object_ref_sink(global.header);
    gtk_box_pack_start(GTK_BOX(global.master), global.header, FALSE, FALSE, 0);

    gtk_widget_set_margin_start(global.header, 5);
    gtk_widget_set_margin_end(global.header, 5);
    gtk_widget_set_margin_top(global.header, 5);

    gtk_box_pack_start(GTK_BOX(global.master), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 0);

    global.main = gtk_scrolled_window_new(NULL, NULL);
    g_object_ref_sink(global.main);
    gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(global.main), TRUE);
    gtk_box_pack_start(GTK_BOX(global.master), global.main, FALSE, FALSE, 0);

    global.known_network_button = gtk_radio_button_new_with_label(NULL, "Known Networks");
    g_object_ref_sink(global.known_network_button);
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(global.known_network_button), FALSE);

    {
	GtkWidget *known_network_button_vbox;

	known_network_button_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_end(GTK_BOX(global.header), known_network_button_vbox, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(known_network_button_vbox), global.known_network_button, FALSE, FALSE, 0);
    }

    g_signal_connect(global.known_network_button, "toggled", G_CALLBACK(known_network_table_show), NULL);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(global.known_network_button), TRUE);

    global.known_network_table = gtk_grid_new();
    g_object_ref_sink(global.known_network_table);
    gtk_container_add(GTK_CONTAINER(global.main), global.known_network_table);

    gtk_widget_set_margin_start(global.known_network_table, 5);
    gtk_widget_set_margin_end(global.known_network_table, 5);
    gtk_widget_set_margin_bottom(global.known_network_table, 5);

    gtk_grid_attach(GTK_GRID(global.known_network_table), new_label_bold("SSID"),            0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(global.known_network_table), new_label_bold("Security"),        1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(global.known_network_table), new_label_bold("Autoconnect"),     2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(global.known_network_table), new_label_bold("Forget"),          3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(global.known_network_table), new_label_bold("Last connection"), 4, 0, 1, 1);

    gtk_grid_set_column_spacing(GTK_GRID(global.known_network_table), 10);
    gtk_grid_set_row_spacing(GTK_GRID(global.known_network_table), 10);

    gtk_widget_show_all(global.window);

    /*
     * Process D-Bus objects
     */

    g_signal_connect(global.manager, "interface-added",   G_CALLBACK(interface_add),    NULL);
    g_signal_connect(global.manager, "interface-removed", G_CALLBACK(interface_remove), NULL);
    g_signal_connect(global.manager, "object-added",      G_CALLBACK(object_add),       NULL);
    g_signal_connect(global.manager, "object-removed",    G_CALLBACK(object_remove),    NULL);

    {
	GList *object_list, *i;

	object_list = g_dbus_object_manager_get_objects(global.manager);
	for (i = object_list; i != NULL; i = i->next) {
	    GDBusObject *object;

	    object = (GDBusObject *) i->data;
	    object_iterate_interfaces(object, interface_add);
	    g_object_unref(object);
	}
	g_list_free(object_list);
    }

    /*
     * Register agent to prompt for passwords, etc.
     */

    {
	GDBusProxy *agent_manager;
	agent_manager = G_DBUS_PROXY(g_dbus_object_manager_get_interface(global.manager, IWD_PATH_AGENT_MANAGER, IWD_IFACE_AGENT_MANAGER));
	agent_register(agent_manager);
    }
}

void iwd_up_handler(GDBusConnection *conn, const gchar *name, const gchar *name_owner) {
    bin_empty(GTK_BIN(global.window));
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

    bin_empty(GTK_BIN(global.window));

    {
	GtkWidget *label;

	label = gtk_label_new("iwd is not running");
	gtk_container_add(GTK_CONTAINER(global.window), label);
    }

    gtk_widget_show_all(global.window);
}

static void activate(GtkApplication *app) {
    {
	volatile gsize error_domain_volatile;

	g_dbus_error_register_error_domain("iwd-error-quark", &error_domain_volatile, iwd_error_codes, G_N_ELEMENTS(iwd_error_codes));
	global.iwd_error_domain = (GQuark) error_domain_volatile;
    }

    global.window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(global.window), "iwgtk");
    gtk_window_set_default_size(GTK_WINDOW(global.window), 600, 700);
    gtk_window_set_position(GTK_WINDOW(global.window), GTK_WIN_POS_CENTER);

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
    puts(version_string);
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

    global.application = gtk_application_new("application.iwgtk", G_APPLICATION_NON_UNIQUE);
    g_signal_connect(global.application, "activate", G_CALLBACK(activate), NULL);

    {
	int status;

	status = g_application_run(G_APPLICATION(global.application), argc, argv);
	g_object_unref(global.application);
	return status;
    }
}
