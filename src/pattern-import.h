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

#ifndef ANTPATT_PATTERN_IMPORT_H_
#define ANTPATT_PATTERN_IMPORT_H_
#include "pattern-signal.h"

enum
{
    PATTERN_IMPORT_OK = 0,
    PATTERN_IMPORT_ERROR,
    PATTERN_IMPORT_INVALID_FORMAT,
    PATTERN_IMPORT_EMPTY_FILE
};

typedef struct pattern_import pattern_import_t;

pattern_import_t* pattern_import_new(void);
void              pattern_import_free(pattern_import_t*, gboolean);
gint              pattern_import(pattern_import_t*, const gchar*);

pattern_signal_t* pattern_import_get_signal(pattern_import_t*);
const gchar*      pattern_import_get_name(pattern_import_t*);
gint              pattern_import_get_freq(pattern_import_t*);

#endif

