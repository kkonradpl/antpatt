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

#include <gtk/gtk.h>
#include <string.h>
#include "pattern-ui-dialogs.h"
#include "version.h"
#ifdef G_OS_WIN32
#include "mingw.h"
#endif

static void pattern_ui_dialog_file_chooser_response(GtkWidget*, gint, gpointer);
static gboolean str_has_suffix(const gchar*, const gchar*);

void
pattern_ui_dialog(GtkWindow      *window,
                  GtkMessageType  icon,
                  gchar          *title,
                  gchar          *format,
                  ...)
{
    GtkWidget *dialog;
    va_list args;
    gchar *msg;

    va_start(args, format);
    msg = g_markup_vprintf_escaped(format, args);
    va_end(args);
    dialog = gtk_message_dialog_new(window,
                                    GTK_DIALOG_MODAL,
                                    icon,
                                    GTK_BUTTONS_CLOSE,
                                    NULL);
#ifdef G_OS_WIN32
    g_signal_connect(dialog, "realize", G_CALLBACK(mingw_realize), NULL);
#endif
    gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), msg);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    if(!window)
        gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    g_free(msg);
}

gboolean
pattern_ui_dialog_yesno(GtkWindow   *parent,
                        const gchar *title,
                        const gchar *text)
{
    GtkWidget *dialog;
    gint response;

    dialog = gtk_message_dialog_new(parent,
                                    GTK_DIALOG_MODAL,
                                    GTK_MESSAGE_QUESTION,
                                    GTK_BUTTONS_YES_NO,
                                    NULL);
#ifdef G_OS_WIN32
    g_signal_connect(dialog, "realize", G_CALLBACK(mingw_realize), NULL);
#endif
    gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), text);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return response == GTK_RESPONSE_YES;
}

gint
pattern_ui_dialog_ask_unsaved(GtkWindow *parent)
{
    GtkWidget *dialog;
    gint response;

    dialog = gtk_message_dialog_new(parent,
                                    GTK_DIALOG_MODAL,
                                    GTK_MESSAGE_QUESTION,
                                    GTK_BUTTONS_NONE,
                                    "There are some unsaved changes.\nDo you want to save them?");
#ifdef G_OS_WIN32
    g_signal_connect(dialog, "realize", G_CALLBACK(mingw_realize), NULL);
#endif
    gtk_dialog_add_buttons(GTK_DIALOG(dialog),
                           "_Cancel", GTK_RESPONSE_CANCEL,
                           "_Save", GTK_RESPONSE_YES,
                           "_Discard", GTK_RESPONSE_NO,
                           NULL);

    gtk_window_set_title(GTK_WINDOW(dialog), "Project");
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return response;
}

gchar*
pattern_ui_dialog_open(GtkWindow *window)
{
    GtkWidget *dialog;
    GtkFileFilter *filter;
    gchar *filename = NULL;

    dialog = gtk_file_chooser_dialog_new("Open project",
                                         window,
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         "_Open", GTK_RESPONSE_ACCEPT,
                                         NULL);
#ifdef G_OS_WIN32
    g_signal_connect(dialog, "realize", G_CALLBACK(mingw_realize), NULL);
#endif

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Antenna pattern project (*.antp.gz, *.antp)");
    gtk_file_filter_add_pattern(filter, "*" APP_FILE_EXT);
    gtk_file_filter_add_pattern(filter, "*" APP_FILE_EXT APP_FILE_COMPRESS);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    gtk_widget_destroy(dialog);

    return filename;
}

gchar*
pattern_ui_dialog_save(GtkWindow *window)
{
    GtkWidget *dialog;
    GtkWidget *box;
    GtkFileFilter *filter;
    gchar *ret = NULL;

    dialog = gtk_file_chooser_dialog_new("Save project",
                                         window,
                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         "_Save", GTK_RESPONSE_ACCEPT,
                                         NULL);
#ifdef G_OS_WIN32
    g_signal_connect(dialog, "realize", G_CALLBACK(mingw_realize), NULL);
#endif
    gtk_file_chooser_set_create_folders(GTK_FILE_CHOOSER(dialog), TRUE);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);

    gtk_widget_show_all(box);
    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), box);

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Antenna pattern project (*.antp.gz)");
    gtk_file_filter_add_pattern(filter, "*" APP_FILE_EXT APP_FILE_COMPRESS);
    g_object_set_data_full(G_OBJECT(filter), "antpatt-ext", g_strdup(APP_FILE_EXT APP_FILE_COMPRESS), g_free);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Antenna pattern project (*.antp)");
    gtk_file_filter_add_pattern(filter, "*" APP_FILE_EXT);
    g_object_set_data_full(G_OBJECT(filter), "antpatt-ext", g_strdup(APP_FILE_EXT), g_free);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    g_signal_connect(dialog, "response", G_CALLBACK(pattern_ui_dialog_file_chooser_response), &ret);
    while(gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_NONE);

    return ret;
}

GSList*
pattern_ui_dialog_import(GtkWindow *window)
{
    GtkWidget *dialog;
    GSList *list = NULL;

    dialog = gtk_file_chooser_dialog_new("Import files",
                                         window,
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         "_Open", GTK_RESPONSE_ACCEPT,
                                         NULL);
#ifdef G_OS_WIN32
    g_signal_connect(dialog, "realize", G_CALLBACK(mingw_realize), NULL);
#endif
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
        list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
    gtk_widget_destroy(dialog);

    return list;
}

gchar*
pattern_ui_dialog_render(GtkWindow *window)
{
    GtkWidget *dialog;
    GtkFileFilter *filter;
    gchar *filename = NULL;

    dialog = gtk_file_chooser_dialog_new("Render pattern plot",
                                         window,
                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         "_Save", GTK_RESPONSE_ACCEPT,
                                         NULL);
#ifdef G_OS_WIN32
    g_signal_connect(dialog, "realize", G_CALLBACK(mingw_realize), NULL);
#endif
    gtk_file_chooser_set_create_folders(GTK_FILE_CHOOSER(dialog), TRUE);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "PNG image");
    gtk_file_filter_add_pattern(filter, "*.png");
    g_object_set_data_full(G_OBJECT(filter), "antpatt-ext", g_strdup(".png"), g_free);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "SVG image");
    gtk_file_filter_add_pattern(filter, "*.svg");
    g_object_set_data_full(G_OBJECT(filter), "antpatt-ext", g_strdup(".svg"), g_free);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    g_signal_connect(dialog, "response", G_CALLBACK(pattern_ui_dialog_file_chooser_response), &filename);
    while(gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_NONE);

    return filename;
}

gchar*
pattern_ui_dialog_export(GtkWindow *window)
{
    GtkWidget *dialog;
    GtkFileFilter *filter;
    gchar *filename = NULL;

    dialog = gtk_file_chooser_dialog_new("Export pattern data",
                                         window,
                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         "_Save", GTK_RESPONSE_ACCEPT,
                                         NULL);
#ifdef G_OS_WIN32
    g_signal_connect(dialog, "realize", G_CALLBACK(mingw_realize), NULL);
#endif
    gtk_file_chooser_set_create_folders(GTK_FILE_CHOOSER(dialog), TRUE);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "XDR-GTK file (*.xdrp)");
    gtk_file_filter_add_pattern(filter, "*.xdrp");
    g_object_set_data_full(G_OBJECT(filter), "antpatt-ext", g_strdup(".xdrp"), g_free);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Radiomobile file (*.ant)");
    gtk_file_filter_add_pattern(filter, "*.ant");
    g_object_set_data_full(G_OBJECT(filter), "antpatt-ext", g_strdup(".ant"), g_free);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    g_signal_connect(dialog, "response", G_CALLBACK(pattern_ui_dialog_file_chooser_response), &filename);
    while(gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_NONE);

    return filename;
}

static void
pattern_ui_dialog_file_chooser_response(GtkWidget *dialog,
                                        gint       response_id,
                                        gpointer   user_data)
{
    gchar **ret = (gchar**)user_data;
    gchar *filename;
    GtkFileFilter *filter;

    if(response_id != GTK_RESPONSE_ACCEPT)
    {
        gtk_widget_destroy(dialog);
        return;
    }

    if(!(filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog))))
    {
        pattern_ui_dialog(GTK_WINDOW(dialog),
                          GTK_MESSAGE_ERROR,
                          "Error",
                          "No file selected.");
        return;
    }

    filter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(dialog));
    const gchar* ext = g_object_get_data(G_OBJECT(filter), "antpatt-ext");
    if (!str_has_suffix(filename, ext))
    {
        filename = (gchar*)g_realloc(filename, strlen(filename) + strlen(ext) + 1);
        strcat(filename, ext);

        /* After adding the suffix, the GTK should check whether we may overwrite something. */
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), filename);
        gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
        g_free(filename);
        return;
    }

    *ret = filename;
    gtk_widget_destroy(dialog);
}

static gboolean
str_has_suffix(const gchar *string,
               const gchar *suffix)
{
    size_t string_len = strlen(string);
    size_t suffix_len = strlen(suffix);

    if(string_len < suffix_len)
        return FALSE;

    return g_ascii_strncasecmp(string + string_len - suffix_len, suffix, suffix_len) == 0;
}

void
pattern_ui_dialog_about(GtkWindow *window)
{
    GtkWidget *dialog = gtk_about_dialog_new();
#ifdef G_OS_WIN32
    g_signal_connect(dialog, "realize", G_CALLBACK(mingw_realize), NULL);
#endif
    gtk_window_set_icon_name(GTK_WINDOW(dialog), "gtk-about");
    gtk_window_set_transient_for(GTK_WINDOW(dialog), window);
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), APP_NAME);
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), APP_VERSION);
    gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(dialog), APP_ICON);
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "Copyright © 2017-2022 Konrad Kosmatka");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "Antenna pattern plotting and analysis software");
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "https://fmdx.pl/antpatt");
    gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(dialog), APP_LICENCE);
#ifdef G_OS_WIN32
    g_signal_connect(dialog, "activate-link", G_CALLBACK(mingw_uri_signal), NULL);
#endif
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
