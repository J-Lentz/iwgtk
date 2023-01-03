/*
 *  Copyright 2020-2023 Jesse Lentz and contributors
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
#include <qrencode.h>

#define QR_CODE_MARGIN 4

void qrcode_draw(GtkDrawingArea *area, cairo_t *cr, int width, int height, cairo_surface_t *qr_surface) {
    cairo_pattern_t *qr_pattern;

    {
	float sx, sy;

	sx = (float) cairo_image_surface_get_width(qr_surface) / (float) width;
	sy = (float) cairo_image_surface_get_height(qr_surface) / (float) height;
	cairo_surface_set_device_scale(qr_surface, sx, sy);
    }

    qr_pattern = cairo_pattern_create_for_surface(qr_surface);
    cairo_pattern_set_filter(qr_pattern, CAIRO_FILTER_NEAREST);
    cairo_set_source(cr, qr_pattern);
    cairo_pattern_destroy(qr_pattern);

    cairo_paint(cr);
}

GtkWidget* qrcode_widget_new(const gchar *uri) {
    QRcode *qrcode;
    cairo_surface_t *qr_surface;
    uint32_t *qr_data;
    gsize width_data, stride, stride_n;
    int width_surface;

    qrcode = QRcode_encodeString8bit(uri, 0, QR_ECLEVEL_L);

    if (!qrcode) {
	g_printerr("Failed to generate QR code\n");
	return NULL;
    }

    width_data = qrcode->width;
    width_surface = width_data + 2*QR_CODE_MARGIN;
    qr_surface = cairo_image_surface_create(CAIRO_FORMAT_A1, width_surface, width_surface);

    stride = cairo_image_surface_get_stride(qr_surface);
    g_assert(stride % 4 == 0);
    stride_n = stride / 4;

    qr_data = (uint32_t *) cairo_image_surface_get_data(qr_surface);
    cairo_surface_flush(qr_surface);

    for (int i = 0; i < width_data; i ++) {
	for (int j = 0; j < width_data; j ++) {
	    if (qrcode->data[i*width_data + j] & 0x01) {
		int index, bit, is, js;

		is = i + QR_CODE_MARGIN;
		js = j + QR_CODE_MARGIN;

		index = is*stride_n + js/32;
		bit = js % 32;

		qr_data[index] |= (0x00000001 << bit);
	    }
	}
    }

    cairo_surface_mark_dirty(qr_surface);
    QRcode_free(qrcode);

    {
	GtkWidget *area;

	area = gtk_drawing_area_new();

	gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(area), PROVISION_MENU_WIDTH);
	gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(area), PROVISION_MENU_WIDTH);

	{
	    GtkStyleContext *context;
	    GtkCssProvider *provider;

	    context = gtk_widget_get_style_context(area);

	    provider = gtk_css_provider_new();
	    gtk_css_provider_load_from_data(provider, "* {background: white;}", -1);
	    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER + 1);
	    g_object_unref(provider);
	}

	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(area), (GtkDrawingAreaDrawFunc) qrcode_draw, qr_surface, (GDestroyNotify) cairo_surface_destroy);

	return area;
    }
}

void dpp_qrcode_add(DPP *dpp) {
    {
	GVariant *uri_var;
	const gchar *uri;

	uri_var = g_dbus_proxy_get_cached_property(dpp->proxy, "URI");
	uri = g_variant_get_string(uri_var, NULL);
	dpp->qrcode = qrcode_widget_new(uri);

	g_variant_unref(uri_var);
    }

    gtk_widget_set_hexpand(dpp->qrcode, FALSE);
    gtk_widget_set_halign(dpp->qrcode, GTK_ALIGN_CENTER);

    {
	GVariant *role_var;
	const gchar *role, *tooltip;

	role_var = g_dbus_proxy_get_cached_property(dpp->proxy, "Role");
	role = g_variant_get_string(role_var, NULL);

	if (strcmp(role, "enrollee") == 0) {
	    tooltip = _("Get network credentials");
	}
	else {
	    tooltip = _("Share network credentials");
	}

	g_variant_unref(role_var);
	gtk_widget_set_tooltip_text(dpp->qrcode, tooltip);
    }

    gtk_box_insert_child_after(GTK_BOX(dpp->station->provision_vbox), dpp->qrcode, dpp->button);
}

void dpp_button_press(DPP *dpp) {
    g_dbus_proxy_call(
	dpp->proxy,
	dpp->button_method,
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) method_call_log,
	"Failed to start or stop DPP: %s\n");
}

void dpp_set(DPP *dpp) {
    gboolean started;

    {
	GVariant *started_var;

	started_var = g_dbus_proxy_get_cached_property(dpp->proxy, "Started");
	started = g_variant_get_boolean(started_var);
	g_variant_unref(started_var);
    }

    if (started) {
	if (!dpp->qrcode) {
	    dpp_qrcode_add(dpp);
	}

	gtk_button_set_label(GTK_BUTTON(dpp->button), _("Stop"));
	dpp->button_method = "Stop";
    }
    else {
	if (dpp->qrcode) {
	    gtk_box_remove(GTK_BOX(dpp->station->provision_vbox), dpp->qrcode);
	    dpp->qrcode = NULL;
	}

	if (dpp->station->state == STATION_CONNECTED) {
	    gtk_button_set_label(GTK_BUTTON(dpp->button), _("Share credentials"));
	    dpp->button_method = "StartConfigurator";
	}
	else {
	    gtk_button_set_label(GTK_BUTTON(dpp->button), _("Get credentials"));
	    dpp->button_method = "StartEnrollee";
	}
    }
}

DPP* dpp_add(Window *window, GDBusObject *object, GDBusProxy *proxy) {
    DPP *dpp;

    dpp = g_malloc(sizeof(DPP));
    dpp->proxy = proxy;
    dpp->qrcode = NULL;

    dpp->label = new_label_bold(_("Easy Connect"));
    g_object_ref_sink(dpp->label);
    gtk_widget_set_margin_top(dpp->label, 10);

    dpp->button = gtk_button_new();
    g_object_ref_sink(dpp->button);
    g_signal_connect_swapped(dpp->button, "clicked", G_CALLBACK(dpp_button_press), dpp);

    couple_register(window, STATION_DPP, 1, dpp, object);
    return dpp;
}

void dpp_remove(Window *window, DPP *dpp) {
    couple_unregister(window, STATION_DPP, 1, dpp);

    g_object_unref(dpp->label);
    g_object_unref(dpp->button);
    g_free(dpp);
}

void bind_station_dpp(Station *station, DPP *dpp) {
    station->dpp = dpp;
    dpp->station = station;

    {
	GtkWidget *hidden;

	hidden = gtk_widget_get_first_child(station->provision_vbox);
	gtk_box_insert_child_after(GTK_BOX(station->provision_vbox), dpp->button, hidden);
	gtk_box_insert_child_after(GTK_BOX(station->provision_vbox), dpp->label, hidden);
    }

    dpp->handler_update = g_signal_connect_swapped(dpp->proxy, "g-properties-changed", G_CALLBACK(dpp_set), dpp);
    dpp_set(dpp);
}

void unbind_station_dpp(Station *station, DPP *dpp) {
    station->dpp = NULL;
    g_signal_handler_disconnect(dpp->proxy, dpp->handler_update);

    if (dpp->qrcode) {
	gtk_box_remove(GTK_BOX(station->provision_vbox), dpp->qrcode);
	dpp->qrcode = NULL;
    }

    gtk_box_remove(GTK_BOX(station->provision_vbox), dpp->label);
    gtk_box_remove(GTK_BOX(station->provision_vbox), dpp->button);
}
