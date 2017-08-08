/* sane - Scanner Access Now Easy.
   Copyright (C) 1996, 1997 David Mosberger-Tang and Andreas Beck
   This file is part of the SANE package.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA.

   As a special exception, the authors of SANE give permission for
   additional uses of the libraries contained in this release of SANE.

   The exception is that, if you link a SANE library with other files
   to produce an executable, this does not by itself cause the
   resulting executable to be covered by the GNU General Public
   License.  Your use of that executable is in no way restricted on
   account of linking the SANE library code into it.

   This exception does not, however, invalidate any other reasons why
   the executable file might be covered by the GNU General Public
   License.

   If you submit changes to SANE to the maintainers to be included in
   a subsequent release, you agree by submitting the changes that
   those changes may be distributed with this exception intact.

   If you write modifications of your own for SANE, it is your choice
   whether to permit this exception to apply to your modifications.
   If you do not wish that, delete this exception notice.  */

//#include "../include/sane/config.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <stdarg.h>
#include <syslog.h>
#ifdef HAVE_OS2_H
#include <sys/types.h>
#endif
#include <sys/stat.h>

#ifdef HAVE_OS2_H
# define INCL_DOS
# include <os2.h>
#endif

#define BACKEND_NAME sanei_debug
#include "sanei_debug.h"

void 
sanei_init_debug (const char * backend, int * var)
{
  char ch, buf[256] = "SANE_DEBUG_";
  const char * val;
  unsigned int i;

  *var = 0;

  for (i = 11; (ch = backend[i - 11]) != 0; ++i)
    {
      if (i >= sizeof (buf) - 1)
        break;
      buf[i] = toupper(ch);
    }
  buf[i] = '\0';

  val = getenv (buf);

  if (!val)
    return;

  *var = atoi (val);

  DBG (0, "Setting debug level of %s to %d.\n", backend, *var);
}

void
sanei_debug_msg
  (int level, int max_level, const char *be, const char *fmt, va_list ap)
{
  if (max_level >= level)
    {
        fprintf (stderr, "[%s] ", be);
        vfprintf (stderr, fmt, ap);
    }
}

#ifdef NDEBUG
void
sanei_debug_ndebug (int level, const char *fmt, ...)
{
  /* this function is never called */
}
#endif
