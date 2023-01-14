/*
 *  antpatt - antenna pattern plotting and analysis software
 *  Copyright (c) 2017-2023  Konrad Kosmatka
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

typedef struct pattern pattern_t;
typedef struct pattern_ui pattern_ui_t;

pattern_ui_t* pattern_ui(pattern_t*);

void pattern_ui_sync_name(pattern_ui_t*, gboolean);
void pattern_ui_sync_freq(pattern_ui_t*, gboolean);
void pattern_ui_sync_avg(pattern_ui_t*, gboolean);
void pattern_ui_sync_color(pattern_ui_t*, gboolean);
void pattern_ui_sync_hide(pattern_ui_t*, gboolean);
void pattern_ui_sync_fill(pattern_ui_t*, gboolean);
void pattern_ui_sync_rev(pattern_ui_t*, gboolean);
void pattern_ui_sync_data(pattern_ui_t*);

pattern_t* pattern_ui_get_pattern(pattern_ui_t*);
GtkWindow* pattern_ui_get_plot_window(pattern_ui_t*);

void pattern_ui_set_focus_idx(pattern_ui_t*, gint);
gint pattern_ui_get_focus_idx(const pattern_ui_t*);
void pattern_ui_set_rotating_idx(pattern_ui_t*, gint);
gint pattern_ui_get_rotating_idx(const pattern_ui_t*);

void pattern_ui_interactive(pattern_ui_t*, gboolean);

#endif
