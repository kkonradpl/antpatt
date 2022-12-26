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
#include <gdk/gdkwin32.h>
#include <windows.h>
#include <dwmapi.h>

#define MINGW_FONT_FILE ".\\share\\fonts\\TTF\\DejaVuSansMono.ttf"

static gint mingw_font = 0;
static const char css_string[] =
"* {\n"
"    font-family: Sans;\n"
"    font-size: 10pt;\n"
"}\n";

static void mingw_dark_titlebar(GtkWidget*);


void
mingw_init(void)
{
    mingw_font = AddFontResourceEx(MINGW_FONT_FILE, FR_PRIVATE, NULL);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css_string, -1, NULL);
    GdkScreen *screen = gdk_display_get_default_screen(gdk_display_get_default());
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

void
mingw_cleanup(void)
{
    if (mingw_font)
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

void
mingw_realize(GtkWidget *widget,
              gpointer   user_data)
{
    gboolean dark_theme = FALSE;

    g_object_get(gtk_settings_get_default(),
                 "gtk-application-prefer-dark-theme",
                 &dark_theme, NULL);

    if (dark_theme)
        mingw_dark_titlebar(widget);
}

static void
mingw_dark_titlebar(GtkWidget *widget)
{
    const DWORD dark_mode = 20;
    const DWORD dark_mode_pre20h1 = 19;
    const BOOL value = TRUE;
    GdkWindow *window = gtk_widget_get_window(widget);

    if (window == NULL)
        return;

    HWND handle = GDK_WINDOW_HWND(window);
    if (!SUCCEEDED(DwmSetWindowAttribute(handle, dark_mode, &value, sizeof(value))))
        DwmSetWindowAttribute(handle, dark_mode_pre20h1, &value, sizeof(value));
}
