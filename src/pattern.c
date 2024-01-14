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

#include <gtk/gtk.h>
#include <math.h>
#include "pattern.h"

/* Default settings */
#define PATTERN_DEFAULT_SIZE       600
#define PATTERN_DEFAULT_TITLE      ""
#define PATTERN_DEFAULT_SCALE      0
#define PATTERN_DEFAULT_LINE       1.0
#define PATTERN_DEFAULT_INTERP     PATTERN_INTERP_LINEAR
#define PATTERN_DEFAULT_FULL_ANGLE TRUE
#define PATTERN_DEFAULT_BLACK      TRUE
#define PATTERN_DEFAULT_NORMALIZE  TRUE
#define PATTERN_DEFAULT_LEGEND     TRUE

typedef struct pattern
{
    pattern_ui_t   *ui;
    GtkListStore   *model;
    pattern_data_t *current;

    gint      size;
    gchar    *title;
    gint      scale;
    gdouble   line;
    gint      interp;
    gboolean  full_angle;
    gboolean  black;
    gboolean  normalize;
    gboolean  legend;

    gchar    *filename;
    gint      visible;
    gboolean  changed;
} pattern_t;

static gboolean pattern_set_interp_foreach(GtkTreeModel*, GtkTreePath*, GtkTreeIter*, gpointer);
static void model_changed(pattern_t*);


pattern_t*
pattern_new()
{
    pattern_t *p = g_malloc0(sizeof(pattern_t));
    p->model = gtk_list_store_new(PATTERN_COLS, G_TYPE_POINTER);

    pattern_reset(p);

    g_signal_connect_swapped(p->model, "row-changed", G_CALLBACK(model_changed), p);
    g_signal_connect_swapped(p->model, "row-inserted", G_CALLBACK(model_changed), p);
    g_signal_connect_swapped(p->model, "row-deleted", G_CALLBACK(model_changed), p);
    g_signal_connect_swapped(p->model, "rows-reordered", G_CALLBACK(model_changed), p);

    return p;
}

void
pattern_free(pattern_t *p)
{
    pattern_clear(p);
    g_free(p->filename);
    g_free(p->title);
    g_free(p->ui);
    g_free(p);
}

void
pattern_reset(pattern_t *p)
{
    pattern_set_size(p, PATTERN_DEFAULT_SIZE);
    pattern_set_title(p, PATTERN_DEFAULT_TITLE);
    pattern_set_scale(p, PATTERN_DEFAULT_SCALE);
    pattern_set_line(p, PATTERN_DEFAULT_LINE);
    pattern_set_interp(p, PATTERN_DEFAULT_INTERP);
    pattern_set_full_angle(p, PATTERN_DEFAULT_FULL_ANGLE);
    pattern_set_black(p, PATTERN_DEFAULT_BLACK);
    pattern_set_normalize(p, PATTERN_DEFAULT_NORMALIZE);
    pattern_set_legend(p, PATTERN_DEFAULT_LEGEND);

    pattern_clear(p);

    pattern_set_filename(p, NULL);
    p->changed = FALSE;
}

gboolean
pattern_changed(const pattern_t *p)
{
    g_assert(p != NULL);
    GtkTreeIter iter;
    pattern_data_t *data;

    if (p->changed)
        return TRUE;

    if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(p->model), &iter))
        return FALSE;

    do
    {
        gtk_tree_model_get(GTK_TREE_MODEL(p->model), &iter,
                           PATTERN_COL_DATA, &data, -1);
        if (pattern_data_changed(data))
            return TRUE;
    } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(p->model), &iter));

    return FALSE;
}

void
pattern_unchanged(pattern_t *p)
{
    g_assert(p != NULL);
    GtkTreeIter iter;
    pattern_data_t *data;

    p->changed = FALSE;

    if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(p->model), &iter))
        return;

    do
    {
        gtk_tree_model_get(GTK_TREE_MODEL(p->model), &iter,
                           PATTERN_COL_DATA, &data, -1);
        pattern_data_unchanged(data);
    } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(p->model), &iter));
}

GtkListStore*
pattern_get_model(pattern_t *p)
{
    g_assert(p != NULL);
    return p->model;
}

pattern_ui_t*
pattern_get_ui(pattern_t *p)
{
    g_assert(p != NULL);
    return p->ui;
}

void
pattern_set_ui(pattern_t    *p,
               pattern_ui_t *ui)
{
    g_assert(p != NULL);
    p->ui = ui;
}

void
pattern_add(pattern_t      *p,
            pattern_data_t *data)
{
    g_assert(p != NULL);
    g_assert(data != NULL);
    gtk_list_store_insert_with_values(p->model, NULL, -1, PATTERN_COL_DATA, data, -1);

    if (!pattern_data_get_hide(data))
        p->visible++;

    pattern_signal_set_interp(pattern_data_get_signal(data), p->interp);

    /* No need to set pattern_changed explicitly */
}

void
pattern_remove(pattern_t   *p,
               GtkTreeIter *remove)
{
    g_assert(p != NULL);
    pattern_data_t *data;
    gboolean visible;

    gtk_tree_model_get(GTK_TREE_MODEL(p->model), remove,
                       PATTERN_COL_DATA, &data,
                       -1);
    visible = !pattern_data_get_hide(data);
    pattern_data_free(data);
    gtk_list_store_remove(p->model, remove);

    if (p->current == data)
        pattern_set_current(p, NULL);

    if (visible)
        p->visible--;

    /* No need to set pattern_changed explicitly */
}

void
pattern_clear(pattern_t *p)
{
    g_assert(p != NULL);
    pattern_data_t *data;
    GtkTreeIter iter;

    pattern_set_current(p, NULL);
    p->visible = 0;

    if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(p->model), &iter))
        return;

    do
    {
        gtk_tree_model_get(GTK_TREE_MODEL(p->model), &iter,
                           PATTERN_COL_DATA, &data, -1);
        pattern_data_free(data);
    } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(p->model), &iter));

    gtk_list_store_clear(p->model);

    /* No need to call pattern_changed explicitly */
}

void
pattern_set_current(pattern_t      *p,
                    pattern_data_t *data)
{
    g_assert(p != NULL);
    p->current = data;
}

pattern_data_t*
pattern_get_current(pattern_t *p)
{
    g_assert(p != NULL);
    return p->current;
}

void
pattern_set_size(pattern_t *p,
                 gint       value)
{
    g_assert(p != NULL);
    value = MIN(PATTERN_MAX_SIZE, value);
    value = MAX(PATTERN_MIN_SIZE, value);
    if (value != p->size)
    {
        p->size = value;
        p->changed = TRUE;
    }
}

gint
pattern_get_size(const pattern_t *p)
{
    g_assert(p != NULL);
    return p->size;
}

void
pattern_set_title(pattern_t   *p,
                  const gchar *value)
{
    g_assert(p != NULL);
    g_assert(value != NULL);
    if (g_strcmp0(value, p->title) != 0)
    {
        g_free(p->title);
        p->title = g_strdup(value);
        p->changed = TRUE;
    }
}

const gchar*
pattern_get_title(const pattern_t *p)
{
    g_assert(p != NULL);
    return p->title;
}

void
pattern_set_scale(pattern_t *p,
                  gint       value)
{
    g_assert(p != NULL);
    if (value != p->scale)
    {
        p->scale = value;
        p->changed = TRUE;
    }
}

gint
pattern_get_scale(const pattern_t *p)
{
    g_assert(p != NULL);
    return p->scale;
}

void
pattern_set_line(pattern_t *p,
                 gdouble    value)
{
    g_assert(p != NULL);
    value = MIN(PATTERN_MAX_LINE, value);
    value = MAX(PATTERN_MIN_LINE, value);
    if (value != p->line)
    {
        p->line = value;
        p->changed = TRUE;
    }
}

gdouble
pattern_get_line(const pattern_t *p)
{
    g_assert(p != NULL);
    return p->line;
}

void
pattern_set_interp(pattern_t *p,
                   gint       value)
{
    g_assert(p != NULL);
    if (value != p->interp)
    {
        p->interp = value;
        p->changed = TRUE;
        gtk_tree_model_foreach(GTK_TREE_MODEL(p->model), pattern_set_interp_foreach, p);
    }
}

static gboolean
pattern_set_interp_foreach(GtkTreeModel *model,
                           GtkTreePath  *path,
                           GtkTreeIter  *iter,
                           gpointer      user_data)
{
    pattern_t *p = (pattern_t*)user_data;
    pattern_data_t *data;
    gtk_tree_model_get(model, iter, PATTERN_COL_DATA, &data, -1);
    pattern_signal_set_interp(pattern_data_get_signal(data), p->interp);
    return FALSE;
}

gint
pattern_get_interp(const pattern_t *p)
{
    g_assert(p != NULL);
    return p->interp;
}

void
pattern_set_full_angle(pattern_t *p,
                       gboolean   value)
{
    g_assert(p != NULL);
    if (value != p->full_angle)
    {
        p->full_angle = value;
        p->changed = TRUE;
    }
}

gboolean
pattern_get_full_angle(const pattern_t *p)
{
    g_assert(p != NULL);
    return p->full_angle;
}

void
pattern_set_black(pattern_t *p,
                  gboolean   value)
{
    g_assert(p != NULL);
    if (value != p->black)
    {
        p->black = value;
        p->changed = TRUE;
    }
}

gboolean
pattern_get_black(const pattern_t *p)
{
    g_assert(p != NULL);
    return p->black;
}

void
pattern_set_normalize(pattern_t *p,
                      gboolean   value)
{
    g_assert(p != NULL);
    if (value != p->normalize)
    {
        p->normalize = value;
        p->changed = TRUE;
    }
}

gboolean
pattern_get_normalize(const pattern_t *p)
{
    g_assert(p != NULL);
    return p->normalize;
}

void
pattern_set_legend(pattern_t *p,
                   gboolean   value)
{
    g_assert(p != NULL);
    if (value != p->legend)
    {
        p->legend = value;
        p->changed = TRUE;
    }
}

gboolean
pattern_get_legend(const pattern_t *p)
{
    g_assert(p != NULL);
    return p->legend;
}

void
pattern_set_filename(pattern_t   *p,
                     const gchar *value)
{
    g_assert(p != NULL);
    if (g_strcmp0(value, p->filename) != 0)
    {
        g_free(p->filename);
        p->filename = g_strdup(value);
        p->changed = TRUE;
    }
}

const gchar*
pattern_get_filename(const pattern_t *p)
{
    g_assert(p != NULL);
    return p->filename;
}

gint
pattern_get_visible_count(const pattern_t *p)
{
    g_assert(p != NULL);
    return p->visible;
}

gdouble
pattern_get_peak(const pattern_t *p)
{
    g_assert(p != NULL);
    pattern_data_t *data;
    GtkTreeIter iter;
    gdouble peak = NAN;

    if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(p->model), &iter))
        return peak;

    do
    {
        gtk_tree_model_get(GTK_TREE_MODEL(p->model), &iter,
                           PATTERN_COL_DATA, &data, -1);

        if (isnan(peak) || peak < pattern_signal_get_peak(pattern_data_get_signal(data)))
            peak = pattern_signal_get_peak(pattern_data_get_signal(data));

    } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(p->model), &iter));

    return peak;
}

void
pattern_hide(pattern_t      *p,
             pattern_data_t *data,
             gboolean        hide)
{
    g_assert(p != NULL);
    g_assert(data != NULL);
    if (pattern_data_get_hide(data) != hide)
    {
        p->visible += (hide ? -1 : 1);
        pattern_data_set_hide(data, hide);
        p->changed = TRUE;
    }
}

static void
model_changed(pattern_t *p)
{
    p->changed = TRUE;
}
