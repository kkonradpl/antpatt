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

#ifndef ANTPATT_PATTERN_UI_DIALOGS_H_
#define ANTPATT_PATTERN_UI_DIALOGS_H_

void pattern_ui_dialog(GtkWindow*, GtkMessageType, gchar*, gchar*, ...);
gchar* pattern_ui_dialog_open(GtkWindow*);
gchar* pattern_ui_dialog_save(GtkWindow*);
GSList* pattern_ui_dialog_import(GtkWindow*);
gchar* pattern_ui_dialog_export(GtkWindow*);
void pattern_ui_dialog_about(GtkWindow*);

#endif
