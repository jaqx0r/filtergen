/* utility functions for filter generation
 *
 * Copyright (c) 2002 Matthew Kirkwood
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *strapp(char *s, const char *n)
{
    size_t l;
    if(!n) return s;
    if(!s) return strdup(n);
    l = strlen(s) + strlen(n) + 1;
    s = realloc(s, l);
    return strcat(s, n);
}

int str_to_int(const char *s, int *i)
{
    long m;
    char *e;

    m = strtol(s, &e, 10);
    if(*e) return -1;
    *i = m;
    return 0;
}

char *int_to_str_dup(int i)
{
    char buf[100];
    snprintf(buf, sizeof(buf)-1, "%d", i);
    buf[sizeof(buf)-1] = 0;
    return strdup(buf);
}
