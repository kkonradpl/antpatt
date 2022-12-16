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

#include <gtk/gtk.h>
#include <gsl/gsl_spline.h>
#include <math.h>
#include "pattern-signal.h"

typedef struct pattern_signal
{
    GArray *arr;
    gint count;
    gboolean finished;
    gdouble peak;
    gboolean rev;
    gint rotate;
    gint avg;
    gint interp;
    gsl_interp_accel *acc;
    gsl_spline *spline;
} pattern_signal_t;

static gdouble pattern_signal_sample(pattern_signal_t*, gint, gint);
static gint pattern_signal_idx(pattern_signal_t*, gint);
static void pattern_signal_interp_init(pattern_signal_t*);
static void pattern_signal_interp_invalidate(pattern_signal_t*);

pattern_signal_t*
pattern_signal_new()
{
    pattern_signal_t *s = g_malloc(sizeof(pattern_signal_t));
    s->arr = g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 360);
    s->count = 0;
    s->finished = FALSE;
    s->peak = NAN;
    s->rev = FALSE;
    s->rotate = 0;
    s->avg = 0;
    s->interp = PATTERN_INTERP_LINEAR;
    s->acc = NULL;
    s->spline = NULL;
    return s;
}

void
pattern_signal_free(pattern_signal_t *s)
{
    if(s)
    {
        g_array_free(s->arr, TRUE);
        if(s->acc)
            gsl_interp_accel_free(s->acc);
        if(s->spline)
            gsl_spline_free(s->spline);
        g_free(s);
    }
}

gint
pattern_signal_count(pattern_signal_t *s)
{
    g_assert(s != NULL);
    return s->count;
}

gint
pattern_signal_interp(pattern_signal_t *s)
{
    gint count = pattern_signal_count(s);

    if(count >= 1024)
        return 1;
    else if(count >= 512)
        return 2;
    else if(count >= 256)
        return 4;
    else if(count >= 128)
        return 8;
    else if(count >= 64)
        return 16;
    else if(count >= 32)
        return 32;
    else if(count >= 16)
        return 64;
    else if(count >= 8)
        return 128;
    else if(count >= 4)
        return 256;
    else if(count >= 2)
        return 512;
    else
        return 1024;
}

void
pattern_signal_push(pattern_signal_t *s,
                    gdouble           val)
{
    g_assert(s != NULL);

    g_array_append_val(s->arr, val);
    s->count++;

    if(isnan(s->peak) || s->peak < val)
        s->peak = val;

    pattern_signal_interp_invalidate(s);
}

gdouble
pattern_signal_get_sample(pattern_signal_t *s,
                              gint          idx)
{
    g_assert(s != NULL);
    g_assert(s->count != 0);

    return pattern_signal_sample(s, idx, s->avg);
}

static gdouble
pattern_signal_sample(pattern_signal_t *s,
                      gint              idx,
                      gint              avg)
{
    gdouble val;
    gint i;

    if(s->rev)
        idx = s->count - idx;

    idx += s->rotate;
    val = pattern_signal_get_sample_raw(s, idx);
    if(avg > 0)
    {
        for(i=1; i<=avg; i++)
        {
           val += pattern_signal_get_sample_raw(s, idx-i);
           val += pattern_signal_get_sample_raw(s, idx+i);
        }
        val /= avg*2.0 + 1.0;
    }
    return val;
}

static gint
pattern_signal_idx(pattern_signal_t *s,
                   gint              idx)
{
    if(idx < 0)
        return s->count - 1 + ((idx + 1) % s->count);
    else
        return idx % s->count;
}


gdouble
pattern_signal_get_sample_raw(pattern_signal_t *s,
                              gint              idx)
{
    idx = pattern_signal_idx(s, idx);
    return g_array_index(s->arr, gdouble, idx);
}

gdouble
pattern_signal_get_sample_interp(pattern_signal_t *s,
                                 gint              idx,
                                 gdouble           frac)
{
    gdouble val, current, next;
    gint idx_new;

    g_assert(s != NULL);
    g_assert(s->count != 0);

    if(!s->acc && !s->spline)
        pattern_signal_interp_init(s);

    if(s->rev)
    {
        idx_new = s->count - 1 - idx;
        if(frac == 0.0)
            idx_new++;
        else
            frac = 1.0 - frac;
    }
    else
    {
        idx_new = idx;
    }

    idx_new = pattern_signal_idx(s, idx_new+s->rotate);
    val = gsl_spline_eval(s->spline, idx_new+frac, s->acc);

    if(s->interp == PATTERN_INTERP_AKIMA_CLIPPED)
    {
        current = pattern_signal_get_sample(s, idx);
        next = pattern_signal_get_sample(s, idx+1);
        if(val < current && val < next)
            val = MIN(current, next);
        if(val > current && val > next)
            val = MAX(current, next);
    }
    return val;
}

gdouble
pattern_signal_get_peak(pattern_signal_t *s)
{
    g_assert(s != NULL);
    return s->peak;
}

void
pattern_signal_set_peak(pattern_signal_t *s,
                        gdouble           peak)
{
    gdouble offset;
    gint i;

    g_assert(s != NULL);

    offset = peak - s->peak;
    s->peak = peak;

    for(i=0; i<s->count; i++)
        g_array_index(s->arr, gdouble, i) = g_array_index(s->arr, gdouble, i) + offset;
}

gboolean
pattern_signal_get_rev(pattern_signal_t *s)
{
    g_assert(s != NULL);
    return s->rev;
}

void
pattern_signal_set_rev(pattern_signal_t *s,
                       gboolean          rev)
{
    g_assert(s != NULL);
    s->rev = rev;
}

gint
pattern_signal_get_avg(pattern_signal_t *s)
{
    g_assert(s != NULL);
    return s->avg;
}

void
pattern_signal_set_avg(pattern_signal_t *s,
                       gint              avg)
{
    g_assert(s != NULL);
    avg = MIN(PATTERN_SIGNAL_MAX_AVG, avg);
    avg = MAX(PATTERN_SIGNAL_MIN_AVG, avg);
    if(s->avg != avg)
    {
        s->avg = avg;
        pattern_signal_interp_invalidate(s);
    }
}

gint
pattern_signal_get_interp(pattern_signal_t *s)
{
    g_assert(s != NULL);
    return s->interp;
}

void
pattern_signal_set_interp(pattern_signal_t *s,
                          gint              interp)
{
    g_assert(s != NULL);
    g_assert(interp < PATTERN_INTERP_N);

    if((s->interp == PATTERN_INTERP_LINEAR && interp != PATTERN_INTERP_LINEAR) ||
       (s->interp != PATTERN_INTERP_LINEAR && interp == PATTERN_INTERP_LINEAR))
        pattern_signal_interp_invalidate(s);

    s->interp = interp;
}

gboolean
pattern_signal_get_finished(pattern_signal_t *s)
{
    g_assert(s != NULL);
    return s->finished;
}

void
pattern_signal_set_finished(pattern_signal_t *s)
{
    g_assert(s != NULL);
    s->finished = TRUE;
}

gint
pattern_signal_get_rotate(pattern_signal_t *s)
{
    g_assert(s != NULL);
    return s->rotate;
}

void
pattern_signal_set_rotate(pattern_signal_t *s,
                          gint              n)
{
    g_assert(s != NULL);
    s->rotate = n;
}

void
pattern_signal_rotate(pattern_signal_t *s,
                      gint              n)
{
    g_assert(s != NULL);
    s->rotate += (s->rev ? -n : n);
}

void
pattern_signal_rotate_0(pattern_signal_t *s)
{
    gdouble max = NAN;
    gdouble val;
    gint idx, i;
    gint rotate = 0;
    gint mainlobe = 0;

    g_assert(s != NULL);
    g_assert(s->count != 0);

    /* count samples with signal over -3dB */
    for(idx=0; idx<s->count; idx++)
    {
        val = pattern_signal_get_sample_raw(s, idx);
        if(s->peak - val <= 3.0)
            mainlobe++;
    }

    mainlobe /= 3;

    for(idx=0; idx<s->count; idx++)
    {
        val = pattern_signal_get_sample_raw(s, idx);

        for(i=1; i<=mainlobe; i++)
        {
           val += pattern_signal_get_sample_raw(s, idx-i);
           val += pattern_signal_get_sample_raw(s, idx+i);
        }
        val /= mainlobe*2.0 + 1.0;

        if(isnan(max) || max < val)
        {
            max = val;
            rotate = idx;
        }
    }

    s->rotate = rotate;
}

void
pattern_signal_rotate_reset(pattern_signal_t *s)
{
    g_assert(s != NULL);
    s->rotate = 0;
}

static void
pattern_signal_interp_init(pattern_signal_t *s)
{
    gdouble *x;
    gdouble *y;
    gint idx, i;
    size_t count;

    count = (size_t)s->count+1;

    /* Akima interpolation requires at least 5 samples */
    if(s->interp != PATTERN_INTERP_LINEAR &&
       count < gsl_interp_type_min_size(gsl_interp_akima_periodic))
        s->interp = PATTERN_INTERP_LINEAR;

    x = g_malloc(count * sizeof(gdouble));
    y = g_malloc(count * sizeof(gdouble));

    /* count+1, loop one more time at the end */
    for(idx=0; idx<=s->count; idx++)
    {
        x[idx] = idx;
        y[idx] = pattern_signal_get_sample_raw(s, idx);

        if(s->avg > 0)
        {
            for(i=1; i<=s->avg; i++)
            {
               y[idx] += pattern_signal_get_sample_raw(s, idx-i);
               y[idx] += pattern_signal_get_sample_raw(s, idx+i);
            }
            y[idx] /= s->avg*2.0 + 1.0;
        }
    }

    s->acc = gsl_interp_accel_alloc();
    s->spline = gsl_spline_alloc((s->interp != PATTERN_INTERP_LINEAR ? gsl_interp_akima_periodic : gsl_interp_linear), count);
    gsl_spline_init(s->spline, x, y, count);

    g_free(x);
    g_free(y);
}

static void
pattern_signal_interp_invalidate(pattern_signal_t *s)
{
    if(s->acc)
    {
        gsl_interp_accel_free(s->acc);
        s->acc = NULL;
    }
    if(s->spline)
    {
        gsl_spline_free(s->spline);
        s->spline = NULL;
    }
}
