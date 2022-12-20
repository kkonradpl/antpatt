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
#include <stdlib.h>
#include "pattern-data.h"

typedef struct pattern_data
{
    pattern_signal_t *s;
    gchar *name;
    gint freq;
    GdkRGBA color;
    gboolean hide;
    gboolean fill;
    gboolean changed;
} pattern_data_t;


pattern_data_t*
pattern_data_new(pattern_signal_t *s)
{
    pattern_data_t *data;
    g_assert(s != NULL);
    data = g_malloc0(sizeof(pattern_data_t));
    data->s = s;
    return data;
}

void
pattern_data_free(pattern_data_t *data)
{
    pattern_signal_free(data->s);
    g_free(data->name);
    g_free(data);
}

gboolean
pattern_data_changed(const pattern_data_t *data)
{
    return data->changed || pattern_signal_changed(data->s);
}

void
pattern_data_unchanged(pattern_data_t *data)
{
    pattern_signal_unchanged(data->s);
    data->changed = FALSE;
}

pattern_signal_t*
pattern_data_get_signal(pattern_data_t *data)
{
    g_assert(data != NULL);
    return data->s;
}

const gchar*
pattern_data_get_name(const pattern_data_t *data)
{
    static const gchar *default_name = "";
    g_assert(data != NULL);
    return (data->name ? data->name : default_name);
}

void
pattern_data_set_name(pattern_data_t *data,
                      const gchar    *value)
{
    g_assert(data != NULL);
    if (g_strcmp0(value, data->name) != 0)
    {
        g_free(data->name);
        data->name = (value ? g_strdup(value) : NULL);
        data->changed = TRUE;
    }
}

gint
pattern_data_get_freq(const pattern_data_t *data)
{
    g_assert(data != NULL);
    return data->freq;
}

void
pattern_data_set_freq(pattern_data_t *data,
                      gint            value)
{
    g_assert(data != NULL);
    value = MIN(PATTERN_DATA_MAX_FREQ, value);
    value = MAX(PATTERN_DATA_MIN_FREQ, value);
    if (value != data->freq)
    {
        data->freq = value;
        data->changed = TRUE;
    }
}

const GdkRGBA*
pattern_data_get_color(const pattern_data_t *data)
{
    g_assert(data != NULL);
    return &data->color;
}

void
pattern_data_set_color(pattern_data_t *data,
                       const GdkRGBA  *value)
{
    g_assert(data != NULL);
    if (!gdk_rgba_equal(value, &data->color))
    {
        data->color = *value;
        data->changed = TRUE;
    }

}

gboolean
pattern_data_get_hide(const pattern_data_t *data)
{
    g_assert(data != NULL);
    return data->hide;
}

void
pattern_data_set_hide(pattern_data_t *data,
                      gboolean        value)
{
    g_assert(data != NULL);
    if (value != data->hide)
    {
        data->hide = value;
        data->changed = TRUE;
    }
}

gboolean
pattern_data_get_fill(const pattern_data_t *data)
{
    g_assert(data != NULL);
    return data->fill;
}

void
pattern_data_set_fill(pattern_data_t *data,
                      gboolean        value)
{
    g_assert(data != NULL);
    if (value != data->fill)
    {
        data->fill = value;
        data->changed = TRUE;
    }
}
