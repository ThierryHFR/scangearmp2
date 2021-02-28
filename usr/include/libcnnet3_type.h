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

#ifndef __INC_CNNET3_TYPE__
#define __INC_CNNET3_TYPE__

typedef int CNNET3_ERROR;
typedef int CNNET3_IFTYPE;
typedef int CNNET3_OPTIONTYPE;
typedef int CNNET3_BOOL;
typedef const char* CNNET3_URL;
typedef void* HCNNET3;
typedef int CNNET3_TOSETTING;
typedef int CNNET3_COMMANDTYPE;
typedef int CNNET3_EVENTTYPE;

#define CNNET3_ERR_SUCCESS                0
#define CNNET3_ERR_NOCONTENT              1
#define CNNET3_ERR_FATAL                  -1
#define CNNET3_ERR_UNKNOWN_IFTYPE         -2
#define CNNET3_ERR_INVALID_HANDLE         -3
#define CNNET3_ERR_INVALID_OPERATION      -4
#define CNNET3_ERR_INVALID_IP_FORMAT      -5
#define CNNET3_ERR_FAILED_TO_RESOLVE      -6
#define CNNET3_ERR_FAILED_TO_CONNECT      -7
#define CNNET3_ERR_TOO_MATCH_SESSIONS     -8
#define CNNET3_ERR_QUEING_SESSION        -9
#define CNNET3_ERR_OPERATION_CONFLICTED   -10
#define CNNET3_ERR_WRITING_TIMEOUT        -11
#define CNNET3_ERR_READING_TIMEOUT        -12
#define CNNET3_ERR_CONNECTION_ABBORTED    -13
#define CNNET3_ERR_CONNECTION_ABORTED    -13
#define CNNET3_ERR_INVALID_PARAMETER      -14
#define CNNET3_ERR_UNKKNOWN_OPTION_TYPE   -15
#define CNNET3_ERR_INSUFFICIENT_BUFFER  -16

#define CNNET3_IFTYPE_HTTP      1
#define CNNET3_IFTYPE_PORT9100  2

#define CNNET3_OPTIONTYPE_DEVICEID_PRINT    1
#define CNNET3_OPTIONTYPE_DEVICEID_SCAN     2
#define CNNET3_OPTIONTYPE_DEVICEID_FAX      3

#define CNNET3_TOSETTING_SERVER 1
#define CNNET3_TOSETTING_CLIENT 2

#define CNNET3_EVENTTYPE_PUSHSCAN 1

#define CNNET3_COMMANDTYPE_BJNPNOTIFY 1

#define CNNET3_FALSE    0
#define CNNET3_TRUE     1

#endif
