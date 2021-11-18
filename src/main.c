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
#include "pattern.h"

#ifdef G_OS_WIN32
#include "win32.h"
#endif

gint
main(gint   argc,
     gchar *argv[])
{
    pattern_t *p;

#ifdef G_OS_WIN32
    win32_init();
#endif

    gtk_disable_setlocale();
    gtk_init(&argc, &argv);

    p = pattern_new();
    pattern_set_ui(p, pattern_ui_window_new());

    gtk_main();

#ifdef G_OS_WIN32
    win32_cleanup();
#endif
    return 0;
}
