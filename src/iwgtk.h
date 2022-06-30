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

#ifndef _IWGTK_H
#define _IWGTK_H

#include <gtk/gtk.h>
#include <gio/gio.h>

#define VERSION_STRING "iwgtk 0.7"
#define APPLICATION_ID "org.twosheds.iwgtk"

#define IWD_BUS_NAME "net.connman.iwd"

#define IWD_PATH_OBJECT_MANAGER "/"
#define IWD_PATH_AGENT_MANAGER  "/net/connman/iwd"
#define IWD_PATH_PREFIX_LENGTH 17

#define IWGTK_PATH_AGENT        "/Agent"

#define IWD_IFACE_ADAPTER            "net.connman.iwd.Adapter"
#define IWD_IFACE_DEVICE             "net.connman.iwd.Device"
#define IWD_IFACE_STATION            "net.connman.iwd.Station"
#define IWD_IFACE_DIAGNOSTIC         "net.connman.iwd.StationDiagnostic"
#define IWD_IFACE_AP                 "net.connman.iwd.AccessPoint"
#define IWD_IFACE_AD_HOC             "net.connman.iwd.AdHoc"
#define IWD_IFACE_DPP                "net.connman.iwd.DeviceProvisioning"
#define IWD_IFACE_WPS                "net.connman.iwd.SimpleConfiguration"
#define IWD_IFACE_NETWORK            "net.connman.iwd.Network"
#define IWD_IFACE_KNOWN_NETWORK      "net.connman.iwd.KnownNetwork"
#define IWD_IFACE_AGENT_MANAGER      "net.connman.iwd.AgentManager"
#define IWD_IFACE_AGENT              "net.connman.iwd.Agent"
#define IWD_IFACE_SIGNAL_LEVEL_AGENT "net.connman.iwd.SignalLevelAgent"

typedef enum {
    ERROR_OTHER = 0,
    IWD_ERROR_BUSY,
    IWD_ERROR_FAILED,
    IWD_ERROR_INVALID_ARGUMENTS,
    IWD_ERROR_ALREADY_EXISTS,
    IWD_ERROR_NOT_CONNECTED,
    IWD_ERROR_NOT_CONFIGURED,
    IWD_ERROR_NOT_FOUND,
    IWD_ERROR_SERVICE_SET_OVERLAP,
    IWD_ERROR_ALREADY_PROVISIONED,
    IWD_ERROR_NOT_HIDDEN,
    IWD_ERROR_ABORTED,
    IWD_ERROR_NO_AGENT,
    IWD_ERROR_NOT_SUPPORTED,
    IWD_ERROR_TIMEOUT,
    IWD_ERROR_IN_PROGRESS,
    IWD_ERROR_WSC_SESSION_OVERLAP,
    IWD_ERROR_WSC_NO_CREDENTIALS,
    IWD_ERROR_WSC_TIME_EXPIRED,
    IWD_ERROR_WSC_WALK_TIME_EXPIRED,
    IWD_ERROR_WSC_NOT_REACHABLE,
    IWD_ERROR_INVALID_FORMAT,
    IWD_ERROR_NOT_AVAILABLE,
    IWD_ERROR_AGENT_CANCELED
} IWDError;

typedef struct ErrorMessage_s ErrorMessage;
typedef struct CallbackMessages_s CallbackMessages;

struct ErrorMessage_s {
    int code;
    const gchar *message;
};

/*
 * If no message is desired on success/failure, set success_message and/or
 * failure_message to NULL.
 *
 * The last entry in error_table must have error code 0.
 * If no message is desired in this case, its message should be NULL.
 */

struct CallbackMessages_s {
    const gchar *success;
    const gchar *failure;
    const ErrorMessage *error_table;
    gboolean free;
};

#include "sni.h"
#include "indicator.h"
#include "window.h"
#include "main.h"
#include "utilities.h"
#include "icon.h"

#include "dialog.h"
#include "device.h"
#include "adapter.h"
#include "station.h"
#include "ap.h"
#include "adhoc.h"
#include "dpp.h"
#include "wps.h"
#include "diagnostic.h"
#include "known_network.h"
#include "agent.h"
#include "network.h"
#include "hidden.h"

#include "switch.h"

#endif
