/* Replacement for GNU C library function getline

   Copyright (C) 1995, 1997, 1999, 2000, 2001, 2002, 2003 Free
   Software Foundation, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef _getline_h_
#define _getline_h_ 1

#include <stdio.h>

#if defined (__GNUC__) || (defined (__STDC__) && __STDC__)
#define __PROTO(args) args
#else
#define __PROTO(args) ()
#endif  /* GCC.  */

#define GETLINE_NO_LIMIT -1

int
getlineMod __PROTO((char **_lineptr, size_t *_n, FILE *_stream));
int
getline_safe __PROTO((char **_lineptr, size_t *_n, FILE *_stream,
                      int limit));
int
getstr __PROTO((char **_lineptr, size_t *_n, FILE *_stream,
                int _terminator, int _offset, int limit));



#endif /* _getline_h_ */

