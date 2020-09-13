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

#ifndef _IWGTK_INDICATOR_H
#define _IWGTK_INDICATOR_H

typedef struct Indicator_s {
    GDBusProxy *proxy;
    StatusNotifierItem *sni;
    gulong update_handler;

    struct Indicator_s *next;
} Indicator;

typedef void (*IndicatorSetter) (Indicator *indicator);

Indicator* indicator_new(GDBusProxy *proxy, IndicatorSetter indicator_set);
void indicator_rm(Indicator *indicator);

void indicator_set_station(Indicator *indicator);
void indicator_set_ap(Indicator *indicator);
void indicator_set_adhoc(Indicator *indicator);

void indicator_activate(GDBusObject *device_object);

#endif
