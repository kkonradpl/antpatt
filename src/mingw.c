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

#define _WIN32_WINNT 0x0500
#include <gtk/gtk.h>
#include <windows.h>

#define MINGW_FONT_FILE ".\\share\\fonts\\TTF\\DejaVuSansMono.ttf"

static gint mingw_font = 0;


void
mingw_init(void)
{
    mingw_font = AddFontResourceEx(MINGW_FONT_FILE, FR_PRIVATE, NULL);
}

void
mingw_cleanup(void)
{
    if(mingw_font)
        RemoveFontResourceEx(MINGW_FONT_FILE, FR_PRIVATE, NULL);
}

gchar*
strsep(gchar       **string,
       const gchar  *del)
{
    gchar *start = *string;
    gchar *p = (start ? strpbrk(start, del) : NULL);

    if(!p)
    {
        *string = NULL;
    }
    else
    {
        *p = '\0';
        *string = p + 1;
    }
    return start;
}

gboolean
mingw_uri_signal(GtkWidget *label,
                 gchar     *uri,
                 gpointer   data)
{
    ShellExecute(0, "open", uri, NULL, NULL, 1);
    return TRUE;
}
