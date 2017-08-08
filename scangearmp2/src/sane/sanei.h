/* sane - Scanner Access Now Easy.
   Copyright (C) 1996 David Mosberger-Tang and Andreas Beck
   Copyright (C) 2002, 2003 Henning Meier-Geinitz

   This file is part of the SANE package.

   SANE is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   SANE is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with sane; see the file COPYING.  If not, write to the Free
   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

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
   If you do not wish that, delete this exception notice. 
*/

/** @file sanei.h
 * Convenience macros and function declarations for backends
 * @sa sanei_backend.h sanei_thread.h
 */

/* Doxygen documentation */

/** @mainpage SANEI (SANE internal routines) documentation
 *
 * @image html ../images/sane-logo2.jpg
 * @section intro Introduction
 *
 * The header files in the include/sane/ directory named sanei_*.h provide
 * function declarations and macros that can be used by every SANE backend.
 * Their implementations can be found in the sanei/ directory. The code aims
 * to be platform-independent to avoid lots of \#ifdef code in the backends.
 * Please use the SANEI functions wherever possible.
 *
 * This documentation was created by the use of doxygen, the 
 * doc/doxygen-sanei.conf configuration file and dcoumentation in the sanei_*.h
 * files.
 *
 * This documenation is far from complete. Any help is appreciated.
 *
 * @section additional Additional documentation
 * - The SANE standard can be found at <a 
 *   href="http://www.sane-project.org/html/">the SANE webserver</a>.
 * - Information on how to write a backend: <a
 *   href="../backend-writing.txt">backend-writing.txt</a>.
 * - General SANE documentation is on <a
 *   href="http://www.sane-project.org/docs.html>the SANE documentation
 *   page</a>.
 * 
 * @section contact Contact 
 *
 * The common way to contact the developers of SANE is the sane-devel
 * mailing list. See the <a
 * href="http://www.sane-project.org/mailing-lists.html">mailing list webpage</a>
 * for details. That's the place to ask questions, report bugs, or announce
 * a new backend.
 * 
 */

#ifndef sanei_h
#define sanei_h

#include "sane.h"

/** @name Public macros and functions
 * @{
 */
/** @def STRINGIFY(x)
 * Turn parameter into string.
 */
/** @def PASTE(x,y)
 * Concatenate parameters.
 *
 */
/** @def NELEMS(a)
 * Return number of elements of an array.
 *
 */

/** @fn extern SANE_Status sanei_check_value (const SANE_Option_Descriptor * opt, void * value);
 * Check the constraints of a SANE option.
 *
 * @param opt   option to check
 * @param value value of the option
 *
 * @return 
 * - SANE_STATUS_GOOD     - on success
 * - SANE_STATUS_INVAL    - if the value doesn't fit inside the constraint
 *   or any other error occured
 * @sa sanei_constrain_value()
 */

/** @fn extern SANE_Status sanei_constrain_value (const SANE_Option_Descriptor * opt, void * value, SANE_Word * info);
 * Check the constraints of a SANE option and adjust its value if necessary.
 *
 * Depending on the type of the option and constraint, value is modified
 * to fit inside constraint.
 *
 * @param opt   option to check
 * @param value value of the option
 * @param info  info is set to SANE_INFO_INEXACT if value was changed
 *
 * @return 
 * - SANE_STATUS_GOOD     - on success
 * - SANE_STATUS_INVAL    - if the function wasn't able to fit value into the
 *   constraint or any other error occured
 * @sa sanei_check_value()
 */

/* @} */

/** @name Private macros
 * @{
 */
/** @def STRINGIFY1(x)
 * Internal use only.
 */
/** @def PASTE1(x,y)
 * Internal use only.
 */
/* @} */

/* A few convenience macros:  */
/** @hideinitializer */
#define NELEMS(a)	((int)(sizeof (a) / sizeof (a[0])))

/** @hideinitializer */
#define STRINGIFY1(x)	#x
/** @hideinitializer */
#define STRINGIFY(x)	STRINGIFY1(x)

/** @hideinitializer */
#define PASTE1(x,y)	x##y
/** @hideinitializer */
#define PASTE(x,y)	PASTE1(x,y)

extern SANE_Status sanei_check_value (const SANE_Option_Descriptor * opt,
				      void * value);

extern SANE_Status sanei_constrain_value (const SANE_Option_Descriptor * opt,
					  void * value, SANE_Word * info);


#endif /* sanei_h */
