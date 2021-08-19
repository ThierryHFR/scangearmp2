#ifndef __CANON_PIXMA_H__
#define __CANON_PIXMA_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifndef _HAVE_SANE
#define _HAVE_SANE
#endif

#include "../errors.h"
#include "errordlg.h"
#include "../../usr/include/cnmstype.h"
#include "../cnmsfunc.h"
#include "../support.h"
#include "../canon_mfp_tools.h"
#include "../keep_setting.h"
#include "../file_control.h"

#ifndef BACKEND_NAME
#define BACKEND_NAME canon_pixma
#endif

#define DEBUG_NOT_STATIC
#include "sanei_debug.h"

#ifndef DBG_LEVEL
#define DBG_LEVEL       PASTE(sanei_debug_, BACKEND_NAME)
#endif
#ifndef NDEBUG
# define DBGDUMP(level, buf, size) \
    do { if (DBG_LEVEL >= (level)) sanei_canon_pixma_dbgdump(buf, size); } while (0)
#else
# define DBGDUMP(level, buf, size)
#endif


enum canon_sane_Option
{
	OPT_NUM_OPTS = 0,
	OPT_MODE_GROUP,
	OPT_MODE,
	OPT_RESOLUTION,
	OPT_PREVIEW,
	OPT_SCAN_SOURCE,

	OPT_GEOMETRY_GROUP,
	OPT_TL_X,
	OPT_TL_Y,
	OPT_BR_X,
	OPT_BR_Y,
	/* must come last: */
	NUM_OPTIONS
};

#endif // __CANON_PIXMA_H__
