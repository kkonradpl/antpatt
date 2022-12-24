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
#include "pattern-ui.h"
#include "pattern-ui-dialogs.h"
#include "pattern-plot.h"
#include "pattern-import.h"
#include "pattern-color.h"
#include "pattern-json.h"
#include "pattern-export.h"
#include "pattern-ipc.h"

#define UI_DRAG_URI_LIST_ID 0

static const GtkTargetEntry drop_types[] = {{ "text/uri-list", 0, UI_DRAG_URI_LIST_ID }};
static const gint n_drop_types = sizeof(drop_types) / sizeof(drop_types[0]);

static gboolean pattern_ui_changed(pattern_t*);
static gboolean pattern_ui_delete(GtkWidget*, GdkEvent*, pattern_t*);
static void pattern_ui_destroy(GtkWidget*, pattern_t*);

static void pattern_ui_new(GtkWidget*, pattern_t*);
static void pattern_ui_load(GtkWidget*, pattern_t*);
static void pattern_ui_save(GtkWidget*, pattern_t*);
static void pattern_ui_save_as(GtkWidget*, pattern_t*);
static void pattern_ui_render(GtkWidget*, pattern_t*);
static void pattern_ui_about(GtkWidget*, pattern_t*);

static void pattern_ui_size(GtkWidget*, pattern_t*);
static void pattern_ui_title(GtkWidget*, pattern_t*);
static void pattern_ui_scale(GtkWidget*, pattern_t*);
static void pattern_ui_line(GtkWidget*, pattern_t*);
static void pattern_ui_interp(GtkWidget*, pattern_t*);
static void pattern_ui_full_angle(GtkWidget*, pattern_t*);
static void pattern_ui_black(GtkWidget*, pattern_t*);
static void pattern_ui_normalize(GtkWidget*, pattern_t*);
static void pattern_ui_legend(GtkWidget*, pattern_t*);

static void pattern_ui_add(GtkWidget*, pattern_t*);
static void pattern_ui_down(GtkWidget*, pattern_t*);
static void pattern_ui_up(GtkWidget*, pattern_t*);

static void pattern_ui_select(GtkWidget*, pattern_t*);
static void pattern_ui_export(GtkWidget*, pattern_t*);
static void pattern_ui_remove(GtkWidget*, pattern_t*);
static void pattern_ui_clear(GtkWidget*, pattern_t*);
static void pattern_ui_name(GtkWidget*, pattern_t*);
static void pattern_ui_freq(GtkWidget*, pattern_t*);
static void pattern_ui_avg(GtkWidget*, pattern_t*);
static void pattern_ui_color(GtkWidget*, pattern_t*);
static void pattern_ui_color_next(GtkWidget*, pattern_t*);
static void pattern_ui_rotate_reset(GtkWidget*, pattern_t*);
static void pattern_ui_rotate_ccw_fast(GtkWidget*, pattern_t*);
static void pattern_ui_rotate_ccw(GtkWidget*, pattern_t*);
static void pattern_ui_rotate_peak(GtkWidget*, pattern_t*);
static void pattern_ui_rotate_cw(GtkWidget*, pattern_t*);
static void pattern_ui_rotate_cw_fast(GtkWidget*, pattern_t*);
static void pattern_ui_fill(GtkWidget*, pattern_t*);
static void pattern_ui_rev(GtkWidget*, pattern_t*);
static void pattern_ui_hide(GtkWidget*, pattern_t*);

static void pattern_ui_sync_full(pattern_t *p);
static void pattern_ui_sync(pattern_t*, gboolean, gboolean);

static void pattern_ui_drag_data_received(GtkWidget*, GdkDragContext*, gint, gint, GtkSelectionData*, guint, guint, gpointer);

static void pattern_ui_format_desc(GtkCellLayout*, GtkCellRenderer*, GtkTreeModel*, GtkTreeIter*, gpointer);
static gboolean pattern_ui_format_avg(GtkSpinButton*, gpointer);
static gboolean pattern_ui_format_freq(GtkSpinButton*, gpointer);

static void pattern_ui_read(pattern_t*, GSList*);


void
pattern_ui_create(pattern_t *p)
{
    pattern_ui_window_t *ui = pattern_get_ui(p);
    gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(ui->c_select), ui->r_select, pattern_ui_format_desc, NULL, NULL);

    /* Signals */
    g_signal_connect(ui->window, "delete-event", G_CALLBACK(pattern_ui_delete), p);
    g_signal_connect(ui->window, "destroy", G_CALLBACK(pattern_ui_destroy), p);

    g_signal_connect(ui->b_new, "clicked", G_CALLBACK(pattern_ui_new), p);
    g_signal_connect(ui->b_load, "clicked", G_CALLBACK(pattern_ui_load), p);
    g_signal_connect(ui->b_save, "clicked", G_CALLBACK(pattern_ui_save), p);
    g_signal_connect(ui->b_save_as, "clicked", G_CALLBACK(pattern_ui_save_as), p);
    g_signal_connect(ui->b_render, "clicked", G_CALLBACK(pattern_ui_render), p);
    g_signal_connect(ui->b_about, "clicked", G_CALLBACK(pattern_ui_about), p);

    g_signal_connect(ui->s_size, "value-changed", G_CALLBACK(pattern_ui_size), p);
    g_signal_connect(ui->e_title, "changed", G_CALLBACK(pattern_ui_title), p);
    g_signal_connect(ui->c_scale, "changed", G_CALLBACK(pattern_ui_scale), p);
    g_signal_connect(ui->s_line, "value-changed", G_CALLBACK(pattern_ui_line), p);

    g_signal_connect(ui->c_interp, "changed", G_CALLBACK(pattern_ui_interp), p);
    g_signal_connect(ui->b_full_angle, "toggled", G_CALLBACK(pattern_ui_full_angle), p);
    g_signal_connect(ui->b_black, "toggled", G_CALLBACK(pattern_ui_black), p);
    g_signal_connect(ui->b_normalize, "toggled", G_CALLBACK(pattern_ui_normalize), p);
    g_signal_connect(ui->b_legend, "toggled", G_CALLBACK(pattern_ui_legend), p);

    g_signal_connect(ui->plot, "motion-notify-event", G_CALLBACK(pattern_plot_motion), p);
    g_signal_connect(ui->plot, "button-press-event", G_CALLBACK(pattern_plot_click), p);
    g_signal_connect(ui->plot, "button-release-event", G_CALLBACK(pattern_plot_click), p);
    g_signal_connect(ui->plot, "leave-notify-event", G_CALLBACK(pattern_plot_leave), p);
    g_signal_connect(ui->plot, "draw", G_CALLBACK(pattern_plot), p);

    g_signal_connect(ui->b_add, "clicked", G_CALLBACK(pattern_ui_add), p);
    g_signal_connect(ui->b_down, "clicked", G_CALLBACK(pattern_ui_down), p);
    g_signal_connect(ui->b_up, "clicked", G_CALLBACK(pattern_ui_up), p);
    g_signal_connect(ui->c_select, "changed", G_CALLBACK(pattern_ui_select), p);
    g_signal_connect(ui->b_export, "clicked", G_CALLBACK(pattern_ui_export), p);
    g_signal_connect(ui->b_remove, "clicked", G_CALLBACK(pattern_ui_remove), p);
    g_signal_connect(ui->b_clear, "clicked", G_CALLBACK(pattern_ui_clear), p);

    g_signal_connect(ui->e_name, "changed", G_CALLBACK(pattern_ui_name), p);
    g_signal_connect(ui->s_freq, "value-changed", G_CALLBACK(pattern_ui_freq), p);
    g_signal_connect(ui->s_freq, "output", G_CALLBACK(pattern_ui_format_freq), NULL);
    g_signal_connect(ui->s_avg, "value-changed", G_CALLBACK(pattern_ui_avg), p);
    g_signal_connect(ui->s_avg, "output", G_CALLBACK(pattern_ui_format_avg), NULL);
    g_signal_connect(ui->b_color, "color-set", G_CALLBACK(pattern_ui_color), p);
    g_signal_connect(ui->b_color_next, "clicked", G_CALLBACK(pattern_ui_color_next), p);
    g_signal_connect(ui->b_rotate_reset, "clicked", G_CALLBACK(pattern_ui_rotate_reset), p);
    g_signal_connect(ui->b_rotate_ccw_fast, "clicked", G_CALLBACK(pattern_ui_rotate_ccw_fast), p);
    g_signal_connect(ui->b_rotate_ccw, "clicked", G_CALLBACK(pattern_ui_rotate_ccw), p);
    g_signal_connect(ui->b_rotate_peak, "clicked", G_CALLBACK(pattern_ui_rotate_peak), p);
    g_signal_connect(ui->b_rotate_cw, "clicked", G_CALLBACK(pattern_ui_rotate_cw), p);
    g_signal_connect(ui->b_rotate_cw_fast, "clicked", G_CALLBACK(pattern_ui_rotate_cw_fast), p);
    g_signal_connect(ui->b_fill, "toggled", G_CALLBACK(pattern_ui_fill), p);
    g_signal_connect(ui->b_rev, "toggled", G_CALLBACK(pattern_ui_rev), p);
    g_signal_connect(ui->b_hide, "toggled", G_CALLBACK(pattern_ui_hide), p);

    /* Drag and drop support */
    gtk_drag_dest_set(ui->window_plot, GTK_DEST_DEFAULT_ALL, drop_types, n_drop_types, GDK_ACTION_COPY);
    gtk_drag_dest_set(ui->window, GTK_DEST_DEFAULT_ALL, drop_types, n_drop_types, GDK_ACTION_COPY);
    g_signal_connect(ui->window_plot, "drag-data-received", G_CALLBACK(pattern_ui_drag_data_received), p);
    g_signal_connect(ui->window, "drag-data-received", G_CALLBACK(pattern_ui_drag_data_received), p);

    pattern_ui_sync_full(p);
    gtk_widget_show_all(ui->window);
}

static gboolean
pattern_ui_changed(pattern_t *p)
{
    if (!pattern_changed(p))
        return FALSE;

    switch (pattern_ui_dialog_ask_unsaved(GTK_WINDOW(pattern_get_ui(p)->window)))
    {
        case GTK_RESPONSE_YES:
            gtk_button_clicked(GTK_BUTTON(pattern_get_ui(p)->b_save));
            return pattern_changed(p);
        case GTK_RESPONSE_NO:
            return FALSE;
        default:
            return TRUE;
    }
}

static gboolean
pattern_ui_delete(GtkWidget *widget,
                  GdkEvent  *event,
                  pattern_t *p)
{
    return pattern_ui_changed(p) ? GDK_EVENT_STOP : GDK_EVENT_PROPAGATE;
}

static void
pattern_ui_destroy(GtkWidget *widget,
                   pattern_t *p)
{
    pattern_free(p);
    gtk_main_quit();
}

static void
pattern_ui_new(GtkWidget *widget,
               pattern_t *p)
{
    if (pattern_ui_changed(p))
        return;

    gint size = pattern_get_size(p);
    pattern_reset(p);
    pattern_set_size(p, size);
    pattern_unchanged(p);
    pattern_ui_sync_full(p);
}

static void
pattern_ui_load(GtkWidget *widget,
                pattern_t *p)
{
    g_autofree gchar *filename = NULL;
    g_autofree gchar *error = NULL;

    if (pattern_ui_changed(p))
        return;

    filename = pattern_ui_dialog_open(GTK_WINDOW(pattern_get_ui(p)->window));
    if (filename)
    {
        pattern_reset(p);
        if (!pattern_json_load(p, filename, &error))
        {
            pattern_ui_dialog(NULL,
                              GTK_MESSAGE_ERROR,
                              "Error",
                              error);
        }

        pattern_ui_sync_full(p);
    }
}

static void
pattern_ui_save(GtkWidget *widget,
                pattern_t *p)
{
    const gchar *filename = pattern_get_filename(p);

    if (!filename)
    {
        pattern_ui_save_as(widget, p);
        return;
    }

    if (!pattern_json_save(p, filename, FALSE))
    {
        pattern_ui_dialog(GTK_WINDOW(pattern_get_ui(p)->window), GTK_MESSAGE_ERROR,
                          APP_TITLE,
                          "Unable to save the file.");
    }
    pattern_unchanged(p);
}

static void
pattern_ui_save_as(GtkWidget *widget,
                   pattern_t *p)
{
    g_autofree gchar *filename = pattern_ui_dialog_save(GTK_WINDOW(pattern_get_ui(p)->window));

    if (filename)
    {
        if (!pattern_json_save(p, filename, FALSE))
        {
            pattern_ui_dialog(GTK_WINDOW(pattern_get_ui(p)->window), GTK_MESSAGE_ERROR,
                              APP_TITLE,
                              "Unable to save the file.");
        }
        pattern_ui_window_set_title(pattern_get_ui(p), filename);
        pattern_set_filename(p, filename);
        pattern_unchanged(p);
    }
}

static void
pattern_ui_render(GtkWidget *widget,
                  pattern_t *p)
{
    g_autofree gchar* filename = pattern_ui_dialog_render(GTK_WINDOW(pattern_get_ui(p)->window));

    if (filename)
    {
        if (!pattern_plot_to_file(p, filename))
        {
            pattern_ui_dialog(GTK_WINDOW(pattern_get_ui(p)->window), GTK_MESSAGE_ERROR,
                              APP_TITLE,
                              "Unable to save the image.");
        }
    }
}

static void
pattern_ui_about(GtkWidget *widget,
                 pattern_t *p)
{
    pattern_ui_dialog_about(GTK_WINDOW(pattern_get_ui(p)->window));
}

static void
pattern_ui_size(GtkWidget *widget,
                pattern_t *p)
{
    gint size = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
    gtk_widget_set_size_request(pattern_get_ui(p)->plot, size, size);

    if (pattern_get_ui(p)->lock)
        return;

    pattern_set_size(p, size);
}

static void
pattern_ui_title(GtkWidget *widget,
                 pattern_t *p)
{
    if (pattern_get_ui(p)->lock)
        return;

    pattern_set_title(p, gtk_entry_get_text(GTK_ENTRY(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_scale(GtkWidget *widget,
                 pattern_t *p)
{
    if (pattern_get_ui(p)->lock)
        return;

    gint scale = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));

    if(scale == PATTERN_UI_SCALE_ARRL)
        pattern_set_scale(p, 0);
    else if(scale == PATTERN_UI_SCALE_LINEAR_20)
        pattern_set_scale(p, -20);
    else if(scale == PATTERN_UI_SCALE_LINEAR_30)
        pattern_set_scale(p, -30);
    else if(scale == PATTERN_UI_SCALE_LINEAR_40)
        pattern_set_scale(p, -40);
    else if(scale == PATTERN_UI_SCALE_LINEAR_50)
        pattern_set_scale(p, -50);
    else if(scale == PATTERN_UI_SCALE_LINEAR_60)
        pattern_set_scale(p, -60);

    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_line(GtkWidget *widget,
                pattern_t *p)
{
    if (pattern_get_ui(p)->lock)
        return;

    pattern_set_line(p, gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_interp(GtkWidget *widget,
                  pattern_t *p)
{
    if (pattern_get_ui(p)->lock)
        return;

    pattern_set_interp(p, gtk_combo_box_get_active(GTK_COMBO_BOX(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_full_angle(GtkWidget *widget,
                      pattern_t *p)
{
    if (pattern_get_ui(p)->lock)
        return;

    pattern_set_full_angle(p, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_black(GtkWidget *widget,
                 pattern_t *p)
{
    if (pattern_get_ui(p)->lock)
        return;

    pattern_set_black(p, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_normalize(GtkWidget *widget,
                     pattern_t *p)
{
    if (pattern_get_ui(p)->lock)
        return;

    pattern_set_normalize(p, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_legend(GtkWidget *widget,
                  pattern_t *p)
{
    pattern_set_legend(p, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_add(GtkWidget *widget,
               pattern_t *p)
{
    GSList *list = pattern_ui_dialog_import(GTK_WINDOW(pattern_get_ui(p)->window));
    if(list)
    {
        pattern_ui_read(p, list);
        g_slist_free_full(list, g_free);
    }
}

static void
pattern_ui_down(GtkWidget *widget,
                pattern_t *p)
{
    GtkTreeIter iter;
    GtkTreeIter next;

    if(!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(pattern_get_ui(p)->c_select), &iter))
        return;
    next = iter;
    if(!gtk_tree_model_iter_next(GTK_TREE_MODEL(pattern_get_model(p)), &next))
        return;

    gtk_list_store_move_after(pattern_get_model(p), &iter, &next);
    gtk_widget_queue_draw(pattern_get_ui(p)->c_select);
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_up(GtkWidget *widget,
              pattern_t *p)
{
    GtkTreeIter iter;
    GtkTreeIter prev;
    GtkTreePath *path;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(pattern_get_ui(p)->c_select), &iter))
        return;

    if (gtk_combo_box_get_active(GTK_COMBO_BOX(pattern_get_ui(p)->c_select)) == 0)
        return;

    path = gtk_tree_model_get_path(GTK_TREE_MODEL(pattern_get_model(p)), &iter);
    gtk_tree_path_prev(path);

    if (gtk_tree_model_get_iter(GTK_TREE_MODEL(pattern_get_model(p)), &prev, path))
    {
        gtk_list_store_move_before(pattern_get_model(p), &iter, &prev);
        gtk_widget_queue_draw(pattern_get_ui(p)->c_select);
        gtk_widget_queue_draw(pattern_get_ui(p)->plot);
    }

    gtk_tree_path_free(path);
}

static void
pattern_ui_select(GtkWidget *widget,
                  pattern_t *p)
{
    if (pattern_get_ui(p)->lock)
        return;

    GtkTreeIter iter;
    pattern_data_t *data = NULL;
    gboolean active = gtk_combo_box_get_active_iter(GTK_COMBO_BOX(pattern_get_ui(p)->c_select), &iter);

    if (active)
        gtk_tree_model_get(GTK_TREE_MODEL(pattern_get_model(p)), &iter, PATTERN_COL_DATA, &data, -1);

    pattern_set_current(p, data);
    pattern_ui_sync(p, !active, FALSE);
}

static void
pattern_ui_export(GtkWidget *widget,
                  pattern_t *p)
{
    GtkTreeIter iter;
    pattern_data_t *data;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(pattern_get_ui(p)->c_select), &iter))
        return;

    gtk_tree_model_get(GTK_TREE_MODEL(pattern_get_model(p)), &iter, PATTERN_COL_DATA, &data, -1);
    if (!pattern_signal_count(pattern_data_get_signal(data)))
    {
        pattern_ui_dialog(GTK_WINDOW(pattern_get_ui(p)->window), GTK_MESSAGE_ERROR,
                          APP_TITLE,
                          "No signal samples available.");
        return;
    }

    gchar *filename = pattern_ui_dialog_export(GTK_WINDOW(pattern_get_ui(p)->window));
    if (filename)
    {
        if (!pattern_export(data, filename))
        {
            pattern_ui_dialog(GTK_WINDOW(pattern_get_ui(p)->window), GTK_MESSAGE_ERROR,
                              APP_TITLE,
                              "Unable to export the pattern data.");
        }
        g_free(filename);
    }
}

static void
pattern_ui_remove(GtkWidget *widget,
                  pattern_t *p)
{
    GtkTreeIter iter;
    GtkTreeIter next;
    GtkTreePath *path;

    if(!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(pattern_get_ui(p)->c_select), &iter))
        return;

    if (!pattern_ui_dialog_yesno(GTK_WINDOW(pattern_get_ui(p)->window),
                                 "Remove pattern data",
                                 "Do you really want to remove current pattern data?"))
    {
        return;
    }

    next = iter;
    if(gtk_tree_model_iter_next(GTK_TREE_MODEL(pattern_get_model(p)), &next))
        gtk_combo_box_set_active_iter(GTK_COMBO_BOX(pattern_get_ui(p)->c_select), &next);
    else
    {
        path = gtk_tree_model_get_path(GTK_TREE_MODEL(pattern_get_model(p)), &iter);
        gtk_tree_path_prev(path);
        if(gtk_tree_model_get_iter(GTK_TREE_MODEL(pattern_get_model(p)), &next, path))
            gtk_combo_box_set_active_iter(GTK_COMBO_BOX(pattern_get_ui(p)->c_select), &next);
        gtk_tree_path_free(path);
    }

    pattern_remove(p, &iter);
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_clear(GtkWidget *widget,
                 pattern_t *p)
{
    if (!pattern_ui_dialog_yesno(GTK_WINDOW(pattern_get_ui(p)->window),
                                 "Remove all pattern data",
                                 "Do you really want to remove all pattern data?"))
    {
        return;
    }

    pattern_clear(p);
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_name(GtkWidget *widget,
                pattern_t *p)
{
    if (pattern_get_ui(p)->lock)
        return;

    GtkTreeIter iter;

    if(!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(pattern_get_ui(p)->c_select), &iter))
        return;

    pattern_data_set_name(pattern_get_current(p), gtk_entry_get_text(GTK_ENTRY(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->c_select);
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_freq(GtkWidget *widget,
                pattern_t *p)
{
    if (pattern_get_ui(p)->lock)
        return;

    pattern_data_set_freq(pattern_get_current(p), gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_avg(GtkWidget *widget,
               pattern_t *p)
{
    if (pattern_get_ui(p)->lock)
        return;

    pattern_signal_set_avg(pattern_data_get_signal(pattern_get_current(p)), gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_color(GtkWidget *widget,
                 pattern_t *p)
{
    if (pattern_get_ui(p)->lock)
        return;

    GdkRGBA color;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), &color);
    pattern_data_set_color(pattern_get_current(p), &color);
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_color_next(GtkWidget *widget,
                      pattern_t *p)
{
    GdkRGBA next = pattern_color_next();
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(pattern_get_ui(p)->b_color), &next);
    g_signal_emit_by_name(pattern_get_ui(p)->b_color, "color-set", p);
}

static void
pattern_ui_rotate_reset(GtkWidget *widget,
                        pattern_t *p)
{
    pattern_signal_rotate_reset(pattern_data_get_signal(pattern_get_current(p)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_rotate_ccw_fast(GtkWidget *widget,
                     pattern_t *p)
{
    gint count = pattern_signal_count(pattern_data_get_signal(pattern_get_current(p)));
    pattern_signal_rotate(pattern_data_get_signal(pattern_get_current(p)), count / 18);
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_rotate_ccw(GtkWidget *widget,
                    pattern_t *p)
{
    pattern_signal_rotate(pattern_data_get_signal(pattern_get_current(p)), 1);
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_rotate_peak(GtkWidget *widget,
                       pattern_t *p)
{
    pattern_signal_rotate_0(pattern_data_get_signal(pattern_get_current(p)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_rotate_cw(GtkWidget *widget,
                    pattern_t *p)
{
    pattern_signal_rotate(pattern_data_get_signal(pattern_get_current(p)), -1);
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_rotate_cw_fast(GtkWidget *widget,
                     pattern_t *p)
{
    gint count = pattern_signal_count(pattern_data_get_signal(pattern_get_current(p)));
    pattern_signal_rotate(pattern_data_get_signal(pattern_get_current(p)), -count/18);
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_fill(GtkWidget *widget,
                pattern_t *p)
{
    if (pattern_get_ui(p)->lock)
        return;

    pattern_data_set_fill(pattern_get_current(p), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_rev(GtkWidget *widget,
               pattern_t *p)
{
    if (pattern_get_ui(p)->lock)
        return;

    pattern_signal_set_rev(pattern_data_get_signal(pattern_get_current(p)), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_hide(GtkWidget *widget,
                pattern_t *p)
{
    if (pattern_get_ui(p)->lock)
        return;

    pattern_hide(p, pattern_get_current(p), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->c_select);
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_sync_full(pattern_t *p)
{
    pattern_ui_window_t *ui = pattern_get_ui(p);
    ui->lock++;

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui->s_size), pattern_get_size(p));
    gtk_entry_set_text(GTK_ENTRY(ui->e_title), pattern_get_title(p));

    switch (pattern_get_scale(p))
    {
        default:
        case 0:
            gtk_combo_box_set_active(GTK_COMBO_BOX(ui->c_scale), PATTERN_UI_SCALE_ARRL);
            break;
        case -20:
            gtk_combo_box_set_active(GTK_COMBO_BOX(ui->c_scale), PATTERN_UI_SCALE_LINEAR_20);
            break;
        case -30:
            gtk_combo_box_set_active(GTK_COMBO_BOX(ui->c_scale), PATTERN_UI_SCALE_LINEAR_30);
            break;
        case -40:
            gtk_combo_box_set_active(GTK_COMBO_BOX(ui->c_scale), PATTERN_UI_SCALE_LINEAR_40);
            break;
        case -50:
            gtk_combo_box_set_active(GTK_COMBO_BOX(ui->c_scale), PATTERN_UI_SCALE_LINEAR_50);
            break;
        case -60:
            gtk_combo_box_set_active(GTK_COMBO_BOX(ui->c_scale), PATTERN_UI_SCALE_LINEAR_60);
            break;
    }

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui->s_line), pattern_get_line(p));
    gtk_combo_box_set_active(GTK_COMBO_BOX(ui->c_interp), pattern_get_interp(p));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui->b_full_angle), pattern_get_full_angle(p));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui->b_black), pattern_get_black(p));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui->b_normalize), pattern_get_normalize(p));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui->b_legend), pattern_get_legend(p));

    /* Set model */
    gtk_combo_box_set_model(GTK_COMBO_BOX(ui->c_select), GTK_TREE_MODEL(pattern_get_model(p)));

    pattern_ui_sync(p, TRUE, FALSE);
    g_signal_emit_by_name(ui->s_size, "value-changed", p);
    ui->lock--;

    if(gtk_tree_model_iter_n_children(GTK_TREE_MODEL(pattern_get_model(p)), NULL))
        gtk_combo_box_set_active(GTK_COMBO_BOX(ui->c_select), 0);

    pattern_ui_window_set_title(pattern_get_ui(p), pattern_get_filename(p));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_sync(pattern_t *p,
                gboolean   lock,
                gboolean   live)
{
    pattern_ui_window_t *ui = pattern_get_ui(p);
    gboolean active = !lock;

    ui->lock++;
    pattern_ui_sync_name(p, FALSE);
    pattern_ui_sync_freq(p, FALSE);
    pattern_ui_sync_avg(p, FALSE);
    pattern_ui_sync_color(p, FALSE);
    pattern_ui_sync_hide(p, FALSE);
    pattern_ui_sync_fill(p, FALSE);
    pattern_ui_sync_rev(p, FALSE);

    gtk_widget_set_sensitive(pattern_get_ui(p)->b_new, (lock && live) ? FALSE : TRUE);
    gtk_widget_set_sensitive(pattern_get_ui(p)->b_load, (lock && live) ? FALSE : TRUE);
    gtk_widget_set_sensitive(pattern_get_ui(p)->b_save, (lock && live) ? FALSE : TRUE);
    gtk_widget_set_sensitive(pattern_get_ui(p)->b_save_as, (lock && live) ? FALSE : TRUE);
    gtk_widget_set_sensitive(pattern_get_ui(p)->b_render, (lock && live) ? FALSE : TRUE);

    gtk_widget_set_sensitive(pattern_get_ui(p)->b_add, (lock && live) ? FALSE : TRUE);
    gtk_widget_set_sensitive(pattern_get_ui(p)->b_down, active);
    gtk_widget_set_sensitive(pattern_get_ui(p)->b_up, active);
    gtk_widget_set_sensitive(pattern_get_ui(p)->box_select, (lock && live) ? FALSE : TRUE);
    gtk_widget_set_sensitive(pattern_get_ui(p)->b_export, active);
    gtk_widget_set_sensitive(pattern_get_ui(p)->b_remove, active);
    gtk_widget_set_sensitive(pattern_get_ui(p)->b_clear, active);

    if (!live)
    {
        gtk_widget_set_sensitive(pattern_get_ui(p)->e_name, active);
        gtk_widget_set_sensitive(pattern_get_ui(p)->s_freq, active);
        gtk_widget_set_sensitive(pattern_get_ui(p)->s_avg, active);
        gtk_widget_set_sensitive(pattern_get_ui(p)->b_color, active);
        gtk_widget_set_sensitive(pattern_get_ui(p)->b_color_next, active);
        gtk_widget_set_sensitive(pattern_get_ui(p)->b_rotate_reset, active);
        gtk_widget_set_sensitive(pattern_get_ui(p)->b_rotate_ccw_fast, active);
        gtk_widget_set_sensitive(pattern_get_ui(p)->b_rotate_ccw, active);
        gtk_widget_set_sensitive(pattern_get_ui(p)->b_rotate_peak, active);
        gtk_widget_set_sensitive(pattern_get_ui(p)->b_rotate_cw, active);
        gtk_widget_set_sensitive(pattern_get_ui(p)->b_rotate_cw_fast, active);
        gtk_widget_set_sensitive(pattern_get_ui(p)->b_fill, active);
        gtk_widget_set_sensitive(pattern_get_ui(p)->b_rev, active);
        gtk_widget_set_sensitive(pattern_get_ui(p)->b_hide, active);
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
                              gpointer          user_data)
{
    pattern_t *p = (pattern_t*)user_data;
    gchar **list, **uri;
    gchar *current;
    GSList *filenames = NULL;

    if(!selection_data || info != UI_DRAG_URI_LIST_ID)
        return;

    list = gtk_selection_data_get_uris(selection_data);
    if(!list)
        return;

    for(uri = list; *uri; uri++)
    {
        current = g_filename_from_uri(*uri, NULL, NULL);
        if(current)
            filenames = g_slist_prepend(filenames, current);
    }
    g_strfreev(list);

    if(filenames)
    {
        pattern_ui_read(p, filenames);
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

    if(interp > 1)
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
pattern_ui_format_avg(GtkSpinButton *widget,
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

static gboolean
pattern_ui_format_freq(GtkSpinButton *widget,
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
pattern_ui_read(pattern_t *p,
                GSList    *list)
{
    GSList *it;
    gchar *filename;
    pattern_import_t *im;
    pattern_data_t *data;
    gint index;
    gint error;
    gboolean added = FALSE;
    GdkRGBA color;

    for(it = list; it; it = it->next)
    {
        filename = (gchar*)it->data;

        im = pattern_import_new();
        error = pattern_import(im, filename);
        if(error != PATTERN_IMPORT_OK)
        {
            pattern_import_free(im, TRUE);
            switch(error)
            {
            case PATTERN_IMPORT_ERROR:
                pattern_ui_dialog(GTK_WINDOW(pattern_get_ui(p)->window), GTK_MESSAGE_ERROR,
                                  APP_TITLE,
                                  "Unable to open the file:\n%s",
                                  filename);
                continue;

            case PATTERN_IMPORT_INVALID_FORMAT:
                pattern_ui_dialog(GTK_WINDOW(pattern_get_ui(p)->window), GTK_MESSAGE_ERROR,
                                  APP_TITLE,
                                  "Invalid file format:\n%s",
                                  filename);
                continue;

            case PATTERN_IMPORT_EMPTY_FILE:
                pattern_ui_dialog(GTK_WINDOW(pattern_get_ui(p)->window), GTK_MESSAGE_ERROR,
                                  APP_TITLE,
                                  "This file does not contain any signal samples:\n%s",
                                  filename);
                continue;

            default:
                pattern_ui_dialog(GTK_WINDOW(pattern_get_ui(p)->window), GTK_MESSAGE_ERROR,
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
        pattern_add(p, data);

        pattern_import_free(im, FALSE);
        added = TRUE;
    }

    if(added)
    {
        index = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(pattern_get_model(p)), NULL) - 1;
        gtk_combo_box_set_active(GTK_COMBO_BOX(pattern_get_ui(p)->c_select), index);
        gtk_widget_queue_draw(pattern_get_ui(p)->plot);
    }
}

void
pattern_ui_sync_name(pattern_t *p,
                     gboolean   redraw)
{
    pattern_data_t *data = pattern_get_current(p);
    const gchar *name = data ? pattern_data_get_name(data) : "";
    pattern_ui_window_t *ui = pattern_get_ui(p);

    ui->lock++;
    gtk_entry_set_text(GTK_ENTRY(pattern_get_ui(p)->e_name), name);
    ui->lock--;

    if (redraw)
    {
        gtk_widget_queue_draw(pattern_get_ui(p)->plot);
        gtk_widget_queue_draw(pattern_get_ui(p)->c_select);
    }
}

void
pattern_ui_sync_freq(pattern_t *p,
                     gboolean   redraw)
{
    pattern_data_t *data = pattern_get_current(p);
    gint freq = data ? pattern_data_get_freq(data) : 0;
    pattern_ui_window_t *ui = pattern_get_ui(p);

    ui->lock++;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pattern_get_ui(p)->s_freq), freq);
    ui->lock--;

    if (redraw)
    {
        gtk_widget_queue_draw(pattern_get_ui(p)->plot);
    }
}

void
pattern_ui_sync_avg(pattern_t *p,
                    gboolean   redraw)
{
    pattern_data_t *data = pattern_get_current(p);
    gint avg = data ? pattern_signal_get_avg(pattern_data_get_signal(data)) : 0;
    pattern_ui_window_t *ui = pattern_get_ui(p);

    ui->lock++;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pattern_get_ui(p)->s_avg), avg);
    ui->lock--;

    if (redraw)
    {
        gtk_widget_queue_draw(pattern_get_ui(p)->plot);
    }
}

void
pattern_ui_sync_color(pattern_t *p,
                      gboolean   redraw)
{
    static const GdkRGBA default_color = {0};
    pattern_data_t *data = pattern_get_current(p);
    const GdkRGBA *color = data ? pattern_data_get_color(data) : &default_color;
    pattern_ui_window_t *ui = pattern_get_ui(p);

    ui->lock++;
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(pattern_get_ui(p)->b_color), color);
    ui->lock--;

    if (redraw)
    {
        gtk_widget_queue_draw(pattern_get_ui(p)->plot);
    }
}

void
pattern_ui_sync_hide(pattern_t *p,
                     gboolean   redraw)
{
    pattern_data_t *data = pattern_get_current(p);
    gboolean hide = data ? pattern_data_get_hide(data) : FALSE;
    pattern_ui_window_t *ui = pattern_get_ui(p);

    ui->lock++;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pattern_get_ui(p)->b_hide), hide);
    ui->lock--;

    if (redraw)
    {
        gtk_widget_queue_draw(pattern_get_ui(p)->plot);
    }
}

void
pattern_ui_sync_fill(pattern_t *p,
                     gboolean   redraw)
{
    pattern_data_t *data = pattern_get_current(p);
    gboolean fill = data ? pattern_data_get_fill(data) : FALSE;
    pattern_ui_window_t *ui = pattern_get_ui(p);

    ui->lock++;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pattern_get_ui(p)->b_fill), fill);
    ui->lock--;

    if (redraw)
    {
        gtk_widget_queue_draw(pattern_get_ui(p)->plot);
    }
}

void
pattern_ui_sync_rev(pattern_t *p,
                    gboolean   redraw)
{
    pattern_data_t *data = pattern_get_current(p);
    gboolean rev = data ? pattern_signal_get_rev(pattern_data_get_signal(data)) : FALSE;
    pattern_ui_window_t *ui = pattern_get_ui(p);

    ui->lock++;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pattern_get_ui(p)->b_rev), rev);
    ui->lock--;

    if (redraw)
    {
        gtk_widget_queue_draw(pattern_get_ui(p)->plot);
    }
}

void
pattern_ui_live(pattern_t *p,
                gboolean   live)
{
    if (live)
    {
        gint index = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(pattern_get_model(p)), NULL) - 1;
        gtk_combo_box_set_active(GTK_COMBO_BOX(pattern_get_ui(p)->c_select), index);
        pattern_ui_sync(p, TRUE, TRUE);
    }
    else
    {
        pattern_data_t *data = pattern_get_current(p);
        pattern_signal_set_finished(pattern_data_get_signal(data));
        pattern_ui_sync(p, FALSE, FALSE);
    }

    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}
