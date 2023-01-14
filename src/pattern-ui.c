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
#include "pattern.h"
#include "pattern-ui.h"
#include "pattern-ui-window.h"
#include "pattern-ui-dialogs.h"
#include "pattern-ui-plot.h"
#include "pattern-plot.h"
#include "pattern-import.h"
#include "pattern-color.h"
#include "pattern-json.h"
#include "pattern-export.h"

#define UI_DRAG_URI_LIST_ID 0

struct pattern_ui
{
    struct pattern_ui_window *window;
    pattern_t *p;
    gint focus_idx;
    gint rotating_idx;
    gint lock;
    gboolean interactive;
};

static const GtkTargetEntry drop_types[] = {{ "text/uri-list", 0, UI_DRAG_URI_LIST_ID }};
static const gint n_drop_types = sizeof(drop_types) / sizeof(drop_types[0]);

static gboolean pattern_ui_changed(pattern_ui_t*);
static gboolean pattern_ui_delete(GtkWidget*, GdkEvent*, pattern_ui_t*);
static void pattern_ui_destroy(GtkWidget*, pattern_ui_t*);

static void pattern_ui_new(GtkWidget*, pattern_ui_t*);
static void pattern_ui_load(GtkWidget*, pattern_ui_t*);
static void pattern_ui_save(GtkWidget*, pattern_ui_t*);
static void pattern_ui_save_as(GtkWidget*, pattern_ui_t*);
static void pattern_ui_render(GtkWidget*, pattern_ui_t*);
static void pattern_ui_about(GtkWidget*, pattern_ui_t*);

static void pattern_ui_size(GtkWidget*, pattern_ui_t*);
static void pattern_ui_title(GtkWidget*, pattern_ui_t*);
static void pattern_ui_scale(GtkWidget*, pattern_ui_t*);
static void pattern_ui_line(GtkWidget*, pattern_ui_t*);
static void pattern_ui_interp(GtkWidget*, pattern_ui_t*);
static void pattern_ui_full_angle(GtkWidget*, pattern_ui_t*);
static void pattern_ui_black(GtkWidget*, pattern_ui_t*);
static void pattern_ui_normalize(GtkWidget*, pattern_ui_t*);
static void pattern_ui_legend(GtkWidget*, pattern_ui_t*);

static void pattern_ui_add(GtkWidget*, pattern_ui_t*);
static void pattern_ui_down(GtkWidget*, pattern_ui_t*);
static void pattern_ui_up(GtkWidget*, pattern_ui_t*);

static void pattern_ui_select(GtkWidget*, pattern_ui_t*);
static void pattern_ui_export(GtkWidget*, pattern_ui_t*);
static void pattern_ui_remove(GtkWidget*, pattern_ui_t*);
static void pattern_ui_clear(GtkWidget*, pattern_ui_t*);
static void pattern_ui_name(GtkWidget*, pattern_ui_t*);
static void pattern_ui_freq(GtkWidget*, pattern_ui_t*);
static void pattern_ui_avg(GtkWidget*, pattern_ui_t*);
static void pattern_ui_color(GtkWidget*, pattern_ui_t*);
static void pattern_ui_color_next(GtkWidget*, pattern_ui_t*);
static void pattern_ui_rotate_reset(GtkWidget*, pattern_ui_t*);
static void pattern_ui_rotate_ccw_fast(GtkWidget*, pattern_ui_t*);
static void pattern_ui_rotate_ccw(GtkWidget*, pattern_ui_t*);
static void pattern_ui_rotate_peak(GtkWidget*, pattern_ui_t*);
static void pattern_ui_rotate_cw(GtkWidget*, pattern_ui_t*);
static void pattern_ui_rotate_cw_fast(GtkWidget*, pattern_ui_t*);
static void pattern_ui_fill(GtkWidget*, pattern_ui_t*);
static void pattern_ui_rev(GtkWidget*, pattern_ui_t*);
static void pattern_ui_hide(GtkWidget*, pattern_ui_t*);

static void pattern_ui_reset(pattern_ui_t*);
static void pattern_ui_sync_full(pattern_ui_t*);
static void pattern_ui_sync(pattern_ui_t*, gboolean, gboolean);

static void pattern_ui_drag_data_received(GtkWidget*, GdkDragContext*, gint, gint, GtkSelectionData*, guint, guint, pattern_ui_t*);

static void pattern_ui_format_desc(GtkCellLayout*, GtkCellRenderer*, GtkTreeModel*, GtkTreeIter*, gpointer);
static gboolean pattern_ui_format_zero(GtkSpinButton*, gpointer);

static void pattern_ui_read(pattern_ui_t*, GSList*);


pattern_ui_t*
pattern_ui(pattern_t *p)
{
    pattern_ui_t *ui = g_malloc0(sizeof(pattern_ui_t));
    ui->window = pattern_ui_window_new();
    ui->p = p;
    pattern_ui_reset(ui);
    pattern_set_ui(p, ui);

    /* Signals */
    g_signal_connect(ui->window->window, "delete-event", G_CALLBACK(pattern_ui_delete), ui);
    g_signal_connect(ui->window->window, "destroy", G_CALLBACK(pattern_ui_destroy), ui);

    g_signal_connect(ui->window->b_new, "clicked", G_CALLBACK(pattern_ui_new), ui);
    g_signal_connect(ui->window->b_load, "clicked", G_CALLBACK(pattern_ui_load), ui);
    g_signal_connect(ui->window->b_save, "clicked", G_CALLBACK(pattern_ui_save), ui);
    g_signal_connect(ui->window->b_save_as, "clicked", G_CALLBACK(pattern_ui_save_as), ui);
    g_signal_connect(ui->window->b_render, "clicked", G_CALLBACK(pattern_ui_render), ui);
    g_signal_connect(ui->window->b_about, "clicked", G_CALLBACK(pattern_ui_about), ui);

    g_signal_connect(ui->window->s_size, "value-changed", G_CALLBACK(pattern_ui_size), ui);
    g_signal_connect(ui->window->e_title, "changed", G_CALLBACK(pattern_ui_title), ui);
    g_signal_connect(ui->window->c_scale, "changed", G_CALLBACK(pattern_ui_scale), ui);
    g_signal_connect(ui->window->s_line, "value-changed", G_CALLBACK(pattern_ui_line), ui);

    g_signal_connect(ui->window->c_interp, "changed", G_CALLBACK(pattern_ui_interp), ui);
    g_signal_connect(ui->window->b_full_angle, "toggled", G_CALLBACK(pattern_ui_full_angle), ui);
    g_signal_connect(ui->window->b_black, "toggled", G_CALLBACK(pattern_ui_black), ui);
    g_signal_connect(ui->window->b_normalize, "toggled", G_CALLBACK(pattern_ui_normalize), ui);
    g_signal_connect(ui->window->b_legend, "toggled", G_CALLBACK(pattern_ui_legend), ui);

    g_signal_connect(ui->window->plot, "motion-notify-event", G_CALLBACK(pattern_ui_plot_motion), ui);
    g_signal_connect(ui->window->plot, "button-press-event", G_CALLBACK(pattern_ui_plot_click), ui);
    g_signal_connect(ui->window->plot, "button-release-event", G_CALLBACK(pattern_ui_plot_click), ui);
    g_signal_connect(ui->window->plot, "leave-notify-event", G_CALLBACK(pattern_ui_plot_leave), ui);
    g_signal_connect(ui->window->plot, "draw", G_CALLBACK(pattern_ui_plot), ui);

    g_signal_connect(ui->window->b_add, "clicked", G_CALLBACK(pattern_ui_add), ui);
    g_signal_connect(ui->window->b_down, "clicked", G_CALLBACK(pattern_ui_down), ui);
    g_signal_connect(ui->window->b_up, "clicked", G_CALLBACK(pattern_ui_up), ui);
    g_signal_connect(ui->window->c_select, "changed", G_CALLBACK(pattern_ui_select), ui);
    g_signal_connect(ui->window->b_export, "clicked", G_CALLBACK(pattern_ui_export), ui);
    g_signal_connect(ui->window->b_remove, "clicked", G_CALLBACK(pattern_ui_remove), ui);
    g_signal_connect(ui->window->b_clear, "clicked", G_CALLBACK(pattern_ui_clear), ui);

    g_signal_connect(ui->window->e_name, "changed", G_CALLBACK(pattern_ui_name), ui);
    g_signal_connect(ui->window->s_freq, "value-changed", G_CALLBACK(pattern_ui_freq), ui);
    g_signal_connect(ui->window->s_freq, "output", G_CALLBACK(pattern_ui_format_zero), NULL);
    g_signal_connect(ui->window->s_avg, "value-changed", G_CALLBACK(pattern_ui_avg), ui);
    g_signal_connect(ui->window->s_avg, "output", G_CALLBACK(pattern_ui_format_zero), NULL);
    g_signal_connect(ui->window->b_color, "color-set", G_CALLBACK(pattern_ui_color), ui);
    g_signal_connect(ui->window->b_color_next, "clicked", G_CALLBACK(pattern_ui_color_next), ui);
    g_signal_connect(ui->window->b_rotate_reset, "clicked", G_CALLBACK(pattern_ui_rotate_reset), ui);
    g_signal_connect(ui->window->b_rotate_ccw_fast, "clicked", G_CALLBACK(pattern_ui_rotate_ccw_fast), ui);
    g_signal_connect(ui->window->b_rotate_ccw, "clicked", G_CALLBACK(pattern_ui_rotate_ccw), ui);
    g_signal_connect(ui->window->b_rotate_peak, "clicked", G_CALLBACK(pattern_ui_rotate_peak), ui);
    g_signal_connect(ui->window->b_rotate_cw, "clicked", G_CALLBACK(pattern_ui_rotate_cw), ui);
    g_signal_connect(ui->window->b_rotate_cw_fast, "clicked", G_CALLBACK(pattern_ui_rotate_cw_fast), ui);
    g_signal_connect(ui->window->b_fill, "toggled", G_CALLBACK(pattern_ui_fill), ui);
    g_signal_connect(ui->window->b_rev, "toggled", G_CALLBACK(pattern_ui_rev), ui);
    g_signal_connect(ui->window->b_hide, "toggled", G_CALLBACK(pattern_ui_hide), ui);

    gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(ui->window->c_select), ui->window->r_select, pattern_ui_format_desc, NULL, NULL);

    /* Drag and drop support */
    gtk_drag_dest_set(ui->window->window_plot, GTK_DEST_DEFAULT_ALL, drop_types, n_drop_types, GDK_ACTION_COPY);
    gtk_drag_dest_set(ui->window->window, GTK_DEST_DEFAULT_ALL, drop_types, n_drop_types, GDK_ACTION_COPY);
    g_signal_connect(ui->window->window_plot, "drag-data-received", G_CALLBACK(pattern_ui_drag_data_received), ui);
    g_signal_connect(ui->window->window, "drag-data-received", G_CALLBACK(pattern_ui_drag_data_received), ui);

    pattern_ui_sync_full(ui);
    gtk_widget_show_all(ui->window->window);

    return ui;
}

static gboolean
pattern_ui_changed(pattern_ui_t *ui)
{
    if (!pattern_changed(ui->p))
        return FALSE;

    switch (pattern_ui_dialog_ask_unsaved(GTK_WINDOW(ui->window->window)))
    {
    case GTK_RESPONSE_YES:
        gtk_button_clicked(GTK_BUTTON(ui->window->b_save));
        return pattern_changed(ui->p);
    case GTK_RESPONSE_NO:
        return FALSE;
    default:
        return TRUE;
    }
}

static gboolean
pattern_ui_delete(GtkWidget    *widget,
                  GdkEvent     *event,
                  pattern_ui_t *ui)
{
    return pattern_ui_changed(ui) ? GDK_EVENT_STOP : GDK_EVENT_PROPAGATE;
}

static void
pattern_ui_destroy(GtkWidget    *widget,
                   pattern_ui_t *ui)
{
    pattern_set_ui(ui->p, NULL);
    g_free(ui->window);
    g_free(ui);
    gtk_main_quit();
}

static void
pattern_ui_new(GtkWidget    *widget,
               pattern_ui_t *ui)
{
    if (pattern_ui_changed(ui))
        return;

    if (ui->interactive)
    {
        /* Ignore in interactive mode */
        pattern_ui_dialog(GTK_WINDOW(ui->window->window), GTK_MESSAGE_ERROR,
                          APP_TITLE,
                          "Unable to continue operation during active interactive console mode");
        return;
    }

    gint size = pattern_get_size(ui->p);
    pattern_reset(ui->p);
    pattern_set_size(ui->p, size);
    pattern_unchanged(ui->p);
    pattern_ui_sync_full(ui);
}

static void
pattern_ui_load(GtkWidget    *widget,
                pattern_ui_t *ui)
{
    g_autofree gchar *filename = NULL;
    g_autofree gchar *error = NULL;

    if (pattern_ui_changed(ui))
        return;

    filename = pattern_ui_dialog_open(GTK_WINDOW(ui->window->window));
    if (filename)
    {
        if (ui->interactive)
        {
            /* Ignore in interactive mode */
            pattern_ui_dialog(GTK_WINDOW(ui->window->window), GTK_MESSAGE_ERROR,
                              APP_TITLE,
                              "Unable to continue operation during active interactive console mode");
            return;
        }

        pattern_reset(ui->p);
        if (!pattern_json_load(ui->p, filename, &error))
        {
            pattern_ui_dialog(NULL,
                              GTK_MESSAGE_ERROR,
                              "Error",
                              error);
        }

        pattern_ui_sync_full(ui);
    }
}

static void
pattern_ui_save(GtkWidget    *widget,
                pattern_ui_t *ui)
{
    const gchar *filename = pattern_get_filename(ui->p);

    if (!filename)
    {
        pattern_ui_save_as(widget, ui);
        return;
    }

    if (!pattern_json_save(ui->p, filename, FALSE))
    {
        pattern_ui_dialog(GTK_WINDOW(ui->window->window), GTK_MESSAGE_ERROR,
                          APP_TITLE,
                          "Unable to save the file.");
    }
    pattern_unchanged(ui->p);
}

static void
pattern_ui_save_as(GtkWidget    *widget,
                   pattern_ui_t *ui)
{
    g_autofree gchar *filename = pattern_ui_dialog_save(GTK_WINDOW(ui->window->window));

    if (filename)
    {
        if (!pattern_json_save(ui->p, filename, FALSE))
        {
            pattern_ui_dialog(GTK_WINDOW(ui->window->window), GTK_MESSAGE_ERROR,
                              APP_TITLE,
                              "Unable to save the file.");
        }
        pattern_ui_window_set_title(ui->window, filename);
        pattern_set_filename(ui->p, filename);
        pattern_unchanged(ui->p);
    }
}

static void
pattern_ui_render(GtkWidget    *widget,
                  pattern_ui_t *ui)
{
    g_autofree gchar *filename = pattern_ui_dialog_render(GTK_WINDOW(ui->window->window));

    if (filename)
    {
        if (!pattern_plot_to_file(filename, ui->p))
        {
            pattern_ui_dialog(GTK_WINDOW(ui->window->window), GTK_MESSAGE_ERROR,
                              APP_TITLE,
                              "Unable to save the image.");
        }
    }
}

static void
pattern_ui_about(GtkWidget    *widget,
                 pattern_ui_t *ui)
{
    pattern_ui_dialog_about(GTK_WINDOW(ui->window->window));
}

static void
pattern_ui_size(GtkWidget    *widget,
                pattern_ui_t *ui)
{
    gint size = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
    gtk_widget_set_size_request(ui->window->plot, size, size);

    if (ui->lock)
        return;

    pattern_set_size(ui->p, size);
}

static void
pattern_ui_title(GtkWidget    *widget,
                 pattern_ui_t *ui)
{
    if (ui->lock)
        return;

    pattern_set_title(ui->p, gtk_entry_get_text(GTK_ENTRY(widget)));
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_scale(GtkWidget    *widget,
                 pattern_ui_t *ui)
{
    if (ui->lock)
        return;

    gint scale = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));

    if (scale == PATTERN_UI_SCALE_ARRL)
        pattern_set_scale(ui->p, 0);
    else if (scale == PATTERN_UI_SCALE_LINEAR_20)
        pattern_set_scale(ui->p, -20);
    else if (scale == PATTERN_UI_SCALE_LINEAR_30)
        pattern_set_scale(ui->p, -30);
    else if (scale == PATTERN_UI_SCALE_LINEAR_40)
        pattern_set_scale(ui->p, -40);
    else if (scale == PATTERN_UI_SCALE_LINEAR_50)
        pattern_set_scale(ui->p, -50);
    else if (scale == PATTERN_UI_SCALE_LINEAR_60)
        pattern_set_scale(ui->p, -60);

    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_line(GtkWidget    *widget,
                pattern_ui_t *ui)
{
    if (ui->lock)
        return;

    pattern_set_line(ui->p, gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget)));
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_interp(GtkWidget    *widget,
                  pattern_ui_t *ui)
{
    if (ui->lock)
        return;

    pattern_set_interp(ui->p, gtk_combo_box_get_active(GTK_COMBO_BOX(widget)));
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_full_angle(GtkWidget    *widget,
                      pattern_ui_t *ui)
{
    if (ui->lock)
        return;

    pattern_set_full_angle(ui->p, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_black(GtkWidget    *widget,
                 pattern_ui_t *ui)
{
    if (ui->lock)
        return;

    pattern_set_black(ui->p, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_normalize(GtkWidget    *widget,
                     pattern_ui_t *ui)
{
    if (ui->lock)
        return;

    pattern_set_normalize(ui->p, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_legend(GtkWidget    *widget,
                  pattern_ui_t *ui)
{
    pattern_set_legend(ui->p, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_add(GtkWidget    *widget,
               pattern_ui_t *ui)
{
    GSList *list = pattern_ui_dialog_import(GTK_WINDOW(ui->window->window));
    if (list)
    {
        if (ui->interactive)
        {
            /* Ignore in interactive mode */
            pattern_ui_dialog(GTK_WINDOW(ui->window->window), GTK_MESSAGE_ERROR,
                              APP_TITLE,
                              "Unable to continue operation during active interactive console mode");
            return;
        }

        pattern_ui_read(ui, list);
        g_slist_free_full(list, g_free);
    }
}

static void
pattern_ui_down(GtkWidget    *widget,
                pattern_ui_t *ui)
{
    GtkTreeIter iter;
    GtkTreeIter next;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui->window->c_select), &iter))
        return;
    next = iter;
    if (!gtk_tree_model_iter_next(GTK_TREE_MODEL(pattern_get_model(ui->p)), &next))
        return;

    gtk_list_store_move_after(pattern_get_model(ui->p), &iter, &next);
    gtk_widget_queue_draw(ui->window->c_select);
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_up(GtkWidget    *widget,
              pattern_ui_t *ui)
{
    GtkTreeIter iter;
    GtkTreeIter prev;
    GtkTreePath *path;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui->window->c_select), &iter))
        return;

    if (gtk_combo_box_get_active(GTK_COMBO_BOX(ui->window->c_select)) == 0)
        return;

    path = gtk_tree_model_get_path(GTK_TREE_MODEL(pattern_get_model(ui->p)), &iter);
    gtk_tree_path_prev(path);

    if (gtk_tree_model_get_iter(GTK_TREE_MODEL(pattern_get_model(ui->p)), &prev, path))
    {
        gtk_list_store_move_before(pattern_get_model(ui->p), &iter, &prev);
        gtk_widget_queue_draw(ui->window->c_select);
        gtk_widget_queue_draw(ui->window->plot);
    }

    gtk_tree_path_free(path);
}

static void
pattern_ui_select(GtkWidget    *widget,
                  pattern_ui_t *ui)
{
    if (ui->lock)
        return;

    GtkTreeIter iter;
    pattern_data_t *data = NULL;
    gboolean active = gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui->window->c_select), &iter);

    if (active)
        gtk_tree_model_get(GTK_TREE_MODEL(pattern_get_model(ui->p)), &iter, PATTERN_COL_DATA, &data, -1);

    pattern_set_current(ui->p, data);
    pattern_ui_reset(ui);

    pattern_ui_sync(ui, !active, FALSE);
}

static void
pattern_ui_export(GtkWidget    *widget,
                  pattern_ui_t *ui)
{
    GtkTreeIter iter;
    pattern_data_t *data;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui->window->c_select), &iter))
        return;

    gtk_tree_model_get(GTK_TREE_MODEL(pattern_get_model(ui->p)), &iter, PATTERN_COL_DATA, &data, -1);
    if (!pattern_signal_count(pattern_data_get_signal(data)))
    {
        pattern_ui_dialog(GTK_WINDOW(ui->window->window), GTK_MESSAGE_ERROR,
                          APP_TITLE,
                          "No signal samples available.");
        return;
    }

    gchar *filename = pattern_ui_dialog_export(GTK_WINDOW(ui->window->window));
    if (filename)
    {
        if (!pattern_export(data, filename))
        {
            pattern_ui_dialog(GTK_WINDOW(ui->window->window), GTK_MESSAGE_ERROR,
                              APP_TITLE,
                              "Unable to export the pattern data.");
        }
        g_free(filename);
    }
}

static void
pattern_ui_remove(GtkWidget    *widget,
                  pattern_ui_t *ui)
{
    GtkTreeIter iter;
    GtkTreeIter next;
    GtkTreePath *path;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui->window->c_select), &iter))
        return;

    if (!pattern_ui_dialog_yesno(GTK_WINDOW(ui->window->window),
                                 "Remove pattern data",
                                 "Do you really want to remove current pattern data?"))
    {
        return;
    }

    if (ui->interactive)
    {
        pattern_ui_dialog(GTK_WINDOW(ui->window->window), GTK_MESSAGE_ERROR,
                          APP_TITLE,
                          "Unable to continue operation during active interactive console mode");
        return;
    }

    next = iter;
    if (gtk_tree_model_iter_next(GTK_TREE_MODEL(pattern_get_model(ui->p)), &next))
        gtk_combo_box_set_active_iter(GTK_COMBO_BOX(ui->window->c_select), &next);
    else
    {
        path = gtk_tree_model_get_path(GTK_TREE_MODEL(pattern_get_model(ui->p)), &iter);
        gtk_tree_path_prev(path);
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(pattern_get_model(ui->p)), &next, path))
            gtk_combo_box_set_active_iter(GTK_COMBO_BOX(ui->window->c_select), &next);
        gtk_tree_path_free(path);
    }

    pattern_remove(ui->p, &iter);
    pattern_ui_reset(ui);
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_clear(GtkWidget    *widget,
                 pattern_ui_t *ui)
{
    if (!pattern_ui_dialog_yesno(GTK_WINDOW(ui->window->window),
                                 "Remove all pattern data",
                                 "Do you really want to remove all pattern data?"))
    {
        return;
    }

    if (ui->interactive)
    {
        pattern_ui_dialog(GTK_WINDOW(ui->window->window), GTK_MESSAGE_ERROR,
                          APP_TITLE,
                          "Unable to continue operation during active interactive console mode");
        return;
    }

    pattern_clear(ui->p);
    pattern_ui_reset(ui);
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_name(GtkWidget    *widget,
                pattern_ui_t *ui)
{
    if (ui->lock)
        return;

    GtkTreeIter iter;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui->window->c_select), &iter))
        return;

    pattern_data_set_name(pattern_get_current(ui->p), gtk_entry_get_text(GTK_ENTRY(widget)));
    gtk_widget_queue_draw(ui->window->c_select);
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_freq(GtkWidget    *widget,
                pattern_ui_t *ui)
{
    if (ui->lock)
        return;

    pattern_data_set_freq(pattern_get_current(ui->p), gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget)));
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_avg(GtkWidget    *widget,
               pattern_ui_t *ui)
{
    if (ui->lock)
        return;

    pattern_signal_set_avg(pattern_data_get_signal(pattern_get_current(ui->p)), gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget)));
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_color(GtkWidget    *widget,
                 pattern_ui_t *ui)
{
    if (ui->lock)
        return;

    GdkRGBA color;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), &color);
    pattern_data_set_color(pattern_get_current(ui->p), &color);
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_color_next(GtkWidget    *widget,
                      pattern_ui_t *ui)
{
    GdkRGBA next = pattern_color_next();
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(ui->window->b_color), &next);
    g_signal_emit_by_name(ui->window->b_color, "color-set", ui->p);
}

static void
pattern_ui_rotate_reset(GtkWidget    *widget,
                        pattern_ui_t *ui)
{
    pattern_signal_rotate_reset(pattern_data_get_signal(pattern_get_current(ui->p)));
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_rotate_ccw_fast(GtkWidget    *widget,
                           pattern_ui_t *ui)
{
    gint count = pattern_signal_count(pattern_data_get_signal(pattern_get_current(ui->p)));
    pattern_signal_rotate(pattern_data_get_signal(pattern_get_current(ui->p)), count / 18);
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_rotate_ccw(GtkWidget    *widget,
                      pattern_ui_t *ui)
{
    pattern_signal_rotate(pattern_data_get_signal(pattern_get_current(ui->p)), 1);
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_rotate_peak(GtkWidget    *widget,
                       pattern_ui_t *ui)
{
    pattern_signal_rotate_0(pattern_data_get_signal(pattern_get_current(ui->p)));
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_rotate_cw(GtkWidget    *widget,
                     pattern_ui_t *ui)
{
    pattern_signal_rotate(pattern_data_get_signal(pattern_get_current(ui->p)), -1);
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_rotate_cw_fast(GtkWidget    *widget,
                          pattern_ui_t *ui)
{
    gint count = pattern_signal_count(pattern_data_get_signal(pattern_get_current(ui->p)));
    pattern_signal_rotate(pattern_data_get_signal(pattern_get_current(ui->p)), -count/18);
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_fill(GtkWidget    *widget,
                pattern_ui_t *ui)
{
    if (ui->lock)
        return;

    pattern_data_set_fill(pattern_get_current(ui->p), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_rev(GtkWidget    *widget,
               pattern_ui_t *ui)
{
    if (ui->lock)
        return;

    pattern_signal_set_rev(pattern_data_get_signal(pattern_get_current(ui->p)), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_hide(GtkWidget    *widget,
                pattern_ui_t *ui)
{
    if (ui->lock)
        return;

    pattern_hide(ui->p, pattern_get_current(ui->p), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(ui->window->c_select);
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_reset(pattern_ui_t *ui)
{
    ui->focus_idx = -1;
    ui->rotating_idx = -1;
}

static void
pattern_ui_sync_full(pattern_ui_t *ui)
{
    ui->lock++;

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui->window->s_size), pattern_get_size(ui->p));
    gtk_entry_set_text(GTK_ENTRY(ui->window->e_title), pattern_get_title(ui->p));

    switch (pattern_get_scale(ui->p))
    {
    default:
    case 0:
        gtk_combo_box_set_active(GTK_COMBO_BOX(ui->window->c_scale), PATTERN_UI_SCALE_ARRL);
        break;
    case -20:
        gtk_combo_box_set_active(GTK_COMBO_BOX(ui->window->c_scale), PATTERN_UI_SCALE_LINEAR_20);
        break;
    case -30:
        gtk_combo_box_set_active(GTK_COMBO_BOX(ui->window->c_scale), PATTERN_UI_SCALE_LINEAR_30);
        break;
    case -40:
        gtk_combo_box_set_active(GTK_COMBO_BOX(ui->window->c_scale), PATTERN_UI_SCALE_LINEAR_40);
        break;
    case -50:
        gtk_combo_box_set_active(GTK_COMBO_BOX(ui->window->c_scale), PATTERN_UI_SCALE_LINEAR_50);
        break;
    case -60:
        gtk_combo_box_set_active(GTK_COMBO_BOX(ui->window->c_scale), PATTERN_UI_SCALE_LINEAR_60);
        break;
    }

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui->window->s_line), pattern_get_line(ui->p));
    gtk_combo_box_set_active(GTK_COMBO_BOX(ui->window->c_interp), pattern_get_interp(ui->p));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui->window->b_full_angle), pattern_get_full_angle(ui->p));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui->window->b_black), pattern_get_black(ui->p));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui->window->b_normalize), pattern_get_normalize(ui->p));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui->window->b_legend), pattern_get_legend(ui->p));

    /* Set model */
    gtk_combo_box_set_model(GTK_COMBO_BOX(ui->window->c_select), GTK_TREE_MODEL(pattern_get_model(ui->p)));

    pattern_ui_sync(ui, TRUE, FALSE);
    g_signal_emit_by_name(ui->window->s_size, "value-changed", ui);
    ui->lock--;

    if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(pattern_get_model(ui->p)), NULL))
        gtk_combo_box_set_active(GTK_COMBO_BOX(ui->window->c_select), 0);

    pattern_ui_window_set_title(ui->window, pattern_get_filename(ui->p));
    gtk_widget_queue_draw(ui->window->plot);
}

static void
pattern_ui_sync(pattern_ui_t *ui,
                gboolean      lock,
                gboolean      interactive)
{
    gboolean active = !lock;

    ui->lock++;
    pattern_ui_sync_name(ui, FALSE);
    pattern_ui_sync_freq(ui, FALSE);
    pattern_ui_sync_avg(ui, FALSE);
    pattern_ui_sync_color(ui, FALSE);
    pattern_ui_sync_hide(ui, FALSE);
    pattern_ui_sync_fill(ui, FALSE);
    pattern_ui_sync_rev(ui, FALSE);

    gtk_widget_set_sensitive(ui->window->b_new, (lock && interactive) ? FALSE : TRUE);
    gtk_widget_set_sensitive(ui->window->b_load, (lock && interactive) ? FALSE : TRUE);
    gtk_widget_set_sensitive(ui->window->b_save, (lock && interactive) ? FALSE : TRUE);
    gtk_widget_set_sensitive(ui->window->b_save_as, (lock && interactive) ? FALSE : TRUE);
    gtk_widget_set_sensitive(ui->window->b_render, (lock && interactive) ? FALSE : TRUE);

    gtk_widget_set_sensitive(ui->window->b_add, (lock && interactive) ? FALSE : TRUE);
    gtk_widget_set_sensitive(ui->window->b_down, active);
    gtk_widget_set_sensitive(ui->window->b_up, active);
    gtk_widget_set_sensitive(ui->window->box_select, (lock && interactive) ? FALSE : TRUE);
    gtk_widget_set_sensitive(ui->window->b_export, active);
    gtk_widget_set_sensitive(ui->window->b_remove, active);
    gtk_widget_set_sensitive(ui->window->b_clear, active);

    if (!interactive)
    {
        gtk_widget_set_sensitive(ui->window->e_name, active);
        gtk_widget_set_sensitive(ui->window->s_freq, active);
        gtk_widget_set_sensitive(ui->window->s_avg, active);
        gtk_widget_set_sensitive(ui->window->b_color, active);
        gtk_widget_set_sensitive(ui->window->b_color_next, active);
        gtk_widget_set_sensitive(ui->window->b_rotate_reset, active);
        gtk_widget_set_sensitive(ui->window->b_rotate_ccw_fast, active);
        gtk_widget_set_sensitive(ui->window->b_rotate_ccw, active);
        gtk_widget_set_sensitive(ui->window->b_rotate_peak, active);
        gtk_widget_set_sensitive(ui->window->b_rotate_cw, active);
        gtk_widget_set_sensitive(ui->window->b_rotate_cw_fast, active);
        gtk_widget_set_sensitive(ui->window->b_fill, active);
        gtk_widget_set_sensitive(ui->window->b_rev, active);
        gtk_widget_set_sensitive(ui->window->b_hide, active);
    }

    ui->lock--;
}

static void
pattern_ui_drag_data_received(GtkWidget        *widget,
                              GdkDragContext   *context,
                              gint              x,
                              gint              y,
                              GtkSelectionData *selection_data,
                              guint             info,
                              guint             time,
                              pattern_ui_t     *ui)
{
    gchar **list, **uri;
    gchar *current;
    GSList *filenames = NULL;

    /* Ignore drag and drop events in interactive mode */
    if (ui->interactive)
        return;

    if (selection_data == NULL)
        return;

    if (info != UI_DRAG_URI_LIST_ID)
        return;

    list = gtk_selection_data_get_uris(selection_data);
    if (list == NULL)
        return;

    for (uri = list; *uri; uri++)
    {
        current = g_filename_from_uri(*uri, NULL, NULL);
        if (current)
            filenames = g_slist_prepend(filenames, current);
    }
    g_strfreev(list);

    if (filenames)
    {
        pattern_ui_read(ui, filenames);
        g_slist_free_full(filenames, g_free);
    }
}

static void
pattern_ui_format_desc(GtkCellLayout   *cell_layout,
                       GtkCellRenderer *renderer,
                       GtkTreeModel    *model,
                       GtkTreeIter     *iter,
                       gpointer         user_data)
{
    pattern_data_t *data;
    pattern_signal_t *s;
    GtkTreePath *path;
    gchar *text;
    gint *n;
    gint count;
    gint interp;

    gtk_tree_model_get(model, iter, PATTERN_COL_DATA, &data, -1);
    s = pattern_data_get_signal(data);
    path = gtk_tree_model_get_path(model, iter);
    n = gtk_tree_path_get_indices(path);
    count = pattern_signal_count(s);
    interp = pattern_signal_interp(s);

    if (interp > 1)
        text = g_strdup_printf("%d. %s (%d samples) [%dx]", n[0]+1, pattern_data_get_name(data), count, interp);
    else
        text = g_strdup_printf("%d. %s (%d samples)", n[0]+1, pattern_data_get_name(data), count);
    g_object_set(renderer,
                 "text", text,
                 "strikethrough", pattern_data_get_hide(data),
                 NULL);

    gtk_tree_path_free(path);
    g_free(text);
}

static gboolean
pattern_ui_format_zero(GtkSpinButton *widget,
                      gpointer       user_data)
{
    GtkAdjustment *adj = gtk_spin_button_get_adjustment(widget);
    gint value = (gint)gtk_adjustment_get_value(adj);

    if (!value)
    {
        gtk_entry_set_text(GTK_ENTRY(widget), "");
        return TRUE;
    }

    return FALSE;
}

static void
pattern_ui_read(pattern_ui_t *ui,
                GSList       *list)
{
    GSList *it;
    gchar *filename;
    pattern_import_t *im;
    pattern_data_t *data;
    gint index;
    gint error;
    gboolean added = FALSE;
    GdkRGBA color;

    for (it = list; it; it = it->next)
    {
        filename = (gchar*)it->data;

        im = pattern_import_new();
        error = pattern_import(im, filename);
        if (error != PATTERN_IMPORT_OK)
        {
            pattern_import_free(im, TRUE);
            switch (error)
            {
            case PATTERN_IMPORT_ERROR:
                pattern_ui_dialog(GTK_WINDOW(ui->window->window), GTK_MESSAGE_ERROR,
                                  APP_TITLE,
                                  "Unable to open the file:\n%s",
                                  filename);
                continue;

            case PATTERN_IMPORT_INVALID_FORMAT:
                pattern_ui_dialog(GTK_WINDOW(ui->window->window), GTK_MESSAGE_ERROR,
                                  APP_TITLE,
                                  "Invalid file format:\n%s",
                                  filename);
                continue;

            case PATTERN_IMPORT_EMPTY_FILE:
                pattern_ui_dialog(GTK_WINDOW(ui->window->window), GTK_MESSAGE_ERROR,
                                  APP_TITLE,
                                  "This file does not contain any signal samples:\n%s",
                                  filename);
                continue;

            default:
                pattern_ui_dialog(GTK_WINDOW(ui->window->window), GTK_MESSAGE_ERROR,
                                  APP_TITLE,
                                  "Unknown error:\n%s",
                                  filename);
                continue;
            }
        }

        data = pattern_data_new(pattern_import_get_signal(im));
        pattern_data_set_name(data, pattern_import_get_name(im));
        pattern_data_set_freq(data, pattern_import_get_freq(im));
        color = pattern_color_next();
        pattern_data_set_color(data, &color);
        pattern_add(ui->p, data);

        pattern_import_free(im, FALSE);
        added = TRUE;
    }

    if (added)
    {
        index = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(pattern_get_model(ui->p)), NULL) - 1;
        gtk_combo_box_set_active(GTK_COMBO_BOX(ui->window->c_select), index);
        gtk_widget_queue_draw(ui->window->plot);
    }
}

void
pattern_ui_sync_name(pattern_ui_t *ui,
                     gboolean      redraw)
{
    pattern_data_t *data = pattern_get_current(ui->p);
    const gchar *name = data ? pattern_data_get_name(data) : "";

    ui->lock++;
    gtk_entry_set_text(GTK_ENTRY(ui->window->e_name), name);
    ui->lock--;

    if (redraw)
    {
        gtk_widget_queue_draw(ui->window->plot);
        gtk_widget_queue_draw(ui->window->c_select);
    }
}

void
pattern_ui_sync_freq(pattern_ui_t *ui,
                     gboolean      redraw)
{
    pattern_data_t *data = pattern_get_current(ui->p);
    gint freq = data ? pattern_data_get_freq(data) : 0;

    ui->lock++;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui->window->s_freq), freq);
    ui->lock--;

    if (redraw)
    {
        gtk_widget_queue_draw(ui->window->plot);
    }
}

void
pattern_ui_sync_avg(pattern_ui_t *ui,
                    gboolean      redraw)
{
    pattern_data_t *data = pattern_get_current(ui->p);
    gint avg = data ? pattern_signal_get_avg(pattern_data_get_signal(data)) : 0;

    ui->lock++;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui->window->s_avg), avg);
    ui->lock--;

    if (redraw)
    {
        gtk_widget_queue_draw(ui->window->plot);
    }
}

void
pattern_ui_sync_color(pattern_ui_t *ui,
                      gboolean      redraw)
{
    static const GdkRGBA default_color = {0};
    pattern_data_t *data = pattern_get_current(ui->p);
    const GdkRGBA *color = data ? pattern_data_get_color(data) : &default_color;

    ui->lock++;
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(ui->window->b_color), color);
    ui->lock--;

    if (redraw)
    {
        gtk_widget_queue_draw(ui->window->plot);
    }
}

void
pattern_ui_sync_hide(pattern_ui_t *ui,
                     gboolean      redraw)
{
    pattern_data_t *data = pattern_get_current(ui->p);
    gboolean hide = data ? pattern_data_get_hide(data) : FALSE;

    ui->lock++;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui->window->b_hide), hide);
    ui->lock--;

    if (redraw)
    {
        gtk_widget_queue_draw(ui->window->plot);
    }
}

void
pattern_ui_sync_fill(pattern_ui_t *ui,
                     gboolean      redraw)
{
    pattern_data_t *data = pattern_get_current(ui->p);
    gboolean fill = data ? pattern_data_get_fill(data) : FALSE;

    ui->lock++;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui->window->b_fill), fill);
    ui->lock--;

    if (redraw)
    {
        gtk_widget_queue_draw(ui->window->plot);
    }
}

void
pattern_ui_sync_rev(pattern_ui_t *ui,
                    gboolean      redraw)
{
    pattern_data_t *data = pattern_get_current(ui->p);
    gboolean rev = data ? pattern_signal_get_rev(pattern_data_get_signal(data)) : FALSE;

    ui->lock++;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui->window->b_rev), rev);
    ui->lock--;

    if (redraw)
    {
        gtk_widget_queue_draw(ui->window->plot);
    }
}

void
pattern_ui_sync_data(pattern_ui_t *ui)
{
    gtk_widget_queue_draw(ui->window->plot);
    gtk_widget_queue_draw(ui->window->c_select);
}

pattern_t*
pattern_ui_get_pattern(pattern_ui_t *ui)
{
    return ui->p;
}

GtkWindow*
pattern_ui_get_plot_window(pattern_ui_t *ui)
{
    gboolean plot_window = gtk_widget_get_visible(ui->window->window_plot);
    return GTK_WINDOW(plot_window ? ui->window->window_plot : ui->window->window);
}

void
pattern_ui_set_focus_idx(pattern_ui_t *ui,
                         gint          value)
{
    ui->focus_idx = value;
}

gint
pattern_ui_get_focus_idx(const pattern_ui_t *ui)
{
    return ui->focus_idx;
}

void
pattern_ui_set_rotating_idx(pattern_ui_t *ui,
                            gint          value)
{
    ui->rotating_idx = value;
}

gint
pattern_ui_get_rotating_idx(const pattern_ui_t *ui)
{
    return ui->rotating_idx;
}

void
pattern_ui_interactive(pattern_ui_t *ui,
                       gboolean      active)
{
    ui->interactive = active;

    if (active)
    {
        gint index = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(pattern_get_model(ui->p)), NULL) - 1;
        gtk_combo_box_set_active(GTK_COMBO_BOX(ui->window->c_select), index);
        pattern_ui_sync(ui, TRUE, TRUE);
    }
    else
    {
        pattern_ui_sync(ui, FALSE, FALSE);
    }

    gtk_widget_queue_draw(ui->window->plot);
}
