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
#include <cairo-svg.h>
#include "pattern.h"
#include "pattern-plot.h"
#include "pattern-misc.h"
#include "pattern-ui.h"

#define PATTERN_FONT_SIZE_TITLE  16.0
#define PATTERN_FONT_SIZE_SCALE  11.0
#define PATTERN_FONT_SIZE_LEGEND 13.0
#define PATTERN_FONT_SIZE_FREQ   15.0
#define PATTERN_LEGEND_SPACING    4.0
#define PATTERN_ANGLE_LABEL_OFF  15.0
#define PATTERN_PLOT_LINE_WIDTH   1.25
#define PATTERN_PLOT_LEGEND_WIDTH 1.5
#define PATTERN_PLOT_FG_ALPHA     0.90
#define PATTERN_PLOT_BG_ALPHA     0.18
#define PATTERN_FONT "DejaVu Sans Mono"

#define DEG2RAD(DEG) ((DEG) * M_PI / 180.0)

typedef struct
{
    gint width;
    gdouble line;
    const gchar *title;
    gint scale;
    gboolean full_angle;
    gboolean black;
    gboolean norm;
    gboolean legend;
    gdouble offset;
    gdouble radius;
    gdouble peak;
} pattern_plot_t;

static void pattern_plot_title(cairo_t*, pattern_plot_t*);
static void pattern_plot_coords(cairo_t*, pattern_plot_t*);
static void pattern_plot_grid(cairo_t*, pattern_plot_t*);
static void pattern_plot_radiation(cairo_t*, pattern_plot_t*, pattern_t*);
static void pattern_plot_radiation_data(cairo_t*, pattern_plot_t*, pattern_data_t*, gdouble, gdouble);
static void pattern_plot_legend(cairo_t*, pattern_plot_t*, pattern_data_t*, gint, gint);
static void pattern_plot_frequency(cairo_t*, pattern_plot_t*, gint);
static void pattern_plot_focus(cairo_t*, pattern_plot_t*, pattern_t*);
static void pattern_plot_pointer(cairo_t*, pattern_plot_t*, pattern_t*, pattern_data_t*);
static void pattern_plot_info(cairo_t*, pattern_plot_t*, pattern_t*, pattern_data_t*);
static void pattern_plot_stats(cairo_t*, pattern_plot_t*, pattern_t*, pattern_data_t*);

static gdouble pattern_plot_signal(gint, gdouble);


void
pattern_plot(cairo_t   *cr,
             pattern_t *p)
{
    pattern_plot_t plot;

    plot.width = pattern_get_size(p);
    plot.line = pattern_get_line(p);
    plot.title = pattern_get_title(p);
    plot.scale = pattern_get_scale(p);
    plot.full_angle = pattern_get_full_angle(p);
    plot.black = pattern_get_black(p);
    plot.norm = pattern_get_normalize(p);
    plot.legend = pattern_get_legend(p);

    plot.offset = plot.width / (PATTERN_PLOT_BASE_SIZE / PATTERN_PLOT_OFFSET);
    plot.radius = plot.width / 2.0 - plot.offset;
    plot.peak = plot.norm ? NAN : pattern_get_peak(p);

    /* clear the canvas */
    cairo_set_source_rgb(cr, (plot.black ? 0.0 : 1.0), (plot.black ? 0.0 : 1.0), (plot.black ? 0.0 : 1.0));
    cairo_paint(cr);

    /* set default font face */
    cairo_select_font_face(cr, PATTERN_FONT, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

    /* display title label */
    pattern_plot_title(cr, &plot);

    /* draw the polar coordinates */
    pattern_plot_coords(cr, &plot);

    /* draw the grid */
    pattern_plot_grid(cr, &plot);

    /* plot all patterns */
    pattern_plot_radiation(cr, &plot, p);

    /* mark focused data point */
    pattern_plot_focus(cr, &plot, p);
}

static void
pattern_plot_title(cairo_t        *cr,
                   pattern_plot_t *plot)
{
    cairo_text_extents_t extents;
    gdouble font_height = plot->width / (PATTERN_PLOT_BASE_SIZE / PATTERN_FONT_SIZE_TITLE);
    gdouble x, y;

    cairo_set_source_rgb(cr, (plot->black ? 1.0 : 0.0), (plot->black ? 1.0 : 0.0), (plot->black ? 1.0 : 0.0));
    cairo_set_font_size(cr, font_height);
    cairo_text_extents(cr, plot->title, &extents);

    x = (plot->width - extents.width) / 2.0;
    y = font_height+(plot->width / (PATTERN_PLOT_BASE_SIZE / 2.0));
    cairo_move_to(cr, round(x), round(y));
    cairo_show_text(cr, plot->title);
    cairo_stroke(cr);
}

static void
pattern_plot_coords(cairo_t        *cr,
                    pattern_plot_t *plot)
{
    gdouble line_width = plot->width / (PATTERN_PLOT_BASE_SIZE / PATTERN_PLOT_BORDER_WIDTH);
    cairo_set_source_rgb(cr, (plot->black ? 0.6 : 0.4), (plot->black ? 0.6 : 0.4), (plot->black ? 0.6 : 0.4));
    cairo_set_line_width(cr, line_width);
    cairo_set_font_size(cr, plot->width / (PATTERN_PLOT_BASE_SIZE / PATTERN_FONT_SIZE_SCALE));
    cairo_arc(cr, plot->radius + plot->offset, plot->radius + plot->offset, plot->radius, 0, 2 * M_PI);
    cairo_stroke(cr);
}

static void
pattern_plot_grid(cairo_t        *cr,
                  pattern_plot_t *plot)
{
    static const gint scales[] = {-3, -10, -20, -30, -40, -50, 0};
    static const gdouble dash[] = {1.0, 2.0};
    static const gint dash_len = sizeof(dash) / sizeof(dash[0]);
    cairo_text_extents_t extents;
    gdouble l, x, y;
    gint i, j, k, n;
    gint font_height = (gint)(plot->width / (PATTERN_PLOT_BASE_SIZE / PATTERN_FONT_SIZE_SCALE));
    gchar text[20];
    gboolean draw;
    gint limit;
    gint fill;

    if (plot->scale)
    {
       limit = plot->scale + 10;
       fill = plot->scale;
    }
    else /* ARRL */
    {
        limit = -40;
        fill = -30;
    }

    cairo_set_line_width(cr, plot->width / (PATTERN_PLOT_BASE_SIZE / PATTERN_PLOT_BORDER_WIDTH));
    cairo_set_font_size(cr, font_height);
    cairo_set_dash(cr, dash, dash_len, 0);
    for (i = 0; scales[i]; i++)
    {
        if (scales[i] < limit)
            break;

        cairo_arc(cr,
                  plot->radius + plot->offset,
                  plot->radius + plot->offset,
                  plot->radius*pattern_plot_signal(plot->scale, scales[i]),
                  -0.5 * M_PI,
                  1.5 * M_PI);
        cairo_stroke_preserve(cr);
        g_snprintf(text, sizeof(text), "%d", scales[i]);
        cairo_text_extents(cr, text, &extents);
        cairo_get_current_point(cr, &x, &y);
        x += -(extents.width / 2.0 + extents.x_bearing) - 0.5;
        y += font_height;
        cairo_move_to(cr, round(x), round(y));
        cairo_show_text(cr, text);
        cairo_stroke(cr);
    }
    cairo_set_dash(cr, dash, 0, 0);

    cairo_set_line_width(cr, plot->width/(PATTERN_PLOT_BASE_SIZE / PATTERN_PLOT_BORDER_WIDTH));
    k = 180;
    for (i = 0; i < 360; i += 10)
    {
        for (j = -1; j > fill; j -= 1)
        {
            if (i % 30 && j % 2)
                continue;

            draw = TRUE;
            for (n = 0; scales[n]; n++)
                if (scales[n] == j)
                    draw = FALSE;

            /* dots */
            if (draw)
            {
                l = plot->radius * pattern_plot_signal(plot->scale, j);
                x = plot->offset + plot->radius + sin(DEG2RAD(i)) * l;
                y = plot->offset + plot->radius + cos(DEG2RAD(i)) * l;
                cairo_set_line_width(cr, plot->width / (PATTERN_PLOT_BASE_SIZE / (PATTERN_PLOT_BORDER_WIDTH / 2.0)));
                cairo_arc(cr, x, y, 0.5, 0, 2*M_PI);
                cairo_stroke_preserve(cr);
                cairo_fill(cr);
            }
        }

        /* angle labels */
        if (k && (k % 30) == 0)
        {
            if (plot->full_angle && k < 0)
                k = 360 + k;

            l = plot->radius + plot->width / (PATTERN_PLOT_BASE_SIZE / PATTERN_ANGLE_LABEL_OFF);
            x = plot->offset + plot->radius + sin(DEG2RAD(i)) * l;
            y = plot->offset + plot->radius + cos(DEG2RAD(i)) * l;

            if (!plot->full_angle && k == 180)
                g_snprintf(text, sizeof(text), "±%d°", k);
            else
                g_snprintf(text, sizeof(text), "%d°", k);

            cairo_text_extents(cr, text, &extents);
            x -= extents.width / 2.0 + extents.x_bearing;
            y -= extents.height / 2.0 + extents.y_bearing;

            cairo_set_source_rgb(cr, (plot->black ? 0.75 : 0.25), (plot->black ? 0.75 : 0.25), (plot->black ? 0.75 : 0.25));
            cairo_move_to(cr, round(x), round(y));
            cairo_show_text(cr, text);
            cairo_stroke(cr);
        }
        k -= 10;
    }
}

static void
pattern_plot_radiation(cairo_t        *cr,
                       pattern_plot_t *plot,
                       pattern_t      *p)
{
    pattern_data_t *data;
    pattern_signal_t *s;
    GtkTreeIter iter;
    gint show_frequency = FALSE;
    gint frequency = 0;
    gint new_frequency;
    gint i = 0;
    gdouble line_width = plot->width / (PATTERN_PLOT_BASE_SIZE / PATTERN_PLOT_LINE_WIDTH) * plot->line;

    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(pattern_get_model(p)), &iter))
    {
        do
        {
            gtk_tree_model_get(GTK_TREE_MODEL(pattern_get_model(p)), &iter, PATTERN_COL_DATA, &data, -1);

            if (pattern_data_get_hide(data))
                continue;

            if (plot->legend)
                pattern_plot_legend(cr, plot, data, pattern_get_visible_count(p), i);

            if (i == 0)
            {
                frequency = pattern_data_get_freq(data);
                show_frequency = TRUE;
            }
            else if (show_frequency)
            {
                new_frequency = pattern_data_get_freq(data);
                if (new_frequency)
                {
                    if (frequency == 0)
                        frequency = new_frequency;
                    else
                        show_frequency = (new_frequency == frequency);
                }
            }

            pattern_plot_radiation_data(cr,
                                        plot,
                                        data,
                                        (plot->norm ? pattern_signal_get_peak(pattern_data_get_signal(data)) : plot->peak),
                                        line_width);

            i++;
        } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(pattern_get_model(p)), &iter));

        /* draw frequency label */
        if (show_frequency && frequency)
            pattern_plot_frequency(cr, plot, frequency);

        cairo_stroke(cr);
    }
}

static void
pattern_plot_radiation_data(cairo_t        *cr,
                            pattern_plot_t *plot,
                            pattern_data_t *data,
                            gdouble         peak,
                            gdouble         line_width)
{
    gdouble x, y, len, ang;
    gint i, j;
    gdouble sample;
    gboolean finished;
    pattern_signal_t *s = pattern_data_get_signal(data);
    const GdkRGBA *color = pattern_data_get_color(data);
    gint interp = pattern_signal_interp(s);
    gint count = pattern_signal_count(s);

    if (!count)
        return;

    cairo_set_line_width(cr, line_width);
    cairo_set_source_rgba(cr,
                          color->red,
                          color->green,
                          color->blue,
                          PATTERN_PLOT_FG_ALPHA);

    finished = pattern_signal_get_finished(s);

    for (i = 0; i < count; i++)
    {
        for (j = 0; j < interp; j++)
        {
            /* We want to start from the first sample */
            gint idx = i - pattern_signal_get_rotate(s);

            sample = pattern_signal_get_sample_interp(s, idx, j / (gdouble)interp);
            len = plot->radius * pattern_plot_signal(plot->scale, sample - peak);
            ang = M_PI - (idx * interp + j) / (count * (gdouble)interp) * 2.0 * M_PI;
            x = plot->offset + plot->radius + sin(ang) * len;
            y = plot->offset + plot->radius + cos(ang) * len;
            cairo_line_to(cr, x, y);

            if (i == count-1 && !finished)
                break;
        }
    }

    if (finished)
        cairo_close_path(cr);

    if (pattern_data_get_fill(data))
    {
        cairo_stroke_preserve(cr);
        cairo_set_source_rgba(cr,
                              color->red,
                              color->green,
                              color->blue,
                              PATTERN_PLOT_BG_ALPHA);
        cairo_fill(cr);
    }
    else
    {
        cairo_stroke(cr);
    }
}

static void
pattern_plot_legend(cairo_t        *cr,
                    pattern_plot_t *plot,
                    pattern_data_t *data,
                    gint            visible,
                    gint            index)
{
    gint offset = (gint)(plot->width / (PATTERN_PLOT_BASE_SIZE / (PATTERN_PLOT_OFFSET / 4.0)));
    const GdkRGBA *color = pattern_data_get_color(data);

    gint line_height = (gint)(plot->width / (PATTERN_PLOT_BASE_SIZE / PATTERN_PLOT_LEGEND_WIDTH));
    gint font_height = (gint)(plot->width / (PATTERN_PLOT_BASE_SIZE / PATTERN_FONT_SIZE_LEGEND));
    gint spacing = (gint)(plot->width / (PATTERN_PLOT_BASE_SIZE / PATTERN_LEGEND_SPACING));
    gint x, y;

    cairo_set_line_width(cr, line_height);
    cairo_set_source_rgba(cr,
                          color->red,
                          color->green,
                          color->blue,
                          PATTERN_PLOT_FG_ALPHA);
    x = offset;
    y = plot->width - (visible - index) * (font_height + spacing + line_height) + spacing - offset;

    cairo_rectangle(cr, x + 0.5, y + 0.5, font_height, font_height);
    cairo_stroke_preserve(cr);
    cairo_set_source_rgba(cr,
                          color->red,
                          color->green,
                          color->blue,
                          PATTERN_PLOT_BG_ALPHA);
    cairo_fill(cr);

    cairo_set_source_rgba(cr,
                          color->red,
                          color->green,
                          color->blue,
                          1.0);

    x += font_height + spacing + line_height;
    y += font_height - line_height;

    cairo_set_font_size(cr, font_height);
    cairo_move_to(cr, round(x), round(y));
    cairo_show_text(cr, pattern_data_get_name(data));
    cairo_stroke(cr);
}

static void
pattern_plot_frequency(cairo_t        *cr,
                       pattern_plot_t *plot,
                       gint            freq)
{
    gdouble offset = plot->width / (PATTERN_PLOT_BASE_SIZE / (PATTERN_PLOT_OFFSET / 4.0));
    cairo_text_extents_t extents;
    gchar *text = pattern_misc_format_frequency(freq);
    gdouble x, y;

    cairo_set_source_rgb(cr, (plot->black ? 1.0 : 0.0), (plot->black ? 1.0 : 0.0), (plot->black ? 1.0 : 0.0));
    cairo_set_font_size(cr, plot->width / (PATTERN_PLOT_BASE_SIZE / PATTERN_FONT_SIZE_FREQ));
    cairo_text_extents(cr, text, &extents);

    x = plot->width - extents.width - offset;
    y = plot->width - offset;
    cairo_move_to(cr, round(x), round(y));
    cairo_show_text(cr, text);
    cairo_stroke(cr);

    g_free(text);
}

static void
pattern_plot_focus(cairo_t        *cr,
                   pattern_plot_t *plot,
                   pattern_t      *p)
{
    pattern_data_t *data = pattern_get_current(p);
    pattern_ui_t *ui = pattern_get_ui(p);

    if (data == NULL)
        return;

    if (ui == NULL)
        return;

    if (pattern_ui_get_focus_idx(ui) == -1)
        return;

    pattern_plot_pointer(cr, plot, p, data);
    pattern_plot_info(cr, plot, p, data);
    pattern_plot_stats(cr, plot, p, data);
}


static void
pattern_plot_pointer(cairo_t        *cr,
                     pattern_plot_t *plot,
                     pattern_t      *p,
                     pattern_data_t *data)
{
    pattern_ui_t *ui = pattern_get_ui(p);
    pattern_signal_t *s = pattern_data_get_signal(data);
    gint count = pattern_signal_count(s);
    gdouble peak = (plot->norm ? pattern_signal_get_peak(s) : plot->peak);
    gdouble value = pattern_signal_get_sample(s, pattern_ui_get_focus_idx(ui));
    gdouble line_width = plot->width / (PATTERN_PLOT_BASE_SIZE / PATTERN_PLOT_LINE_WIDTH);
    gdouble len = plot->radius * pattern_plot_signal(plot->scale, value - peak);
    gdouble ang = M_PI - pattern_ui_get_focus_idx(ui) / (gdouble) count * 2.0 * M_PI;
    gdouble x = plot->offset + plot->radius + sin(ang) * len;
    gdouble y = plot->offset + plot->radius + cos(ang) * len;

    cairo_set_source_rgb(cr, (plot->black ? 0.75 : 0.25), (plot->black ? 0.75 : 0.25), (plot->black ? 0.75 : 0.25));
    cairo_set_line_width(cr, line_width);
    cairo_arc(cr, x, y, line_width * 3.0, 0, 2 * M_PI);
    cairo_stroke(cr);
}

static void
pattern_plot_info(cairo_t        *cr,
                  pattern_plot_t *plot,
                  pattern_t      *p,
                  pattern_data_t *data)
{
    pattern_ui_t *ui = pattern_get_ui(p);
    pattern_signal_t *s = pattern_data_get_signal(data);
    gint count = pattern_signal_count(s);
    gint idx = pattern_ui_get_focus_idx(ui);
    gdouble angle = idx / (gdouble)count * 360.0;
    gdouble peak = (plot->norm ? pattern_signal_get_peak(pattern_data_get_signal(data)) : plot->peak);
    const GdkRGBA *color = pattern_data_get_color(data);
    gint offset = (gint)(plot->width / (PATTERN_PLOT_BASE_SIZE / (PATTERN_PLOT_OFFSET / 4.0)));
    gint font_height = (gint)(plot->width / (PATTERN_PLOT_BASE_SIZE / PATTERN_FONT_SIZE_LEGEND));
    gint spacing = (gint)(plot->width / (PATTERN_PLOT_BASE_SIZE / PATTERN_LEGEND_SPACING));
    gchar buff[50];
    gint x, y;

    if (!pattern_get_full_angle(p) && angle > 180.0)
        angle -= 360.0;

    cairo_set_source_rgba(cr,
                          color->red,
                          color->green,
                          color->blue,
                          1.0);
    cairo_set_font_size(cr, font_height);
    x = offset;

    y = (gint)plot->offset + spacing;
    cairo_move_to(cr, round(x), round(y));
    g_snprintf(buff, sizeof(buff), "Angle: %.1f°", angle);
    cairo_show_text(cr, buff);
    cairo_stroke(cr);

    y += font_height + spacing;
    cairo_move_to(cr, round(x), round(y));
    g_snprintf(buff, sizeof(buff), "Val: %.1f dB", pattern_signal_get_sample(s, idx));
    cairo_show_text(cr, buff);
    cairo_stroke(cr);

    y += font_height + spacing;
    cairo_move_to(cr, round(x), round(y));
    g_snprintf(buff, sizeof(buff), "Att: %.1f dB", pattern_signal_get_sample(s, idx) - peak);
    cairo_show_text(cr, buff);
    cairo_stroke(cr);
}

static void
pattern_plot_stats(cairo_t        *cr,
                   pattern_plot_t *plot,
                   pattern_t      *p,
                   pattern_data_t *data)
{
    const pattern_signal_t *s = pattern_data_get_signal(data);
    gdouble max = pattern_signal_get_peak(s);
    gdouble min = pattern_signal_get_min(s);
    gdouble delta = max - min;

    const GdkRGBA *color = pattern_data_get_color(data);
    cairo_set_source_rgba(cr,
                          color->red,
                          color->green,
                          color->blue,
                          1.0);


    gint offset = (gint)(plot->width / (PATTERN_PLOT_BASE_SIZE / (PATTERN_PLOT_OFFSET / 4.0)));
    gint font_height = (gint)(plot->width / (PATTERN_PLOT_BASE_SIZE / PATTERN_FONT_SIZE_LEGEND));
    gint spacing = (gint)(plot->width / (PATTERN_PLOT_BASE_SIZE / PATTERN_LEGEND_SPACING));
    gchar text[50];
    gint x, y;

    cairo_text_extents_t extents;
    cairo_set_font_size(cr, font_height);

    g_snprintf(text, sizeof(text), "Max: %.1f dB", max);
    cairo_text_extents(cr, text, &extents);
    x = plot->width - extents.width - offset;
    y = (gint)plot->offset + spacing;
    cairo_move_to(cr, round(x), round(y));
    cairo_show_text(cr, text);
    cairo_stroke(cr);

    g_snprintf(text, sizeof(text), "Min: %.1f dB", min);
    cairo_text_extents(cr, text, &extents);
    x = plot->width - extents.width - offset;
    y += font_height + spacing;
    cairo_move_to(cr, round(x), round(y));
    cairo_show_text(cr, text);
    cairo_stroke(cr);

    g_snprintf(text, sizeof(text), "\u0394: %.1f dB", delta);
    cairo_text_extents(cr, text, &extents);
    x = plot->width - extents.width - offset;
    y += font_height + spacing;
    cairo_move_to(cr, round(x), round(y));
    cairo_show_text(cr, text);
    cairo_stroke(cr);
}

gboolean
pattern_plot_to_file(const gchar *filename,
                     pattern_t   *p)
{
    cairo_surface_t *surface;
    cairo_t *cr;
    gboolean ret = FALSE;
    gint size = pattern_get_size(p);
    gchar *ext = strrchr(filename, '.');
    gboolean png;

    if (ext && g_ascii_strcasecmp(ext, ".svg") == 0)
    {
        /* SVG vector file */
        surface = cairo_svg_surface_create(filename, size, size);
        png = FALSE;
    }
    else
    {
        /* Other: PNG file */
        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size, size);
        png = TRUE;
    }

    if (surface == NULL)
        return FALSE;

    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
    {
        cairo_surface_destroy(surface);
        return FALSE;
    }

    cr = cairo_create(surface);
    if (cr)
    {
        pattern_plot(cr, p);
        cairo_destroy(cr);

        if (png)
            ret = cairo_surface_write_to_png(surface, filename) == CAIRO_STATUS_SUCCESS;
        else
            ret = cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS;
    }

    cairo_surface_destroy(surface);
    return ret;
}

static gdouble
pattern_plot_signal(gint    scale,
                    gdouble x)
{
    if (scale) /* Linear */
        return (x > scale ? -x / scale + 1.0 : 0.0);
    else /* ARRL */
        return pow(0.89, (-0.5 * x));
}
