/** @file sanei_debug.h
 * Support for printing debug messages.
 *
 * Use the functions of this header file to print debug or warning messages.
 */

#ifndef _SANEI_DEBUG_H
#define _SANEI_DEBUG_H

#include "sanei.h"

/** @name Public macros
 * These macros can be used in backends and other SANE-related
 * code.
 *
 * Before including sanei_debug.h, the following macros must be set:
 *
 * - BACKEND_NAME - The name of your backend without double-quotes (must be set in any case)
 * - STUBS - If this is defined, no macros will be included. Used in 
 *   backends consisting of more than one .c file.
 * - DEBUG_DECLARE_ONLY - Generates prototypes instead of functions. Used in 
 *   backends consisting of more than one .c file.
 * - DEBUG_NOT_STATIC - Doesn't generate static functions. Used in header files if
 *   they are include in more than one .c file.
 *
 * @{ 
 */

/** @def DBG_INIT()
 * Initialize sanei_debug.
 *
 * Call this function before you use any DBG function.
 */

/** @def DBG(level, fmt, ...)
 * Print a message at debug level `level' or higher using a printf-like
 * function. Example: DBG(1, "sane_open: opening fd \%d\\n", fd).
 *
 * @param level debug level
 * @param fmt format (see man 3 printf for details)
 * @param ... additional arguments
 */

/** @def IF_DBG(x)
 * Compile code only if debugging is enabled.
 *
 * Expands to x if debug support is enabled at compile-time. If NDEBUG is 
 * defined at compile-time this macro expands to nothing.
 *
 * @param x code to expand when debugging is enabled
 */

/**
 * @def DBG_LEVEL
 * Current debug level. 
 *
 * You can only read this "variable".
 */

/** @def ENTRY(name)
 * Expands to sane_BACKEND_NAME_name.
 *
 * Example: ENTRY(init) in mustek.c will expand to sane_mustek_init.
 */

/* @} */

/** @name Internal macros and functions
 * Do not use in your own code.
 * @{
 */

/** @def DBG_LOCAL
 * Do not use in backends directly.
 *
 * Internal wrapper for printing function.
 */

/** @fn extern void sanei_init_debug (const char * backend, int * debug_level_var);
 * Do not use in backends directly.
 *
 * Actual init function.
 */

/** @fn extern void sanei_debug_msg (int level, int max_level, const char *be, const char *fmt, va_list ap);
 * Do not use in backends directly.
 *
 * Actual printing function.
 */
/* @} */

                                  /** @hideinitializer*/
#define ENTRY(name)     PASTE(PASTE(PASTE(sane_,BACKEND_NAME),_),name)

#ifdef NDEBUG
  
extern void sanei_debug_ndebug (int level, const char *msg, ...);
	
# define DBG_LEVEL	(0)
# define DBG_INIT()
# define DBG		sanei_debug_ndebug
# define IF_DBG(x)
	
#else /* !NDEBUG */
	
                                  /** @hideinitializer*/
# define DBG_LEVEL      PASTE(sanei_debug_,BACKEND_NAME)

# if defined(BACKEND_NAME) && !defined(STUBS)
#  ifdef DEBUG_DECLARE_ONLY
extern int DBG_LEVEL;
#  else /* !DEBUG_DECLARE_ONLY */
int DBG_LEVEL = 0;
#  endif /* DEBUG_DECLARE_ONLY */
# endif /* BACKEND_NAME && !STUBS */

                                  /** @hideinitializer*/
# define DBG_INIT()                                     \
  sanei_init_debug (STRINGIFY(BACKEND_NAME), &DBG_LEVEL)

                                  /** @hideinitializer*/
# define DBG_LOCAL	PASTE(DBG_LEVEL,_call)


# ifndef STUBS

#  ifdef DEBUG_DECLARE_ONLY

extern void DBG_LOCAL (int level, const char *msg, ...) 
#ifdef __GNUC__
__attribute__ ((format (printf, 2, 3)))
#endif
;

#  else /* !DEBUG_DECLARE_ONLY */

#   include <stdarg.h>
	
extern void sanei_debug_msg 
  (int level, int max_level, const char *be, const char *fmt, va_list ap);

#ifdef __GNUC__
#   ifndef DEBUG_NOT_STATIC
static
#   endif /* !DEBUG_NOT_STATIC */
void DBG_LOCAL (int level, const char *msg, ...) __attribute__ ((format (printf, 2, 3)));
#endif /* __GNUC__ */

#   ifndef DEBUG_NOT_STATIC
static
#   endif /* !DEBUG_NOT_STATIC */
void
DBG_LOCAL (int level, const char *msg, ...)
{
  va_list ap;

  va_start (ap, msg);
  sanei_debug_msg (level, DBG_LEVEL, STRINGIFY(BACKEND_NAME), msg, ap);
  va_end (ap);
}

#  endif /* DEBUG_DECLARE_ONLY */

# endif /* !STUBS */

                                  /** @hideinitializer*/
# define DBG            DBG_LOCAL

extern void sanei_init_debug (const char * backend, int * debug_level_var);
  
                                  /** @hideinitializer*/
# define IF_DBG(x)      x

#endif /* NDEBUG */

#endif /* _SANEI_DEBUG_H */
