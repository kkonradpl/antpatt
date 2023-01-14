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
#include "pattern-ui.h"
#include "pattern-ui-dialogs.h"
#include "pattern-plot.h"
#include "pattern-misc.h"

#define RAD2DEG(RAD) ((RAD) * 180.0 / M_PI)


gboolean
pattern_ui_plot(GtkWidget    *widget,
                cairo_t      *cr,
                pattern_ui_t *ui)
{
    pattern_t *p = pattern_ui_get_pattern(ui);
    pattern_plot(cr, p);
    return FALSE;
}

gboolean
pattern_ui_plot_motion(GtkWidget      *widget,
                       GdkEventMotion *event,
                       pattern_ui_t   *ui)
{
    pattern_t *p = pattern_ui_get_pattern(ui);
    pattern_data_t *data;
    gint width;
    gdouble offset;
    gdouble line_width;
    gdouble radius;
    gint count;
    gdouble x, y;
    gdouble angle;
    gdouble step;
    gint i;
    gint rotating;
    gboolean redraw = FALSE;

    data = pattern_get_current(p);
    if (data == NULL || pattern_data_get_hide(data))
        return TRUE;

    width = pattern_get_size(p);
    offset = width / (PATTERN_PLOT_BASE_SIZE / PATTERN_PLOT_OFFSET);
    line_width  = width / (PATTERN_PLOT_BASE_SIZE / PATTERN_PLOT_BORDER_WIDTH);
    radius = width / 2.0 - offset + line_width;

    count = pattern_signal_count(pattern_data_get_signal(data));
    if (!count)
        return TRUE;

    x = event->x - (offset + radius);
    y = event->y - (offset + radius);
    angle = RAD2DEG(atan2(y, x) + M_PI / 2.0);
    if (angle < 0.0)
        angle += 360.0;

    step = 360.0 / count;
    i = (gint)lround(angle / step) % count;
    rotating = pattern_ui_get_rotating_idx(ui);
    if (rotating != -1 &&
        i != rotating)
    {
        pattern_signal_rotate(pattern_data_get_signal(data),
                              pattern_ui_get_rotating_idx(ui) - i);
        pattern_ui_set_rotating_idx(ui, i);
        redraw = TRUE;
    }

    if ((event->x - width / 2.0) * (event->x - width / 2.0) + (event->y - width / 2.0) * (event->y - width / 2.0) > radius * radius)
    {
        if (pattern_ui_get_focus_idx(ui) != -1)
        {
            pattern_ui_set_focus_idx(ui, -1);
            redraw = TRUE;
        }
    }
    else
    {
        if (pattern_ui_get_focus_idx(ui) != i)
        {
            pattern_ui_set_focus_idx(ui, i);
            redraw = TRUE;
        }
    }

    if (redraw)
        gtk_widget_queue_draw(widget);

    return TRUE;
}

gboolean
pattern_ui_plot_click(GtkWidget      *widget,
                      GdkEventButton *event,
                      pattern_ui_t   *ui)
{
    pattern_t *p = pattern_ui_get_pattern(ui);
    pattern_data_t *data;
    gint width;
    gdouble offset;
    gdouble line_width;
    gdouble radius;
    gdouble angle;
    gchar *string;

    data = pattern_get_current(p);
    if (data == NULL || pattern_data_get_hide(data))
        return FALSE;

    width = pattern_get_size(p);
    offset = width / (PATTERN_PLOT_BASE_SIZE / PATTERN_PLOT_OFFSET);
    line_width = width / (PATTERN_PLOT_BASE_SIZE / PATTERN_PLOT_BORDER_WIDTH);
    radius = width / 2.0 - offset + line_width;

    if (event->type == GDK_BUTTON_RELEASE &&
        event->button == 1)
    {
        /* Left button release */
        if (pattern_ui_get_rotating_idx(ui) != -1)
            pattern_ui_set_rotating_idx(ui, -1);
        return FALSE;
    }

    if ((event->x - width / 2.0) * (event->x - width / 2.0) + (event->y - width / 2.0) * (event->y - width / 2.0) > radius * radius)
    {
        /* Out of the plot */
        return FALSE;
    }

    if (event->type == GDK_BUTTON_PRESS &&
        event->button == 1)
    {
        /* Left button press */
        pattern_ui_set_rotating_idx(ui, pattern_ui_get_focus_idx(ui));
        return FALSE;
    }

    if (event->type == GDK_BUTTON_PRESS &&
        event->button == 3)
    {
        /* Right button press */
        angle = RAD2DEG(atan2(event->y - (offset + radius), event->x - (offset + radius)) + M_PI / 2.0);
        if (angle < 0.0)
            angle += 360.0;
        string = pattern_misc_info_all(p, angle);
        pattern_ui_dialog(pattern_ui_get_plot_window(ui),
                          GTK_MESSAGE_INFO,
                          "Interpolation",
                          string);
        g_free(string);
    }

    return FALSE;
}

gboolean
pattern_ui_plot_leave(GtkWidget    *widget,
                      GdkEvent     *event,
                      pattern_ui_t *ui)
{
    if (pattern_ui_get_rotating_idx(ui) != -1)
        pattern_ui_set_rotating_idx(ui, -1);

    if (pattern_ui_get_focus_idx(ui) != -1)
    {
        pattern_ui_set_focus_idx(ui, -1);
        gtk_widget_queue_draw(widget);
    }

    return TRUE;
}
