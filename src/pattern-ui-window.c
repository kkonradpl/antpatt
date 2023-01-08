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
#include "version.h"
#include "pattern-ui-window.h"
#include "pattern-data.h"
#include "pattern-signal.h"
#include "pattern.h"
#ifdef G_OS_WIN32
#include "mingw.h"
#endif

static gboolean pattern_ui_window_delete(pattern_ui_window_t*);
static gboolean pattern_ui_window_attach(pattern_ui_window_t*);
static void pattern_ui_window_detach(pattern_ui_window_t*);

pattern_ui_window_t*
pattern_ui_window_new()
{
    pattern_ui_window_t *ui = g_malloc0(sizeof(pattern_ui_window_t));

    ui->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#ifdef G_OS_WIN32
    g_signal_connect(ui->window, "realize", G_CALLBACK(mingw_realize), NULL);
#endif
    gtk_window_set_title(GTK_WINDOW(ui->window), APP_TITLE);
    gtk_window_set_icon_name(GTK_WINDOW(ui->window), APP_ICON);
    gtk_window_set_resizable(GTK_WINDOW(ui->window), FALSE);
    gtk_window_set_position(GTK_WINDOW(ui->window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(ui->window), 5);

    ui->window_plot = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#ifdef G_OS_WIN32
    g_signal_connect(ui->window_plot, "realize", G_CALLBACK(mingw_realize), NULL);
#endif
    gtk_window_set_title(GTK_WINDOW(ui->window_plot), APP_TITLE_PLOT);
    gtk_window_set_icon_name(GTK_WINDOW(ui->window_plot), APP_ICON);
    gtk_window_set_resizable(GTK_WINDOW(ui->window_plot), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(ui->window_plot), 0);

    ui->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_add(GTK_CONTAINER(ui->window), ui->box);

    ui->box_buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_container_add(GTK_CONTAINER(ui->box), ui->box_buttons);

    ui->box_buttons_main = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_set_homogeneous(GTK_BOX(ui->box_buttons_main), TRUE);
    gtk_box_pack_start(GTK_BOX(ui->box_buttons), ui->box_buttons_main, TRUE, TRUE, 0);

    ui->b_new = gtk_button_new_with_label("New");
    gtk_button_set_image(GTK_BUTTON(ui->b_new), gtk_image_new_from_icon_name("document-new", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_button_set_always_show_image(GTK_BUTTON(ui->b_new), TRUE);
    gtk_box_pack_start(GTK_BOX(ui->box_buttons_main), ui->b_new, TRUE, TRUE, 0);

    ui->b_load = gtk_button_new_with_label("Load");
    gtk_button_set_image(GTK_BUTTON(ui->b_load), gtk_image_new_from_icon_name("document-open", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_button_set_always_show_image(GTK_BUTTON(ui->b_load), TRUE);
    gtk_box_pack_start(GTK_BOX(ui->box_buttons_main), ui->b_load, TRUE, TRUE, 0);

    ui->b_save = gtk_button_new_with_label("Save");
    gtk_button_set_image(GTK_BUTTON(ui->b_save), gtk_image_new_from_icon_name("document-save", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_button_set_always_show_image(GTK_BUTTON(ui->b_save), TRUE);
    gtk_box_pack_start(GTK_BOX(ui->box_buttons_main), ui->b_save, TRUE, TRUE, 0);

    ui->b_save_as = gtk_button_new_with_label("Save as");
    gtk_button_set_image(GTK_BUTTON(ui->b_save_as), gtk_image_new_from_icon_name("document-save-as", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_button_set_always_show_image(GTK_BUTTON(ui->b_save_as), TRUE);
    gtk_box_pack_start(GTK_BOX(ui->box_buttons_main), ui->b_save_as, TRUE, TRUE, 0);

    ui->b_render = gtk_button_new_with_label("Render");
    gtk_button_set_image(GTK_BUTTON(ui->b_render), gtk_image_new_from_icon_name("document-save-as", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_button_set_always_show_image(GTK_BUTTON(ui->b_render), TRUE);
    gtk_box_pack_start(GTK_BOX(ui->box_buttons_main), ui->b_render, TRUE, TRUE, 0);

    ui->b_detach = gtk_button_new();
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->b_detach), "Detach");
    gtk_button_set_image(GTK_BUTTON(ui->b_detach), gtk_image_new_from_icon_name("view-restore", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_box_pack_start(GTK_BOX(ui->box_buttons), ui->b_detach, FALSE, FALSE, 0);

    ui->b_about = gtk_button_new();
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->b_about), "About...");
    gtk_button_set_image(GTK_BUTTON(ui->b_about), gtk_image_new_from_icon_name("help-about-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR));
    gtk_box_pack_start(GTK_BOX(ui->box_buttons), ui->b_about, FALSE, FALSE, 0);

    ui->box_header1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_container_add(GTK_CONTAINER(ui->box), ui->box_header1);

    ui->l_size = gtk_label_new("Size:");
    gtk_box_pack_start(GTK_BOX(ui->box_header1), ui->l_size, FALSE, FALSE, 0);

    ui->s_size = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(0.0, PATTERN_MIN_SIZE, PATTERN_MAX_SIZE, 10.0, 25.0, 0.0)), 0, 0);
    gtk_box_pack_start(GTK_BOX(ui->box_header1), ui->s_size, FALSE, FALSE, 0);

    ui->l_title = gtk_label_new("Title:");
    gtk_box_pack_start(GTK_BOX(ui->box_header1), ui->l_title, FALSE, FALSE, 0);

    ui->e_title = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(ui->e_title), 100);
    gtk_box_pack_start(GTK_BOX(ui->box_header1), ui->e_title, TRUE, TRUE, 0);

    ui->c_scale = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ui->c_scale), "ARRL");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ui->c_scale), "20 dB");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ui->c_scale), "30 dB");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ui->c_scale), "40 dB");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ui->c_scale), "50 dB");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ui->c_scale), "60 dB");
    gtk_box_pack_start(GTK_BOX(ui->box_header1), ui->c_scale, FALSE, FALSE, 0);

    ui->l_line = gtk_label_new("Line:");
    gtk_box_pack_start(GTK_BOX(ui->box_header1), ui->l_line, FALSE, FALSE, 0);

    ui->s_line = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(0.0, PATTERN_MIN_LINE, PATTERN_MAX_LINE, 0.1, 0.2, 0.0)), 0, 1);
    gtk_box_pack_start(GTK_BOX(ui->box_header1), ui->s_line, FALSE, FALSE, 0);

    ui->box_header2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_container_add(GTK_CONTAINER(ui->box), ui->box_header2);

    ui->l_interp = gtk_label_new("Interpolation:");
    gtk_box_pack_start(GTK_BOX(ui->box_header2), ui->l_interp, FALSE, FALSE, 0);

    ui->c_interp = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ui->c_interp), "Linear");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ui->c_interp), "Akima");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ui->c_interp), "Akima'");
    gtk_box_pack_start(GTK_BOX(ui->box_header2), ui->c_interp, TRUE, TRUE, 0);

    ui->b_full_angle = gtk_check_button_new_with_label("360°");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui->b_full_angle), TRUE);
    gtk_box_pack_start(GTK_BOX(ui->box_header2), ui->b_full_angle, FALSE, FALSE, 0);

    ui->b_black = gtk_check_button_new_with_label("Black");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui->b_black), TRUE);
    gtk_box_pack_start(GTK_BOX(ui->box_header2), ui->b_black, FALSE, FALSE, 0);

    ui->b_normalize = gtk_check_button_new_with_label("Normalize");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui->b_normalize), TRUE);
    gtk_box_pack_start(GTK_BOX(ui->box_header2), ui->b_normalize, FALSE, FALSE, 0);

    ui->b_legend = gtk_check_button_new_with_label("Legend");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui->b_legend), TRUE);
    gtk_box_pack_start(GTK_BOX(ui->box_header2), ui->b_legend, FALSE, FALSE, 0);

    ui->box_plot = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(ui->box), ui->box_plot);

    ui->separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(ui->box_plot), ui->separator, FALSE, FALSE, 0);

    ui->plot = gtk_drawing_area_new();
    gtk_widget_add_events(ui->plot, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);
    gtk_box_set_center_widget(GTK_BOX(ui->box_plot), ui->plot);

    ui->box_select = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_container_add(GTK_CONTAINER(ui->box), ui->box_select);

    ui->b_add = gtk_button_new_with_label("Add");
    gtk_button_set_image(GTK_BUTTON(ui->b_add), gtk_image_new_from_icon_name("list-add", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_button_set_always_show_image(GTK_BUTTON(ui->b_add), TRUE);
    gtk_box_pack_start(GTK_BOX(ui->box_select), ui->b_add, FALSE, FALSE, 0);

    ui->b_down = gtk_button_new();
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->b_down), "Move down");
    gtk_button_set_image(GTK_BUTTON(ui->b_down), gtk_image_new_from_icon_name("go-down", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_box_pack_start(GTK_BOX(ui->box_select), ui->b_down, FALSE, FALSE, 0);

    ui->b_up = gtk_button_new();
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->b_up), "Move up");
    gtk_button_set_image(GTK_BUTTON(ui->b_up), gtk_image_new_from_icon_name("go-up", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_box_pack_start(GTK_BOX(ui->box_select), ui->b_up, FALSE, FALSE, 0);

    ui->c_select = gtk_combo_box_new();
    ui->r_select = gtk_cell_renderer_text_new();
    g_object_set(ui->r_select,
                 "ellipsize", PANGO_ELLIPSIZE_END,
                 "ellipsize-set", TRUE,
                 NULL);
    gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(ui->r_select), 1);
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(ui->c_select), ui->r_select, TRUE);
    gtk_box_pack_start(GTK_BOX(ui->box_select), ui->c_select, TRUE, TRUE, 0);

    ui->b_export = gtk_button_new();
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->b_export), "Export");
    gtk_button_set_image(GTK_BUTTON(ui->b_export), gtk_image_new_from_icon_name("document-save", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_box_pack_start(GTK_BOX(ui->box_select), ui->b_export, FALSE, FALSE, 0);

    ui->b_remove = gtk_button_new();
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->b_remove), "Remove");
    gtk_button_set_image(GTK_BUTTON(ui->b_remove), gtk_image_new_from_icon_name("list-remove", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_box_pack_start(GTK_BOX(ui->box_select), ui->b_remove, FALSE, FALSE, 0);

    ui->b_clear = gtk_button_new();
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->b_clear), "Remove all");
    gtk_button_set_image(GTK_BUTTON(ui->b_clear), gtk_image_new_from_icon_name("edit-clear", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_box_pack_start(GTK_BOX(ui->box_select), ui->b_clear, FALSE, FALSE, 0);

    ui->box_edit1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_container_add(GTK_CONTAINER(ui->box), ui->box_edit1);

    ui->l_name = gtk_label_new("Name:");
    gtk_box_pack_start(GTK_BOX(ui->box_edit1), ui->l_name, FALSE, FALSE, 0);

    ui->e_name = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(ui->e_name), 12);
    gtk_entry_set_max_length(GTK_ENTRY(ui->e_name), 50);
    gtk_box_pack_start(GTK_BOX(ui->box_edit1), ui->e_name, TRUE, TRUE, 0);

    ui->l_freq = gtk_label_new("Freq:");
    gtk_box_pack_start(GTK_BOX(ui->box_edit1), ui->l_freq, FALSE, FALSE, 0);

    ui->s_freq = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(0.0, PATTERN_DATA_MIN_FREQ, PATTERN_DATA_MAX_FREQ, 1000.0, 10000.0, 0.0)), 0, 0);
    gtk_box_pack_start(GTK_BOX(ui->box_edit1), ui->s_freq, FALSE, FALSE, 0);

    ui->l_avg = gtk_label_new("Avg:");
    gtk_box_pack_start(GTK_BOX(ui->box_edit1), ui->l_avg, FALSE, FALSE, 0);

    ui->s_avg = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(0.0, PATTERN_SIGNAL_MIN_AVG, PATTERN_SIGNAL_MAX_AVG, 1.0, 2.0, 0.0)), 0, 0);
    gtk_box_pack_start(GTK_BOX(ui->box_edit1), ui->s_avg, FALSE, FALSE, 0);

    ui->b_color = gtk_color_button_new();
    gtk_box_pack_start(GTK_BOX(ui->box_edit1), ui->b_color, FALSE, FALSE, 0);

    ui->b_color_next = gtk_button_new();
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->b_color_next), "Pick another color");
    gtk_button_set_image(GTK_BUTTON(ui->b_color_next), gtk_image_new_from_icon_name("gtk-select-color", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_box_pack_start(GTK_BOX(ui->box_edit1), ui->b_color_next, FALSE, FALSE, 0);

    ui->box_edit2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_container_add(GTK_CONTAINER(ui->box), ui->box_edit2);

    ui->b_rotate_reset = gtk_button_new();
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->b_rotate_reset), "Reset rotation");
    gtk_button_set_image(GTK_BUTTON(ui->b_rotate_reset), gtk_image_new_from_icon_name("go-home", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_box_pack_start(GTK_BOX(ui->box_edit2), ui->b_rotate_reset, TRUE, TRUE, 0);

    ui->b_rotate_ccw_fast = gtk_button_new();
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->b_rotate_ccw_fast), "Rotate CCW fast");
    gtk_button_set_image(GTK_BUTTON(ui->b_rotate_ccw_fast), gtk_image_new_from_icon_name("media-seek-backward", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_box_pack_start(GTK_BOX(ui->box_edit2), ui->b_rotate_ccw_fast, TRUE, TRUE, 0);

    ui->b_rotate_ccw = gtk_button_new();
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->b_rotate_ccw), "Rotate CCW");
    gtk_button_set_image(GTK_BUTTON(ui->b_rotate_ccw), gtk_image_new_from_icon_name("go-previous", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_box_pack_start(GTK_BOX(ui->box_edit2), ui->b_rotate_ccw, TRUE, TRUE, 0);

    ui->b_rotate_peak = gtk_button_new();
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->b_rotate_peak), "Find peak");
    gtk_button_set_image(GTK_BUTTON(ui->b_rotate_peak), gtk_image_new_from_icon_name("go-top", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_box_pack_start(GTK_BOX(ui->box_edit2), ui->b_rotate_peak, TRUE, TRUE, 0);

    ui->b_rotate_cw = gtk_button_new();
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->b_rotate_cw), "Rotate CW");
    gtk_button_set_image(GTK_BUTTON(ui->b_rotate_cw), gtk_image_new_from_icon_name("go-next", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_box_pack_start(GTK_BOX(ui->box_edit2), ui->b_rotate_cw, TRUE, TRUE, 0);

    ui->b_rotate_cw_fast = gtk_button_new();
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->b_rotate_cw_fast), "Rotate CW fast");
    gtk_button_set_image(GTK_BUTTON(ui->b_rotate_cw_fast), gtk_image_new_from_icon_name("media-seek-forward", GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_box_pack_start(GTK_BOX(ui->box_edit2), ui->b_rotate_cw_fast, TRUE, TRUE, 0);

    ui->b_fill = gtk_check_button_new_with_label("Fill");
    gtk_box_pack_start(GTK_BOX(ui->box_edit2), ui->b_fill, FALSE, FALSE, 0);

    ui->b_rev = gtk_check_button_new_with_label("Rev");
    gtk_box_pack_start(GTK_BOX(ui->box_edit2), ui->b_rev, FALSE, FALSE, 0);

    ui->b_hide = gtk_check_button_new_with_label("Hide");
    gtk_box_pack_start(GTK_BOX(ui->box_edit2), ui->b_hide, FALSE, FALSE, 0);

    g_signal_connect_data(ui->window, "delete-event", G_CALLBACK(pattern_ui_window_delete), ui, NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    g_signal_connect_swapped(ui->window_plot, "delete-event", G_CALLBACK(pattern_ui_window_attach), ui);
    g_signal_connect_swapped(ui->b_detach, "clicked", G_CALLBACK(pattern_ui_window_detach), ui);
    return ui;
}

static gboolean
pattern_ui_window_delete(pattern_ui_window_t *ui)
{
    /* Destroy the plot window first */
    gtk_widget_destroy(GTK_WIDGET(ui->window_plot));

    return FALSE;
}

static gboolean
pattern_ui_window_attach(pattern_ui_window_t *ui)
{
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->b_detach), "Detach");
    g_signal_handlers_disconnect_by_func(ui->b_detach, G_CALLBACK(pattern_ui_window_attach), (gpointer)ui);
    g_signal_connect_swapped(ui->b_detach, "clicked", G_CALLBACK(pattern_ui_window_detach), (gpointer)ui);

    gtk_widget_hide(ui->window_plot);
    gtk_widget_hide(ui->separator);
    g_object_ref(ui->plot);
    gtk_container_remove(GTK_CONTAINER(ui->window_plot), ui->plot);
    gtk_box_set_center_widget(GTK_BOX(ui->box_plot), ui->plot);
    g_object_unref(ui->plot);
}

static void
pattern_ui_window_detach(pattern_ui_window_t *ui)
{
    gtk_widget_set_tooltip_text(GTK_WIDGET(ui->b_detach), "Attach");
    g_signal_handlers_disconnect_by_func(ui->b_detach, G_CALLBACK(pattern_ui_window_detach), (gpointer)ui);
    g_signal_connect_swapped(ui->b_detach, "clicked", G_CALLBACK(pattern_ui_window_attach), (gpointer)ui);

    g_object_ref(ui->plot);
    gtk_container_remove(GTK_CONTAINER(ui->box_plot), ui->plot);
    gtk_container_add(GTK_CONTAINER(ui->window_plot), ui->plot);
    g_object_unref(ui->plot);
    gtk_widget_show(ui->separator);
    gtk_window_set_transient_for(GTK_WINDOW(ui->window_plot), GTK_WINDOW(ui->window));
    gtk_widget_show_all(ui->window_plot);
    gtk_window_set_transient_for(GTK_WINDOW(ui->window_plot), NULL);
}

void
pattern_ui_window_set_title(pattern_ui_window_t *ui,
                            const gchar         *filename)
{
    gchar *title;
    gchar *plot_title;
    gchar *name;
    gchar *ext;

    if (filename)
    {
        name = g_path_get_basename(filename);
        ext = strrchr(name, '.');
        if (ext && !g_ascii_strcasecmp(ext, APP_FILE_COMPRESS))
        {
            *ext = '\0';
            ext = strrchr(name, '.');
        }
        if (ext && !g_ascii_strcasecmp(ext, APP_FILE_EXT))
            *ext = '\0';

        title = g_strdup_printf("%s [%s]", APP_TITLE, name);
        plot_title = g_strdup_printf("%s [%s]", APP_TITLE_PLOT, name);
        g_free(name);
    }
    else
    {
        title = g_strdup_printf("%s", APP_TITLE);
        plot_title = g_strdup_printf("%s", APP_TITLE_PLOT);
    }

    gtk_window_set_title(GTK_WINDOW(ui->window), title);
    gtk_window_set_title(GTK_WINDOW(ui->window_plot), plot_title);
    g_free(title);
    g_free(plot_title);
}
