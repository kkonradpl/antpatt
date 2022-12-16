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

#ifndef ANTPATT_PATTERN_SIGNAL_H_
#define ANTPATT_PATTERN_SIGNAL_H_

#define PATTERN_SIGNAL_MIN_AVG 0
#define PATTERN_SIGNAL_MAX_AVG 10

enum
{
    PATTERN_INTERP_LINEAR = 0,
    PATTERN_INTERP_AKIMA,
    PATTERN_INTERP_AKIMA_CLIPPED,
    PATTERN_INTERP_N
};

typedef struct pattern_signal pattern_signal_t;

pattern_signal_t* pattern_signal_new(void);
void pattern_signal_free(pattern_signal_t *s);
gint pattern_signal_count(pattern_signal_t*);
gint pattern_signal_interp(pattern_signal_t*);
void pattern_signal_push(pattern_signal_t*, gdouble);

gdouble  pattern_signal_get_sample(pattern_signal_t*, gint);
gdouble  pattern_signal_get_sample_raw(pattern_signal_t*, gint);
gdouble  pattern_signal_get_sample_interp(pattern_signal_t*, gint, gdouble);

gdouble  pattern_signal_get_peak(pattern_signal_t*);
void     pattern_signal_set_peak(pattern_signal_t*, gdouble);

gboolean pattern_signal_get_rev(pattern_signal_t*);
void     pattern_signal_set_rev(pattern_signal_t*, gboolean);

gint     pattern_signal_get_avg(pattern_signal_t*);
void     pattern_signal_set_avg(pattern_signal_t*, gint);

gint     pattern_signal_get_interp(pattern_signal_t*);
void     pattern_signal_set_interp(pattern_signal_t*, gint);

gboolean pattern_signal_get_finished(pattern_signal_t*);
void     pattern_signal_set_finished(pattern_signal_t*);

gint     pattern_signal_get_rotate(pattern_signal_t*);
void     pattern_signal_set_rotate(pattern_signal_t*, gint);
void     pattern_signal_rotate(pattern_signal_t*, gint);
void     pattern_signal_rotate_0(pattern_signal_t*);
void     pattern_signal_rotate_reset(pattern_signal_t*);

#endif
