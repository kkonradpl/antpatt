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

#ifndef ANTPATT_PATTERN_UI_WINDOW_H_
#define ANTPATT_PATTERN_UI_WINDOW_H_

enum
{
    PATTERN_UI_SCALE_ARRL = 0,
    PATTERN_UI_SCALE_LINEAR_20,
    PATTERN_UI_SCALE_LINEAR_30,
    PATTERN_UI_SCALE_LINEAR_40,
    PATTERN_UI_SCALE_LINEAR_50,
    PATTERN_UI_SCALE_LINEAR_60
};

struct pattern_ui_window
{
    GtkWidget *window;
    GtkWidget *window_plot;
    GtkWidget *box;

    GtkWidget *box_buttons;
    GtkWidget *box_buttons_main;
    GtkWidget *b_new;
    GtkWidget *b_load;
    GtkWidget *b_save;
    GtkWidget *b_save_as;
    GtkWidget *b_render;
    GtkWidget *b_detach;
    GtkWidget *b_about;

    GtkWidget *box_header1;
    GtkWidget *l_title, *e_title;
    GtkWidget *l_size, *s_size;
    GtkWidget *c_scale;
    GtkWidget *l_line, *s_line;

    GtkWidget *box_header2;
    GtkWidget *l_interp, *c_interp;
    GtkWidget *b_full_angle;
    GtkWidget *b_black;
    GtkWidget *b_normalize;
    GtkWidget *b_legend;

    GtkWidget *box_plot;
    GtkWidget *plot;
    GtkWidget *separator;

    GtkWidget *box_select;
    GtkWidget *b_add;
    GtkWidget *b_down;
    GtkWidget *b_up;
    GtkCellRenderer *r_select;
    GtkWidget *c_select;
    GtkWidget *b_export;
    GtkWidget *b_remove;
    GtkWidget *b_clear;

    GtkWidget *box_edit1;
    GtkWidget *l_name, *e_name;
    GtkWidget *l_freq, *s_freq;
    GtkWidget *l_avg, *s_avg;
    GtkWidget *b_color;
    GtkWidget *b_color_next;

    GtkWidget *box_edit2;
    GtkWidget *b_rotate_reset;
    GtkWidget *b_rotate_ccw_fast;
    GtkWidget *b_rotate_ccw;
    GtkWidget *b_rotate_peak;
    GtkWidget *b_rotate_cw;
    GtkWidget *b_rotate_cw_fast;
    GtkWidget *b_fill;
    GtkWidget *b_rev;
    GtkWidget *b_hide;
};

struct pattern_ui_window* pattern_ui_window_new(void);
void pattern_ui_window_set_title(struct pattern_ui_window*, const gchar*);

#endif
