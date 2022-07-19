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

void dpp_qrcode_add(GDBusProxy *proxy, GAsyncResult *res, DPP *dpp) {
    GVariant *ret;
    GError *err;

    err = NULL;
    ret = g_dbus_proxy_call_finish(proxy, res, &err);

    if (ret) {
	{
	    const gchar *uri;

	    g_variant_get(ret, "(s)", &uri);
	    dpp->qrcode = qrcode_widget_new(uri);
	    g_variant_unref(ret);
	}

	gtk_widget_set_hexpand(dpp->qrcode, FALSE);
	gtk_widget_set_halign(dpp->qrcode, GTK_ALIGN_CENTER);

	{
	    const gchar *tooltip;

	    if (dpp->mode == DPP_MODE_ENROLLEE) {
		tooltip = _("Get network credentials");
	    }
	    else {
		tooltip = _("Share network credentials");
	    }

	    gtk_widget_set_tooltip_text(dpp->qrcode, tooltip);
	}

	dpp_set(dpp);
	gtk_box_insert_child_after(GTK_BOX(dpp->station->provision_vbox), dpp->qrcode, dpp->button);
    }
    else {
	g_printerr("DPP enrollment failed: %s\n", err->message);
	g_error_free(err);
    }
}

/*
 * We have no ability to see whether DPP is already started, so issue Stop method calls
 * before starting to avoid InProgress errors.
 */

void dpp_start_enrollee(DPP *dpp) {
    dpp_stop(dpp);
    dpp->mode = DPP_MODE_ENROLLEE;

    g_dbus_proxy_call(
	dpp->proxy,
	"StartEnrollee",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) dpp_qrcode_add,
	dpp);
}

void dpp_start_configurator(DPP *dpp) {
    dpp_stop(dpp);
    dpp->mode = DPP_MODE_CONFIGURATOR;

    g_dbus_proxy_call(
	dpp->proxy,
	"StartConfigurator",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) dpp_qrcode_add,
	dpp);
}

void dpp_stop(DPP *dpp) {
    if (dpp->qrcode) {
	gtk_box_remove(GTK_BOX(dpp->station->provision_vbox), dpp->qrcode);
	dpp->qrcode = NULL;
	dpp_set(dpp);
    }

    g_dbus_proxy_call(
	dpp->proxy,
	"Stop",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) method_call_log,
	"Failed to cancel DPP enrollment: %s\n");
}

void dpp_set(DPP *dpp) {
    if (dpp->handler != 0) {
	g_signal_handler_disconnect(dpp->button, dpp->handler);
    }

    if (dpp->qrcode != NULL) {
	gtk_button_set_label(GTK_BUTTON(dpp->button), _("Stop"));
	dpp->handler = g_signal_connect_swapped(dpp->button, "clicked", G_CALLBACK(dpp_stop), dpp);
    }
    else if (dpp->station->state == STATION_CONNECTED) {
	gtk_button_set_label(GTK_BUTTON(dpp->button), _("Share credentials"));
	dpp->handler = g_signal_connect_swapped(dpp->button, "clicked", G_CALLBACK(dpp_start_configurator), dpp);
    }
    else {
	gtk_button_set_label(GTK_BUTTON(dpp->button), _("Get credentials"));
	dpp->handler = g_signal_connect_swapped(dpp->button, "clicked", G_CALLBACK(dpp_start_enrollee), dpp);
    }
}

DPP* dpp_add(Window *window, GDBusObject *object, GDBusProxy *proxy) {
    DPP *dpp;

    dpp = g_malloc(sizeof(DPP));
    dpp->proxy = proxy;
    dpp->handler = 0;
    dpp->qrcode = NULL;

    dpp->label = new_label_bold(_("Easy Connect"));
    g_object_ref_sink(dpp->label);
    gtk_widget_set_margin_top(dpp->label, 10);

    dpp->button = gtk_button_new();
    g_object_ref_sink(dpp->button);

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

    dpp_set(dpp);

    {
	GtkWidget *hidden;

	hidden = gtk_widget_get_first_child(station->provision_vbox);
	gtk_box_insert_child_after(GTK_BOX(station->provision_vbox), dpp->button, hidden);
	gtk_box_insert_child_after(GTK_BOX(station->provision_vbox), dpp->label, hidden);
    }
}

void unbind_station_dpp(Station *station, DPP *dpp) {
    station->dpp = NULL;

    if (dpp->qrcode) {
	gtk_box_remove(GTK_BOX(station->provision_vbox), dpp->qrcode);
	dpp->qrcode = NULL;
    }

    gtk_box_remove(GTK_BOX(station->provision_vbox), dpp->label);
    gtk_box_remove(GTK_BOX(station->provision_vbox), dpp->button);
}
