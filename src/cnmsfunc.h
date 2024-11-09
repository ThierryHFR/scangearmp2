/*
 *  ScanGear MP for Linux
 *  Copyright CANON INC. 2007-2024
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * NOTE:
 *  - As a special exception, this program is permissible to link with the
 *    libraries released as the binary modules.
 *  - If you write modifications of your own for these programs, it is your
 *    choice whether to permit this exception to apply to your modifications.
 *    If you do not wish that, delete this exception.
*/

#ifndef	_CNMSFUNC_H_
#define	_CNMSFUNC_H_

#include <limits.h>
#include "../usr/include/cnmstype.h"

#define	CNMS_SCALE_100_PERCENT	(10000)

#define DEFTOSTR1(def) #def
#define DEFTOSTR(def) DEFTOSTR1(def)

enum{
	CNMS_FIT_INSIDE = 0,
	CNMS_FIT_OUTSIDE,
	CNMS_FIT_MAX,
};

enum{
	CNMS_ROUND_CUT = 0,	/* cut under decFig */
	CNMS_ROUND_UP_DOWN,	/* round up over 5 and down under 5 */
	CNMS_ROUND_MAX,
};

CNMSInt32 CnmsStrLen( CNMSLPSTR lpStr );
CNMSInt32 CnmsStrCopy( CNMSLPSTR lpSrc, CNMSLPSTR lpDst, CNMSInt32 dstLen );
CNMSInt32 CnmsStrCat( CNMSLPSTR lpSrc, CNMSLPSTR lpDst, CNMSInt32 dstLen );
CNMSInt32 CnmsStrCompare( const CNMSLPSTR lpStr1, const CNMSLPSTR lpStr2 );

CNMSLPSTR CnmsGetMem( CNMSInt32 size );
CNMSVoid  CnmsFreeMem( CNMSLPSTR ptr );
CNMSVoid  CnmsCopyMem( CNMSLPSTR lpSrc, CNMSLPSTR lpDst, CNMSInt32 size );
CNMSVoid  CnmsMoveMem( CNMSLPSTR lpSrc, CNMSLPSTR lpDst, CNMSInt32 size );	/* white src by NULL after copy */
CNMSVoid  CnmsSetMem( CNMSLPSTR lpDst, CNMSByte val, CNMSInt32 size );


/*----- file list ----*/
enum {
	CNMS_NODE_TAIL = 0,
	CNMS_NODE_HEAD,
	CNMS_NODE_MAX,
};

typedef struct mynode
{
	CNMSInt32		page;
	CNMSBool		show_page;
	CNMSBool		rotate;
	struct mynode	*prev;
	struct mynode	*next;
	CNMSFd			fd;
	CNMSInt32		file_size;
	CNMSByte		file_path[ PATH_MAX ];
} CNMS_NODE, *LPCNMS_NODE;

typedef struct
{
	CNMSInt32		page_num;

	LPCNMS_NODE		head;
	LPCNMS_NODE		tail;
} CNMS_ROOT, *LPCNMS_ROOT;

#define	CnmsExistQueue(x)	( (x)->head != CNMSNULL )

LPCNMS_ROOT CnmsCreateRoot( CNMSVoid );
CNMSInt32 CnmsDisposeRoot( LPCNMS_ROOT *root );

LPCNMS_NODE CnmsNewNode( CNMSByte *file_path );
CNMSVoid CnmsDisposeNode( LPCNMS_NODE *pnode );
LPCNMS_NODE CnmsPutQueue( LPCNMS_ROOT root, LPCNMS_NODE node );
CNMSInt32 CnmsDisposeQueue( LPCNMS_ROOT root, CNMSInt32 type );

#endif	/* _CNMSFUNC_H_ */
