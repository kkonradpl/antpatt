/*
 *  antpatt - antenna pattern plotting and analysis software
 *  Copyright (c) 2017  Konrad Kosmatka
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

#ifndef ANTPATT_PATTERN_DATA_H_
#define ANTPATT_PATTERN_DATA_H_
#include "pattern-signal.h"

typedef struct pattern_data pattern_data_t;

pattern_data_t* pattern_data_new(pattern_signal_t*);
void            pattern_data_free(pattern_data_t*);

pattern_signal_t* pattern_data_get_signal(pattern_data_t*);
const gchar*      pattern_data_get_name(pattern_data_t*);
void              pattern_data_set_name(pattern_data_t*, const gchar*);
gint              pattern_data_get_freq(pattern_data_t*);
void              pattern_data_set_freq(pattern_data_t*, gint);
GdkColor*         pattern_data_get_color(pattern_data_t*);
void              pattern_data_set_color(pattern_data_t*, const GdkColor*);
gboolean          pattern_data_get_hide(pattern_data_t*);
void              pattern_data_set_hide(pattern_data_t*, gboolean);
gboolean          pattern_data_get_fill(pattern_data_t*);
void              pattern_data_set_fill(pattern_data_t*, gboolean);

#endif
