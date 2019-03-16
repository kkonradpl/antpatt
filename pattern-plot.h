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

#ifndef ANTPATT_PATTERN_PLOT_H_
#define ANTPATT_PATTERN_PLOT_H_

#if GTK_CHECK_VERSION (3, 0, 0)
gboolean pattern_plot(GtkWidget*, cairo_t*, pattern_t*);
#else
gboolean pattern_plot(GtkWidget*, GdkEventExpose*, pattern_t*);
#endif

gboolean pattern_plot_motion(GtkWidget*, GdkEventMotion*, pattern_t*);
gboolean pattern_plot_click(GtkWidget*, GdkEventButton*, pattern_t*);
gboolean pattern_plot_leave(GtkWidget*, GdkEvent*, pattern_t*);

gboolean pattern_plot_to_file(pattern_t*, const gchar*);

#endif
