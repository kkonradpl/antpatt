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
#include <glib/gstdio.h>
#include <string.h>
#include <math.h>
#include "pattern-import.h"
#ifdef G_OS_WIN32
#include "mingw.h"
#endif

typedef struct pattern_import
{
    pattern_signal_t *samples;
    gchar *name;
    gint freq;
} pattern_import_t;

static gint pattern_import_xdrp(pattern_import_t*, FILE*);
static gint pattern_import_mmanagal(pattern_import_t*, FILE*, const gchar*);
static gint pattern_import_ant(pattern_import_t*, FILE*);
static gint pattern_import_msi(pattern_import_t*, FILE*);

static void replace_comma_with_dot(gchar*);


pattern_import_t*
pattern_import_new()
{
    pattern_import_t *im;
    im = g_malloc(sizeof(pattern_import_t));
    im->samples = pattern_signal_new();
    im->name = NULL;
    im->freq = 0;
    return im;
}

void
pattern_import_free(pattern_import_t *im,
                    gboolean          s)
{
    if(im)
    {
        if(s)
            pattern_signal_free(im->samples);
        g_free(im->name);
        g_free(im);
    }
}

gint
pattern_import(pattern_import_t *im,
               const gchar      *filename)
{
    const gchar *ext;
    FILE *fp;
    gint ret;

    g_assert(im != NULL);
    g_assert(filename != NULL);

    fp = g_fopen(filename, "r");
    if(!fp)
        return PATTERN_IMPORT_ERROR;

    ext = strrchr(filename, '.');
    /* CSV: MMANA-GAL file */
    if(ext && g_ascii_strcasecmp(ext, ".csv") == 0)
        ret = pattern_import_mmanagal(im, fp, "total");
    /* ANT: Radio Mobile file */
    else if(ext && g_ascii_strcasecmp(ext, ".ant") == 0)
        ret = pattern_import_ant(im, fp);
    /* MSI: Planet antenna file */
    else if(ext && g_ascii_strcasecmp(ext, ".msi") == 0)
        ret = pattern_import_msi(im, fp);
    /* Other: XDR-GTK pattern file */
    else
        ret = pattern_import_xdrp(im, fp);

    fclose(fp);

    if(!im->name)
        im->name = g_path_get_basename(filename);
    pattern_signal_set_finished(im->samples);
    return ret;
}

static gint
pattern_import_xdrp(pattern_import_t *im,
                    FILE             *fp)
{
    gchar buff[256];
    gdouble sample;

    /* First line: frequency [kHz] */
    fgets(buff, sizeof(buff), fp);
    if(!sscanf(buff, "%d", &im->freq))
        return PATTERN_IMPORT_INVALID_FORMAT;

    /* Second line: name */
    fgets(buff, sizeof(buff), fp);
    if(strlen(buff) > 1)
    {
        buff[strcspn(buff, "\r\n")] = 0;
        im->name = g_strdup(buff);
    }

    /* Next lines: signal samples */
    while(fscanf(fp, "%lf", &sample) && !feof(fp))
        pattern_signal_push(im->samples, sample);

    if(!pattern_signal_count(im->samples))
        return PATTERN_IMPORT_EMPTY_FILE;

    return PATTERN_IMPORT_OK;
}

static gint
pattern_import_mmanagal(pattern_import_t *im,
                        FILE             *fp,
                        const gchar      *column_name)
{
    gchar buff[256], *ptr, *token;
    gdouble sample;
    gint i, column = -1;

    /* First line: CSV header */
    fgets(buff, sizeof(buff), fp);
    ptr = buff;
    for(i = 0; (token = strsep(&ptr, ",")); i++)
    {
        if(!g_ascii_strncasecmp(column_name, token, strlen(column_name)))
            column = i;
    }

    if(column == -1)
        return PATTERN_IMPORT_INVALID_FORMAT;

    /* Next lines: signal samples */
    while(!feof(fp) && fgets(buff, sizeof(buff), fp))
    {
        ptr = buff;
        for(i = 0; (token = strsep(&ptr, ",")); i++)
        {
            if(i == column)
            {
                if(sscanf(token, "%lf", &sample))
                    pattern_signal_push(im->samples, sample);
                else
                    return PATTERN_IMPORT_INVALID_FORMAT;
                break;
            }
        }
    }

    if(!pattern_signal_count(im->samples))
        return PATTERN_IMPORT_EMPTY_FILE;

    return PATTERN_IMPORT_OK;
}

static gint
pattern_import_ant(pattern_import_t *im,
                   FILE             *fp)
{
    gchar buff[256];
    gdouble sample;
    gint i;

    for(i=0; i<360 && !feof(fp); i++)
    {
        if(!fgets(buff, sizeof(buff), fp))
            break;
        if(sscanf(buff, "%lf", &sample))
            pattern_signal_push(im->samples, sample);
    }

    if(pattern_signal_count(im->samples) != 360)
        return PATTERN_IMPORT_INVALID_FORMAT;

    return PATTERN_IMPORT_OK;
}

static gint
pattern_import_msi(pattern_import_t *im,
                   FILE             *fp)
{
    static const gchar *name_str = "NAME ";
    static const gchar *freq_str = "FREQUENCY ";
    static const gchar *gain_str = "GAIN ";
    static const gchar *data_str = "HORIZONTAL ";
    gchar buff[256];
    gdouble sample;
    gdouble gain = NAN;
    gboolean data = FALSE;
    gint count = 0;
    gint i = 0;
    gint current;

    while(!feof(fp) && fgets(buff, sizeof(buff), fp) && (!data || (i != count)))
    {
        if(!im->name && !g_ascii_strncasecmp(name_str, buff, strlen(name_str)))
        {
            buff[strcspn(buff, "\r\n")] = 0;
            im->name = g_strdup(buff+strlen(name_str));
        }
        else if(!im->freq && !g_ascii_strncasecmp(freq_str, buff, strlen(freq_str)))
        {
            sscanf(buff+strlen(freq_str), "%d", &im->freq);
            im->freq *= 1000;
        }
        else if(isnan(gain) && !g_ascii_strncasecmp(gain_str, buff, strlen(gain_str)))
        {
            sscanf(buff+strlen(gain_str), "%lf", &gain);
        }
        else if(!data && !g_ascii_strncasecmp(data_str, buff, strlen(data_str)))
        {
            if(sscanf(buff+strlen(data_str), "%d", &count))
                data = TRUE;
        }
        else if(data)
        {
            replace_comma_with_dot(buff);
            if(sscanf(buff, "%d %lf", &current, &sample) == 2 && current == i)
                pattern_signal_push(im->samples, -sample);
            else
                break;
            i++;
        }
    }

    if(!count || pattern_signal_count(im->samples) != count)
        return PATTERN_IMPORT_INVALID_FORMAT;

    if(!isnan(gain))
        pattern_signal_set_peak(im->samples, gain);

    return PATTERN_IMPORT_OK;
}

static void
replace_comma_with_dot(gchar *buff)
{
    size_t length = strlen(buff);
    gint i;
    for(i=0; i<length; i++)
        if(buff[i] == ',')
            buff[i] = '.';
}

pattern_signal_t*
pattern_import_get_signal(pattern_import_t *r)
{
    g_assert(r != NULL);
    g_assert(r->samples != NULL);
    return r->samples;
}

const gchar*
pattern_import_get_name(pattern_import_t *r)
{
    g_assert(r != NULL);
    return r->name;
}

gint
pattern_import_get_freq(pattern_import_t *r)
{
    g_assert(r != NULL);
    return r->freq;
}
