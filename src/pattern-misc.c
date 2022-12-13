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
#include <string.h>
#include "pattern.h"
#include "pattern-color.h"

gchar*
pattern_misc_format_frequency(gint freq)
{
    gchar buff[10];
    size_t i;

    if(freq < 1000)
    {
        return g_strdup_printf("%d kHz", freq);
    }
    else
    {
        g_snprintf(buff, sizeof(buff), "%.3f", freq / 1000.0);

        for (i = strlen(buff) - 1; i >= 0 && buff[i] == '0'; i--);
        if (i >= 0 && buff[i] == '.')
            i++;
        buff[i + 1] = '\0';

        return g_strdup_printf("%s MHz", buff);
    }
}

gchar*
pattern_misc_info_all(pattern_t *p,
                      gdouble    angle)
{
    GtkTreeIter iter;
    pattern_data_t *data;
    pattern_signal_t *s;
    gint count;
    gdouble angle_displ;
    gdouble x;
    gchar *color;
    gdouble peak;
    GString *str;
    gchar *cstr;

    str = g_string_new(NULL);

    angle_displ = (!pattern_get_full_angle(p) && angle > 180.0) ? angle-360.0 : angle;
    g_string_append_printf(str,
                           "<big>Angle: %.2f°</big>\n\n",
                           angle_displ);

    if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(pattern_get_model(p)), &iter))
    {
        do
        {
            gtk_tree_model_get(GTK_TREE_MODEL(pattern_get_model(p)), &iter, PATTERN_COL_DATA, &data, -1);
            if(pattern_data_get_hide(data))
                continue;

            s = pattern_data_get_signal(data);
            count = pattern_signal_count(s);
            x = angle/360.0 * count;
            peak = (pattern_get_normalize(p) ? pattern_signal_get_peak(pattern_data_get_signal(data)) : pattern_get_peak(p)),
            color = pattern_color_to_string(pattern_data_get_color(data));

            g_string_append_printf(str,
                                   "<span background=\"%s\" foreground=\"%s\"><b>%s</b></span>\n"
                                   "<b>%.2f</b> (<b>%.2f</b>) dB\n\n",
                                   (pattern_get_black(p) ? "black" : "white"),
                                   color,
                                   pattern_data_get_name(data),
                                   pattern_signal_get_sample_interp(s, (gint)x, x-(gint)x),
                                   pattern_signal_get_sample_interp(s, (gint)x, x-(gint)x) - peak);

            g_free(color);
        } while(gtk_tree_model_iter_next(GTK_TREE_MODEL(pattern_get_model(p)), &iter));
    }

    cstr = g_string_free(str, FALSE);
    cstr[strlen(cstr)-2] = '\0'; // remove last two line feeds
    return cstr;
}
