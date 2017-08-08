#ifndef __CANON_PIXMA_H__
#define __CANON_PIXMA_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "errors.h"
#include "errordlg.h"
#include "cnmstype.h"
#include "cnmsfunc.h"
#include "support.h"
#include "canon_mfp_tools.h"
#include "keep_setting.h"
#include "file_control.h"
//#include <glib.h>
//#include "usr/include/glib-2.0/glib.h"

#ifndef BACKEND_NAME
#define BACKEND_NAME canon_pixma
#endif

enum canon_sane_Option
{
	OPT_NUM_OPTS = 0,
	OPT_MODE_GROUP,
	OPT_MODE,
	OPT_RESOLUTION,
	OPT_PREVIEW,

	OPT_GEOMETRY_GROUP,
	OPT_TL_X,
	OPT_TL_Y,
	OPT_BR_X,
	OPT_BR_Y,
	/* must come last: */
	NUM_OPTIONS
};
  
#endif // __CANON_PIXMA_H__
