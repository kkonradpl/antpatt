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
#include <getopt.h>
#include "pattern.h"
#include "pattern-ui.h"
#include "pattern-ipc.h"
#include "pattern-json.h"
#ifdef G_OS_WIN32
#include "mingw.h"
#endif

typedef struct antpatt_arg
{
    gboolean interactive;
    const char *project;
    gboolean dark_theme;
} antpatt_arg_t;

static antpatt_arg_t args =
{
    .interactive = FALSE,
    .project = NULL,
    .dark_theme = FALSE
};

static void
antpatt_usage(void)
{
    printf("antpatt " APP_VERSION " - antenna pattern plotting and analysis software\n");
    printf("usage: antpatt [-i] [-d] project\n");
    printf("options:\n");
    printf("  -i  interactive console mode\n");
    printf("  -d  prefer dark theme\n");
}

static void
parse_args(gint   argc,
           gchar *argv[])
{
    gint c;
    while ((c = getopt(argc, argv, "hid")) != -1)
    {
        switch (c)
        {
            case 'h':
                antpatt_usage();
                exit(0);

            case 'i':
                args.interactive = TRUE;
                break;

            case 'd':
                args.dark_theme = TRUE;

            default:
                break;
        }
    }

    if (optind == argc - 1)
        args.project = argv[optind];
}

gint
main(gint   argc,
     gchar *argv[])
{
    pattern_t *p = pattern_new();
    gchar *error = NULL;

    gtk_disable_setlocale();
    gtk_init(&argc, &argv);
    parse_args(argc, argv);

#ifdef G_OS_WIN32
    mingw_init();
#endif

    if (args.dark_theme)
        g_object_set(gtk_settings_get_default(), "gtk-application-prefer-dark-theme", TRUE, NULL);

    if (args.project)
    {
        if (!pattern_json_load(p, args.project, &error))
        {
            fprintf(stderr, "Error: %s\n", error);
            g_free(error);
        }
    }

    pattern_ui(p);

    if (args.interactive)
        pattern_ipc_init(p);

    gtk_main();

#ifdef G_OS_WIN32
    mingw_cleanup();
#endif

    if (args.interactive)
        pattern_ipc_cleanup();

    pattern_free(p);
    return 0;
}
