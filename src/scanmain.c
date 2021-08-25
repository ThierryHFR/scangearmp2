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

#ifndef	_SCANMAIN_C_
#define	_SCANMAIN_C_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "support.h"
#include "callbacks.h"

#include "errors.h"

#include "../usr/include/cnmstype.h"
#include "canon_mfp_tools.h"
#include "errordlg.h"
#include "cnmsfunc.h"
#include "file_control.h"
#include "jpeg2pdf.h"

#include "progressbar.h"

enum{
	CIJSC_SCANMAIN_GO_NEXT = 0,
	CIJSC_SCANMAIN_CHECK_ERROR_VALUE,
	CIJSC_SCANMAIN_ERROR,
};

enum{
	CIJSC_SCANMAIN_SCAN_FINISHED = 0,
	CIJSC_SCANMAIN_SCAN_CANCELED,
	CIJSC_SCANMAIN_SCAN_ERROR,
};

#define	JPEGSCANBUFSIZE	(0x4000)	/* 16k */

typedef struct {
	int			id;
	int			right;
	int			bottom;
} CIJSC_SIZE_TABLE;

/* for 300dpi */
static const CIJSC_SIZE_TABLE sourceSize[] = {
	{ CIJSC_SIZE_CARD,		1074,  649 },		// Card
	{ CIJSC_SIZE_L_L,		1500, 1051 },		// L Landscape
	{ CIJSC_SIZE_L_P,		1051, 1500 },		// L Portrait
	{ CIJSC_SIZE_4X6_L,		1800, 1200 },		// 4"x6" Landscape
	{ CIJSC_SIZE_4X6_P,		1200, 1800 },		// 4"x6" Portrait
	{ CIJSC_SIZE_HAGAKI_L,	1748, 1181 },		// Hagaki Landscape
	{ CIJSC_SIZE_HAGAKI_P,	1181, 1748 },		// Hagaki Portrait
	{ CIJSC_SIZE_2L_L,		2102, 1500 },		// 2L Landscape
	{ CIJSC_SIZE_2L_P,		1500, 2102 },		// 2L Portrait
	{ CIJSC_SIZE_A5,		1748, 2480 },		// A5
	{ CIJSC_SIZE_B5,		2149, 3035 },		// B5
	{ CIJSC_SIZE_A4,		2480, 3507 },		// A4
	{ CIJSC_SIZE_LETTER,	2550, 3300 }		// Letter
};

static void ui_dialog_save_gtk_main_iteration(void)
{
	while( gtk_events_pending() ){
		gtk_main_iteration();
	}
}

/*
static void wait_msec(long msec)
{
	struct	timespec mytime;
	CnmsSetMem( (CNMSLPSTR)&mytime, 0, sizeof(mytime) );
	mytime.tv_sec   = 0;
	mytime.tv_nsec  = 1000000*msec;	// nsec = 1/1000000 msec
	
	nanosleep( &mytime, NULL );
}
*/

static gboolean ui_dialog_notify_hide( gpointer gdata )
{
	SGMP_Data	*data = (SGMP_Data *)gdata;
	
	DBGMSG("->\n");
	
	if ( gtk_widget_get_visible( data->dialog_notify ) ) {
		DBGMSG("hide dialog\n");
		gtk_widget_hide( data->dialog_notify );
	}
	else {
		DBGMSG("do nothing\n");
	}
	return FALSE;
}

static int ui_dialog_save_append_ext( SGMP_Data *data )
{
	char	*ext[2] = { ".jpg", ".pdf" };
	int		ret = CNMS_ERR, ldata;
	int		len;
	
	len = strlen( data->file_path );
	DBGMSG("original data->file_path(%d) = %s\n", len, data->file_path );
	DBGMSG("format = %s\n", ext[ data->scan_format ]);
	
	/* check file-ext. */
	if ( len >= 4 ) {
		if ( data->file_path[len-4] == ext[ data->scan_format ][0] &&
			 data->file_path[len-3] == ext[ data->scan_format ][1] &&
			 data->file_path[len-2] == ext[ data->scan_format ][2] &&
			 data->file_path[len-1] == ext[ data->scan_format ][3] ) {
			 
			 DBGMSG("do nothing.\n");
			 goto _EXIT_NOERR;
		}
	}
	/* append file-ext. */
	if( len < PATH_MAX - 5 ) {
		if( ( ldata = CnmsStrCat( ext[ data->scan_format ], data->file_path, PATH_MAX ) ) < 0 ){
			DBGMSG( "Can't add file ext string.\n" );
			goto _EXIT;
		}
	}
	else {
		DBGMSG( "file name too long.\n" );
		goto _EXIT;
	}
	
_EXIT_NOERR:
	ret = CNMS_NO_ERR;
	DBGMSG("data->file_path = %s\n", data->file_path );

_EXIT:
	return	ret;
}

static int ui_dialog_save_check_status( SGMP_Data *data )
{
	int		ret = CIJSC_SCANMAIN_ERROR;
	
	switch( FileControlGetStatus( data->file_path, PATH_MAX ) ){
		case	FILECONTROL_STATUS_NOT_EXIST:		/* No Error Save */
			ret = CIJSC_SCANMAIN_GO_NEXT;
			break;
		case	FILECONTROL_STATUS_WRITE_OK:		/* Overwrite Save */
			lastBackendErrCode = BERRCODE_SAVE_OVERWRITE;
			ret = CIJSC_SCANMAIN_CHECK_ERROR_VALUE;
			break;
		case	FILECONTROL_STATUS_WRITE_NG:		/* No Permission */
			lastBackendErrCode = BERRCODE_SAVE_NO_ACCESS;
			break;
		case	FILECONTROL_STATUS_ISNOT_FILE:		/* Not file */
			lastBackendErrCode = BERRCODE_SAVE_OTHER;
			break;
		case	FILECONTROL_STATUS_INVALID_DIR:		/* Invalid directory */
			lastBackendErrCode = BERRCODE_SAVE_INVALID_DIR;
			break;
		case	FILECONTROL_STATUS_OTHER_ERROR:		/* Other error */
			lastBackendErrCode = BERRCODE_SAVE_OTHER;
			break;
		default:	/* May not use */
			lastBackendErrCode = BERRCODE_SAVE_OTHER;
			break;
	}
	return ret;
}

static int ui_dialog_save_scan_create_file( SGMP_Data *data, LPCNMS_NODE *pnode )
{
	CNMSInt32		ret = CNMS_ERR;
	CNMSFd			fd = CNMS_FILE_ERR;
	CNMSByte		file_path[ PATH_MAX ];
	
	if( pnode == CNMSNULL ) {
		DBGMSG( "[CnmsScanFlowMakeDstFile]pnode is CNMSNULL.\n" );
		goto	EXIT;
	}
	
	CnmsSetMem( file_path, 0, sizeof( file_path ) );
	
	if( data->scan_format == CIJSC_FORMAT_JPEG ) {
		if( ( fd = FileControlOpenFile( FILECONTROL_OPEN_TYPE_NEW, data->file_path ) ) == CNMS_FILE_ERR ){
			DBGMSG( "Error is occured in FileControlOpenFile.\n" );
			goto	EXIT;
		}
		if( ( *pnode = CnmsNewNode( data->file_path ) ) == CNMSNULL ) {
			DBGMSG( "Error is occured in CnmsNewNode.\n" );
			goto 	EXIT;
		}
	}
	else {
		if( ( fd = FileControlMakeTempFile( file_path, PATH_MAX ) ) == CNMS_FILE_ERR ){
			DBGMSG( "Error is occured in FileControlMakeTempFile.\n" );
			goto	EXIT;
		}
		if( ( *pnode = CnmsNewNode( file_path ) ) == CNMSNULL ) {
			DBGMSG( "Error is occured in CnmsNewNode.\n" );
			goto 	EXIT;
		}
	}
	(*pnode)->fd = fd;
	DBGMSG( " created file [%s]\n", (*pnode)->file_path );
	
	ret = CNMS_NO_ERR;

EXIT:
	return ret;
}

static void ui_dialog_save_scan_dispose_file( SGMP_Data *data, LPCNMS_NODE *pnode )
{

	(void)data;
	if( pnode == CNMSNULL ) {
		DBGMSG( "[CnmsScanFlowDisposeDstFile]pnode is CNMSNULL.\n" );
		return ;
	}
	else if ( *pnode != CNMSNULL ) {
		if( (*pnode)->fd != CNMS_FILE_ERR ) {
			DBGMSG( " delete file [%s]\n", (*pnode)->file_path );
			FileControlDeleteFile( (*pnode)->file_path, (*pnode)->fd );
		}
		CnmsDisposeNode( pnode );	/* *pnode -> CNMSNULL */
	}
}

static int ui_dialog_save_scan_add_file_list( SGMP_Data *data, LPCNMS_ROOT root, LPCNMS_NODE *pnode )
{
	CNMSInt32		ret = CNMS_ERR;

	if( pnode == CNMSNULL || root == CNMSNULL ) {
		DBGMSG( "[CnmsScanFlowAddList]parameter is error.\n" );
		goto	EXIT;
	}
	else if ( *pnode == CNMSNULL ) {
		DBGMSG( "[CnmsScanFlowAddList]parameter is error.\n" );
		goto	EXIT;
	}
	
	FileControlCloseFile( (*pnode)->fd );
	(*pnode)->fd = CNMS_FILE_ERR;
	
	(*pnode)->page = data->scanning_page;
	(*pnode)->rotate = ( ( data->scanning_page % 2 ) || data->scan_scanmode != CIJSC_SCANMODE_ADF_D_L ) ? CNMS_FALSE : CNMS_TRUE ;
	CnmsPutQueue( root, *pnode );
	DBGMSG( " <%d> file [%s]\n", (*pnode)->page, (*pnode)->file_path );
	*pnode = CNMSNULL;
	
	ret = CNMS_NO_ERR;

EXIT:
	return ret;
}

static void ui_dialog_save_scan_dispose_file_list( SGMP_Data *data, LPCNMS_ROOT root )
{
	LPCNMS_NODE		node;
	
	while( CnmsDisposeRoot( &root ) > 0 ) {
		node = root->head;
		DBGMSG( "delete file in list [%s] size = %d\n", node->file_path, node->file_size );
		if( data->scan_format == CIJSC_FORMAT_PDF ) {
			FileControlDeleteFile( node->file_path, CNMS_FILE_ERR );
		}
		CnmsDisposeQueue( root, CNMS_NODE_HEAD );
	}
}

static int ui_dialog_save_scan_start( SGMP_Data *data, LPCNMS_ROOT root )
{
	CANON_ScanParam		param;
	// CANON_SCANDATA		scandata;
	int					i;
	// int					ret = -1;
	int					status;
	unsigned char				*buf = NULL;
	int					errCode;
	int					readBytes = 0;
	// int					result = CIJSC_SCANMAIN_SCAN_FINISHED;
	int					pc_canceled = 0, updated_label = 0;
	int					progress_id;
	LPCNMS_NODE			node = CNMSNULL;
	
	memset( &param, 0, sizeof(param) );
	for ( i = 0; i < (int)(sizeof( sourceSize ) / sizeof( CIJSC_SIZE_TABLE )) ; i++ ) {
		if ( sourceSize[i].id == data->scan_size ) {
			break;
		}
	}
	if ( i == (int)( sizeof( sourceSize ) / sizeof( CIJSC_SIZE_TABLE ) ) ) {
		set_module_error();
		goto EXIT_ERR;
	}
        int table_res[] = {75, 150, 300, 600};
        double table_res_fact[] = {4.0, 2.0, 1.0, 0.5};
	param.XRes			= data->scan_res = table_res[data->scan_resolution];
	param.YRes			= data->scan_res; // 300;
	param.Left			= 0;
	param.Top			= 0;
	param.Right			= data->scan_w = (int) ((double)sourceSize[i].right / table_res_fact[data->scan_resolution]);
	param.Bottom		= data->scan_h = (int) ((double)sourceSize[i].bottom / table_res_fact[data->scan_resolution]);
	param.ScanMode		= ( data->scan_color == CIJSC_COLOR_COLOR ) ? 4 : 2;
	param.ScanMethod	= ( data->scan_scanmode == CIJSC_SCANMODE_ADF_D_S ) ? CIJSC_SCANMODE_ADF_D_L : data->scan_scanmode;
	param.opts.p1_0		= 0;
	param.opts.p2_0		= 0;
	param.opts.p3_3		= 3;
	param.opts.DocumentType		= data->scan_source + 1;
	param.opts.p4_0		= 0;
	param.opts.p5_0		= 0;
	param.opts.p6_1		= 1;
	
	progress_id			= ( data->scan_scanmode == CIJSC_SCANMODE_PLATEN ) ? CIJSC_PROGRESS_SCANNING_PLATEN : CIJSC_PROGRESS_SCANNING_ADF;
	
	data->scan_result	= CIJSC_SCANMAIN_SCAN_FINISHED;
	data->scanning_page = 1;
	data->last_error_quit = CIJSC_ERROR_DLG_QUIT_FALSE;
	
	while(1){
		DBGMSG( "scan start(%d) ->\n", data->scanning_page );
		/* create file. */
		if( ui_dialog_save_scan_create_file( data, &node ) != CNMS_NO_ERR ) {
			data->scan_result = CIJSC_SCANMAIN_SCAN_ERROR;
			break;
		}
SCAN_START:
		if( data->scanning_page == 1 ) {
			/* show progress bar */
			CIJSC_UI_progress_disable_cancel_button( data );
			CIJSC_UI_progress_show( data, progress_id );
		}
		/* scan start */
		if( ( status = CIJSC_start( &param ) ) != CMT_STATUS_GOOD ){
			/* ADF : check status. */
			if ( param.ScanMethod != CIJSC_SCANMODE_PLATEN && status == CMT_STATUS_NO_DOCS ) {
				/* no paper */
				if( data->scanning_page == 1 ) {
					CIJSC_UI_progress_hide( data );
					lastBackendErrCode = BERRCODE_ADF_NO_PAPER;
					CIJSC_UI_error_show( data, NULL );
					if ( data->dialog_error_value == CIJSC_VALUE_OK ) {
						/* retry */
						goto SCAN_START;
					}
					else {
						/* scan canceled. */
						pc_canceled = 1;
						/* delete disused file. */
						ui_dialog_save_scan_dispose_file( data, &node );
						goto FINISHED_WITH_NO_DOCS;
					}
				}else {
					/* delete disused file. */
					ui_dialog_save_scan_dispose_file( data, &node );
					goto FINISHED_WITH_NO_DOCS;		/* scan finished. */
				}
			}
			DBGMSG("Error in CIJSC_start \n");
			goto ERROR_BACKEND;
		}
		/* get parameters */
		if( ( status = CIJSC_get_parameters( CIJSC_UI_progress_update_pulse ) ) != CMT_STATUS_GOOD ){
			goto ERROR_BACKEND;
		}
		
		if ( buf ) {
			free( buf );
			buf = NULL;
		}
		buf = (unsigned char *)malloc( JPEGSCANBUFSIZE );
		readBytes = JPEGSCANBUFSIZE;
		
		if( data->scanning_page == 1 ) {
			CIJSC_UI_progress_enable_cancel_button( data );
		}
		i = 0;
		/* read scanned page data. */
		while( 1 ) {
#ifdef _SGMP_DEBUG_VERBOSE_
			DBGMSG("CIJSC_read(%d)->\n",i++);
#endif
			readBytes = JPEGSCANBUFSIZE;
			/* check cancel. */
			if( CIJSC_UI_progress_update_pulse() < 0 ) {
				/* clicked cancel button. */
				pc_canceled = 1;
				data->scan_result = CIJSC_SCANMAIN_SCAN_CANCELED;
				if( !updated_label ) {
					CIJSC_UI_progress_update_label( data );
					updated_label = 1;
				}
			}
			/* read scanned data. */
			if( ( status = CIJSC_read( buf, &readBytes ) ) == CMT_STATUS_EOF ){
				/* no remaining scanned data. */
				DBGMSG( "CIJSC_read -> CMT_STATUS_EOF\n" );
				break;
			}
			if ( status != CMT_STATUS_GOOD ){
				DBGMSG( "Error in CIJSC_read \n" );
				goto ERROR_BACKEND;
			}
			
			if( !pc_canceled ) {
				if( FileControlWriteFile( node->fd, (CNMSLPSTR)buf, readBytes ) != CNMS_NO_ERR ){
					pc_canceled = 1;
					data->scan_result = CIJSC_SCANMAIN_SCAN_ERROR;
				}
				node->file_size += readBytes;
			}
		}
		if( pc_canceled ) {		/* pc canceled or file write error. */
			/* delete disused scanned-file. */
			ui_dialog_save_scan_dispose_file( data, &node );
			break;
		}
		
		/* append scanned-file data. */
		ui_dialog_save_scan_add_file_list( data, root, &node );
		
		data->scanning_page++;
		DBGMSG( "scan end(%d)...\n", data->scanning_page );
		
		if ( param.ScanMethod == CIJSC_SCANMODE_PLATEN ) {
			break;
		}
	
	}/* while(1) */
	
	/* for ADF scan cancel. */
	if( pc_canceled && data->scan_scanmode != CIJSC_SCANMODE_PLATEN ) {
		lastBackendErrCode = BERRCODE_CANCELED_ADF;
	}

FINISHED_WITH_NO_DOCS:	/* ADF scan finished. */
	DBGMSG("CIJSC_cancel->\n");
	CIJSC_cancel();
	/* hide progress bar. */
	CIJSC_UI_progress_hide( data );
	
EXIT_FREE_BUF:
	if ( buf ) {
		free( buf );
		buf = NULL;
	}
	
EXIT_ERR:
	return data->scan_result;

ERROR_BACKEND:
	/* set backend error code. */
	errCode = 0;
	CIJSC_get_backend_error_code( &errCode );
	if( errCode ) {
		DBGMSG("backend errCode = %d\n", errCode );
		lastBackendErrCode = errCode;
		data->scan_result = CIJSC_SCANMAIN_SCAN_ERROR;
	}
	CIJSC_cancel();
	
	/* delete scanned file. */
	ui_dialog_save_scan_dispose_file( data, &node );
	
	/* hide progress bar. */
	CIJSC_UI_progress_hide( data );

	goto EXIT_FREE_BUF;
}

static int ui_dialog_save_create_pdf( SGMP_Data *data, LPCNMS_ROOT root )
{
	CNMSFd			fd = CNMS_FILE_ERR;
	CNMSVoid		*p;
	int				result = CNMS_ERR;
	
	DBGMSG("->\n");
	if ( root->head == CNMSNULL ){
		DBGMSG("no scanned page data. -> do nothing.\n" );
		result = CNMS_NO_ERR_CANCLED;
		goto EXIT_ERR;
	}
	CIJSC_UI_progress_show( data, CIJSC_PROGRESS_SAVING );
	CIJSC_UI_progress_enable_cancel_button( data );
	
	/* create scanned-data file. */
	DBGMSG( "create file [%s].\n", data->file_path );
	if( ( fd = FileControlOpenFile( FILECONTROL_OPEN_TYPE_NEW, data->file_path ) ) == CNMS_FILE_ERR ){
		DBGMSG( "Error is occured in FileControlOpenFile.\n" );
		goto EXIT_ERR;
	}
	
	if ( ( result = CnmsPDF_Open( &p, fd ) ) != CNMS_NO_ERR ) {
		DBGMSG( "CnmsPDF_Open : error\n\n" );
		goto EXIT_ERR;
	}
	if ( ( result = CnmsPDF_StartDoc( p ) ) != CNMS_NO_ERR ) {
		DBGMSG( "CnmsPDF_StartDoc : error\n\n" );
		goto EXIT_CNMS_PDF_CLOSE;
	}
	while( root->head != CNMSNULL ){
		if ( ( result = CnmsPDF_StartPage( p, data->scan_w, data->scan_h, data->scan_res, CNMS_PDF_IMAGE_COLOR, root->head->rotate ) ) != CNMS_NO_ERR ) {
			DBGMSG( "CnmsPDF_StartPage : error\n\n" );
			goto EXIT_CNMS_PDF_CLOSE;
		}
		if ( ( result = CnmsPDF_WriteJpegData( p, root->head, CIJSC_UI_progress_update_value ) ) != CNMS_NO_ERR ) {
			DBGMSG( "CnmsPDF_WriteJpegData : error or canceled.\n\n" );
			goto EXIT_CNMS_PDF_CLOSE;
		}
		if ( ( result = CnmsPDF_EndPage( p ) ) != CNMS_NO_ERR ) {
			DBGMSG( "CnmsPDF_EndPage : error\n\n" );
			goto EXIT_CNMS_PDF_CLOSE;
		}
		/* dispose page data. */
		DBGMSG( "delete file in list [%s]\n", root->head->file_path );
		CnmsDisposeQueue( root, CNMS_NODE_HEAD );
	}
	if ( ( result = CnmsPDF_EndDoc( p ) ) != CNMS_NO_ERR ) {
		DBGMSG( "CnmsPDF_Open : error\n\n" );
		goto EXIT_ERR;
	}
	result = CNMS_NO_ERR;
	
EXIT_CNMS_PDF_CLOSE:
	CnmsPDF_Close( p );
	
EXIT_ERR:
	if ( fd != CNMS_FILE_ERR ) {
		FileControlCloseFile( fd );
	}
	/* error or canceled -> delete scanned-data file. */
	if ( result != CNMS_NO_ERR ) {
		if ( fd != CNMS_FILE_ERR ) {
			FileControlDeleteFile( data->file_path, CNMS_FILE_ERR );
		}
	}
	CIJSC_UI_progress_hide( data );
	
	return result;
}

static void ui_dialog_save_show_notify( SGMP_Data *data )
{
	gtk_widget_show( data->dialog_notify );
	g_timeout_add_seconds( 2, ui_dialog_notify_hide, data );
	gtk_main();
}

static int ui_dialog_save_save_result( SGMP_Data *data, LPCNMS_ROOT root )
{
	int				result;
	
	if( data->scan_format == CIJSC_FORMAT_JPEG ) {
		ui_dialog_save_scan_dispose_file_list( data, root );
		gtk_main_quit();	/* exit loop : CIJSC_Scan_And_Save() */
		if( data->scan_result == CIJSC_SCANMAIN_SCAN_FINISHED ) {
			ui_dialog_save_show_notify( data );
		}
	}
	else {	/* pdf */
		/* jpg -> pdf */
		result = ui_dialog_save_create_pdf( data, root );
		
		ui_dialog_save_scan_dispose_file_list( data, root );
		gtk_main_quit();	/* exit loop : CIJSC_Scan_And_Save() */
		
		if( result == CNMS_NO_ERR ) {
			ui_dialog_save_show_notify( data );
		}
		else if ( result == CNMS_ERR ) {
			CIJSC_UI_error_show( data, NULL );
		}
	}
	return 0;
}

void CIJSC_Scan_And_Save( SGMP_Data	*data )
{
	DBGMSG("->\n");
	/* show save dialog. */
	gtk_widget_grab_focus( data->button_save_save );
	gtk_widget_show( data->dialog_save );
	gtk_main();
	if( data->last_error_quit == CIJSC_ERROR_DLG_QUIT_TRUE ) {
		gtk_main_quit();	/* exit loop : CIJSC_UI_main_show() */
	}
}

void CIJSC_UI_save_button_save_clicked( SGMP_Data *data )
{
	gchar	*filename = NULL;
	int		check_status = -1;
	LPCNMS_ROOT		root = CNMSNULL;
	int		scan_result = -1;

	DBGMSG("->\n");
	memset( data->file_path, 0, sizeof( data->file_path ) );
	/* get file name. */
	filename = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER( data->filechooserwidget_save ) );
	if ( filename ) {
		DBGMSG( " filename = %s\n", filename );
		CnmsStrCopy( filename, data->file_path, sizeof( data->file_path ) );
		DBGMSG( "  data->file_path = %s\n",  data->file_path );
		g_free( filename );
	}
	else {
		DBGMSG( " filename = NULL\n" );
		lastBackendErrCode = BERRCODE_SAVE_NO_FILE_NAME;
		CIJSC_UI_error_show( data, data->dialog_save );
		return;	/* cancel saving. */
	}
	/* append file-ext. */
	if ( ui_dialog_save_append_ext( data ) != CNMS_NO_ERR ) {
		lastBackendErrCode = BERRCODE_SAVE_OTHER;
		CIJSC_UI_error_show( data, data->dialog_save );
		return;	/* cancel saving. */
	}
	/* check status of file_path. */
	if ( ( check_status = ui_dialog_save_check_status( data ) ) != CIJSC_SCANMAIN_GO_NEXT ) {
		CIJSC_UI_error_show( data, data->dialog_save );
		
		if ( check_status == CIJSC_SCANMAIN_CHECK_ERROR_VALUE ) {
			/* overwrite error */
			if ( data->dialog_error_value == CIJSC_VALUE_CANCEL ) {
				return;	/* cancel saving. */
			}
		}
		else {
			return;	/* cancel saving. */
		}
	}
	
	/* hide save dialog. */
	gtk_widget_hide( data->dialog_save );
	ui_dialog_save_gtk_main_iteration();
	
	/* create list of scanned data file. */
	if( ( root = CnmsCreateRoot() ) == CNMSNULL ){
		DBGMSG( "Error is occured in CnmsCreateRoot.\n" );
		return;	/* cancel saving. */
	}
	
	/* scan */
	scan_result = ui_dialog_save_scan_start( data, root );
	
	if ( scan_result != CIJSC_SCANMAIN_SCAN_FINISHED ) {
		/* error while scanning. */
		CIJSC_UI_error_show( data, NULL );
	}
	/* save */
	ui_dialog_save_save_result( data, root );
}

void CIJSC_UI_save_button_cancel_clicked( SGMP_Data *data )
{
	DBGMSG("->\n");
	
	/* hide save dialog. */
	gtk_widget_hide( data->dialog_save );
	gtk_main_quit();	/* exit loop : CIJSC_Scan_And_Save() */
}

void CIJSC_UI_notify_hide( SGMP_Data *data )
{
	(void)data;
	DBGMSG("->\n");
	
	gtk_main_quit();	/* exit loop : show dialog_notify */
	ui_dialog_save_gtk_main_iteration();
	
	gtk_main_quit();	/* exit loop : CIJSC_Scan_And_Save() */
}


#endif	/* _SCANMAIN_C_ */




