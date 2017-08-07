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

#ifndef __INC_CNNET3_HEADER__
#define __INC_CNNET3_HEADER__

#include "libcnnet3_type.h"

#ifdef __cplusplus
extern "C" {
#endif

HCNNET3 CNNET3_Open();
CNNET3_ERROR CNNET3_Close(HCNNET3 clientHandle);

CNNET3_ERROR CNNET3_SetIF(HCNNET3 handle, CNNET3_IFTYPE ifType);
CNNET3_ERROR CNNET3_SetIP(HCNNET3 handle, const char* address);
CNNET3_ERROR CNNET3_SetPrinterName(HCNNET3 handle, const char* printerName);
CNNET3_ERROR CNNET3_SetURL(HCNNET3 handle, CNNET3_URL url);
CNNET3_ERROR CNNET3_SetTimeout(HCNNET3 handle, CNNET3_TOSETTING toSetting, unsigned int second);
CNNET3_ERROR CNNET3_SetMasterPortOption(HCNNET3 handle, CNNET3_OPTIONTYPE optionType);

CNNET3_ERROR CNNET3_Write(HCNNET3 handle, unsigned char* sendBuffer, unsigned long bufferSize, CNNET3_BOOL needContinue);
CNNET3_ERROR CNNET3_Read(HCNNET3 handle, unsigned char* recvBuffer, unsigned long* bufferSize, CNNET3_BOOL* needContinue);

CNNET3_ERROR CNNET3_SetEventType(HCNNET3 handle, CNNET3_EVENTTYPE eventtype);
CNNET3_ERROR CNNET3_SetCommandType(HCNNET3 handle, CNNET3_COMMANDTYPE commandtype);
CNNET3_ERROR CNNET3_SetEventBufferSize(HCNNET3 handle, unsigned int size);
CNNET3_ERROR CNNET3_Send(HCNNET3 handle,unsigned char* sendBuffer,unsigned long bufferSize,unsigned long* sentSize);

#ifdef __cplusplus
}
#endif

#endif
