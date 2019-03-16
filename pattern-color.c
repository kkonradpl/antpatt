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

#include <gtk/gtk.h>

#define HUE_INITIAL 0.558
#define HUE_STEP    0.618

static void hsv2rgb(gdouble, gdouble, gdouble, GdkColor*);

GdkColor
pattern_color_next()
{
    static gdouble h = HUE_INITIAL;
    GdkColor color;

    h -= HUE_STEP;
    if(h < 0.0)
        h += 1.0;

    hsv2rgb(h, 1.0, 1.0, &color);
    return color;
}

static void
hsv2rgb(gdouble   h,
        gdouble   s,
        gdouble   v,
        GdkColor *rgb)
{
    gint i = (gint)(h * 6.0);
    gdouble f = h * 6.0 - i;
    gdouble p = v * (1.0 - s);
    gdouble q = v * (1.0 - f * s);
    gdouble t = v * (1.0 - (1.0 - f) * s);

    switch(i % 6)
    {
    case 0:
        rgb->red   = (guint16)(v*65535.0);
        rgb->green = (guint16)(t*65535.0);
        rgb->blue  = (guint16)(p*65535.0);
        break;
    case 1:
        rgb->red   = (guint16)(q*65535.0);
        rgb->green = (guint16)(v*65535.0);
        rgb->blue  = (guint16)(p*65535.0);
        break;
    case 2:
        rgb->red   = (guint16)(p*65535.0);
        rgb->green = (guint16)(v*65535.0);
        rgb->blue  = (guint16)(t*65535.0);
        break;
    case 3:
        rgb->red   = (guint16)(p*65535.0);
        rgb->green = (guint16)(q*65535.0);
        rgb->blue  = (guint16)(v*65535.0);
        break;
    case 4:
        rgb->red   = (guint16)(t*65535.0);
        rgb->green = (guint16)(p*65535.0);
        rgb->blue  = (guint16)(v*65535.0);
        break;
    case 5:
        rgb->red   = (guint16)(v*65535.0);
        rgb->green = (guint16)(p*65535.0);
        rgb->blue  = (guint16)(q*65535.0);
        break;
    }
}
