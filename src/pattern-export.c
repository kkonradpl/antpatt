/*
 *  antpatt - antenna pattern plotting and analysis software
 *  Copyright (c) 2022-2023  Konrad Kosmatka
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
#include <glib/gstdio.h>
#include "pattern-export.h"

static gboolean pattern_export_xdrp(pattern_data_t*, FILE*);
static gboolean pattern_export_ant(pattern_data_t*, FILE*);

static gboolean write_to_file(const char*, FILE*);


gboolean
pattern_export(pattern_data_t *data,
               const gchar    *filename)
{
    gboolean ret;

    g_assert(data != NULL);
    g_assert(filename != NULL);

    FILE *fp = g_fopen(filename, "w");
    if (fp == NULL)
        return FALSE;

    gchar *ext = strrchr(filename, '.');
    if (ext && g_ascii_strcasecmp(ext, ".ant") == 0)
    {
        /* ANT: Radio Mobile file */
        ret = pattern_export_ant(data, fp);
    }
    else
    {
        /* Other: XDR-GTK pattern file */
        ret = pattern_export_xdrp(data, fp);
    }

    if (fclose(fp))
        return FALSE;

    return ret;
}

static gboolean
pattern_export_xdrp(pattern_data_t *data,
                    FILE           *fp)
{
    pattern_signal_t *s = pattern_data_get_signal(data);
    gint count = pattern_signal_count(s);
    gchar buff[1024];
    gint i;

    snprintf(buff, sizeof(buff),
             "%d\n",
             pattern_data_get_freq(data));
    if (!write_to_file(buff, fp))
        return FALSE;

    snprintf(buff, sizeof(buff),
             "%s\n",
             pattern_data_get_name(data));
    if (!write_to_file(buff, fp))
        return FALSE;

    for (i = 0; i < count; i++)
    {
        snprintf(buff, sizeof(buff),
                 "%.2f\n",
                 pattern_signal_get_sample(s, i));
        if (!write_to_file(buff, fp))
            return FALSE;
    }

    return TRUE;
}

static gboolean
pattern_export_ant(pattern_data_t *data,
                   FILE           *fp)
{
    pattern_signal_t *s = pattern_data_get_signal(data);
    gint count = pattern_signal_count(s);
    gchar buff[128];
    gint i;

    for (i = 0; i < 360; i++)
    {
        gdouble x = i / 360.0 * count;
        snprintf(buff, sizeof(buff),
                 "%.2f\r\n",
                 pattern_signal_get_sample_interp(s, (gint) x, x - (gint) x));
        if (!write_to_file(buff, fp))
            return FALSE;
    }

    return TRUE;
}

static gboolean
write_to_file(const char *string,
              FILE       *fp)
{
    size_t len = strlen(string);
    size_t out = fwrite(string, sizeof(gchar), len, fp);
    return (out == len);
}