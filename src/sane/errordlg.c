/*
 *  ScanGear MP for Linux
 *  Copyright CANON INC. 2007-2022
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

#ifndef	_ERRORDLG_C_
#define	_ERRORDLG_C_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef _HAVE_SANE
#define _HAVE_SANE
#endif


#include <stddef.h>
#include "../support.h"
#include "../cnmsstrings.h"
#include "../errors.h"

#include "errordlg.h"

/* error dialog message type */
enum{
	CIJSC_ERROR_NO_DEV = 0,
	CIJSC_ERROR_CONNECT_FAILED,
	CIJSC_ERROR_DEVICE_CANCEL,
	CIJSC_ERROR_DEVICE_BUSY,
	CIJSC_ERROR_NO_PAPER,
	CIJSC_ERROR_DEVICE_ADF_CANCEL,
	CIJSC_ERROR_DEVICE_OTHER,
	CIJSC_ERROR_SAVE_NO_FILE_NAME,
	CIJSC_ERROR_SAVE_OVERWRITE,
	CIJSC_ERROR_SAVE_NO_ACCESS,
	CIJSC_ERROR_SAVE_INVALID_DIR,
	CIJSC_ERROR_SAVE_OTHER,
	CIJSC_ERROR_SAVE_DISK_FULL,
	CIJSC_ERROR_INTERNAL,
	CIJSC_ERROR_SFS_LOCKED,
	CIJSC_ERROR_SFS_INITIALIZE,
};

enum{
	CIJSC_ERROR_DLG_TYPE_OK = 0,
	CIJSC_ERROR_DLG_TYPE_OK_CANCEL,
};

typedef struct {
	const int		id;
	const char		*msg;
	const int		type;
	const int		quit;
} CIJSC_ERROR_MSG_TABLE;

static CIJSC_ERROR_MSG_TABLE error_msg_table[] = {
	{ CIJSC_ERROR_NO_DEV,			STR_CNMS_LS_010_07,		CIJSC_ERROR_DLG_TYPE_OK, CIJSC_ERROR_DLG_QUIT_FALSE, },
	{ CIJSC_ERROR_CONNECT_FAILED,	STR_CNMS_LS_010_05,		CIJSC_ERROR_DLG_TYPE_OK, CIJSC_ERROR_DLG_QUIT_TRUE, },
	{ CIJSC_ERROR_DEVICE_CANCEL,	STR_CNMS_LS_010_01,		CIJSC_ERROR_DLG_TYPE_OK, CIJSC_ERROR_DLG_QUIT_FALSE, },
	{ CIJSC_ERROR_DEVICE_BUSY,		STR_CNMS_LS_010_03,		CIJSC_ERROR_DLG_TYPE_OK, CIJSC_ERROR_DLG_QUIT_TRUE, },
	{ CIJSC_ERROR_NO_PAPER,			STR_CNMS_LS_010_08,		CIJSC_ERROR_DLG_TYPE_OK_CANCEL, CIJSC_ERROR_DLG_QUIT_FALSE, },
	{ CIJSC_ERROR_DEVICE_ADF_CANCEL,STR_CNMS_LS_010_02,		CIJSC_ERROR_DLG_TYPE_OK, CIJSC_ERROR_DLG_QUIT_FALSE, },
	{ CIJSC_ERROR_DEVICE_OTHER,		STR_CNMS_LS_010_04,		CIJSC_ERROR_DLG_TYPE_OK, CIJSC_ERROR_DLG_QUIT_TRUE, },
	{ CIJSC_ERROR_SAVE_NO_FILE_NAME,STR_CNMS_LS_008_03,		CIJSC_ERROR_DLG_TYPE_OK, CIJSC_ERROR_DLG_QUIT_FALSE, },
	{ CIJSC_ERROR_SAVE_OVERWRITE,	STR_CNMS_LS_008_04,		CIJSC_ERROR_DLG_TYPE_OK_CANCEL, CIJSC_ERROR_DLG_QUIT_FALSE, },
	{ CIJSC_ERROR_SAVE_NO_ACCESS,	STR_CNMS_LS_008_05,		CIJSC_ERROR_DLG_TYPE_OK, CIJSC_ERROR_DLG_QUIT_FALSE, },
	{ CIJSC_ERROR_SAVE_INVALID_DIR,	STR_CNMS_LS_008_06,		CIJSC_ERROR_DLG_TYPE_OK, CIJSC_ERROR_DLG_QUIT_FALSE, },
	{ CIJSC_ERROR_SAVE_OTHER,		STR_CNMS_LS_008_07,		CIJSC_ERROR_DLG_TYPE_OK, CIJSC_ERROR_DLG_QUIT_FALSE, },
	{ CIJSC_ERROR_SAVE_DISK_FULL,	STR_CNMS_LS_010_06,		CIJSC_ERROR_DLG_TYPE_OK, CIJSC_ERROR_DLG_QUIT_TRUE, },
	{ CIJSC_ERROR_INTERNAL,			STR_CNMS_LS_010_09,		CIJSC_ERROR_DLG_TYPE_OK, CIJSC_ERROR_DLG_QUIT_TRUE, },
	{ CIJSC_ERROR_SFS_LOCKED,		STR_CNMS_LS_010_10,		CIJSC_ERROR_DLG_TYPE_OK, CIJSC_ERROR_DLG_QUIT_TRUE, },
	{ CIJSC_ERROR_SFS_INITIALIZE,	STR_CNMS_LS_010_11,		CIJSC_ERROR_DLG_TYPE_OK, CIJSC_ERROR_DLG_QUIT_TRUE, },
	{ -1,	"", -1, -1, },
};


typedef struct {
	const int		code;
	const int		id;
} CIJSC_ERROR_INDEX_TABLE;

static CIJSC_ERROR_INDEX_TABLE error_index_table[] = {
	{ BERRCODE_CONNECT_NO_DEVICE,			CIJSC_ERROR_NO_DEV },
	{ BERRCODE_CANCELD_BY_STOP,				CIJSC_ERROR_DEVICE_CANCEL },
	{ BERRCODE_SCANNER_BUSY_COPYING,		CIJSC_ERROR_DEVICE_OTHER },
	{ BERRCODE_SCANNER_BUSY_PRINTING,		CIJSC_ERROR_DEVICE_OTHER },
	{ BERRCODE_SCANNER_BUSY_PRINTERMNT,		CIJSC_ERROR_DEVICE_OTHER },
	{ 101,									CIJSC_ERROR_CONNECT_FAILED },
	{ BERRCODE_CONNECT_FAILED,				CIJSC_ERROR_CONNECT_FAILED },
	{ 162,									CIJSC_ERROR_CONNECT_FAILED },
	{ 172,									CIJSC_ERROR_CONNECT_FAILED },
	{ 173,									CIJSC_ERROR_CONNECT_FAILED },
	{ ERR_CODE_ENOSPC,						CIJSC_ERROR_SAVE_DISK_FULL },
	{ ERR_CODE_INT,							CIJSC_ERROR_INTERNAL },
	{ BERRCODE_DEVICE_NOT_AVAILABLE,		CIJSC_ERROR_DEVICE_BUSY },
	{ BERRCODE_DEVICE_INITIALIZE,			CIJSC_ERROR_DEVICE_BUSY },
	{ BERRCODE_SYSTEM,						CIJSC_ERROR_DEVICE_OTHER },
	{ BERRCODE_CANCELED_ADF,				CIJSC_ERROR_DEVICE_ADF_CANCEL },
	{ BERRCODE_ADF_JAM,						CIJSC_ERROR_DEVICE_OTHER },
	{ BERRCODE_ADF_COVER_OPEN,				CIJSC_ERROR_DEVICE_OTHER },
	{ BERRCODE_ADF_NO_PAPER,				CIJSC_ERROR_NO_PAPER },
	{ BERRCODE_ADF_PAPER_FEED,				CIJSC_ERROR_DEVICE_OTHER },
	{ BERRCODE_ADF_MISMATCH_SIZE,			CIJSC_ERROR_DEVICE_OTHER },
	{ BERRCODE_DEVICE_NOT_AVAILABLE_LAN,	CIJSC_ERROR_CONNECT_FAILED },
	{ BERRCODE_DEVICE_NOT_AVAILABLE_LAN_LOCK,	CIJSC_ERROR_CONNECT_FAILED },
	{ BERRCODE_SCANNER_LOCKED,					CIJSC_ERROR_DEVICE_OTHER },
	{ BERRCODE_SCANNER_CONNECT_FAILED_USB,		CIJSC_ERROR_CONNECT_FAILED },
	{ BERRCODE_SCANNER_CONNECT_FAILED_LAN,		CIJSC_ERROR_CONNECT_FAILED },
	{ BERRCODE_SFS_LOCKED,					CIJSC_ERROR_SFS_LOCKED },
	{ BERRCODE_SFS_INITIALIZE,				CIJSC_ERROR_SFS_INITIALIZE },
	{ BERRCODE_SAVE_NO_FILE_NAME,		CIJSC_ERROR_SAVE_NO_FILE_NAME },
	{ BERRCODE_SAVE_OVERWRITE,			CIJSC_ERROR_SAVE_OVERWRITE },
	{ BERRCODE_SAVE_NO_ACCESS,			CIJSC_ERROR_SAVE_NO_ACCESS },
	{ BERRCODE_SAVE_INVALID_DIR,		CIJSC_ERROR_SAVE_INVALID_DIR },
	{ BERRCODE_SAVE_OTHER,				CIJSC_ERROR_SAVE_OTHER },
};



int CIJSC_UI_error_show( SGMP_Data_Lite *data )
{
	int		index_id, index_mes;
	int		ret = -1;
	int		errorCode = 0;

	DBGMSG("->\n");

	if ( lastIOErrCode ) {
		errorCode = -lastIOErrCode;
		if( errorCode != ERR_CODE_ENOSPC ) {
			errorCode = ERR_CODE_INT;
		}
	}
	else if ( lastModuleErrCode ) {
		errorCode = ERR_CODE_INT;
	}
	else {
		errorCode = lastBackendErrCode;
	}
	DBGMSG("errorCode = %d\n", errorCode );
	data->last_error_quit = CIJSC_ERROR_DLG_QUIT_FALSE;

	/* error occurred. */
	if ( errorCode ) {
	/* get error id. */
		for( index_id = 0; index_id < (int)(sizeof( error_index_table ) / sizeof( CIJSC_ERROR_INDEX_TABLE )) ; index_id++ ) {
			if ( error_index_table[index_id].code == errorCode ) {
				break;
			}
		}
		DBGMSG("index_id = %d\n", index_id );
		if ( index_id == ( sizeof( error_index_table ) / sizeof( CIJSC_ERROR_INDEX_TABLE ) ) ) {
			goto _EXIT;
		}
		/* get error message. */
		for( index_mes = 0; error_msg_table[index_mes].id >= 0 ; index_mes++ ) {
			if ( error_msg_table[index_mes].id == error_index_table[index_id].id ) {
				break;
			}
		}
		DBGMSG("index_mes = %d\n", index_mes );
		if ( error_msg_table[index_mes].id < 0 ) {
			goto _EXIT;
		}

		ret = error_msg_table[index_mes].quit;
		data->last_error_quit = ret;

	}
_EXIT:
	lastBackendErrCode = 0;

	return ret;
}


#endif	/* _ERRORDLG_C_ */

