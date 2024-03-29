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

#ifndef ANTPATT_PATTERN_PLOT_H_
#define ANTPATT_PATTERN_PLOT_H_

#define PATTERN_PLOT_BASE_SIZE    500.0
#define PATTERN_PLOT_OFFSET        32.0
#define PATTERN_PLOT_BORDER_WIDTH   1.0

void pattern_plot(cairo_t*, pattern_t*);
gboolean pattern_plot_to_file(const gchar*, pattern_t*);

#endif
