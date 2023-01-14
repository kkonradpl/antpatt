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

#ifndef ANTPATT_PATTERN_COLOR_H_
#define ANTPATT_PATTERN_COLOR_H_

GdkRGBA pattern_color_next(void);
gchar* pattern_color_to_string(const GdkRGBA*);

#endif
