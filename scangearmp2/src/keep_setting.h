/*
 *  ScanGear MP for Linux
 *  Copyright CANON INC. 2007-2019
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

#ifndef	_KEEP_SETTING_H_
#define	_KEEP_SETTING_H_

#include "cnmstype.h"

#define	KEEPSETTING_MAC_ADDRESS_USB	("00-00-00-00-00-00\0")

enum{
	KEEPSETTING_COMMON_ID_MACADDRESS = 0,
	KEEPSETTING_COMMON_ID_MAX,
};

CNMSInt32 KeepSettingCommonOpen( CNMSVoid );
CNMSVoid  KeepSettingCommonClose( CNMSVoid );
CNMSInt32 KeepSettingCommonSetString( CNMSInt32	id, CNMSLPSTR str );
CNMSLPSTR KeepSettingCommonGetString( CNMSInt32 id );


#endif	/* _KEEP_SETTING_H_ */
