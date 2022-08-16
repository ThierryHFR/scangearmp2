/*
 *  ScanGear MP for Linux
 *  Copyright CANON INC. 2007-2021
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

#ifndef _CANON_MFP_TOOLS_C_
#define _CANON_MFP_TOOLS_C_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libusb.h>
#include <pthread.h>
#include <unistd.h>
#include <libgen.h>
#include <stdnoreturn.h>

#include "support.h"
#include "errors.h"

#include "canon_mfp_tools.h"

/*
	definitions for libusb
*/
#define _CANON_VENDOR_ID (0x04a9)

#define LIBUSB_DEV_MAX (16)
typedef struct {
	struct libusb_device *dev;
	struct libusb_device_handle *handle;
	struct libusb_device_descriptor devdesc;
	uint8_t interface;
	uint8_t ep_bulk_in_address;
	uint8_t ep_bulk_out_address;

	int bcdUSB;
	int idVendor;
	int idProduct;
	char *devname;
	int opened;
} LIB_USB_DEV;

static LIB_USB_DEV libusbdev[LIBUSB_DEV_MAX];

static struct libusb_device **g_devlist = NULL;				/* device list */
static struct libusb_context *g_context = NULL;				/* libusb context */
int manual_len = 0;
CNNLNICINFO *manual_nic = NULL;

static char *
get_cnmslibpath(void)
{
	static char *cnmslibpath = NULL;
	if (cnmslibpath) return (char*)cnmslibpath;
	if (access("/usr/lib/x86_64-linux-gnu/bjlib", F_OK) != -1)
	   cnmslibpath = strdup("/usr/lib/x86_64-linux-gnu/bjlib");
	else if (access("/usr/lib/i386-linux-gnu/bjlib", F_OK) != -1)
	   cnmslibpath = strdup("/usr/lib/i386-linux-gnu/bjlib");
	else if (access("/usr/lib/aarch64-linux-gnu/bjlib", F_OK) != -1)
	   cnmslibpath = strdup("/usr/lib/aarch64-linux-gnu/bjlib");
	else if (access("/usr/lib/mips64el-linux-gnu/bjlib", F_OK) != -1)
	   cnmslibpath = strdup("/usr/lib/mips64el-linux-gnu/bjlib");
	else if (access("/usr/lib64/bjlib", F_OK) != -1)
	   cnmslibpath = strdup("/usr/lib64/bjlib");
	else
	   cnmslibpath = strdup("/usr/lib/bjlib");
	return cnmslibpath;
}

static char *
get_cnmslibmfp2(void)
{
	static char *cnmslibmfp2 = NULL;
	if (cnmslibmfp2) return (char*)cnmslibmfp2;
	char *libpath = get_cnmslibpath();
	cnmslibmfp2 = (char*)malloc(1024*sizeof(char));
	strcpy(cnmslibmfp2,libpath);
        strcat(cnmslibmfp2, "/canon_mfp2_net.ini");
	return cnmslibmfp2;
}

/*
	definitions for network
*/
#define CACHE_PATH				get_cnmslibmfp2()
#define NETWORK_DEV_MAX			(64)
#define STRING_SHORT			(32)
#define TIMEOUT_MSEC			(80)

#define WAIT_SECOND				{usleep(500000);usleep(500000);}
#define WAIT_300MSEC			{usleep(300000);}
#define WAIT_50MSEC				{usleep( 50000);}

#define SESSION_TIMEOUT			(25)

#define	RETRY_COUNT_START_SESSION		(3)
#define	RETRY_COUNT_KEEP_SESSION_CANCEL	(50)

static int network_inited = 0;

int					cmt_network_mode = 0;
pthread_mutex_t		cmt_net_mutex;
pthread_t			cmt_network_thread = (pthread_t)NULL;

static int			cmt_net_aborted = 0;

typedef struct {
	char 		modelName[STRING_SHORT];
	char 		ipAddStr[STRING_SHORT];
	char 		macAddStr[STRING_SHORT];
	CNNLNICINFO	nic;
} NETWORK_DEV;

static NETWORK_DEV	networkdev[NETWORK_DEV_MAX];


static int network2_inited = 0;

static int			cmt_net2_aborted = 0;
static NETWORK_DEV	network2dev[NETWORK_DEV_MAX];

/*
	open conf file
*/
FILE *cmt_conf_file_open(const char *conf)
{
	char *path = NULL;get_cnmslibpath();
	char dst[PATH_MAX];
	FILE *fp = NULL;
	
	if ( !conf ) return NULL;
	
	memset( dst, 0, sizeof(dst) );
	if (*conf == '/') {
	    snprintf( dst, sizeof(dst), "%s", conf );
	}
	else {
	    path = get_cnmslibpath();
	    snprintf( dst, sizeof(dst), "%s/%s", path, conf );
	}
	DBGMSG( " conf file \"%s\".\n", dst );
	fp = fopen( dst, "r" );
	if (fp) {
		DBGMSG( " use conf file \"%s\".\n", dst );
	}
	else {
		set_module_error();
		DBGMSG( " could not open conf file \"%s\".\n", (char *)conf );
	}

	return fp;
}

/*
	return : length of line or -1(EOF).
*/
int cmt_conf_file_read_line(char *line, int size, FILE *fp)
{
	int length = 0;
	char *c;
	
	if ( fp ) {
		memset( line, 0, size );
		
		c = fgets( line, size, fp );
		
		if ( !c ) {
			/* EOF */
			return -1;
		}
		/* comment? */
		if ( line[0] == '#' ) {
			length = 0;
		} else {
			length = strlen( line );
		}
	}
	else {
		return -1;
	}
	return length;
}

int cmt_get_device_info( char *line, int len, CANON_Device *c_dev )
{
	int ret = -1;
	char *start_model = NULL;
	char *start_type = NULL;
	char *start_id = NULL;
	int len_model, len_type, len_id;
	
	if ( !line || !c_dev || len < 0 ) {
		goto _EXIT;
	}
	memset( c_dev, 0, sizeof(CANON_Device) );
	
	/* get [model] */
	if( *line != '\[' ) {
		goto _EXIT;
	}
	/* skip : '[' */
	line++;
	len--;
	
	start_model = line;
	len_model = len;
	while( *line != ']' ) {
		line++;
		len--;
		if( !len ) {
			goto _EXIT;
		}
	}
	len_model -= len;
	/* skip : ']' */
	line++;
	len--;
	
	/* skip space */
	while( isspace( *line ) ) {
		line++;
		len--;
		if( !len ) {
			goto _EXIT;
		}
	}
	/* get product id */
	start_id = line;
	len_id = len;
	while( !isspace( *line ) ) {
		line++;
		len--;
		if( !len ) {
			goto _EXIT;
		}
	}
	len_id -= len;
	/* skip space */
	while( isspace( *line ) ) {
		line++;
		len--;
		if( !len ) {
			goto _EXIT;
		}
	}
	/* get device type */
	start_type = line;
	len_type = len;
	while( !isspace( *line ) ) {
		line++;
		len--;
		if( !len ) {
			break;
		}
	}
	len_type -= len;
	
	*(start_model + len_model) = '\0';
	*(start_id + len_id) = '\0';
	*(start_type + len_type) = '\0';
	
	c_dev->model = start_model;
	c_dev->product_id = strtol( start_id, 0, 0 );
	c_dev->type = strtol( start_type, 0, 0 );
	
	if ( c_dev->type == 0 || c_dev->product_id == 0 ) {
		goto _EXIT;
	}
	
	/* no error */
	ret = 0;
	
_EXIT:
	return ret;
}

char *
cmt_config_skip_whitespace (char *str)
{
	while (str && *str && isspace (*str))
		++str;
	return str;
}

char *
cmt_config_get_string (char *str, char **string_const) {
	char *start;
	size_t len;
	str = cmt_config_skip_whitespace (str);
	if (*str == '"')
	{
		start = ++str;
		while (*str && *str != '"')
			++str;
		len = str - start;
		if (*str == '"')
			++str;
		else
			start = 0;
		/* final double quote is missing */
	}   else     {
		start = str;
		while (*str && !isspace (*str)) 	++str;
		len = str - start;
	}
	if (start)
		*string_const = strndup (start, len);
	else
		*string_const = 0;
	return str;
}

int
cmt_convert_macadress_to_array(char *str, CNNLNICINFO* info)
{
        uint8_t values[6] = { 0 };
	if (sscanf(str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
				&values[0],
				&values[1],
				&values[2],
				&values[3],
				&values[4],
				&values[5]) < 6)
	{
                return 0;
	}
	info->macaddr[0] = values[0];
	info->macaddr[1] = values[1];
	info->macaddr[2] = values[2];
	info->macaddr[3] = values[3];
	info->macaddr[4] = values[4];
	info->macaddr[5] = values[5];
        return 1;
}

int
cmt_convert_ipadress_to_array(char *str, CNNLNICINFO* info)
{
        char *tmp = str;
        char *tmp2 = str;
	int cpt = 0;
	short oct[4] = { 0 };

	while ((tmp2 = strchr(tmp, '.')) != NULL) {
		*tmp2 = 0;
		oct[cpt++] = atoi((const char*)tmp);
		*tmp2 = '.';
		tmp2++;
		tmp = tmp2;
	}
	if (tmp) {
		oct[cpt++] = atoi((const char*)tmp);
	}
	if (cpt < 4)
	{
		return 0;
	}
	info->ipaddr[0] = oct[0];
	info->ipaddr[1] = oct[1];
	info->ipaddr[2] = oct[2];
	info->ipaddr[3] = oct[3];
	return 1;
}

/*
	find canon_mfp usb device.
*/
char *cmt_find_device_usb( CANON_Device *c_dev, int *index )
{
	int i;
	
	if ( !c_dev || !index ) {
		return NULL; /* error */
	}
	if ( *index < 0 ) {
		return NULL; /* error */
	}
	for ( i = *index ; i < LIBUSB_DEV_MAX; i++) {
		if ( libusbdev[i].idVendor == _CANON_VENDOR_ID && libusbdev[i].idProduct == c_dev->product_id ) {
			if ( libusbdev[i].devname ) {
				DBGMSG( " find : %s\n", libusbdev[i].devname );
				*index = i;
				c_dev->speed = 0;
				/* clear ip address. */
				if ( c_dev->ipaddress ) {
					free( (void *)c_dev->ipaddress );
					c_dev->ipaddress = NULL;
				}
				return libusbdev[i].devname;
			}
		}
	}
	return NULL;
}

/*
	find canon_mfp net device.
*/
char *cmt_find_device_net( CANON_Device *c_dev, int *index )
{
	int i, len, modelName_len;
	
	if ( !c_dev || !index ) {
		return NULL; /* error */
	}
	if ( *index < 0 || !network_inited ) {
		return NULL; /* error */
	}
	for ( i = *index ; i < NETWORK_DEV_MAX; i++) {
		len = strlen(c_dev->model);
		modelName_len = strlen(networkdev[i].modelName);
		if ( len < modelName_len || !modelName_len ) {
			continue;
		}
		if ( strncmp( networkdev[i].modelName, c_dev->model, modelName_len ) == 0 ){
			DBGMSG( " find : %s\n", networkdev[i].macAddStr );
			*index = i;
			c_dev->speed = -1;
			/* clear ip address. */
			if ( c_dev->ipaddress ) {
				free( (void *)c_dev->ipaddress );
				c_dev->ipaddress = NULL;
			}
			return networkdev[i].macAddStr;
		}
	}
	
	return NULL;
}

/*
	find canon_mfp net2 device.
*/
char *cmt_find_device_net2( CANON_Device *c_dev, int *index )
{
	int i, len, modelName_len;
	
	if ( !c_dev || !index ) {
		return NULL; /* error */
	}
	if ( *index < 0 || !network2_inited ) {
		return NULL; /* error */
	}
	DBGMSG( "-> finding %s.\n",c_dev->model );
	for ( i = *index ; i < NETWORK_DEV_MAX; i++) {
		len = strlen(c_dev->model);
		modelName_len = strlen(network2dev[i].modelName);
		if ( len < modelName_len || !modelName_len ) {
			continue;
		}
		if ( strncmp( network2dev[i].modelName, c_dev->model, modelName_len ) == 0 ){
			DBGMSG( " find : %s\n", network2dev[i].macAddStr );
			*index = i;
			c_dev->speed = -2;
			if ( c_dev->ipaddress ) {
				free( (void *)c_dev->ipaddress );
			}
			/* set ip address. */
			c_dev->ipaddress = strdup(network2dev[i].ipAddStr);
			DBGMSG( " ip address : %s\n", c_dev->ipaddress );
			/* return MAC address. */
			return network2dev[i].macAddStr;
		}
	}
	
	return NULL;
}

/*
	initialize libusb.
*/
int cmt_libusb_init(void)
{
	int err = 0;
	char name[256];
	int scanner_num = 0;
	ssize_t numdev = 0;
	struct libusb_device *dev = NULL;				/* usb device */
	struct libusb_device_descriptor devdesc;
	const struct libusb_interface *iptr = NULL;					/* Array of interface descriptors */
	const struct libusb_interface_descriptor *altptr = NULL;	/* interface descriptor */
	struct libusb_config_descriptor *cptr = NULL;				/* configuration descriptor */
	int i;
	unsigned char busnum, address;
	
	if( !g_context ){
		memset( libusbdev, 0, sizeof(libusbdev) );
		
		err = libusb_init( &g_context );
		if (err < 0){
			DBGMSG( "ERROR: [discover]libusb_init was failed\n");
			err = CN_USB_WRITE_ERROR;
			goto onErr;
		}
	}
	else {
		/* inited already. */
		err = CN_USB_WRITE_OK;
		goto onErr;
	}
	/* search canon mfp */
	numdev = libusb_get_device_list(g_context, &g_devlist);
	if((int)numdev == 0) {
		err = CN_USB_WRITE_ERROR;
		goto onErr;
	}
	
	for(i = 0; i < numdev; i++) {
		dev = g_devlist[i];
		
		err = libusb_get_device_descriptor(dev, &devdesc);
		if(err < 0) {
			DBGMSG( "ERROR: [discover]get dev descriptor was failed\n");
			err = CN_USB_WRITE_ERROR;
			goto onErr;
		}
		if(devdesc.idVendor == CN_USB_VENDERID) {
			DBGMSG( "DEBUG: [discover]canon dev found\n");
			/* search interface */
			DBGMSG( "[%02d]: devdesc.bNumConfigurations = %d\n", i, devdesc.bNumConfigurations);
			if(libusb_get_config_descriptor(dev, 0, &cptr) < 0){
				continue;	/* goto next dev. */
			}
			DBGMSG( "[%02d]: cptr->bNumInterfaces = %d\n", i, cptr->bNumInterfaces);
			iptr = cptr->interface;		/* Interface No = 0 */
			DBGMSG( "[%02d]: iptr->num_altsetting = %d\n", i,iptr->num_altsetting);
			altptr = iptr->altsetting;	/* alternate = 0 */
			
			if(altptr->bInterfaceClass == LIBUSB_CLASS_VENDOR_SPEC){
				/* CANON MFP scanner */
				busnum = libusb_get_bus_number(dev);
				address = libusb_get_device_address(dev);
				snprintf( name, sizeof(name), "libusb:%03d:%03d", busnum, address );
				libusbdev[scanner_num].dev			= dev;
				libusbdev[scanner_num].bcdUSB		= devdesc.bcdUSB;
				libusbdev[scanner_num].idVendor		= devdesc.idVendor;
				libusbdev[scanner_num].idProduct	= devdesc.idProduct;
				libusbdev[scanner_num].devname		= strdup( name );
				libusbdev[scanner_num].interface	= altptr->bInterfaceNumber;
				libusbdev[scanner_num].opened		= 0;
				
				DBGMSG( " libusbdev[scanner_num].bcdUSB = %d.%02d\n", ( libusbdev[scanner_num].bcdUSB >> 8 ), ( libusbdev[scanner_num].bcdUSB & 0xFF )  );
				
				DBGMSG( " CANON MFP found! [0x%04X:0x%04X] %s\n",
						libusbdev[scanner_num].idVendor, libusbdev[scanner_num].idProduct, libusbdev[scanner_num].devname );
				
				if ( ++scanner_num == LIBUSB_DEV_MAX ) {
					DBGMSG( " can not add device anymore.\n" );
					err = CN_USB_WRITE_OK;
					goto onErr;
				}
			}
			/* free configuration */
			if(cptr != NULL){
				libusb_free_config_descriptor(cptr);
				cptr = NULL;
			}
		}
	}
onErr:
	/* free configuration */
	if(cptr != NULL){
		libusb_free_config_descriptor(cptr);
		cptr = NULL;
	}
	if( scanner_num ) {
		DBGMSG( " CANON MFP num = %d\n", scanner_num );
	}
	
	return err;
}


/*
	dispose LIB_USB_DEV.
*/
void cmt_libusb_exit(void)
{
	int i;
	
	/*----------free libusb context----------- */
	if(g_context != NULL) {
		libusb_exit(g_context);
		g_context = NULL;
	}
	for ( i = 0; i < LIBUSB_DEV_MAX ; i++ ) {
		if ( libusbdev[i].devname ) {
			free( libusbdev[i].devname );
		}
	}
}


/*
	open libusb devices.
*/
CMT_Status cmt_libusb_open(const char *devname, int *index)
{
	int err = 0;		/* error number */
	int len;
	struct libusb_device_descriptor devdesc;					/* device descriptor */
	const struct libusb_interface *iptr = NULL;					/* Array of interface descriptors */
	const struct libusb_interface_descriptor *altptr = NULL;	/* interface descriptor */
	int ret;
	int dev_index, ep_no, numEndpoints;
	struct libusb_config_descriptor *cptr = NULL;		/* configuration descriptor */
	const struct libusb_endpoint_descriptor *endpoint;
	
	DBGMSG( " --->\n" );
	if ( !g_context ) return CMT_STATUS_INVAL;
	if ( !devname ) return CMT_STATUS_INVAL;
	if ( !index ) return CMT_STATUS_INVAL;

	len = strlen( devname );
	for ( dev_index = 0; dev_index < LIBUSB_DEV_MAX ; dev_index++ ) {
		if ( libusbdev[dev_index].devname ) {
			if ( strncmp( devname, libusbdev[dev_index].devname, len ) == 0 ) {
				DBGMSG(" found [%s].\n", devname);
				break; /* found! */
			}
		}
	}
	if ( dev_index == LIBUSB_DEV_MAX ) {
		DBGMSG( " could not find [%s].\n", devname );
		return CMT_STATUS_INVAL;
	}
	if ( libusbdev[dev_index].opened ) {
		DBGMSG( " [%s] already open.\n", devname );
		return CMT_STATUS_INVAL;
	}
	
	/* open device */
	err = libusb_open( libusbdev[dev_index].dev, &libusbdev[dev_index].handle );
	if ( err < 0 ) {
		DBGMSG( " libusb_open error [%s].\n", devname );
		return CMT_STATUS_INVAL;
	}
	
	/* get device discriptor */
	err = libusb_get_device_descriptor( libusbdev[dev_index].dev, &devdesc);
	if ( err < 0 ) {
		DBGMSG( " libusb_get_device_descriptor error [%s].\n", devname );
		return CMT_STATUS_INVAL;
	}
	
	/* get configration discriptor */
	err = libusb_get_config_descriptor( libusbdev[dev_index].dev, 0, &cptr );
	if ( err < 0 ) {
		DBGMSG( " libusb_get_config_descriptor error [%s].\n", devname );
		return CMT_STATUS_INVAL;
	}
	
	/* set configuration */
	err = libusb_set_configuration( libusbdev[dev_index].handle, cptr->bConfigurationValue );
	libusb_free_config_descriptor( cptr );
	cptr = NULL;
	if ( err < 0 ) {
		if ( err == LIBUSB_ERROR_BUSY ) {
			DBGMSG( " Though libusb returns LIBUSB_ERROR_BUSY, go to next step.\n" );
		}
		else {
			if ( err == LIBUSB_ERROR_ACCESS ) {
				ret = CMT_STATUS_ACCESS_DENIED;
			}
			else {
				ret = CMT_STATUS_INVAL;
			}
			libusb_close( libusbdev[dev_index].handle );
			libusbdev[dev_index].handle = NULL;
			
			return ret;
		}
	}
	
	/* claim interface */
	err = libusb_claim_interface( libusbdev[dev_index].handle, libusbdev[dev_index].interface );
	if ( err < 0 ) {
		DBGMSG( " could not claim interface\n" );
		libusb_close( libusbdev[dev_index].handle );
		libusbdev[dev_index].handle = NULL;
		
		return CMT_STATUS_INVAL;
	}
	
	/* search interface */
	if(libusb_get_config_descriptor( libusbdev[dev_index].dev, 0, &cptr ) < 0){
		return CMT_STATUS_INVAL;
	}
	DBGMSG( "[%02d]: devdesc.bNumConfigurations = %d\n", dev_index, devdesc.bNumConfigurations);
	DBGMSG( "[%02d]: cptr->bNumInterfaces = %d\n", dev_index, cptr->bNumInterfaces);
	iptr = cptr->interface;		/* Interface No = 0 */
	DBGMSG( "[%02d]: iptr->bNumInterfaces = %d\n", dev_index, iptr->num_altsetting);
	altptr = iptr->altsetting;	/* alternate = 0 */
	
	if( altptr->bInterfaceClass != LIBUSB_CLASS_VENDOR_SPEC ) {
		return CMT_STATUS_INVAL;
	}
	
	/* set endpoints (bulk-in, bulk-out) */
	numEndpoints = altptr->bNumEndpoints;
	endpoint = altptr->endpoint;
	for ( ep_no = 0; ep_no < numEndpoints; ep_no++, endpoint++) {
		if ( ( endpoint->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK ) != LIBUSB_TRANSFER_TYPE_BULK ) {
			continue;
		}
		/* bulk-in or bulk-out */
		if ( ( endpoint->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK ) == LIBUSB_ENDPOINT_IN ) {
			libusbdev[dev_index].ep_bulk_in_address = endpoint->bEndpointAddress;
			DBGMSG( " ep_bulk_in_address wMaxPacketSize=%d\n",endpoint->wMaxPacketSize );
		}
		else if ( ( endpoint->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK ) == LIBUSB_ENDPOINT_OUT ) {
			libusbdev[dev_index].ep_bulk_out_address = endpoint->bEndpointAddress;
		}
	}
	
	/* free configuration */
	if(cptr != NULL){
		libusb_free_config_descriptor(cptr);
		cptr = NULL;
	}
	
	*index = dev_index;
	libusbdev[dev_index].opened = 1;
	DBGMSG( " *index = %d <---\n",*index );
	
	return CMT_STATUS_GOOD;
}


/*
	close libusb devices.
*/
void cmt_libusb_close(int index)
{
	DBGMSG( " --->\n" );
	if ( index > LIBUSB_DEV_MAX ) {
		DBGMSG( " can not close this device.\n" );
	}
	if ( libusbdev[index].opened ) {
		libusbdev[index].opened = 0;
		if ( libusbdev[index].handle ) {
			libusb_release_interface( libusbdev[index].handle, libusbdev[index].interface );
			libusb_close( libusbdev[index].handle );
			/* bzero */
			libusbdev[index].handle = NULL;
		}
	}
	DBGMSG( " <---\n" );
}

CMT_Status cmt_libusb_get_id( const char *devname, int *idVendor, int *idProduct, int *speed )
{
	int len;
	int dev_index;
	
	if ( !g_context ) return CMT_STATUS_INVAL;
	if ( !devname ) return CMT_STATUS_INVAL;

	len = strlen( devname );
	for ( dev_index = 0; dev_index < LIBUSB_DEV_MAX ; dev_index++ ) {
		if ( libusbdev[dev_index].devname ) {
			if ( strncmp( devname, libusbdev[dev_index].devname, len ) == 0 ) {
				DBGMSG(" found [%s].\n", devname);
				break; /* found! */
			}
		}
	}
	if ( dev_index == LIBUSB_DEV_MAX ) {
		DBGMSG( " could not find [%s].\n", devname );
		return CMT_STATUS_INVAL;
	}
	*idVendor = libusbdev[dev_index].idVendor;
	*idProduct = libusbdev[dev_index].idProduct;
	*speed = libusbdev[dev_index].bcdUSB >> 8; /* USB2.0->2, USB1.1->1 */

	return CMT_STATUS_GOOD;
}

#define LIBUSB_TIMEOUT 10000	/* 10sec */
/*
	bulk-write to libusb devices.
*/
CMT_Status cmt_libusb_bulk_write( int index, unsigned char *buffer, unsigned long *size )
{
	int err;
	int ret_bytes, request_bytes;
	
	if ( !buffer || !size ) {
		return CMT_STATUS_INVAL;
	}
	if ( index > LIBUSB_DEV_MAX || index < 0 ) {
		return CMT_STATUS_INVAL;
	}
	if ( libusbdev[index].handle == NULL || !libusbdev[index].ep_bulk_in_address ) {
		return CMT_STATUS_INVAL;
	}
	
	request_bytes = *size;
#ifdef _SGMP_DEBUG_VERBOSE_
	DBGMSG( " (*size:%ld,request_bytes:%d)--->\n" ,*size,request_bytes);
#endif
	err = libusb_bulk_transfer( libusbdev[index].handle, libusbdev[index].ep_bulk_out_address,
					(unsigned char *)buffer, request_bytes, &ret_bytes, LIBUSB_TIMEOUT );
	
	if ( err < 0 ) { /* error happend. */
		libusb_clear_halt( libusbdev[index].handle, libusbdev[index].ep_bulk_out_address );
		*size = 0;
		DBGMSG( " IO error.(%d)\n", err );
		return CMT_STATUS_IO_ERROR;
	}
	
#ifdef _SGMP_DEBUG_VERBOSE_
	DBGMSG( " *size:%d, ret_bytes:%d<---\n", request_bytes, ret_bytes );
#endif
	*size = ret_bytes;

	return CMT_STATUS_GOOD;
}

#define LIBUSB_READ_MAX_SIZE	0x8000	/* 32k */
/*
	bulk-read from libusb devices.
*/
CMT_Status cmt_libusb_bulk_read( int index, unsigned char *buffer, unsigned long *size )
{
	int err;
	int ret_bytes, request_bytes;
	
	if ( !buffer || !size ) {
		return CMT_STATUS_INVAL;
	}
	if ( index > LIBUSB_DEV_MAX || index < 0 ) {
		return CMT_STATUS_INVAL;
	}
	if ( libusbdev[index].handle == NULL || !libusbdev[index].ep_bulk_in_address ) {
		return CMT_STATUS_INVAL;
	}
	
	request_bytes = ( *size > LIBUSB_READ_MAX_SIZE ) ? LIBUSB_READ_MAX_SIZE : *size;
#ifdef _SGMP_DEBUG_VERBOSE_
	DBGMSG( " (*size:%ld,request_bytes:%d)--->\n" ,*size,request_bytes);
#endif
	err = libusb_bulk_transfer( libusbdev[index].handle, libusbdev[index].ep_bulk_in_address,
					(unsigned char *)buffer, request_bytes, &ret_bytes, LIBUSB_TIMEOUT );
	
	if ( !ret_bytes ) {
		if ( err < 0 ) { /* error happend. */
			libusb_clear_halt( libusbdev[index].handle, libusbdev[index].ep_bulk_in_address );
			*size = 0;
			DBGMSG( " IO error.(code=%d,retbytes=%d)\n", err, ret_bytes );
			return CMT_STATUS_IO_ERROR;
		}
	}
#ifdef _SGMP_DEBUG_VERBOSE_
	DBGMSG( "request_bytes:%d, ret_bytes:%d<---\n", request_bytes, ret_bytes );
#endif
	*size = ret_bytes;
	
	return CMT_STATUS_GOOD;
}


/*
	initialize network devices list.
*/
void cmt_network_init( void *cnnl_callback )
{
	CNNLHANDLE hmdl=NULL;
	int j=0, k=0, max = NETWORK_DEV_MAX, found=0, found_cache=0, timeout_msec = 0;
	CNNLNICINFO *nic;
	char model[STRING_SHORT], ipaddr[STRING_SHORT];
	unsigned long version = 110, versize;
	unsigned long	cnnl_callback_size = sizeof( cnnl_callback );
	unsigned long	cnnl_interval = TIMEOUT_MSEC, cnnl_interval_size = sizeof( cnnl_interval );

	int cnnl_mode = CNNET_SEARCH_CACHE_ACTIVEONLY;

	if( network_inited ) return;
	network_inited = 1;
	
	DBGMSG( "CACHE_PATH = %s\n", CACHE_PATH );
	
	versize = sizeof( unsigned long );
	
	memset( networkdev, 0, sizeof(networkdev) );

	if( CNNL_Init( &hmdl ) != CNNL_RET_SUCCESS ) goto error;
	if( CNNL_Config( hmdl, CNNL_CONFIG_SET_VERSION, &version, &versize ) != CNNL_RET_SUCCESS) goto error;
	if ( cnnl_callback ) {
		if (CNNL_Config( hmdl, CNNL_CONFIG_SET_CALLBACK_FUNCTION, cnnl_callback, &cnnl_callback_size ) != CNNL_RET_SUCCESS) goto error;
		if (CNNL_Config( hmdl, CNNL_CONFIG_SET_CALLBACK_INTERVAL, &cnnl_interval, &cnnl_interval_size ) != CNNL_RET_SUCCESS) goto error;
	}
	
	memset(model, 0x00, sizeof(model));
	
	if( ( nic = (CNNLNICINFO *)malloc(sizeof(CNNLNICINFO)*max) ) == NULL ) goto error;
	
	// count cache num
	if ( CNNL_SearchPrintersEx( hmdl, nic, CACHE_PATH, max, &found_cache, CNNET_SEARCH_CACHE_ALL, 1, 5000 ) != CNNL_RET_SUCCESS ){
		free(nic);
		goto error;
	}
	timeout_msec = ( found_cache ) ? found_cache * 1000 : 5000;
	DBGMSG( " cache num = %d, timeout = %d msec\n", found_cache, timeout_msec );


	// find printers
	memset(nic, 0x00, sizeof(CNNLNICINFO)*max);
	if( CNNL_SearchPrintersEx( hmdl, nic, CACHE_PATH, max, &found, cnnl_mode, 1, timeout_msec ) == CNNL_RET_SUCCESS ){
                for (j = 0; j < manual_len; j++) {
		    if (manual_nic[j].macaddr[0] != 0) {
                       nic[found] = manual_nic[j];
	               found += 1;
		       break;
		    }
                }
		for (j=0; j<found; j++){
			
			memset(ipaddr, 0x00, STRING_SHORT);
			snprintf(ipaddr, STRING_SHORT-1, "%d.%d.%d.%d", 
					nic[j].ipaddr[0],nic[j].ipaddr[1],nic[j].ipaddr[2],nic[j].ipaddr[3]);
			
			if( CNNL_OpenEx( hmdl, ipaddr, CNNET_TYPE_MULTIPASS, 1, 1000 ) == CNNL_RET_SUCCESS ){
				if( CNNL_GetModelName( hmdl, model, STRING_SHORT, 3, 3000) == CNNL_RET_SUCCESS){
					
					strncpy( networkdev[j].modelName, model, STRING_SHORT );
					strncpy( networkdev[j].ipAddStr, ipaddr, STRING_SHORT );
					snprintf( networkdev[j].macAddStr, STRING_SHORT-1, "%02X-%02X-%02X-%02X-%02X-%02X",
						nic[j].macaddr[0],nic[j].macaddr[1],nic[j].macaddr[2], nic[j].macaddr[3],nic[j].macaddr[4],nic[j].macaddr[5] );
					
					for( k = 0 ; k < 4 ; k++ ){
						networkdev[j].nic.ipaddr[k] = nic[j].ipaddr[k];
					}
					for( k = 0 ; k < 6 ; k++ ){
						networkdev[j].nic.macaddr[k] = nic[j].macaddr[k];
					}
					
					DBGMSG(" CANON MFP found! %s (%02X-%02X-%02X-%02X-%02X-%02X)\n", 
						model,
						nic[j].macaddr[0],nic[j].macaddr[1],nic[j].macaddr[2],
						nic[j].macaddr[3],nic[j].macaddr[4],nic[j].macaddr[5]);

				}
				CNNL_Close( hmdl );
			}
		}
	} else {
		free(nic);
		goto error;
	}
	
	free(nic);
	CNNL_Terminate( &hmdl );
	
	return;
	
error:
	if (hmdl!= NULL) CNNL_Terminate( &hmdl );

	return;
}

void cmt_network_exit( void )
{
	if ( !network_inited ) return;
	network_inited = 0;
}

/*
	Update network devices list.
*/
CMT_Status cmt_network_update( void *cnnl_callback )
{
	CNNLHANDLE hmdl=NULL;
	int			ret = -1,
				max = NETWORK_DEV_MAX;
	int			found = 0;
	CNNLNICINFO *nic = NULL;

	unsigned long	cnnl_ver = 110L, cnnl_ver_size = sizeof( cnnl_ver );
	unsigned long	cnnl_callback_size = sizeof( cnnl_callback );
	unsigned long	cnnl_interval = TIMEOUT_MSEC, cnnl_interval_size = sizeof( cnnl_interval );
	
	if( CNNL_Init( &hmdl ) != CNNL_RET_SUCCESS ) goto EXIT;
	
	if (CNNL_Config( hmdl, CNNL_CONFIG_SET_VERSION, &cnnl_ver, &cnnl_ver_size ) != CNNL_RET_SUCCESS) goto EXIT;
	if (CNNL_Config( hmdl, CNNL_CONFIG_SET_CALLBACK_FUNCTION, cnnl_callback, &cnnl_callback_size ) != CNNL_RET_SUCCESS) goto EXIT;
	if (CNNL_Config( hmdl, CNNL_CONFIG_SET_CALLBACK_INTERVAL, &cnnl_interval, &cnnl_interval_size ) != CNNL_RET_SUCCESS) goto EXIT;

	if( ( nic = (CNNLNICINFO *)malloc( sizeof( CNNLNICINFO ) * max ) ) == NULL ) goto EXIT;
	if( CNNL_SearchPrintersEx( hmdl, nic, CACHE_PATH, max, &found, CNNET_SEARCH_AUTO, 1, 5000 ) != CNNL_RET_SUCCESS ){
		DBGMSG( "Error is occured in CNNL_SearchPrintersEx().\n" );
		goto EXIT;
	}
	
	ret = 0;
EXIT:
	//ProgressBarWaitFinish();
	if( nic )			free( nic );
	if( hmdl != NULL)	CNNL_Terminate( &hmdl );

	return ret;
}

void cmt_network_mutex_lock( void )
{
	if( cmt_network_mode ) {
#ifdef _SGMP_DEBUG_VERBOSE_
		DBGMSG( "->\n" );
#endif
		pthread_mutex_lock( &cmt_net_mutex );
	}
}

static int cmt_network_mutex_trylock( void )
{
	if( cmt_network_mode ) {
#ifdef _SGMP_DEBUG_VERBOSE_
		DBGMSG( "->\n" );
#endif
		return pthread_mutex_trylock( &cmt_net_mutex );
	}
	else {
		return -1;
	}
}

void cmt_network_mutex_unlock( void )
{
	if( cmt_network_mode ) {
#ifdef _SGMP_DEBUG_VERBOSE_
		DBGMSG( "->\n" );
#endif
		pthread_mutex_unlock( &cmt_net_mutex );
	}
}

void *cmt_network_keep_session( void *hnd )
{
	int				i;
	unsigned long	d_time;
	unsigned char	tmpbuf[8];
	unsigned long	size;
	
	while(1) {
		DBGMSG( "->\n" );
		if( cmt_network_mutex_trylock() ) {
#ifdef _SGMP_DEBUG_VERBOSE_
			DBGMSG( " cmt_network_mutex_trylock() failed.\n" );
#endif
		}
		else {
			if( cmt_network_mode == 1 ) {
				if( !cmt_net_aborted ) {
#ifdef _SGMP_DEBUG_VERBOSE_
					DBGMSG( " call CNNL_GetTimeout()\n" );
#endif
					CNNL_GetTimeout( (CNNLHANDLE)hnd, &d_time, 3, 3000);
				}
			}
			else if ( cmt_network_mode == 2 ) {
				if( !cmt_net2_aborted ) {
					tmpbuf[0] = 0;
					size = 1;
					cmt_network2_write( (HCNNET3)hnd, tmpbuf, &size );
					size = sizeof(tmpbuf);
					cmt_network2_read( (HCNNET3)hnd, tmpbuf, &size );
#ifdef _SGMP_DEBUG_VERBOSE_
					DBGMSG( " call cmt_network2_write/read\n" );
#endif
				}
			}
			
			cmt_network_mutex_unlock();
		}
		
		/* sleep and wait pthread_testcancel */
		for( i = 0; i < RETRY_COUNT_KEEP_SESSION_CANCEL; i++ ) {
			pthread_testcancel();
#ifdef _SGMP_DEBUG_VERBOSE_
			DBGMSG( " wait 300 msec... (%02d/%d)\n", i+1, RETRY_COUNT_KEEP_SESSION_CANCEL );
#endif
			WAIT_300MSEC;
		}
	}
	return NULL;
}


/*
	open network devices.
*/
CMT_Status cmt_network_open(const char *macaddr, CNNLHANDLE *handle)
{
	char		ipaddr[64];
	int			ret, j;
	CMT_Status	status = CMT_STATUS_IO_ERROR;
	CNNLHANDLE	hnd = NULL;

	if( CNNL_Init( &hnd ) != CNNL_RET_SUCCESS ){
		goto error;
	}
	if( CNNL_GetIPAddressEx( hnd, CACHE_PATH, macaddr, ipaddr, STRING_SHORT, CNNET_SEARCH_CACHE_ALL, 1, 5000 ) != CNNL_RET_SUCCESS ){
		goto	error;
	}
	if( CNNL_OpenEx( hnd, ipaddr, CNNET_TYPE_MULTIPASS, 1, 5000 ) != CNNL_RET_SUCCESS ){
		goto	error1;
	}
	if( CNNL_CheckVersion( hnd, 3, 5000 ) != CNNL_RET_SUCCESS ){
		goto	error1;
	}
	
	for( j = 0; j < RETRY_COUNT_START_SESSION; j++) {
		if( ( ret = CNNL_SessionStart( hnd, "\0", "\0", "\0", 3, 9000 ) ) == CNNL_RET_SUCCESS ){
			/* succeed to start session */
			DBGMSG( " succeed to start session.\n" );
			break;
		}
		/* failed to start session */
		if (ret == CNNL_RET_BUSY){
			DBGMSG( " wait 1 sec...\n" );
			WAIT_SECOND;
		}
		else {
			goto error2;
		}
	}
	/* failed to start session (busy error) */
	if( j == RETRY_COUNT_START_SESSION ) {
		status = CMT_STATUS_DEVICE_BUSY;
		goto error2;
	}
	
	/* set timeout */
	CNNL_SetTimeout( hnd, SESSION_TIMEOUT, 3, 30000 );
	
	cmt_network_mode = 1;
	/* keep TCP session thread */
	pthread_mutex_init( &cmt_net_mutex, NULL );
	if( ( ret = pthread_create( &cmt_network_thread, NULL, cmt_network_keep_session, (void *)hnd ) ) ) {
		goto error2;
	}
	cmt_net_aborted = 0;
	
	*handle = hnd;
	return CMT_STATUS_GOOD;

error2:
		CNNL_SessionEnd( hnd, 3, 9000 );
error1:
		CNNL_Close( hnd );
error:
	if( hnd ){
		CNNL_Terminate( &hnd );
	}
	*handle = hnd;
	return status;	/* CMT_STATUS_IO_ERROR or CMT_STATUS_DEVICE_BUSY */
}

/*
	close network devices.
*/
void cmt_network_close(CNNLHANDLE *handle)
{
	CNNLHANDLE	hnd = *handle;

	if( hnd ){
		if ( cmt_network_thread ) {
			cmt_network_mutex_lock();
			pthread_cancel( cmt_network_thread );
			pthread_join( cmt_network_thread, NULL );
			cmt_network_thread = (pthread_t)NULL;
			DBGMSG( " cmt_network_thread canceled.\n" );
			cmt_network_mutex_unlock();
		}
		pthread_mutex_destroy( &cmt_net_mutex );
		
		CNNL_SessionEnd( hnd, 3, 1000 );
		CNNL_Close( hnd );
		CNNL_Terminate( &hnd );
	}

	*handle = hnd;	
	return;
}

/*
	write to canon network devices.
*/
CMT_Status cmt_network_write( CNNLHANDLE handle, unsigned char *buffer, unsigned long *size )
{
	int request_bytes, status = -1;
	unsigned long	ret_bytes = 0;

	if ( !buffer || !size ) {
		return CMT_STATUS_INVAL;
	}
	if ( handle == NULL ) {
		return CMT_STATUS_INVAL;
	}
	if( cmt_net_aborted ) {
		*size = 0;
		return CMT_STATUS_IO_ERROR;
	}
	
	request_bytes = *size;
	status = CNNL_DataWrite( handle, (char*)buffer, request_bytes, &ret_bytes, 3, 30000 );
	
	if ( status != CNNL_RET_SUCCESS ) { /* error happend. */
		DBGMSG( " IO error.\n" );
		*size = 0;
		CNNL_Abort( handle );
		cmt_net_aborted = -1;
		return CMT_STATUS_IO_ERROR;
	}
	
	*size = ret_bytes;
	
	return CMT_STATUS_GOOD;
}


/*
	read from canon network devices.
*/
CMT_Status cmt_network_read( CNNLHANDLE handle, unsigned char *buffer, unsigned long *size )
{
	int request_bytes, status = -1;
	unsigned long	ret_bytes = 0;
	
	
	if ( !buffer || !size ) {
		return CMT_STATUS_INVAL;
	}
	if ( handle == NULL ) {
		return CMT_STATUS_INVAL;
	}
	if( cmt_net_aborted ) {
		*size = 0;
		return CMT_STATUS_IO_ERROR;
	}
	
	request_bytes = *size;
	status = CNNL_DataRead( handle, buffer, &ret_bytes, request_bytes, 3, 30000 );
	
	if ( status != CNNL_RET_SUCCESS ) { /* error happend. */
		DBGMSG( " IO error.\n" );
		*size = 0;
		CNNL_Abort( handle );
		cmt_net_aborted = -1;
		return CMT_STATUS_IO_ERROR;
	}
	
	*size = ret_bytes;
	
	return CMT_STATUS_GOOD;
}


/*
	initialize network2 devices list.
*/
void cmt_network2_init( void *cnnl_callback )
{
	void *instance = NULL;
	tagSearchPrinterInfo *infoList = NULL;
	CNNET2_ERROR_CODE err;
	unsigned int size = 0;
	int num = 0, i = 0;
	unsigned int timeout;
	
	if( network2_inited ) return;
	network2_inited = 1;
	
	memset( network2dev, 0, sizeof(networkdev) );
	
	if ( ( instance = CNNET2_CreateInstance() ) == NULL ) {
		goto error;
	}
	
	if ( cnnl_callback ) {
		timeout = 3000;
	}
	else {
		timeout = 1000;
	}
	
	DBGMSG( "CNNET2_OptSetting ->\n" );
	if ( ( err = CNNET2_OptSetting( instance, CNNET2_SETTING_FLAG_DISCOVER_PRINTER_TIMEOUT_MILLIS, timeout ) ) != CNNET2_ERROR_CODE_SUCCESS ) {
		DBGMSG( "Error.\n" );
		goto error;
	}
	
        DBGMSG( "CNNET2_Search ->\n" );
        for (i = 0; i < manual_len; i++) {
           if (manual_nic[i].macaddr[0] != 0) continue;
	   char strip[16] = { 0 };
           snprintf(strip, sizeof(strip), "%d.%d.%d.%d",
                    manual_nic[i].ipaddr[0],
                    manual_nic[i].ipaddr[1],
                    manual_nic[i].ipaddr[2],
                    manual_nic[i].ipaddr[3]);
           num = CNNET2_Search( instance, strip, NULL, NULL );
           DBGMSG( "CNNET2_Search Add manual IP -> [%s]\n", strip );
           if ( num < CNNET2_ERROR_CODE_SUCCESS ) {
                DBGMSG( "Error.\n" );
		continue;
           }
	   break;
        }
	if (num == 0) {
	   num = CNNET2_Search( instance, NULL, NULL, NULL );
	   if ( num < CNNET2_ERROR_CODE_SUCCESS ) {
		DBGMSG( "Error.\n" );
		goto error;
	  }
	}

	if ( num > 0 ) {
		DBGMSG( "CNNET2_Search : %d printer(s) found.\n", num );
		if ( (infoList = malloc( sizeof(tagSearchPrinterInfo) * num )) == NULL )
			goto error2;
	}
	else{
		DBGMSG( "no printer found.\n" );
	}
	
	size = sizeof(tagSearchPrinterInfo) * num;
	DBGMSG( "CNNET2_EnumSearchInfo ->\n" );
	if ( ( err = CNNET2_EnumSearchInfo( instance, infoList, &size ) ) != CNNET2_ERROR_CODE_SUCCESS ) {
		DBGMSG( "Error.(%d)\n", err );
		goto error2;
	}
	
	if ( num > NETWORK_DEV_MAX ) {
		num = NETWORK_DEV_MAX;
	}
	for ( i=0; i<num; i++ ) {
		strncpy( network2dev[i].modelName, infoList[i].modelName_, STRING_SHORT-1 );
		strncpy( network2dev[i].ipAddStr, infoList[i].ipAddressStr_, STRING_SHORT-1 );
		snprintf( network2dev[i].macAddStr, STRING_SHORT-1, "%c%c-%c%c-%c%c-%c%c-%c%c-%c%c",
				infoList[i].MacAddressStr_[0], infoList[i].MacAddressStr_[1], infoList[i].MacAddressStr_[2], infoList[i].MacAddressStr_[3],
				infoList[i].MacAddressStr_[4], infoList[i].MacAddressStr_[5], infoList[i].MacAddressStr_[6], infoList[i].MacAddressStr_[7],
				infoList[i].MacAddressStr_[8], infoList[i].MacAddressStr_[9], infoList[i].MacAddressStr_[10], infoList[i].MacAddressStr_[11] );
		DBGMSG(" CANON IJ-device found! %s (%s)\n", network2dev[i].modelName, network2dev[i].macAddStr );
	}

error2:
	if ( infoList != NULL ) {
		free( infoList );
	}
error:
	if ( instance!= NULL ) {
		CNNET2_DestroyInstance( instance );
	}
	
	return;
}

void cmt_network2_exit( void )
{
	if ( !network2_inited ) return;
	network2_inited = 0;
}


/*
	open network2 devices.
	devname : ip address
*/
CMT_Status cmt_network2_open( const char * devname, HCNNET3 *handle )
{
	int			ret;
	CMT_Status	status = CMT_STATUS_IO_ERROR;
	HCNNET3		hnd = NULL;
	
	if( ( hnd = CNNET3_Open() ) == NULL ) {
		DBGMSG( "ERROR: CNNET3_Open\n" );
		goto error;
	}
	if( ( ret = CNNET3_SetIF( hnd, CNNET3_IFTYPE_HTTP ) ) != CNNET3_ERR_SUCCESS ) {
		DBGMSG( "ERROR: CNNET3_SetIF (%d)\n", ret );
		goto error;
	}
	if( ( ret = CNNET3_SetIP( hnd, devname ) ) != CNNET3_ERR_SUCCESS ) {
		DBGMSG( "ERROR: CNNET3_SetIP (%d)\n", ret );
		goto error;
	}
	if( ( ret = CNNET3_SetURL( hnd, CNNET3_URL_SCAN ) ) != CNNET3_ERR_SUCCESS ) {
		DBGMSG( "ERROR: CNNET3_SetURL (%d)\n", ret );
		goto error;
	}
	
	cmt_network_mode = 2;
	/* keep TCP session thread */
	pthread_mutex_init( &cmt_net_mutex, NULL );
	if( ( ret = pthread_create( &cmt_network_thread, NULL, cmt_network_keep_session, (void *)hnd ) ) ) {
		goto error;
	}
	cmt_net2_aborted = 0;
	
	*handle = hnd;
	return CMT_STATUS_GOOD;
	
error:
	if( hnd ){
		CNNET3_Close( hnd );
		hnd = NULL;
	}
	*handle = hnd;
	
	return status;	/* CMT_STATUS_IO_ERROR or CMT_STATUS_DEVICE_BUSY */
}

/*
	close network2 devices.
*/
void cmt_network2_close(HCNNET3 *handle)
{
	HCNNET3	hnd = *handle;

	if( hnd ){
		if ( cmt_network_thread ) {
			cmt_network_mutex_lock();
			pthread_cancel( cmt_network_thread );
			pthread_join( cmt_network_thread, NULL );
			cmt_network_thread = (pthread_t)NULL;
			DBGMSG( " cmt_network_thread canceled.\n" );
			cmt_network_mutex_unlock();
		}
		pthread_mutex_destroy( &cmt_net_mutex );
		
		CNNET3_Close( hnd );
		hnd = NULL;
	}

	*handle = hnd;	
	return;
}

/*
	write to canon network2 devices.
*/
CMT_Status cmt_network2_write( HCNNET3 handle, unsigned char *buffer, unsigned long *size )
{
	int request_bytes, status = -1;
	unsigned long	ret_bytes = 0;
	CNNET3_BOOL		needContinue = 0;

	if ( !buffer || !size ) {
		return CMT_STATUS_INVAL;
	}
	if ( handle == NULL ) {
		return CMT_STATUS_INVAL;
	}
	if( cmt_net2_aborted ) {
		*size = 0;
		return CMT_STATUS_IO_ERROR;
	}
	
	request_bytes = *size;
#ifdef _SGMP_DEBUG_VERBOSE_
	DBGMSG( " CNNET3_Write (%d)\n", request_bytes );
#endif

	status = CNNET3_Write( handle, buffer, request_bytes, needContinue );
	
	if ( status != CNNET3_ERR_SUCCESS ) { /* error happend. */
		DBGMSG( " IO error. (%d)\n", status );
		*size = 0;
		cmt_net2_aborted = -1;
		return CMT_STATUS_IO_ERROR;
	}
	
	*size = ret_bytes;
	
	return CMT_STATUS_GOOD;
}


/*
	read from canon network2 devices.
*/
CMT_Status cmt_network2_read( HCNNET3 handle, unsigned char *buffer, unsigned long *size )
{
	int request_bytes, status = -1;
	unsigned long	ret_bytes = 0;
	CNNET3_BOOL		needContinue = 1;
	unsigned char	*tmp_buffer;
	unsigned long	tmp_read_bytes;
	
	if ( !buffer || !size ) {
		return CMT_STATUS_INVAL;
	}
	if ( handle == NULL ) {
		return CMT_STATUS_INVAL;
	}
	if( cmt_net2_aborted ) {
		*size = 0;
		return CMT_STATUS_IO_ERROR;
	}
	
	request_bytes = *size;
	tmp_buffer = buffer;
	while( needContinue ) {
		tmp_read_bytes = request_bytes;

#ifdef _SGMP_DEBUG_VERBOSE_
		DBGMSG( " CNNET3_Read (request:%ld)\n", tmp_read_bytes );
#endif
		
		status = CNNET3_Read( handle, tmp_buffer, &tmp_read_bytes, &needContinue );
		
#ifdef _SGMP_DEBUG_VERBOSE_
		DBGMSG( " CNNET3_Read (recieve:%ld) needContinue = %d\n", tmp_read_bytes, needContinue );
#endif
		
		if ( status != CNNET3_ERR_SUCCESS ) { /* error happend. */
			DBGMSG( " IO error. (%d)\n", status );
			*size = 0;
			cmt_net2_aborted = -1;
			return CMT_STATUS_IO_ERROR;
		}
		else {
			tmp_buffer += tmp_read_bytes;
			ret_bytes += tmp_read_bytes;
		}
	}
	
#ifdef _SGMP_DEBUG_VERBOSE_
	DBGMSG( " CNNET3_Read result (%ld/%ld)\n", *size, ret_bytes );
#endif
	*size = ret_bytes;
	
	return CMT_STATUS_GOOD;
}


#endif	/* _CANON_MFP_TOOLS_C_ */
