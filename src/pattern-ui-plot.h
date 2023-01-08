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

#ifndef ANTPATT_PATTERN_UI_PLOT_H_
#define ANTPATT_PATTERN_UI_PLOT_H_

gboolean pattern_ui_plot(GtkWidget*, cairo_t*, pattern_ui_t*);

gboolean pattern_ui_plot_motion(GtkWidget*, GdkEventMotion*, pattern_ui_t*);
gboolean pattern_ui_plot_click(GtkWidget*, GdkEventButton*, pattern_ui_t*);
gboolean pattern_ui_plot_leave(GtkWidget*, GdkEvent*, pattern_ui_t*);

#endif
