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

#define QR_CODE_SIZE 250

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
    gsize width, stride, stride_n;

    qrcode = QRcode_encodeString8bit(uri, 0, QR_ECLEVEL_L);

    if (!qrcode) {
	g_printerr("Error generating QR code\n");
	return NULL;
    }

    width = qrcode->width;
    qr_surface = cairo_image_surface_create(CAIRO_FORMAT_A1, width, width);

    stride = cairo_image_surface_get_stride(qr_surface);
    g_assert(stride % 4 == 0);
    stride_n = stride / 4;

    qr_data = (uint32_t *) cairo_image_surface_get_data(qr_surface);
    cairo_surface_flush(qr_surface);

    for (int i = 0; i < width; i ++) {
	for (int j = 0; j < width; j ++) {
	    if (qrcode->data[i*width + j] & 0x01) {
		int index, bit;

		index = i*stride_n + j/32;
		bit = j % 32;

		qr_data[index] |= (0x00000001 << bit);
	    }
	}
    }

    cairo_surface_mark_dirty(qr_surface);
    QRcode_free(qrcode);

    {
	GtkWidget *area;

	area = gtk_drawing_area_new();

	gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(area), QR_CODE_SIZE);
	gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(area), QR_CODE_SIZE);

	{
	    int margin;

	    margin = 4 * QR_CODE_SIZE / width;

	    gtk_widget_set_margin_start(area, margin);
	    gtk_widget_set_margin_end(area, margin);
	    gtk_widget_set_margin_top(area, margin);
	    gtk_widget_set_margin_bottom(area, margin);
	}

	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(area), (GtkDrawingAreaDrawFunc) qrcode_draw, qr_surface, (GDestroyNotify) cairo_surface_destroy);

	return area;
    }
}

void dpp_window_launch(GDBusProxy *proxy, GAsyncResult *res, DPP *dpp) {
    GVariant *ret;
    GError *err;

    err = NULL;
    ret = g_dbus_proxy_call_finish(proxy, res, &err);

    if (ret) {
	const gchar *uri;

	g_variant_get(ret, "(s)", &uri);

	{
	    GtkWidget *window;
	    GtkWidget *vbox;
	    GtkWidget *close;
	    GtkWidget *qrcode;

	    window = gtk_window_new();
	    gtk_window_set_icon_name(GTK_WINDOW(window), "iwgtk");

	    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	    gtk_window_set_child(GTK_WINDOW(window), vbox);

	    {
		const gchar *title;

		if (dpp->mode == DPP_MODE_ENROLLEE) {
		    title = "Get network credentials";
		}
		else {
		    title = "Share network credentials";
		}

		gtk_window_set_title(GTK_WINDOW(window), title);
	    }

	    qrcode = qrcode_widget_new(uri);
	    gtk_box_append(GTK_BOX(vbox), qrcode);
	    g_signal_connect_swapped(window, "destroy", G_CALLBACK(dpp_stop), dpp);
	    gtk_widget_set_hexpand(qrcode, FALSE);
	    gtk_widget_set_halign(qrcode, GTK_ALIGN_CENTER);

	    close = gtk_button_new_with_label("Close");
	    gtk_box_append(GTK_BOX(vbox), close);
	    g_signal_connect_swapped(close, "clicked", G_CALLBACK(gtk_window_destroy), window);
	    gtk_widget_set_hexpand(close, FALSE);
	    gtk_widget_set_halign(close, GTK_ALIGN_CENTER);

	    gtk_widget_show(window);
	}

	g_variant_unref(ret);
    }
    else {
	g_printerr("DPP enrollment failed: %s\n", err->message);
	g_error_free(err);
    }
}

void dpp_start_enrollee(DPP *dpp) {
    dpp->mode = DPP_MODE_ENROLLEE;

    g_dbus_proxy_call(
	dpp->proxy,
	"StartEnrollee",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) dpp_window_launch,
	dpp);
}

void dpp_start_configurator(DPP *dpp) {
    dpp->mode = DPP_MODE_CONFIGURATOR;

    g_dbus_proxy_call(
	dpp->proxy,
	"StartConfigurator",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) dpp_window_launch,
	dpp);
}

void dpp_stop(DPP *dpp) {
    g_dbus_proxy_call(
	dpp->proxy,
	"Stop",
	NULL,
	G_DBUS_CALL_FLAGS_NONE,
	-1,
	NULL,
	(GAsyncReadyCallback) method_call_log,
	"Error canceling DPP enrollment: %s\n");
}

DPP* dpp_add(Window *window, GDBusObject *object, GDBusProxy *proxy) {
    DPP *dpp;

    dpp = g_malloc(sizeof(DPP));
    dpp->proxy = proxy;

    dpp->label = new_label_bold("DPP");
    g_object_ref_sink(dpp->label);
    gtk_widget_set_margin_top(dpp->label, 10);

    dpp->start_enrollee = gtk_button_new_with_label("Get credentials");
    g_object_ref_sink(dpp->start_enrollee);

    dpp->start_configurator = gtk_button_new_with_label("Share credentials");
    g_object_ref_sink(dpp->start_configurator);

    g_signal_connect_swapped(dpp->start_enrollee, "clicked", G_CALLBACK(dpp_start_enrollee), dpp);
    g_signal_connect_swapped(dpp->start_configurator, "clicked", G_CALLBACK(dpp_start_configurator), dpp);

    couple_register(window, STATION_DPP, 1, dpp, object);
    return dpp;
}

void dpp_remove(Window *window, DPP *dpp) {
    couple_unregister(window, STATION_DPP, 1, dpp);

    g_object_unref(dpp->label);
    g_object_unref(dpp->start_enrollee);
    g_object_unref(dpp->start_configurator);
    g_free(dpp);
}

void bind_station_dpp(Station *station, DPP *dpp) {
    station->dpp = dpp;
    station_dpp_set(station);

    {
	GtkWidget *hidden;

	hidden = gtk_widget_get_first_child(station->provision_vbox);
	gtk_box_insert_child_after(GTK_BOX(station->provision_vbox), dpp->start_configurator, hidden);
	gtk_box_insert_child_after(GTK_BOX(station->provision_vbox), dpp->start_enrollee, hidden);
	gtk_box_insert_child_after(GTK_BOX(station->provision_vbox), dpp->label, hidden);
    }
}

void unbind_station_dpp(Station *station, DPP *dpp) {
    station->dpp = NULL;
    gtk_box_remove(GTK_BOX(station->provision_vbox), dpp->label);
    gtk_box_remove(GTK_BOX(station->provision_vbox), dpp->start_enrollee);
    gtk_box_remove(GTK_BOX(station->provision_vbox), dpp->start_configurator);
}
