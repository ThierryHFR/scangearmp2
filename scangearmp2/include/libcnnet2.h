/*
 *  Canon Inkjet Printer Driver for Linux
 *  Copyright CANON INC. 2014-2016
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

#ifndef	__INC_CNNET2_HEADER__
#define	__INC_CNNET2_HEADER__

#include "libcnnet2_type.h"

typedef void (*search_callback)(void *arg, const tagSearchPrinterInfo *printerInfo);

#ifdef __cplusplus
extern "C" {
#endif    //__cplusplus

void * CNNET2_CreateInstance(void);
void CNNET2_DestroyInstance(void *instance);
CNNET2_ERROR_CODE CNNET2_OptSetting(void *instance, CNNET2_SETTING_FLAGS settingFlag, unsigned int settingInfo);
int CNNET2_Search(void *instance, const char *ipv4Address, search_callback callback, void *arg);
int CNNET2_SearchByIpv6(void *instance, const char *ipv6Address, search_callback callback, void *arg);
void CNNET2_CancelSearch(void *instance);
CNNET2_ERROR_CODE CNNET2_EnumSearchInfo(void *instance, tagSearchPrinterInfo *searchPrinterInfoList, unsigned int *ioSize);

#ifdef __cplusplus
}
#endif    //__cplusplus
#endif    //__INC_CNNET2_HEADER__
