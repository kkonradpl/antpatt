/*
 *  antpatt - antenna pattern plotting and analysis software
 *  Copyright (c) 2022-2023  Konrad Kosmatka
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
#include "pattern-ui.h"
#include "pattern-color.h"
#ifdef G_OS_WIN32
#include "mingw.h"
#endif

static const gchar command_start[] = "START";
static const gchar command_stop[] = "STOP";
static const gchar command_push[] = "PUSH";
static const gchar command_name[] = "NAME";
static const gchar command_freq[] = "FREQ";
static const gchar command_color[] = "COLOR";
static const gchar command_avg[] = "AVG";
static const gchar command_fill[] = "FILL";
static const gchar command_rev[] = "REV";

static const gchar response_ready[] = "READY";
static const gchar response_bye[] = "BYE";
static const gchar response_ok[] = "OK";
static const gchar response_error[] = "ERROR";

static GIOChannel *channel_stdin = NULL;

static gboolean handle_stdin(GIOChannel*, GIOCondition, gpointer);
static void parse_command(pattern_t*, gchar*);
static void send_response(const gchar*);
static gboolean set_running(pattern_t*, gboolean);

static pattern_t *instance = NULL;


void
pattern_ipc_init(pattern_t *p)
{
#ifdef G_OS_WIN32
    channel_stdin = g_io_channel_win32_new_fd(fileno(stdin));
#else
    channel_stdin = g_io_channel_unix_new(fileno(stdin));
#endif
    if (channel_stdin)
    {
        instance = p;
        g_io_channel_set_close_on_unref(channel_stdin, TRUE);
        g_io_add_watch(channel_stdin, G_IO_IN | G_IO_ERR | G_IO_HUP, (GIOFunc)handle_stdin, NULL);
    }

    send_response(channel_stdin ? response_ready : response_error);
}

void
pattern_ipc_cleanup()
{
    if (channel_stdin)
    {
        send_response(response_bye);
        g_io_channel_unref(channel_stdin);
        channel_stdin = NULL;
    }
}

static gboolean
handle_stdin(GIOChannel   *source,
             GIOCondition  cond,
             gpointer      user_data)
{
    g_autofree gchar *buff = NULL;
    GIOStatus status;

    status = g_io_channel_read_line(source, &buff, NULL, NULL, NULL);

    if (instance == NULL)
    {
        send_response(response_error);
        return G_SOURCE_CONTINUE;
    }

    switch (status)
    {
        case G_IO_STATUS_AGAIN:
            return G_SOURCE_CONTINUE;
        case G_IO_STATUS_NORMAL:
            parse_command(instance, buff);
            return G_SOURCE_CONTINUE;
        default:
            parse_command(instance, NULL);
            return G_SOURCE_REMOVE;
    }
}

static void
parse_command(pattern_t *p,
              gchar     *command)
{
    pattern_data_t *current = pattern_get_current(p);
    pattern_ui_t *ui = pattern_get_ui(p);
    static gboolean running = FALSE;
    gchar *value = command;
    gboolean ack = FALSE;

    if (command == NULL)
    {
        if (running)
            running = set_running(p, FALSE);
        return;
    }

    g_strchomp(command);
    strsep(&value, " ");

    if (g_ascii_strcasecmp(command, command_start) == 0)
    {
        if (running)
            running = set_running(p, FALSE);

        pattern_data_t *data = pattern_data_new(pattern_signal_new());
        GdkRGBA color = pattern_color_next();
        pattern_data_set_name(data, "Measurement");
        pattern_data_set_color(data, &color);
        pattern_add(p, data);

        running = set_running(p, TRUE);
        ack = TRUE;
    }
    else if (running)
    {
        if (g_ascii_strcasecmp(command, command_stop) == 0)
        {
            running = set_running(p, FALSE);
            ack = TRUE;
        }
        else if (g_ascii_strcasecmp(command, command_name) == 0)
        {
            pattern_data_set_name(current, value);
            if (ui)
                pattern_ui_sync_name(ui, TRUE);
            ack = TRUE;
        }
        else if (g_ascii_strcasecmp(command, command_freq) == 0)
        {
            gint freq;
            if (value && sscanf(value, "%d", &freq))
            {
                pattern_data_set_freq(current, freq);
                if (ui)
                    pattern_ui_sync_freq(ui, TRUE);
                ack = TRUE;
            }
        }
        else if (g_ascii_strcasecmp(command, command_color) == 0)
        {
            GdkRGBA color;
            if (value && gdk_rgba_parse(&color, value))
            {
                pattern_data_set_color(current, &color);
                if (ui)
                    pattern_ui_sync_color(ui, TRUE);
                ack = TRUE;
            }
        }
        else if (g_ascii_strcasecmp(command, command_avg) == 0)
        {
            gint avg;
            if (value)
            {
                avg = g_ascii_strtoll(value, NULL, 10);
                pattern_signal_set_avg(pattern_data_get_signal(current), avg);
                if (ui)
                    pattern_ui_sync_avg(ui, TRUE);
                ack = TRUE;
            }
        }
        else if (g_ascii_strcasecmp(command, command_fill) == 0)
        {
            gint fill;
            if (value)
            {
                fill = (g_ascii_strtoll(value, NULL, 10) != 0);
                pattern_data_set_fill(current, fill);
                if (ui)
                    pattern_ui_sync_fill(ui, TRUE);
                ack = TRUE;
            }
        }
        else if (g_ascii_strcasecmp(command, command_rev) == 0)
        {
            gint rev;
            if (value)
            {
                rev = (g_ascii_strtoll(value, NULL, 10) != 0);
                pattern_signal_set_rev(pattern_data_get_signal(current), rev);
                if (ui)
                    pattern_ui_sync_rev(ui, TRUE);
                ack = TRUE;
            }
        }
        else if (g_ascii_strcasecmp(command, command_push) == 0)
        {
            gdouble sample;
            if (value && sscanf(value, "%lf", &sample))
            {
                pattern_signal_push(pattern_data_get_signal(current), sample);
                if (ui)
                    pattern_ui_sync_data(ui);
                ack = TRUE;
            }
        }
    }

    send_response(ack ? response_ok : response_error);
}

static void
send_response(const gchar* response)
{
    fprintf(stdout,"%s", response);
    fprintf(stdout,"\n");
    fflush(stdout);
}

static gboolean
set_running(pattern_t *p,
            gboolean   running)
{
    pattern_data_t *data = pattern_get_current(p);
    pattern_ui_t *ui = pattern_get_ui(p);

    if (!running &&
        data)
    {
        pattern_signal_set_finished(pattern_data_get_signal(data));
    }

    if (ui)
    {
        pattern_ui_interactive(ui, running);
    }

    return running;
}