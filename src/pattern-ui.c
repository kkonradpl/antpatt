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
#include "pattern-ui.h"
#include "pattern-ui-dialogs.h"
#include "pattern-plot.h"
#include "pattern-import.h"
#include "pattern-color.h"
#include "pattern-json.h"

#define UI_DRAG_URI_LIST_ID 0

static const GtkTargetEntry drop_types[] = {{ "text/uri-list", 0, UI_DRAG_URI_LIST_ID }};
static const gint n_drop_types = sizeof(drop_types) / sizeof(drop_types[0]);

static gint pattern_ui_instances = 0;

static void pattern_ui_exit(GtkWidget*, pattern_t*);
static void pattern_ui_new(GtkWidget*, pattern_t*);
static void pattern_ui_load(GtkWidget*, pattern_t*);
static void pattern_ui_save(GtkWidget*, pattern_t*);
static void pattern_ui_export(GtkWidget*, pattern_t*);
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
static void pattern_ui_lock(pattern_t*, gboolean);

static void pattern_ui_drag_data_received(GtkWidget*, GdkDragContext*, gint, gint, GtkSelectionData*, guint, guint, gpointer);

static void pattern_ui_format_desc(GtkCellLayout*, GtkCellRenderer*, GtkTreeModel*, GtkTreeIter*, gpointer);
static gboolean pattern_ui_format_avg(GtkSpinButton*, gpointer);
static gboolean pattern_ui_format_freq(GtkSpinButton*, gpointer);

static void pattern_ui_read(pattern_t*, GSList*);


void
pattern_ui_create(pattern_t *p)
{
    pattern_ui_window_t *ui = pattern_get_ui(p);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui->s_size), pattern_get_size(p));
    gtk_entry_set_text(GTK_ENTRY(ui->e_title), pattern_get_title(p));
    switch(pattern_get_scale(p))
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
    gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(ui->c_select), ui->r_select, pattern_ui_format_desc, NULL, NULL);

    /* Signals */
    g_signal_connect(ui->window, "destroy", G_CALLBACK(pattern_ui_exit), p);

    g_signal_connect(ui->b_new, "clicked", G_CALLBACK(pattern_ui_new), p);
    g_signal_connect(ui->b_load, "clicked", G_CALLBACK(pattern_ui_load), p);
    g_signal_connect(ui->b_save, "clicked", G_CALLBACK(pattern_ui_save), p);
    g_signal_connect(ui->b_export, "clicked", G_CALLBACK(pattern_ui_export), p);
    g_signal_connect(ui->b_about, "clicked", G_CALLBACK(pattern_ui_about), p);

    g_signal_connect(ui->s_size, "changed", G_CALLBACK(pattern_ui_size), p);
    g_signal_connect(ui->e_title, "changed", G_CALLBACK(pattern_ui_title), p);
    g_signal_connect(ui->c_scale, "changed", G_CALLBACK(pattern_ui_scale), p);
    g_signal_connect(ui->s_line, "changed", G_CALLBACK(pattern_ui_line), p);

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
    g_signal_connect(ui->b_remove, "clicked", G_CALLBACK(pattern_ui_remove), p);
    g_signal_connect(ui->b_clear, "clicked", G_CALLBACK(pattern_ui_clear), p);

    g_signal_connect(ui->e_name, "changed", G_CALLBACK(pattern_ui_name), p);
    g_signal_connect(ui->s_freq, "changed", G_CALLBACK(pattern_ui_freq), p);
    g_signal_connect(ui->s_freq, "output", G_CALLBACK(pattern_ui_format_freq), NULL);
    g_signal_connect(ui->s_avg, "changed", G_CALLBACK(pattern_ui_avg), p);
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

    pattern_ui_lock(p, TRUE);
    g_signal_emit_by_name(ui->s_size, "changed", p);

    if(gtk_tree_model_iter_n_children(GTK_TREE_MODEL(pattern_get_model(p)), NULL))
        gtk_combo_box_set_active(GTK_COMBO_BOX(ui->c_select), 0);

    gtk_widget_show_all(ui->window);

    pattern_ui_instances++;
}

static void
pattern_ui_exit(GtkWidget *widget,
                pattern_t *p)
{
    pattern_free(p);

    if(!--pattern_ui_instances)
        gtk_main_quit();
}

static void
pattern_ui_new(GtkWidget *widget,
               pattern_t *p)
{
    pattern_t *pp = pattern_new();
    pattern_set_ui(pp, pattern_ui_window_new());
}

static void
pattern_ui_load(GtkWidget *widget,
                pattern_t *p)
{
    gchar *filename = pattern_ui_dialog_open(GTK_WINDOW(pattern_get_ui(p)->window));
    gchar *error = NULL;
    pattern_t *pp;

    if(filename)
    {
        pp = pattern_json_load(filename, &error);
        if(pp)
        {
            pattern_set_ui(pp, pattern_ui_window_new());
        }
        else
        {
            pattern_ui_dialog(NULL,
                              GTK_MESSAGE_ERROR,
                              "Error",
                              error);
            g_free(error);
        }
        g_free(filename);
    }
}

static void
pattern_ui_save(GtkWidget *widget,
                pattern_t *p)
{
    gchar *filename = pattern_ui_dialog_save(GTK_WINDOW(pattern_get_ui(p)->window));
    if(filename)
    {
        if(!pattern_json_save(p, filename, FALSE))
        {
            pattern_ui_dialog(GTK_WINDOW(pattern_get_ui(p)->window), GTK_MESSAGE_ERROR,
                              APP_TITLE,
                              "Unable to save the file.");
        }
        g_free(filename);
    }
}

static void
pattern_ui_export(GtkWidget *widget,
                  pattern_t *p)
{
    gchar *filename = pattern_ui_dialog_export(GTK_WINDOW(pattern_get_ui(p)->window));
    if(filename)
    {
        if(!pattern_plot_to_file(p, filename))
        {
            pattern_ui_dialog(GTK_WINDOW(pattern_get_ui(p)->window), GTK_MESSAGE_ERROR,
                              APP_TITLE,
                              "Unable to save the image.");
        }
        g_free(filename);
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
    pattern_set_size(p, size);
    gtk_widget_set_size_request(pattern_get_ui(p)->plot, size, size);
}

static void
pattern_ui_title(GtkWidget *widget,
                 pattern_t *p)
{
    pattern_set_title(p, gtk_entry_get_text(GTK_ENTRY(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_scale(GtkWidget *widget,
                 pattern_t *p)
{
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
    pattern_set_line(p, gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_interp(GtkWidget *widget,
                  pattern_t *p)
{
    pattern_set_interp(p, gtk_combo_box_get_active(GTK_COMBO_BOX(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_full_angle(GtkWidget *widget,
                      pattern_t *p)
{
    pattern_set_full_angle(p, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_black(GtkWidget *widget,
                 pattern_t *p)
{
    pattern_set_black(p, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_normalize(GtkWidget *widget,
                     pattern_t *p)
{
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

    if(!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(pattern_get_ui(p)->c_select), &iter))
        return;

    path = gtk_tree_model_get_path(GTK_TREE_MODEL(pattern_get_model(p)), &iter);
    gtk_tree_path_prev(path);
    if(gtk_tree_model_get_iter(GTK_TREE_MODEL(pattern_get_model(p)), &prev, path))
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
    GtkTreeIter iter;
    pattern_data_t *data = NULL;
    pattern_signal_t *signal;
    gboolean active;


    active = gtk_combo_box_get_active_iter(GTK_COMBO_BOX(pattern_get_ui(p)->c_select), &iter);
    if(active)
    {
        gtk_tree_model_get(GTK_TREE_MODEL(pattern_get_model(p)), &iter, PATTERN_COL_DATA, &data, -1);
        signal = pattern_data_get_signal(data);

        g_signal_handlers_block_by_func(G_OBJECT(pattern_get_ui(p)->e_name), GINT_TO_POINTER(pattern_ui_name), p);
        gtk_entry_set_text(GTK_ENTRY(pattern_get_ui(p)->e_name), pattern_data_get_name(data));
        g_signal_handlers_unblock_by_func(G_OBJECT(pattern_get_ui(p)->e_name), GINT_TO_POINTER(pattern_ui_name), p);

        g_signal_handlers_block_by_func(G_OBJECT(pattern_get_ui(p)->s_freq), GINT_TO_POINTER(pattern_ui_freq), p);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(pattern_get_ui(p)->s_freq), pattern_data_get_freq(data));
        g_signal_handlers_unblock_by_func(G_OBJECT(pattern_get_ui(p)->s_freq), GINT_TO_POINTER(pattern_ui_freq), p);

        g_signal_handlers_block_by_func(G_OBJECT(pattern_get_ui(p)->s_avg), GINT_TO_POINTER(pattern_ui_avg), p);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(pattern_get_ui(p)->s_avg), pattern_signal_get_avg(signal));
        g_signal_handlers_unblock_by_func(G_OBJECT(pattern_get_ui(p)->s_avg), GINT_TO_POINTER(pattern_ui_avg), p);

        g_signal_handlers_block_by_func(G_OBJECT(pattern_get_ui(p)->b_color), GINT_TO_POINTER(pattern_ui_color), p);
        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(pattern_get_ui(p)->b_color), pattern_data_get_color(data));
        g_signal_handlers_unblock_by_func(G_OBJECT(pattern_get_ui(p)->b_color), GINT_TO_POINTER(pattern_ui_color), p);

        g_signal_handlers_block_by_func(G_OBJECT(pattern_get_ui(p)->b_hide), GINT_TO_POINTER(pattern_ui_hide), p);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pattern_get_ui(p)->b_hide), pattern_data_get_hide(data));
        g_signal_handlers_unblock_by_func(G_OBJECT(pattern_get_ui(p)->b_hide), GINT_TO_POINTER(pattern_ui_hide), p);

        g_signal_handlers_block_by_func(G_OBJECT(pattern_get_ui(p)->b_fill), GINT_TO_POINTER(pattern_ui_fill), p);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pattern_get_ui(p)->b_fill), pattern_data_get_fill(data));
        g_signal_handlers_unblock_by_func(G_OBJECT(pattern_get_ui(p)->b_fill), GINT_TO_POINTER(pattern_ui_fill), p);

        g_signal_handlers_block_by_func(G_OBJECT(pattern_get_ui(p)->b_rev), GINT_TO_POINTER(pattern_ui_rev), p);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pattern_get_ui(p)->b_rev), pattern_signal_get_rev(signal));
        g_signal_handlers_unblock_by_func(G_OBJECT(pattern_get_ui(p)->b_rev), GINT_TO_POINTER(pattern_ui_rev), p);
    }

    pattern_set_current(p, data);
    pattern_ui_lock(p, !active);
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
    pattern_clear(p);
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_name(GtkWidget *widget,
                pattern_t *p)
{
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
    pattern_data_set_freq(pattern_get_current(p), gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_avg(GtkWidget *widget,
               pattern_t *p)
{
    pattern_signal_set_avg(pattern_data_get_signal(pattern_get_current(p)), gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_color(GtkWidget *widget,
                 pattern_t *p)
{
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), pattern_data_get_color(pattern_get_current(p)));
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
    pattern_data_set_fill(pattern_get_current(p), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_rev(GtkWidget *widget,
               pattern_t *p)
{
    pattern_signal_set_rev(pattern_data_get_signal(pattern_get_current(p)), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_hide(GtkWidget *widget,
                  pattern_t *p)
{
    GtkTreeIter iter;
    if(!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(pattern_get_ui(p)->c_select), &iter))
        return;

    pattern_hide(p, pattern_get_current(p), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
    gtk_widget_queue_draw(pattern_get_ui(p)->c_select);
    gtk_widget_queue_draw(pattern_get_ui(p)->plot);
}

static void
pattern_ui_lock(pattern_t *p,
                gboolean   lock)
{
    gboolean active = !lock;
    gtk_widget_set_sensitive(pattern_get_ui(p)->b_down, active);
    gtk_widget_set_sensitive(pattern_get_ui(p)->b_up, active);
    gtk_widget_set_sensitive(pattern_get_ui(p)->b_remove, active);
    gtk_widget_set_sensitive(pattern_get_ui(p)->b_clear, active);
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

    if(lock)
    {
        g_signal_handlers_block_by_func(G_OBJECT(pattern_get_ui(p)->e_name), GINT_TO_POINTER(pattern_ui_name), p);
        gtk_entry_set_text(GTK_ENTRY(pattern_get_ui(p)->e_name), "");
        g_signal_handlers_unblock_by_func(G_OBJECT(pattern_get_ui(p)->e_name), GINT_TO_POINTER(pattern_ui_name), p);

        g_signal_handlers_block_by_func(G_OBJECT(pattern_get_ui(p)->s_freq), GINT_TO_POINTER(pattern_ui_freq), p);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(pattern_get_ui(p)->s_freq), 0);
        g_signal_handlers_unblock_by_func(G_OBJECT(pattern_get_ui(p)->s_freq), GINT_TO_POINTER(pattern_ui_freq), p);

        g_signal_handlers_block_by_func(G_OBJECT(pattern_get_ui(p)->s_avg), GINT_TO_POINTER(pattern_ui_avg), p);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(pattern_get_ui(p)->s_avg), 0);
        g_signal_handlers_unblock_by_func(G_OBJECT(pattern_get_ui(p)->s_avg), GINT_TO_POINTER(pattern_ui_avg), p);

        g_signal_handlers_block_by_func(G_OBJECT(pattern_get_ui(p)->b_fill), GINT_TO_POINTER(pattern_ui_fill), p);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pattern_get_ui(p)->b_fill), FALSE);
        g_signal_handlers_unblock_by_func(G_OBJECT(pattern_get_ui(p)->b_fill), GINT_TO_POINTER(pattern_ui_fill), p);

        g_signal_handlers_block_by_func(G_OBJECT(pattern_get_ui(p)->b_rev), GINT_TO_POINTER(pattern_ui_rev), p);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pattern_get_ui(p)->b_rev), FALSE);
        g_signal_handlers_unblock_by_func(G_OBJECT(pattern_get_ui(p)->b_rev), GINT_TO_POINTER(pattern_ui_rev), p);

        g_signal_handlers_block_by_func(G_OBJECT(pattern_get_ui(p)->b_hide), GINT_TO_POINTER(pattern_ui_hide), p);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pattern_get_ui(p)->b_hide), FALSE);
        g_signal_handlers_unblock_by_func(G_OBJECT(pattern_get_ui(p)->b_hide), GINT_TO_POINTER(pattern_ui_hide), p);
    }
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
    gchar text[8];

    if(!value)
    {
        gtk_entry_set_text(GTK_ENTRY(widget), "");
        return TRUE;
    }
    g_snprintf(text, sizeof(text), "%d", value);
    gtk_entry_set_text(GTK_ENTRY(widget), text);
    return TRUE;
}

static gboolean
pattern_ui_format_freq(GtkSpinButton *widget,
                       gpointer       user_data)
{
    GtkAdjustment *adj = gtk_spin_button_get_adjustment(widget);
    gint value = (gint)gtk_adjustment_get_value(adj);

    if(!value)
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
