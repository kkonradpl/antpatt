/*
 *  antpatt - antenna pattern plotting and analysis software
 *  Copyright (c) 2017-2022  Konrad Kosmatka
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifndef ANTPATT_PATTERN_UI_H_
#define ANTPATT_PATTERN_UI_H_
#include "pattern.h"

void pattern_ui_create(pattern_t*);

void pattern_ui_sync_name(pattern_t*, gboolean);
void pattern_ui_sync_freq(pattern_t*, gboolean);
void pattern_ui_sync_avg(pattern_t*, gboolean);
void pattern_ui_sync_color(pattern_t*, gboolean);
void pattern_ui_sync_hide(pattern_t*, gboolean);
void pattern_ui_sync_fill(pattern_t*, gboolean);
void pattern_ui_sync_rev(pattern_t*, gboolean);

void pattern_ui_live(pattern_t*, gboolean);

#endif
