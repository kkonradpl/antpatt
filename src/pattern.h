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

#ifndef ANTPATT_PATTERN_H_
#define ANTPATT_PATTERN_H_
#include "version.h"
#include "pattern-ui-window.h"
#include "pattern-data.h"

#define PATTERN_MIN_SIZE 350
#define PATTERN_MAX_SIZE 3000

#define PATTERN_MIN_LINE 0.1
#define PATTERN_MAX_LINE 2.0

enum
{
    PATTERN_COL_DATA = 0,
    PATTERN_COLS
};

typedef struct pattern pattern_t;

pattern_t* pattern_new(void);
void pattern_free(pattern_t*);
pattern_t* pattern_get_main_instance(void);

GtkListStore*        pattern_get_model(pattern_t*);
void                 pattern_set_ui(pattern_t*, pattern_ui_window_t*);
pattern_ui_window_t* pattern_get_ui(pattern_t*);

void pattern_add(pattern_t*, pattern_data_t*);
void pattern_remove(pattern_t*, GtkTreeIter*);
void pattern_clear(pattern_t*);

void            pattern_set_current(pattern_t*, pattern_data_t*);
pattern_data_t* pattern_get_current(pattern_t*);
void            pattern_set_size(pattern_t*, gint);
gint            pattern_get_size(pattern_t*);
void            pattern_set_title(pattern_t*, const gchar*);
const gchar*    pattern_get_title(pattern_t*);
void            pattern_set_scale(pattern_t*, gint);
gint            pattern_get_scale(pattern_t*);
void            pattern_set_line(pattern_t*, gdouble);
gdouble         pattern_get_line(pattern_t*);
void            pattern_set_interp(pattern_t*, gint);
gint            pattern_get_interp(pattern_t*);
void            pattern_set_full_angle(pattern_t*, gboolean);
gboolean        pattern_get_full_angle(pattern_t*);
void            pattern_set_black(pattern_t*, gboolean);
gboolean        pattern_get_black(pattern_t*);
void            pattern_set_normalize(pattern_t*, gboolean);
gboolean        pattern_get_normalize(pattern_t*);
void            pattern_set_legend(pattern_t*, gboolean);
gboolean        pattern_get_legend(pattern_t*);

void pattern_set_focus_idx(pattern_t*, gint);
gint pattern_get_focus_idx(pattern_t*);
void pattern_set_rotating_idx(pattern_t*, gint);
gint pattern_get_rotating_idx(pattern_t*);

gint pattern_get_visible(pattern_t*);
gdouble pattern_get_peak(pattern_t*);

void pattern_hide(pattern_t*, pattern_data_t*, gboolean);

void pattern_live_start(pattern_t*);
void pattern_live_stop(pattern_t*);

#endif
