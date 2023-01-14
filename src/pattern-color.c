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

#include <gtk/gtk.h>
#include <math.h>

#define HUE_INITIAL 0.558
#define HUE_STEP    0.618

static void hsv2rgb(gdouble, gdouble, gdouble, GdkRGBA*);

GdkRGBA
pattern_color_next()
{
    static gdouble h = HUE_INITIAL;
    GdkRGBA color;

    h -= HUE_STEP;
    if (h < 0.0)
        h += 1.0;

    hsv2rgb(h, 1.0, 1.0, &color);
    return color;
}

gchar*
pattern_color_to_string(const GdkRGBA *color)
{
    g_assert(color != NULL);
    return g_strdup_printf("#%02X%02X%02X",
                           (uint8_t)round(color->red*255),
                           (uint8_t)round(color->green*255),
                           (uint8_t)round(color->blue*255));
}

static void
hsv2rgb(gdouble  h,
        gdouble  s,
        gdouble  v,
        GdkRGBA *rgb)
{
    gint i = (gint)(h * 6.0);
    gdouble f = h * 6.0 - i;
    gdouble p = v * (1.0 - s);
    gdouble q = v * (1.0 - f * s);
    gdouble t = v * (1.0 - (1.0 - f) * s);

    switch (i % 6)
    {
    case 0:
        rgb->red   = v;
        rgb->green = t;
        rgb->blue  = p;
        break;
    case 1:
        rgb->red   = q;
        rgb->green = v;
        rgb->blue  = p;
        break;
    case 2:
        rgb->red   = p;
        rgb->green = v;
        rgb->blue  = t;
        break;
    case 3:
        rgb->red   = p;
        rgb->green = q;
        rgb->blue  = v;
        break;
    case 4:
        rgb->red   = t;
        rgb->green = p;
        rgb->blue  = v;
        break;
    case 5:
        rgb->red   = v;
        rgb->green = p;
        rgb->blue  = q;
        break;
    }

    rgb->alpha = 1.0;
}
