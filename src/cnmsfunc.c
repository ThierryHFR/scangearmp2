/*
 *  ScanGear MP for Linux
 *  Copyright CANON INC. 2007-2020
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

#ifndef	_CNMSFUNC_C_
#define	_CNMSFUNC_C_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "support.h"

#include "../usr/include/cnmstype.h"

#include "cnmsfunc.h"

/*	#define	__CNMS_DEBUG_FUNC__	*/

CNMSInt32 CnmsStrLen(
		CNMSLPSTR		lpStr )
{
	CNMSInt32	ret = CNMS_ERR;

	if( lpStr == CNMSNULL ){
		DBGMSG( "[CnmsStrLen]Parameter is error.\n" );
		goto	EXIT;
	}
	ret = (CNMSInt32)strlen( (char*)lpStr );
EXIT:
#ifdef	__CNMS_DEBUG_FUNC__
	DBGMSG( "[CnmsStrLen(lpStr:%s)]=%d.\n", lpStr, ret );
#endif
	return	ret;
}

CNMSInt32 CnmsStrCopy(
		CNMSLPSTR		lpSrc,
		CNMSLPSTR		lpDst,
		CNMSInt32		dstLen )
{
	CNMSInt32	ret = CNMS_ERR, srcLen;
	
	if( ( lpSrc == CNMSNULL ) || ( lpDst == CNMSNULL ) || ( dstLen <= 0 ) ){
		DBGMSG( "[CnmsStrCopy]Parameter is error.\n" );
		goto	EXIT;
	}

	if( ( srcLen  = CnmsStrLen( (CNMSInt8 *)lpSrc ) ) >= dstLen ){
		DBGMSG( "[CnmsStrCopy]src string(%d) is too long(>%d).\n", srcLen, dstLen );
		goto	EXIT;
	}
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wstringop-truncation"
//#pragma GCC diagnostic ignored "-Wstringop-overflow"
	snprintf((char *)lpDst, (int)srcLen, "%s", (char *)lpSrc);
//#pragma GCC diagnostic pop

	lpDst[ srcLen ] = '\0';

	ret = srcLen;
EXIT:
#ifdef	__CNMS_DEBUG_FUNC__
	DBGMSG( "[CnmsStrCopy(lpSrc:%s,lpDst:%s,dstLen:%d)]=%d.\n", lpSrc, lpDst, dstLen, ret );
#endif
	return	ret;
}

CNMSInt32 CnmsStrCat(
		CNMSLPSTR		lpSrc,
		CNMSLPSTR		lpDst,
		CNMSInt32		dstLen )
{
	CNMSInt32	ret = CNMS_ERR, totalLen, srcLen;
	
	if( ( lpSrc == CNMSNULL ) || ( lpDst == CNMSNULL ) || ( dstLen <= 0 ) ){
		DBGMSG( "[CnmsStrCat]Parameter is error.\n" );
		goto	EXIT;
	}

	srcLen = CnmsStrLen( (CNMSInt8 *)lpSrc );
	if( ( totalLen  = srcLen + CnmsStrLen( (CNMSInt8 *)lpDst ) ) >= dstLen ){
		DBGMSG( "[CnmsStrCat]total string(%d) is too long(>%d).\n", totalLen, dstLen );
		goto	EXIT;
	}

//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wstringop-truncation"
//#pragma GCC diagnostic ignored "-Wstringop-overflow"
	(void)strncat( (char *)lpDst, (char *)lpSrc, (int)srcLen );
//#pragma GCC diagnostic pop
	lpDst[ totalLen ] = '\0';

	ret = totalLen;
EXIT:
#ifdef	__CNMS_DEBUG_FUNC__
	DBGMSG( "[CnmsStrCat(lpSrc:%s,lpDst:%s,dstLen:%d)]=%d.\n", lpSrc, lpDst, dstLen, ret );
#endif
	return	ret;
}

CNMSInt32 CnmsStrCompare(
		const CNMSLPSTR		lpStr1,
		const CNMSLPSTR		lpStr2 )
{
	CNMSInt32	ret = CNMS_ERR;
	
	if( ( lpStr1 == CNMSNULL ) || ( lpStr2 == CNMSNULL ) ){
		DBGMSG( "[CnmsStrCompare]Parameter is error.\n" );
		goto	EXIT;
	}
	else if( strcmp( lpStr1, lpStr2 ) != 0 ){
/*		DBGMSG( "[CnmsStrCompare]Error is occured in strcmp.\n" );	*/
		goto	EXIT;
	}
	ret = CNMS_NO_ERR;
EXIT:
#ifdef	__CNMS_DEBUG_FUNC__
	DBGMSG( "[CnmsStrCompare(lpStr1:%s,lpStr2:%s)]=%d.\n", lpStr1, lpStr2, ret );
#endif
	return	ret;
}


CNMSLPSTR CnmsGetMem(
		CNMSInt32		size )
{
	CNMSLPSTR	ptr = CNMSNULL;

	if( ( ptr = (CNMSLPSTR)malloc( size ) ) == CNMSNULL ){
		DBGMSG( "[CnmsGetMem]Error is occured in malloc.\n" );
		goto	EXIT;
	}
	CnmsSetMem( ptr, 0, size );
EXIT:
#ifdef	__CNMS_DEBUG_FUNC__
	DBGMSG( "[CnmsGetMem(size:%d)]=%d.\n", size, ptr );
#endif
	return	ptr;
}

CNMSVoid  CnmsFreeMem(
		CNMSLPSTR		ptr )
{
	if( ptr != CNMSNULL ){
		free( ptr );
	}
#ifdef	__CNMS_DEBUG_FUNC__
	DBGMSG( "[CnmsFreeMem(ptr:%d)].\n", ptr );
#endif
	return;
}

CNMSVoid CnmsCopyMem(
		CNMSLPSTR		lpSrc,
		CNMSLPSTR		lpDst,
		CNMSInt32		size )
{
	if( ( lpSrc == CNMSNULL ) || ( lpDst == CNMSNULL ) || ( size <= 0 ) ){
		DBGMSG( "[CnmsCopyMem]Parameter is error.\n" );
		goto	EXIT;
	}
	memcpy( lpDst, lpSrc, size );
EXIT:
#ifdef	__CNMS_DEBUG_FUNC__
	DBGMSG( "[CnmsCopyMem(lpSrc:%d,lpDst:%d,size:%d)].\n", lpSrc, lpDst, size );
#endif
	return;
}

CNMSVoid CnmsMoveMem(
		CNMSLPSTR		lpSrc,
		CNMSLPSTR		lpDst,
		CNMSInt32		size )
{
	/* white src by NULL after copy */
	if( ( lpSrc == CNMSNULL ) || ( lpDst == CNMSNULL ) || ( size <= 0 ) ){
		DBGMSG( "[CnmsMoveMem]Parameter is error.\n" );
		goto	EXIT;
	}
	CnmsCopyMem( lpSrc, lpDst, size );
	CnmsSetMem( lpSrc, 0, size );
EXIT:
#ifdef	__CNMS_DEBUG_FUNC__
	DBGMSG( "[CnmsMoveMem(lpSrc:%d,lpDst:%d,size:%d)].\n", lpSrc, lpDst, size );
#endif
	return;
}

CNMSVoid CnmsSetMem(
		CNMSLPSTR		lpDst,
		CNMSByte		val,
		CNMSInt32		size )
{
	if( ( lpDst == CNMSNULL ) || ( size <= 0 ) ){
		DBGMSG( "[CnmsSetMem]Parameter is error.\n" );
		goto	EXIT;
	}
	memset( lpDst, val, size );
EXIT:
#ifdef	__CNMS_DEBUG_FUNC__
	DBGMSG( "[CnmsSetMem(lpDst:%d,val:%d,size:%d)].\n", lpDst, val, size );
#endif
	return;
}

LPCNMS_ROOT CnmsCreateRoot( CNMSVoid )
{
	LPCNMS_ROOT root = CNMSNULL;
	
	if( ( root = (LPCNMS_ROOT)CnmsGetMem( sizeof( CNMS_ROOT ) ) ) == CNMSNULL ) {
		goto EXIT;
	}
	root->page_num = 0;
	root->head = CNMSNULL;
	root->tail = CNMSNULL;
EXIT:
	return root;
}

CNMSInt32 CnmsDisposeRoot( LPCNMS_ROOT *root )
{
	if( root == CNMSNULL ) {
		return -1;
	}
	else if( *root == CNMSNULL ) {
		return -1;
	}
	else if( (*root)->tail != CNMSNULL  ) {
		return 1;
	}

	CnmsFreeMem( (CNMSLPSTR)(*root) );
	*root = CNMSNULL;
	return 0;
}

LPCNMS_NODE CnmsNewNode( CNMSByte *file_path )
{
	LPCNMS_NODE	node = CNMSNULL;

	if( file_path == CNMSNULL) {
		goto EXIT;
	}

	if( ( node = (LPCNMS_NODE)CnmsGetMem( sizeof( CNMS_NODE ) ) ) == CNMSNULL ) {
		goto EXIT;
	}
	if( CnmsStrCopy( file_path, node->file_path, sizeof( node->file_path ) ) == CNMS_ERR ) {
		CnmsFreeMem( (CNMSLPSTR)node );
		goto EXIT;
	}
	node->page = -1;
	node->show_page = CNMS_FALSE;
	node->rotate = CNMS_FALSE;
	node->prev = CNMSNULL;
	node->next = CNMSNULL;
	node->fd   = CNMS_FILE_ERR;
	node->file_size = 0;

EXIT:
	return node;
}

CNMSVoid CnmsDisposeNode( LPCNMS_NODE *pnode )
{
	LPCNMS_NODE	node = CNMSNULL;
	
	if( pnode == CNMSNULL ) {
		return;
	}
	node = *pnode;
	
	if( node != CNMSNULL ) {
		CnmsFreeMem( (CNMSLPSTR)node );
		*pnode = CNMSNULL;
	}
}

LPCNMS_NODE CnmsPutQueue( LPCNMS_ROOT root, LPCNMS_NODE node )
{
	LPCNMS_NODE	ret = CNMSNULL;
	
	if( root == CNMSNULL || node == CNMSNULL ) {
		goto EXIT;
	}
	
	if( root->head == CNMSNULL ) {
		root->head = node; /* first node */
	}
	if( root->tail != CNMSNULL ) {
		node->prev = root->tail;
		root->tail->next = node;
	}
	root->tail = node;
	root->page_num++;
	node->page = root->page_num;

	ret = node;

EXIT:
	return ret;
}

CNMSInt32 CnmsDisposeQueue( LPCNMS_ROOT root, CNMSInt32 type )
{
	CNMSInt32	ret = CNMS_ERR;
	LPCNMS_NODE	node = CNMSNULL;
	
	if( root == CNMSNULL ){
		goto EXIT;
	}
	else if ( root->tail == CNMSNULL || root->head == CNMSNULL ) {
		goto EXIT; /* no node */
	}
	
	switch( type )
	{
		case CNMS_NODE_TAIL :
			/* remove node. */
			node = root->tail;
			root->tail = node->prev;
			
			if( node->prev == CNMSNULL)
				root->head = CNMSNULL; /* removed last node. */
			else
				node->prev->next = CNMSNULL;

			break;
			
		case CNMS_NODE_HEAD :
			/* remove node. */
			node = root->head;
			root->head = node->next;
			
			if( node->next == CNMSNULL)
				root->tail = CNMSNULL; /* removed last node. */
			else
				node->next->prev = CNMSNULL;

			break;
			
		default :
			goto EXIT;
	}

	CnmsFreeMem( (CNMSLPSTR)node );
	ret = CNMS_NO_ERR;
	
EXIT:
	return ret;
}


#endif	/* _CNMSFUNC_C_ */
