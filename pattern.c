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

#include <gtk/gtk.h>
#include <math.h>
#include "pattern.h"
#include "pattern-ui.h"

/* Default settings */
#define PATTERN_DEFAULT_SIZE       500
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
    pattern_ui_window_t *ui;
    GtkListStore        *model;
    pattern_data_t      *current;

    gint      size;
    gchar    *title;
    gint      scale;
    gdouble   line;
    gint      interp;
    gboolean  full_angle;
    gboolean  black;
    gboolean  normalize;
    gboolean  legend;

    gint      focus_idx;
    gint      rotating_idx;

    gint      visible;
    gdouble   peak;
} pattern_t;

static gboolean pattern_set_interp_foreach(GtkTreeModel*, GtkTreePath*, GtkTreeIter*, gpointer);

pattern_t*
pattern_new()
{
    pattern_t *p = g_malloc(sizeof(pattern_t));

    p->ui = NULL;
    p->model = gtk_list_store_new(PATTERN_COLS, G_TYPE_POINTER);
    pattern_set_current(p, NULL);

    p->size = PATTERN_DEFAULT_SIZE;
    p->title = g_strdup(PATTERN_DEFAULT_TITLE);
    p->scale = PATTERN_DEFAULT_SCALE;
    p->line = PATTERN_DEFAULT_LINE;
    p->interp = PATTERN_DEFAULT_INTERP;
    p->full_angle = PATTERN_DEFAULT_FULL_ANGLE;
    p->black = PATTERN_DEFAULT_BLACK;
    p->normalize = PATTERN_DEFAULT_NORMALIZE;
    p->legend = PATTERN_DEFAULT_LEGEND;

    p->visible = 0;
    p->peak = NAN;
    return p;
}

void
pattern_free(pattern_t *p)
{
    pattern_clear(p);
    g_free(p->title);
    g_free(p->ui);
    g_free(p);
}

GtkListStore*
pattern_get_model(pattern_t *p)
{
    g_assert(p != NULL);
    return p->model;
}

pattern_ui_window_t*
pattern_get_ui(pattern_t *p)
{
    g_assert(p != NULL);
    return p->ui;
}

void
pattern_set_ui(pattern_t           *p,
               pattern_ui_window_t *ui)
{
    g_assert(p != NULL);
    g_assert(ui != NULL);
    g_assert(p->ui == NULL);

    p->ui = ui;
    pattern_ui_create(p);
}

void
pattern_add(pattern_t      *p,
            pattern_data_t *data)
{
    gtk_list_store_insert_with_values(p->model, NULL, -1, PATTERN_COL_DATA, data, -1);

    if(isnan(p->peak) || p->peak < pattern_signal_get_peak(pattern_data_get_signal(data)))
        p->peak = pattern_signal_get_peak(pattern_data_get_signal(data));

    if(!pattern_data_get_hide(data))
        p->visible++;

    pattern_signal_set_interp(pattern_data_get_signal(data), p->interp);
}

void
pattern_remove(pattern_t   *p,
               GtkTreeIter *remove)
{
    pattern_data_t *data;
    GtkTreeIter iter;
    gboolean visible;

    gtk_tree_model_get(GTK_TREE_MODEL(p->model), remove,
                       PATTERN_COL_DATA, &data,
                       -1);
    visible = !pattern_data_get_hide(data);
    pattern_data_free(data);
    gtk_list_store_remove(p->model, remove);
    p->peak = NAN;

    if(p->current == data)
        pattern_set_current(p, NULL);

    if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(p->model), &iter))
    {
        do
        {
            gtk_tree_model_get(GTK_TREE_MODEL(p->model), &iter, PATTERN_COL_DATA, &data, -1);

            if(isnan(p->peak) || p->peak < pattern_signal_get_peak(pattern_data_get_signal(data)))
                p->peak = pattern_signal_get_peak(pattern_data_get_signal(data));
        } while(gtk_tree_model_iter_next(GTK_TREE_MODEL(p->model), &iter));
    }

    if(visible)
        p->visible--;
}

void
pattern_clear(pattern_t *p)
{
    pattern_data_t *data;
    GtkTreeIter iter;

    if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(p->model), &iter))
    {
        do
        {
            gtk_tree_model_get(GTK_TREE_MODEL(p->model), &iter,
                               PATTERN_COL_DATA, &data, -1);
            pattern_data_free(data);
        } while(gtk_tree_model_iter_next(GTK_TREE_MODEL(p->model), &iter));
    }

    gtk_list_store_clear(p->model);
    pattern_set_current(p, NULL);
    p->visible = 0;
    p->peak = NAN;
}

void
pattern_set_current(pattern_t      *p,
                    pattern_data_t *data)
{
    g_assert(p != NULL);

    p->current = data;
    p->focus_idx = -1;
    p->rotating_idx = -1;
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
    if(value > 0)
        p->size = value;
}

gint
pattern_get_size(pattern_t *p)
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

    g_free(p->title);
    p->title = g_strdup(value);
}

const gchar*
pattern_get_title(pattern_t *p)
{
    g_assert(p != NULL);
    return p->title;
}

void
pattern_set_scale(pattern_t *p,
                  gint       value)
{
    g_assert(p != NULL);
    p->scale = value;
}

gint
pattern_get_scale(pattern_t *p)
{
    g_assert(p != NULL);
    return p->scale;
}

void
pattern_set_line(pattern_t *p,
                 gdouble    value)
{
    g_assert(p != NULL);
    p->line = value;
}

gdouble
pattern_get_line(pattern_t *p)
{
    g_assert(p != NULL);
    return p->line;
}

void
pattern_set_interp(pattern_t *p,
                   gint       value)
{
    g_assert(p != NULL);

    p->interp = value;
    gtk_tree_model_foreach(GTK_TREE_MODEL(p->model), pattern_set_interp_foreach, p);
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
pattern_get_interp(pattern_t *p)
{
    g_assert(p != NULL);
    return p->interp;
}

void
pattern_set_full_angle(pattern_t *p,
                       gboolean   value)
{
    g_assert(p != NULL);
    p->full_angle = value;
}

gboolean
pattern_get_full_angle(pattern_t *p)
{
    g_assert(p != NULL);
    return p->full_angle;
}

void
pattern_set_black(pattern_t *p,
                  gboolean   value)
{
    g_assert(p != NULL);
    p->black = value;
}

gboolean
pattern_get_black(pattern_t *p)
{
    g_assert(p != NULL);
    return p->black;
}

void
pattern_set_normalize(pattern_t *p,
                      gboolean   value)
{
    g_assert(p != NULL);
    p->normalize = value;
}

gboolean
pattern_get_normalize(pattern_t *p)
{
    g_assert(p != NULL);
    return p->normalize;
}

void
pattern_set_legend(pattern_t *p,
                   gboolean   value)
{
    g_assert(p != NULL);
    p->legend = value;
}

gboolean
pattern_get_legend(pattern_t *p)
{
    g_assert(p != NULL);
    return p->legend;
}

void
pattern_set_focus_idx(pattern_t *p,
                      gint       value)
{
    g_assert(p != NULL);
    p->focus_idx = value;
}

gint
pattern_get_focus_idx(pattern_t *p)
{
    g_assert(p != NULL);
    return p->focus_idx;
}

void
pattern_set_rotating_idx(pattern_t *p,
                         gint       value)
{
    g_assert(p != NULL);
    p->rotating_idx = value;
}

gint
pattern_get_rotating_idx(pattern_t *p)
{
    g_assert(p != NULL);
    return p->rotating_idx;
}

gint
pattern_get_visible(pattern_t *p)
{
    g_assert(p != NULL);
    return p->visible;
}

gdouble
pattern_get_peak(pattern_t *p)
{
    g_assert(p != NULL);
    return p->peak;
}

void
pattern_hide(pattern_t      *p,
             pattern_data_t *data,
             gboolean        hide)
{
    g_assert(p != NULL);
    g_assert(data != NULL);

    if(pattern_data_get_hide(data) != hide)
    {
        p->visible += (hide ? -1 : 1);
        pattern_data_set_hide(data, hide);
    }
}
