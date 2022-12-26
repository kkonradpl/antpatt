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

#ifndef ANTPATT_MINGW_H_
#define ANTPATT_MINGW_H_

void mingw_init(void);
void mingw_cleanup(void);

gchar* strsep(gchar**, const gchar*);

gboolean mingw_uri_signal(GtkWidget*, gchar*, gpointer);
void mingw_realize(GtkWidget*, gpointer);

#endif
