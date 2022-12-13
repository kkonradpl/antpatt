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
#include <json-c/json.h>
#include <zlib.h>
#include <string.h>
#include "pattern.h"
#include "pattern-color.h"

#define PATTERN_JSON_VERSION 1

#define READ_BUFFER   50*1024

#define KEY_VERSION    APP_NAME
#define KEY_SIZE       "size"
#define KEY_TITLE      "title"
#define KEY_SCALE      "scale"
#define KEY_LINE       "line"
#define KEY_INTERP     "interp"
#define KEY_FULL_ANGLE "full_angle"
#define KEY_BLACK      "black"
#define KEY_NORMALIZE  "normalize"
#define KEY_LEGEND     "legend"
#define KEY_DATA       "data"
#define KEY_NAME       "name"
#define KEY_FREQ       "freq"
#define KEY_COLOR      "color"
#define KEY_HIDE       "hide"
#define KEY_FILL       "fill"
#define KEY_REV        "rev"
#define KEY_AVG        "avg"
#define KEY_ROTATE     "rotate"
#define KEY_SAMPLES    "samples"

static pattern_t* pattern_json_parse(json_object*, gchar**);
static pattern_data_t* pattern_json_parse_data(json_object*);
static json_object* pattern_json_build(pattern_t*, gboolean);
static gboolean pattern_json_build_foreach(GtkTreeModel*, GtkTreePath*, GtkTreeIter*, gpointer);
static const gchar* pattern_json_format_double(gdouble);

pattern_t*
pattern_json_load(const gchar  *filename,
                  gchar       **error)
{
    gzFile gzfp;
    gchar buffer[READ_BUFFER];
    json_tokener *json;
    json_object *root;
    enum json_tokener_error jerr;
    gint n, gerr;
    const gchar *err_string;
    pattern_t *p;

	gzfp = gzopen(filename, "r");
	if(!gzfp)
	{
        *error = g_strdup_printf("Failed to open a file:\n%s", filename);
        return NULL;
	}

    json = json_tokener_new();
	do
    {
        n = gzread(gzfp, buffer, READ_BUFFER);
        root = json_tokener_parse_ex(json, buffer, n);

        if(gzeof(gzfp))
        {
            jerr = json_tokener_get_error(json);
            break;
        }
        else if(n < READ_BUFFER)
        {
            err_string = gzerror(gzfp, &gerr);
            if(gerr)
            {
                *error = g_strdup_printf("Failed to read a file:\n%s\n%s", filename, err_string);
                gzclose(gzfp);
                json_tokener_free(json);
                return NULL;
            }
        }
    } while ((jerr = json_tokener_get_error(json)) == json_tokener_continue);

    if(jerr != json_tokener_success)
    {
        *error = g_strdup_printf("Failed to parse a file:\n%s\n%s", filename, json_tokener_error_desc(jerr));
        if(root)
            json_object_put(root);
        json_tokener_free(json);
        return NULL;
    }
    gzclose(gzfp);

    p = pattern_json_parse(root, error);
    json_object_put(root);
    json_tokener_free(json);
    return p;
}

static pattern_t*
pattern_json_parse(json_object  *root,
                   gchar       **error)
{
    json_object *object;
    json_object *array;
    gint version;
    pattern_t *p;
    pattern_data_t *data;
    size_t len, i;

    if(!json_object_object_get_ex(root, KEY_VERSION, &object))
    {
        *error = g_strdup("Invalid file format");
        return NULL;
    }

    version = json_object_get_int(object);
    if(version != PATTERN_JSON_VERSION)
    {
        *error = g_strdup("Invalid file format version");
        return NULL;
    }

    p = pattern_new();

    /* KEY_SIZE (int) */
    if(json_object_object_get_ex(root, KEY_SIZE, &object) &&
       json_object_is_type(object, json_type_int))
    {
        pattern_set_size(p, json_object_get_int(object));
    }

    /* KEY_TITLE (string) */
    if(json_object_object_get_ex(root, KEY_TITLE, &object) &&
       json_object_is_type(object, json_type_string))
    {
        pattern_set_title(p, json_object_get_string(object));
    }

    /* KEY_SCALE (int) */
    if(json_object_object_get_ex(root, KEY_SCALE, &object) &&
       json_object_is_type(object, json_type_int))
    {
        pattern_set_scale(p, json_object_get_int(object));
    }

    /* KEY_LINE (double) */
    if(json_object_object_get_ex(root, KEY_LINE, &object))
    {
        if(json_object_is_type(object, json_type_double))
            pattern_set_line(p, json_object_get_double(object));
        else if(json_object_is_type(object, json_type_int))
            pattern_set_line(p, (gdouble)json_object_get_int(object));
    }

    /* KEY_INTERP (int) */
    if(json_object_object_get_ex(root, KEY_INTERP, &object) &&
       json_object_is_type(object, json_type_int))
    {
        pattern_set_interp(p, json_object_get_int(object));
    }

    /* KEY_FULL_ANGLE (boolean) */
    if(json_object_object_get_ex(root, KEY_FULL_ANGLE, &object) &&
       json_object_is_type(object, json_type_boolean))
    {
        pattern_set_full_angle(p, json_object_get_boolean(object));
    }

    /* KEY_BLACK (boolean) */
    if(json_object_object_get_ex(root, KEY_BLACK, &object) &&
       json_object_is_type(object, json_type_boolean))
    {
        pattern_set_black(p, json_object_get_boolean(object));
    }

    /* KEY_NORMALIZE (boolean) */
    if(json_object_object_get_ex(root, KEY_NORMALIZE, &object) &&
       json_object_is_type(object, json_type_boolean))
    {
        pattern_set_normalize(p, json_object_get_boolean(object));
    }

    /* KEY_LEGEND (boolean) */
    if(json_object_object_get_ex(root, KEY_LEGEND, &object) &&
       json_object_is_type(object, json_type_boolean))
    {
        pattern_set_legend(p, json_object_get_boolean(object));
    }

    /* KEY_DATA (array) */
    if(json_object_object_get_ex(root, KEY_DATA, &array) &&
       json_object_is_type(array, json_type_array))
    {
        len = json_object_array_length(array);
        for(i=0; i<len; i++)
        {
            object = json_object_array_get_idx(array, i);
            data = pattern_json_parse_data(object);
            if(data)
                pattern_add(p, data);
        }
    }

    return p;
}

static pattern_data_t*
pattern_json_parse_data(json_object *root)
{
    json_object *object, *array;
    pattern_data_t *data;
    pattern_signal_t *s;
    size_t len, i;
    GdkRGBA color;

    if(!json_object_object_get_ex(root, KEY_SAMPLES, &array) ||
       !json_object_is_type(array, json_type_array) ||
       !(len = json_object_array_length(array)))
    {
        /* No signal samples */
        return NULL;
    }

    s = pattern_signal_new();
    for(i=0; i<len; i++)
    {
        object = json_object_array_get_idx(array, i);
        if(json_object_is_type(object, json_type_double))
            pattern_signal_push(s, json_object_get_double(object));
        else if(json_object_is_type(object, json_type_int))
            pattern_signal_push(s, json_object_get_int(object));
    }

    if(!pattern_signal_count(s))
    {
        /* No valid signal samples */
        pattern_signal_free(s);
        return NULL;
    }

    pattern_signal_set_finished(s);
    data = pattern_data_new(s);

    /* KEY_NAME (string) */
    if(json_object_object_get_ex(root, KEY_NAME, &object) &&
       json_object_is_type(object, json_type_string))
    {
        pattern_data_set_name(data, json_object_get_string(object));
    }

    /* KEY_FREQ (int) */
    if(json_object_object_get_ex(root, KEY_FREQ, &object) &&
       json_object_is_type(object, json_type_int))
    {
        pattern_data_set_freq(data, json_object_get_int(object));
    }

    /* KEY_COLOR (string) */
    if(json_object_object_get_ex(root, KEY_COLOR, &object) &&
       json_object_is_type(object, json_type_string))
    {
        if(gdk_rgba_parse(&color, json_object_get_string(object)))
            pattern_data_set_color(data, &color);
    }

    /* KEY_HIDE (boolean) */
    if(json_object_object_get_ex(root, KEY_HIDE, &object) &&
       json_object_is_type(object, json_type_boolean))
    {
        pattern_data_set_hide(data, json_object_get_boolean(object));
    }

    /* KEY_FILL (boolean) */
    if(json_object_object_get_ex(root, KEY_FILL, &object) &&
       json_object_is_type(object, json_type_boolean))
    {
        pattern_data_set_fill(data, json_object_get_boolean(object));
    }

    /* KEY_REV (boolean) */
    if(json_object_object_get_ex(root, KEY_REV, &object) &&
       json_object_is_type(object, json_type_boolean))
    {
        pattern_signal_set_rev(s, json_object_get_boolean(object));
    }

    /* KEY_AVG (int) */
    if(json_object_object_get_ex(root, KEY_AVG, &object) &&
       json_object_is_type(object, json_type_int))
    {
        pattern_signal_set_avg(s, json_object_get_int(object));
    }

    /* KEY_ROTATE (boolean) */
    if(json_object_object_get_ex(root, KEY_ROTATE, &object) &&
       json_object_is_type(object, json_type_int))
    {
        pattern_signal_set_rotate(s, json_object_get_int(object));
    }

    return data;
}

gboolean
pattern_json_save(pattern_t   *p,
                  const gchar *filename,
                  gboolean     config)
{
    gzFile gzfp = NULL;
    FILE *fp = NULL;
    const gchar *ext;
    json_object *json;
    const gchar *json_string;
    size_t json_length;
    size_t wrote;
    gint wrote_gz;

    ext = strrchr(filename, '.');
    if(ext && !g_ascii_strcasecmp(ext, ".gz"))
        gzfp = gzopen(filename, "wb");
    else
        fp = fopen(filename, "w");

    if(!gzfp && !fp)
        return FALSE;

    json = pattern_json_build(p, config);
    json_string = json_object_to_json_string_ext(json, JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED);
    json_length = strlen(json_string);

    if(gzfp)
    {
        wrote_gz = gzwrite(gzfp, json_string, (guint)json_length);
        wrote = wrote_gz >= 0 ? (size_t)wrote_gz : 0;
        gzclose(gzfp);
    }
    else
    {
        wrote = fwrite(json_string, sizeof(gchar), json_length, fp);
        fclose(fp);
    }

    json_object_put(json);
    return (json_length == wrote);
}

static json_object*
pattern_json_build(pattern_t *p,
                   gboolean   config)
{
    json_object *root = json_object_new_object();
    json_object *array;

    json_object_object_add(root, KEY_VERSION,    json_object_new_int(PATTERN_JSON_VERSION));
    json_object_object_add(root, KEY_SIZE,       json_object_new_int(pattern_get_size(p)));
    json_object_object_add(root, KEY_TITLE,      json_object_new_string(pattern_get_title(p)));
    json_object_object_add(root, KEY_SCALE,      json_object_new_int(pattern_get_scale(p)));
    json_object_object_add(root, KEY_LINE,       json_object_new_double_s(pattern_get_line(p), pattern_json_format_double(pattern_get_line(p))));
    json_object_object_add(root, KEY_INTERP,     json_object_new_int(pattern_get_interp(p)));
    json_object_object_add(root, KEY_FULL_ANGLE, json_object_new_boolean(pattern_get_full_angle(p)));
    json_object_object_add(root, KEY_BLACK,      json_object_new_boolean(pattern_get_black(p)));
    json_object_object_add(root, KEY_NORMALIZE,  json_object_new_boolean(pattern_get_normalize(p)));
    json_object_object_add(root, KEY_LEGEND,     json_object_new_boolean(pattern_get_legend(p)));

    if(!config)
    {
        array = json_object_new_array();
        gtk_tree_model_foreach(GTK_TREE_MODEL(pattern_get_model(p)), pattern_json_build_foreach, array);
        json_object_object_add(root, KEY_DATA, array);
    }

    return root;
}

static gboolean
pattern_json_build_foreach(GtkTreeModel *model,
                           GtkTreePath  *path,
                           GtkTreeIter  *iter,
                           gpointer      user_data)
{
    json_object *parent = (json_object*)user_data;
    json_object *child = json_object_new_object();
    json_object *array;
    pattern_data_t *data;
    pattern_signal_t *signal;
    gint n, i;
    gdouble sample;
    const gchar *format;
    gchar *color;

    gtk_tree_model_get(model, iter, PATTERN_COL_DATA, &data, -1);
    signal = pattern_data_get_signal(data);

    json_object_object_add(child, KEY_NAME,     json_object_new_string(pattern_data_get_name(data)));
    json_object_object_add(child, KEY_FREQ,     json_object_new_int(pattern_data_get_freq(data)));

    color = pattern_color_to_string(pattern_data_get_color(data));
    json_object_object_add(child, KEY_COLOR,    json_object_new_string(color));
    g_free(color);

    json_object_object_add(child, KEY_HIDE,     json_object_new_boolean(pattern_data_get_hide(data)));
    json_object_object_add(child, KEY_FILL,     json_object_new_boolean(pattern_data_get_fill(data)));
    json_object_object_add(child, KEY_REV,      json_object_new_boolean(pattern_signal_get_rev(signal)));
    json_object_object_add(child, KEY_AVG,      json_object_new_int(pattern_signal_get_avg(signal)));
    json_object_object_add(child, KEY_ROTATE,   json_object_new_int(pattern_signal_get_rotate(signal)));

    n = pattern_signal_count(signal);
    if(n)
    {
        array = json_object_new_array();
        for(i=0; i<n; i++)
        {
            sample = pattern_signal_get_sample_raw(signal, i);
            format = pattern_json_format_double(sample);
            json_object_array_add(array, json_object_new_double_s(sample, format));
        }
        json_object_object_add(child, KEY_SAMPLES, array);
    }

    json_object_array_add(parent, child);
    return FALSE;
}

static const gchar*
pattern_json_format_double(gdouble value)
{
    static gchar output[G_ASCII_DTOSTR_BUF_SIZE];
    g_ascii_formatd(output, sizeof(output), "%.10g", value);
    return output;
}
