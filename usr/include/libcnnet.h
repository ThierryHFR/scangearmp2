/*
 *  Canon Inkjet Printer Driver for Linux
 *  Copyright CANON INC. 2009-2014
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
#ifndef __INC_CNNET_HEADER__
#define __INC_CNNET_HEADER__

/* handle */
typedef void* CNNLHANDLE;

/* printer network infomation */
typedef struct _CNNLNICINFO{
	unsigned char		macaddr[6];
	unsigned char		ipaddr[4];
} CNNLNICINFO;

/* Return Code */
#define CNNL_RET_SUCCESS      (0)
#define CNNL_RET_FAILURE      (1)
#define CNNL_RET_BUSY         (2)
#define CNNL_RET_NOT_WORKING  (3)
#define CNNL_RET_POWEROFF     (4)
#define CNNL_RET_BUSY_RESPONSE (5)

/* Command Type */
#define CNNL_COMMAND_SUPPORT	(0)		/* added ver.1.20 */
#define CNNL_COMMAND_NOSUPPORT	(1)		/* added ver.1.20 */
#define CNNL_COMMAND_IVEC		(CNNL_COMMAND_SUPPORT)		/* duplicated ver.1.20 */
#define CNNL_COMMAND_NONIVEC	(CNNL_COMMAND_NOSUPPORT)	/* duplicated ver.1.20 */

/* Config Type */
#define CNNL_CONFIG_SET_VERSION							(0)
#define CNNL_CONFIG_SET_CALLBACK_FUNCTION				(1)
#define CNNL_CONFIG_SET_CALLBACK_INTERVAL				(2)

/* Searching Mode */
#define CNNET_SEARCH_BROADCAST	(0)
#define CNNET_SEARCH_UNICAST	(1)
#define CNNET_SEARCH_AUTO		(2)
#define CNNET_SEARCH_CACHE_ALL	(3)			/* added ver.1.10 */
#define CNNET_SEARCH_CACHE_ACTIVEONLY (4)	/* added ver.1.10 */

/* Command Set */
#define CNNET_TYPE_CONTROL		(0)		/* added ver.1.10 */
#define CNNET_TYPE_PRINTER		(1)		/* added ver.1.10 */
#define CNNET_TYPE_MULTIPASS	(2)		/* added ver.1.10 */
#define CNNET_TYPE_STORAGE		(3)		/* added ver.1.10 */

/* functions */

/* for ver.1.00 */
int CNNL_Open(CNNLHANDLE h, const char *host);
int CNNL_StartPrint(CNNLHANDLE h, const int retry, const unsigned long timeout);
int CNNL_CheckPrint(CNNLHANDLE h, const int retry, const unsigned long timeout);
int CNNL_EndPrint(CNNLHANDLE h, const int retry, const unsigned long timeout);

/* for ver.1.10 */
int CNNL_Init(CNNLHANDLE *h);
int CNNL_Close(CNNLHANDLE h);
int CNNL_Terminate(CNNLHANDLE *h);
int CNNL_CheckVersion(CNNLHANDLE h, const int retry, const unsigned long timeout);
int CNNL_GetNICInfo(CNNLHANDLE h, char *hwaddr, const int hwsize, char *ipaddr, const int ipsize, const int retry, const unsigned long timeout);
int CNNL_SessionStart(CNNLHANDLE h, const char *user, const char *computer, const char *document, const int retry, const unsigned long timeout);
int CNNL_SessionEnd(CNNLHANDLE h, const int retry, const unsigned long timeout);
int CNNL_SetTimeout(CNNLHANDLE h, const unsigned long time, const int retry, const unsigned long timeout);
int CNNL_GetTimeout(CNNLHANDLE h, unsigned long *time, const int retry, const unsigned long timeout);
int CNNL_GetSessionInfo(CNNLHANDLE h, int *count, int *activeid, char *user, const int usersz, char *computer, const int computersz, char *document, const int documentsz, const int retry, const unsigned long timeout);
int CNNL_DataRead(CNNLHANDLE h, void *buf, unsigned long *readsz, const unsigned long bufsz, const int retry, const unsigned long timeout);
int CNNL_DataWrite(CNNLHANDLE h, const void *buf, const unsigned long bufsz, unsigned long *writesize, const int retry, const unsigned long timeout);
int CNNL_GetDeviceID(CNNLHANDLE h, void *buf, unsigned long *readsz, const unsigned long bufsz, const int retry, const unsigned long timeout);
int CNNL_GetModelName(CNNLHANDLE h, char *model, const int modelsz, const int retry, const unsigned long timeout);
int CNNL_SoftReset(CNNLHANDLE h, const int retry, const unsigned long timeout);
int CNNL_Abort(CNNLHANDLE h);
int CNNL_GetIPAddress(const char *cachefile, const char *macaddr, char *ipaddr, const unsigned long bufsz, int mode, const int retry, const unsigned long timeout);
int CNNL_SearchPrinters(CNNLNICINFO *nic, const char *cachefile, const int maxprinters, int *foundprinters, int mode, const int retry, const unsigned long timeout);

/* added from ver.1.10 */
int CNNL_OpenEx(CNNLHANDLE h, const char *host, int command_type, const int retry, const unsigned long timeout);
int CNNL_Config(CNNLHANDLE h, const unsigned long mode, void *val, unsigned long *valsz);
int CNNL_GetCommandType(CNNLHANDLE h, int *commandtype, const int retry, const unsigned long timeout);
int CNNL_SearchPrintersEx(CNNLHANDLE h, CNNLNICINFO *nic, const char *cachefile, const int maxprinters, int *foundprinters, int mode, const int retry, const unsigned long timeout);
int CNNL_GetIPAddressEx(CNNLHANDLE h, const char *cachefile, const char *macaddr, char *ipaddr, const unsigned long bufsz, int mode, const int retry, const unsigned long timeout);

/* added from ver.1.20 */
int CNNL_GetExtensionSupport(CNNLHANDLE h, int *support_type, const int retry, const unsigned long timeout);
int CNNL_GetMaxDataSize(CNNLHANDLE h, unsigned long *maxDataSize, const int retry, const unsigned long timeout);

#endif
