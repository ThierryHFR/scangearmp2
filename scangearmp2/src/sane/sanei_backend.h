/** @file sanei_backend.h
 * Compatibility header file for backends
 *
 * This file provides some defines for macros missing on some platforms.
 * It also has the SANE API entry points. sanei_backend.h must be included
 * by every backend.
 *
 * @sa sanei.h sanei_thread.h
 */


/** @name Compatibility macros
 * @{
 */
#include "sanei_debug.h"

#if __STDC_VERSION__ >= 199901L
/* __func__ is provided */
#elif __GNUC__ >= 5
/* __func__ is provided */
#elif __GNUC__ >= 2
# define __func__ __FUNCTION__
#else
# define __func__ "(unknown)"
#endif

#ifdef HAVE_SYS_HW_H
  /* OS/2 i/o-port access compatibility macros: */
# define inb(p)         _inp8 (p)
# define outb(v,p)      _outp8 ((p),(v))
# define ioperm(b,l,o)  _portaccess ((b),(b)+(l)-1)
# define HAVE_IOPERM    1
#endif

#ifndef HAVE_OS2_H
#include <fcntl.h>
#ifndef O_NONBLOCK
# ifdef O_NDELAY
#  define O_NONBLOCK O_NDELAY
# else
#  ifdef FNDELAY
#   define O_NONBLOCK FNDELAY    /* last resort */
#  endif
# endif
#endif
#endif /* HAVE_OS2_H */

#include <limits.h>
#ifndef PATH_MAX
# define PATH_MAX 1024
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef MM_PER_INCH
#define MM_PER_INCH 25.4
#endif

#ifdef HAVE_SIGPROCMASK
# define SIGACTION      sigaction
#else

/* Just enough backwards compatibility that we get by in the backends
   without making handstands.  */
# ifdef sigset_t
#  undef sigset_t
# endif
# ifdef sigemptyset
#  undef sigemptyset
# endif
# ifdef sigfillset
#  undef sigfillset
# endif
# ifdef sigaddset
#  undef sigaddset
# endif
# ifdef sigdelset
#  undef sigdelset
# endif
# ifdef sigprocmask
#  undef sigprocmask
# endif
# ifdef SIG_BLOCK
#  undef SIG_BLOCK
# endif
# ifdef SIG_UNBLOCK
#  undef SIG_UNBLOCK
# endif
# ifdef SIG_SETMASK
#  undef SIG_SETMASK
# endif

# define sigset_t               int
# define sigemptyset(set)       do { *(set) = 0; } while (0)
# define sigfillset(set)        do { *(set) = ~0; } while (0)
# define sigaddset(set,signal)  do { *(set) |= sigmask (signal); } while (0)
# define sigdelset(set,signal)  do { *(set) &= ~sigmask (signal); } while (0)
# define sigaction(sig,new,old) sigvec (sig,new,old)

  /* Note: it's not safe to just declare our own "struct sigaction" since
     some systems (e.g., some versions of OpenStep) declare that structure,
     but do not implement sigprocmask().  Hard to believe, aint it?  */
# define SIGACTION              sigvec
# define SIG_BLOCK      1
# define SIG_UNBLOCK    2
# define SIG_SETMASK    3
#endif /* !HAVE_SIGPROCMASK */
/* @} */


/** @name Declaration of entry points:
 * @{
 */
extern SANE_Status ENTRY(init) (SANE_Int *, SANE_Auth_Callback);
extern SANE_Status ENTRY(get_devices) (const SANE_Device ***, SANE_Bool);
extern SANE_Status ENTRY(open) (SANE_String_Const, SANE_Handle *);
extern const SANE_Option_Descriptor *
  ENTRY(get_option_descriptor) (SANE_Handle, SANE_Int);
extern SANE_Status ENTRY(control_option) (SANE_Handle, SANE_Int, SANE_Action,
                                          void *, SANE_Word *);
extern SANE_Status ENTRY(get_parameters) (SANE_Handle, SANE_Parameters *);
extern SANE_Status ENTRY(start) (SANE_Handle);
extern SANE_Status ENTRY(read) (SANE_Handle, SANE_Byte *, SANE_Int,
                                SANE_Int *);
extern SANE_Status ENTRY(set_io_mode) (SANE_Handle, SANE_Bool);
extern SANE_Status ENTRY(get_select_fd) (SANE_Handle, SANE_Int *);
extern void ENTRY(cancel) (SANE_Handle);
extern void ENTRY(close) (SANE_Handle);
extern void ENTRY(exit) (void);

#ifndef STUBS
/* Now redirect sane_* calls to backend's functions: */

#define sane_init(a,b)                  ENTRY(init) (a,b)
#define sane_get_devices(a,b)           ENTRY(get_devices) (a,b)
#define sane_open(a,b)                  ENTRY(open) (a,b)
#define sane_get_option_descriptor(a,b) ENTRY(get_option_descriptor) (a,b)
#define sane_control_option(a,b,c,d,e)  ENTRY(control_option) (a,b,c,d,e)
#define sane_get_parameters(a,b)        ENTRY(get_parameters) (a,b)
#define sane_start(a)                   ENTRY(start) (a)
#define sane_read(a,b,c,d)              ENTRY(read) (a,b,c,d)
#define sane_set_io_mode(a,b)           ENTRY(set_io_mode) (a,b)
#define sane_get_select_fd(a,b)         ENTRY(get_select_fd) (a,b)
#define sane_cancel(a)                  ENTRY(cancel) (a)
#define sane_close(a)                   ENTRY(close) (a)
#define sane_exit(a)                    ENTRY(exit) (a)
#endif /* STUBS */
/* @} */

/** Internationalization for SANE backends
 *
 * Add SANE_I18N() to all texts that can be translated.
 * E.g. out_txt = SANE_I18N("Hello");
 */
#ifndef SANE_I18N
#define SANE_I18N(text) text
#endif

/** Option_Value union
 *
 * Convenience union to access option values given to the backend
 */
#ifndef SANE_OPTION
typedef union
{
  SANE_Bool b;		/**< bool */
  SANE_Word w;		/**< word */
  SANE_Word *wa;	/**< word array */
  SANE_String s;	/**< string */
}
Option_Value;
#define SANE_OPTION 1
#endif
