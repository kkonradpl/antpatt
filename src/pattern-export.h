/*
 *  antpatt - antenna pattern plotting and analysis software
 *  Copyright (c) 2022  Konrad Kosmatka
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

#ifndef ANTPATT_PATTERN_EXPORT_H_
#define ANTPATT_PATTERN_EXPORT_H_
#include "pattern-data.h"

gboolean pattern_export(pattern_data_t*, const gchar*);

#endif

