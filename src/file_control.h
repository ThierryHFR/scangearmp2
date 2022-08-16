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

#ifndef	_FILE_CONTROL_H_
#define	_FILE_CONTROL_H_

#include <sys/types.h>
#include "../usr/include/cnmstype.h"

enum{
	FILECONTROL_STATUS_NOT_EXIST = 0,
	FILECONTROL_STATUS_WRITE_OK,
	FILECONTROL_STATUS_WRITE_NG,
	FILECONTROL_STATUS_ISNOT_FILE,
	FILECONTROL_STATUS_OTHER_ERROR,
	FILECONTROL_STATUS_INVALID_DIR,
	FILECONTROL_STATUS_MAX,
};

enum{
	FILECONTROL_OPEN_TYPE_READ = 0,
	FILECONTROL_OPEN_TYPE_NEW,
	FILECONTROL_OPEN_TYPE_NEW_ALL,	/* for setting file */
	FILECONTROL_OPEN_TYPE_MAX,
};

enum{
	FILECONTROL_SEEK_FROM_TOP = 0,
	FILECONTROL_SEEK_FROM_CURRENT,
	FILECONTROL_SEEK_FROM_END,
	FILECONTROL_SEEK_MAX,
};

CNMSInt32 FileControlGetStatus( CNMSLPSTR lpPath, CNMSInt32 pathLen );
CNMSFd    FileControlMakeTempFile( CNMSLPSTR lpPath, CNMSInt32 pathLen );
CNMSFd    FileControlOpenFile( CNMSInt32 type, CNMSLPSTR lpPath );
CNMSVoid  FileControlCloseFile( CNMSFd fd );
CNMSVoid  FileControlDeleteFile( CNMSLPSTR lpPath, CNMSFd fd );

CNMSInt32 FileControlReadFile( CNMSFd fd, CNMSLPSTR lpDst, CNMSInt32 readSize );
CNMSInt32 FileControlWriteFile( CNMSFd fd, CNMSLPSTR lpSrc, CNMSInt32 writeSize );

CNMSInt32 FileControlSeekFile( CNMSFd fd, CNMSInt32 offset, CNMSInt32 seekPoint );
off_t FileControlSeekFileOFF_T( CNMSFd fd, off_t offset, CNMSInt32 seekPoint );

CNMSInt32 FileControlReadRasterString( CNMSFd fd, CNMSLPSTR lpDst, CNMSInt32 dstSize );

#endif	/* _FILE_CONTROL_H_ */
