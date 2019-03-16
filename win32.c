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

#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <glib.h>

#define WIN32_FONT_FILE "DejaVuSansMono.ttf"

static gint win32_font = 0;


void
win32_init(void)
{
    win32_font = AddFontResourceEx(WIN32_FONT_FILE, FR_PRIVATE, NULL);
}

void
win32_cleanup(void)
{
    if(win32_font)
        RemoveFontResourceEx(WIN32_FONT_FILE, FR_PRIVATE, NULL);
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
