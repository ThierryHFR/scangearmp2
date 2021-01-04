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

#ifndef _CANON_MFP_IO_C_
#define _CANON_MFP_IO_C_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "support.h"

#define CANONMUD	(9600)

#define CANON_CONFIG_FILE "canon_mfp2.conf"

#include "canon_mfp_tools.h"

/*---------------------------------------------------------------------------------------*/
/*    definitions for CIJSC ---->                                                         */
/*---------------------------------------------------------------------------------------*/
typedef struct {
	int			product;
	int			api_ver;
	const char	*model;
	const char 	*libname;
} CANON_PRODUCT_INFO;

typedef struct CANON_Scanner
{
    struct CANON_Scanner *next;
    int fd;

    int		xres;
    int		yres;
    int		ulx;
    int		uly;
    int		width;
    int		length;

    int		image_composition;
    int		bpp;


    size_t		bytes_to_read;
    int			scanning;
    
    /* add Ver.1.30 */
    int			scanMethod;
    int			CIJSC_start_status;
    
    int		scanFinished;
  }
CANON_Scanner;



static int num_devices = 0;
static CANON_Device *first_dev = NULL;
static CANON_Device *opened_handle = NULL;
static const CANON_Device **devlist = NULL;
static CANON_Scanner canon_device;

static int CANON_fd = -1;		/* file descriptor of canon_mfp device */

static CNNLHANDLE CANON_hnd = NULL;		/* handle of canon_mfp network device */
static HCNNET3 CANON_hnd2 = NULL;	/* handle of canon_mfp network2 device */


/*---------------------------------------------------------------------------------------*/
/*    internal functions for USB  ---->                                                  */
/*---------------------------------------------------------------------------------------*/
static CMT_Status canon_usb_open( const char * devname )
{
	CMT_Status status;
	int	fd = -1;

	if ( CANON_fd < 0 ) {
		if ( ( status = cmt_libusb_open (devname, &fd) ) != CMT_STATUS_GOOD ) {
			return status;
		}
		CANON_fd = fd;
		
		return CMT_STATUS_GOOD;
	}
	else { /* already open. */
		return CMT_STATUS_IO_ERROR;
	}
}

static void canon_usb_close( void )
{
	if ( CANON_fd < 0 ) {
		return ;
	}
	else {
		cmt_libusb_close( CANON_fd );
		CANON_fd = -1;
	}
}

static int canon_usb_write( unsigned char * buffer, unsigned long size )
{
	unsigned long n = size;
	CMT_Status status;
	
	if ( CANON_fd < 0 ) {
		return -1; /* error */
	}
	status = cmt_libusb_bulk_write ( CANON_fd, buffer, &n );
	
	if ( status != CMT_STATUS_GOOD ) {
		return -1; /* error */
	}
	
	if ( size != n ) {
		return n; /* error */
	}
	return 0; /* no error */
}

static int canon_usb_read( unsigned char * buffer, unsigned long * size )
{
	unsigned long n = *size;
	CMT_Status status;
	
	if ( CANON_fd < 0 ) {
		return -1; /* error */
	}
	status = cmt_libusb_bulk_read ( CANON_fd, buffer, &n );
		
	if ( status != CMT_STATUS_GOOD ) {
		return -1; /* error */
	}
	
	*size = n;
	
	if ( *size != n ) {
		return n; /* error */
	}
	return 0; /* no error */
}

/*---------------------------------------------------------------------------------------*/
/*    internal functions for Network  ---->                                              */
/*---------------------------------------------------------------------------------------*/
static CMT_Status canon_network_open( const char * devname )
{
	CMT_Status status;
	CNNLHANDLE handle = NULL;
	
	if ( CANON_hnd == NULL ) {
		if ( ( status = cmt_network_open ((const char *)devname, &handle) ) != CMT_STATUS_GOOD ) {
			return status;
		}
		CANON_hnd = handle;
		return CMT_STATUS_GOOD;
	}
	else { /* already open. */
		return CMT_STATUS_IO_ERROR;
	}

}

static void canon_network_close( void )
{
	if ( CANON_hnd == NULL ) {
		return ;
	}
	else {
		cmt_network_close( &CANON_hnd );
		CANON_hnd = NULL;
	}
}

static int canon_network_write( unsigned char * buffer, unsigned long size )
{
	unsigned long n = size;
	CMT_Status status;
	
	if ( CANON_hnd == NULL ) {
		return -1; /* error */
	}
	status = cmt_network_write ( CANON_hnd, buffer, &n );
	
	if ( status != CMT_STATUS_GOOD ) {
		return -1; /* error */
	}
	
	if ( size != n ) {
		return n; /* error */
	}
	return 0; /* no error */
}

static int canon_network_read( unsigned char * buffer, unsigned long * size )
{
	unsigned long n = *size;
	CMT_Status status;
	
	if ( CANON_hnd == NULL ) {
		return -1; /* error */
	}
	status = cmt_network_read ( CANON_hnd, buffer, &n );
		
	if ( status != CMT_STATUS_GOOD ) {
		return -1; /* error */
	}
	
	*size = n;
	
	if ( *size != n ) {
		return n; /* error */
	}
	return 0; /* no error */
}

/*---------------------------------------------------------------------------------------*/
/*    internal functions for Network2  ---->                                             */
/*---------------------------------------------------------------------------------------*/
static CMT_Status canon_network2_open( const char * devname )
{
	CMT_Status status;
	HCNNET3 handle = NULL;
	
	if ( CANON_hnd2 == NULL ) {
		if ( ( status = cmt_network2_open ((const char *)devname, &handle) ) != CMT_STATUS_GOOD ) {
			return status;
		}
		CANON_hnd2 = handle;
		return CMT_STATUS_GOOD;
	}
	else { /* already open. */
		return CMT_STATUS_IO_ERROR;
	}

}

static void canon_network2_close( void )
{
	if ( CANON_hnd2 == NULL ) {
		return ;
	}
	else {
		cmt_network2_close( &CANON_hnd2 );
		CANON_hnd2 = NULL;
	}
}

static int canon_network2_write( unsigned char * buffer, unsigned long size )
{
	unsigned long n = size;
	CMT_Status status;
	
	if ( CANON_hnd2 == NULL ) {
		return -1; /* error */
	}
	status = cmt_network2_write ( CANON_hnd2, buffer, &n );
	
	if ( status != CMT_STATUS_GOOD ) {
		return -1; /* error */
	}
	
	if ( size != n ) {
		return n; /* error */
	}
	return 0; /* no error */
}

static int canon_network2_read( unsigned char * buffer, unsigned long * size )
{
	unsigned long n = *size;
	CMT_Status status;
	
	if ( CANON_hnd2 == NULL ) {
		return -1; /* error */
	}
	status = cmt_network2_read ( CANON_hnd2, buffer, &n );
		
	if ( status != CMT_STATUS_GOOD ) {
		return -1; /* error */
	}
	
	*size = n;
	
	if ( *size != n ) {
		return n; /* error */
	}
	return 0; /* no error */
}


/*-------------------------------------------------
	dispose_canon_dev()
-------------------------------------------------*/
static void dispose_canon_dev( CANON_Device *dev )
{
	if ( dev ) {
		if ( dev->name ) {
			free( (void *)dev->name );
		}
		if ( dev->model ) {
			free( (void *)dev->model );
		}
		if ( dev->fullname ) {
			free( (void *)dev->fullname );
		}
		if ( dev->ipaddress ) {
			free( (void *)dev->ipaddress );
		}
		free( dev );
	}
}


/*-------------------------------------------------
	attach()
-------------------------------------------------*/
static CMT_Status attach(
	CANON_Device	**first,
	CANON_Device	*attach_dev,
	const char		*name )
{
	CANON_Device	*dev = NULL, *devloop = NULL;
	CMT_Status		status;
	char			dev_fullname[256];
	int				vendor;
	int				product;
	int				speed = 2;
	
	int				usb_opend = 0;
	int				flag_usb = 0, i = 0;

	if( strncmp( name, "libusb:", 7 ) >= 0 ){
		flag_usb = 1;
	}
	
	for (dev = *first; dev; dev = dev->next) {
		if ( strcmp(dev->name, name) == 0 ) {
			return (CMT_STATUS_GOOD);		/* already exists. */
		}
	}
	
	dev = malloc( sizeof( CANON_Device ) );
	if ( !dev ) {
		status = CMT_STATUS_NO_MEM;
		goto _ERROR;
	}
	memset( dev, 0, sizeof(*dev) );
	
	if(flag_usb){
		/* open succeed -> append to list */
		if ( ( status = canon_usb_open(name) ) != CMT_STATUS_GOOD )
		{
			DBGMSG("name = \"%s\" not found.\n", name);
			goto _ERROR;
		}
		usb_opend = 1;

		/* get vendor & product id */
		if ( ( status = cmt_libusb_get_id( name, &vendor, &product, &speed ) ) != CMT_STATUS_GOOD )
		{
			goto _ERROR;;
		}
	}
	else{
		speed = attach_dev->speed;
		product = attach_dev->product_id;
	}
	
	/* set name */
	dev->name = strdup( name );
	/* set model */
	dev->model = strdup( attach_dev->model );
	/* set full name */
	snprintf( dev_fullname, sizeof(dev_fullname), "Canon %s (%s)", dev->model, dev->name );
	dev->fullname = strdup( dev_fullname );
	
	if(flag_usb){
		canon_usb_close();
		usb_opend = 0;
	}
	
	dev->product_id = product;
	dev->type = attach_dev->type;
	dev->speed = speed;
	if ( attach_dev->ipaddress ) {
		dev->ipaddress = strdup( attach_dev->ipaddress );
	}
	num_devices++;

	if( *first == NULL ){
		*first = dev;
	}
	else{
		for( devloop = *first ; devloop ; devloop = devloop->next ){
			if( devloop->next == NULL ){
				devloop->next = dev;
				break;
			}
		}
	}
	
	return status;

_ERROR:
	dispose_canon_dev( dev );
	
	if(flag_usb){
		if ( usb_opend ) {
			canon_usb_close();
		}
	}
	return status;
}


/*-------------------------------------------------
	CIJSC_init()
-------------------------------------------------*/
CMT_Status CIJSC_init( void *cnnl_callback )
{
	CMT_Status status = CMT_STATUS_GOOD;
	
	FILE *fp = NULL;
	CANON_Device	*first_usb = NULL, *first_net = NULL, *first_net2 = NULL, *tmp_dev;
	CANON_Device	c_dev;
	int		i;
	
	num_devices = 0;
	
	/* initialize libUSB */
	cmt_libusb_init();
	
	/* initialize Network */
	cmt_network_init( cnnl_callback );
	
	/* initialize Network2 */
	cmt_network2_init( cnnl_callback );
	
	/*--- read Configuration file. ---*/
	fp = cmt_conf_file_open( CANON_CONFIG_FILE );
	
	if ( fp ) {
		char line[PATH_MAX];
		int len, ret;
		char *dev;
		int index_usb, index_net, index_net2;
		
		/* Set USB/Network device list */
		while ( ( len = cmt_conf_file_read_line( line, sizeof(line), fp ) ) >= 0 ) {
			index_usb = 0;
			index_net = 0;
			index_net2 = 0;
			
			if ( ( ret = cmt_get_device_info( line, len, &c_dev ) ) < 0 ) {
				continue;	/* next */
			}
			DBGMSG("Conf data : 0x%04x, 0x%04x, %s\n", c_dev.product_id, c_dev.type, c_dev.model );
			/* USB search */
			c_dev.speed = 0;
			if ( CIJSC_GET_SUPPORT_USB( c_dev.type ) ) {
				while( ( dev = cmt_find_device_usb( &c_dev, &index_usb ) ) != NULL ) {
					DBGMSG("attach(USB)  : 0x%04X, 0x%04X, %s -> %s\n", c_dev.product_id, c_dev.type, c_dev.model, dev );
					attach( &first_usb, &c_dev, dev );
					index_usb++;	/* find next device */
				}
			}
			/* Network search */
			c_dev.speed = 0;
			if ( CIJSC_GET_SUPPORT_NET( c_dev.type ) ) {
				while( ( dev = cmt_find_device_net( &c_dev, &index_net ) ) != NULL ){
					DBGMSG("attach(NET)  : 0x%04X, 0x%04X, %s -> %s\n", c_dev.product_id, c_dev.type, c_dev.model, dev );
					attach( &first_net, &c_dev, dev );
					index_net++;	/* find next device */
				}
			}
			/* Network2 search */
			c_dev.speed = 0;
			if ( CIJSC_GET_SUPPORT_NET2( c_dev.type ) ) {
				while( ( dev = cmt_find_device_net2( &c_dev, &index_net2 ) ) != NULL ){
					DBGMSG("attach(NET2)  : 0x%04X, 0x%04X, %s -> %s\n", c_dev.product_id, c_dev.type, c_dev.model, dev );
					attach( &first_net2, &c_dev, dev );
					index_net2++;	/* find next device */
				}
			}
		}
		if ( c_dev.ipaddress ) {
			free( (void *)c_dev.ipaddress );
		}
		DBGMSG("attached : %d\n", num_devices );
		fclose ( fp );
	}
	else {
		return CMT_STATUS_INVAL;
	}
	/* USB dev + NET2 dev */
	if ( first_usb != NULL ) {
		/* append USB dev */
		first_dev = first_usb;
		/* append NET2 dev */
		for (tmp_dev = first_dev; tmp_dev->next; tmp_dev = tmp_dev->next);
		tmp_dev->next = first_net2;
	}
	else {
		/* no USB dev */
		first_dev = first_net2;
	}
	/* USB dev + NET2 dev + NET dev */
	if ( first_dev != NULL ) {
		/* append NET dev */
		for (tmp_dev = first_dev; tmp_dev->next; tmp_dev = tmp_dev->next);
		tmp_dev->next = first_net;
	}
	else {
		/* only NET dev */
		first_dev = first_net;
	}

	return status;
}

/*-------------------------------------------------
	CIJSC_exit()
-------------------------------------------------*/
void CIJSC_exit(void)
{
	CANON_Device *dev, *next;

	for (dev = first_dev; dev; dev = next) {
		next = dev->next;
		dispose_canon_dev( dev );
	}
	first_dev = NULL;
	
	if( devlist ){
		free( devlist );
	}
	devlist = NULL;
	
	num_devices = 0;
	cmt_libusb_exit();
	cmt_network_exit();
	cmt_network2_exit();

}

/*-------------------------------------------------
	CIJSC_get_devices()
-------------------------------------------------*/
CMT_Status CIJSC_get_devices(
	const CANON_Device ***device_list )
{
	CANON_Device *dev;
	const CANON_Device **devlisttemp;

	if( devlist ){
		free( devlist );
	}
	
	devlist = malloc( ( num_devices + 1 ) * sizeof( CANON_Device * ) );
	if( !devlist ) {
		return (CMT_STATUS_NO_MEM);
	}
	for( dev = first_dev, devlisttemp = devlist; dev; dev = dev->next ) {
		*devlisttemp++ = dev;
	}
	
	*devlisttemp = NULL;
	*device_list = devlist;

	return CMT_STATUS_GOOD;
}

/*-------------------------------------------------
	CIJSC_open()
-------------------------------------------------*/
CMT_Status CIJSC_open(
	const char *name )	/* libusb:00X:00Y or MAC address */
{
	CANON_Device *dev;
	return CIJSC_open2(name,dev);
}

CMT_Status CIJSC_open2(
	const char *name,CANON_Device *dev )	/* libusb:00X:00Y or MAC address */
{
	CMT_Status status;
	CANON_Scanner *s = &canon_device;
	
	if ( opened_handle ) {
		DBGMSG("ERROR : Another CANON MFP Deviece has opened already.\n");
		return (CMT_STATUS_INVAL);
	}
	
	if ( !name ) {
		return (CMT_STATUS_INVAL);
	}
	
	if ( name[0] != '\0' ) {
		for (dev = first_dev; dev; dev = dev->next) {
			if (strcmp (dev->name, name) == 0) {
				break;
			}
		}
		
		if (!dev) {
			return (CMT_STATUS_INVAL);
		}
	}
	else {
		dev = first_dev;
	}
	
	DBGMSG(" dev->speed = %d \n" ,dev->speed);
	if ( dev->speed == -1 ){	/* NET */
		if( ( status = canon_network_open ( name ) ) != CMT_STATUS_GOOD ){
			DBGMSG("ERROR : canon_network_open() \n");
			return status;
		}
		else {
			/* set func-pointer (read, write) */
			if ( canon_init_driver( canon_network_read, canon_network_write ) < 0 ) {
				DBGMSG("ERROR : p_canon_init_driver() \n");
				return (CMT_STATUS_INVAL);
			}
		}
	}
	else if ( dev->speed == -2 ){	/* NET2 */
		if( ( status = canon_network2_open ( dev->ipaddress ) ) != CMT_STATUS_GOOD ){
			DBGMSG("ERROR : canon_network2_open() \n");
			return status;
		}
		else {
			/* set func-pointer (read, write) */
			if ( canon_init_driver( canon_network2_read, canon_network2_write ) < 0 ) {
				DBGMSG("ERROR : p_canon_init_driver() \n");
				return (CMT_STATUS_INVAL);
			}
		}
	}
	else {	/* USB */
		if ( ( status = canon_usb_open ( name ) ) != CMT_STATUS_GOOD ) {
			DBGMSG("ERROR : canon_usb_open() \n");
			return status;
		}
		else {
			/* set func-pointer (read, write) */
			if ( canon_init_driver( canon_usb_read, canon_usb_write ) < 0 ) {
				DBGMSG("ERROR : p_canon_init_driver() \n");
				return (CMT_STATUS_INVAL);
			}
		}
	}
	
	/* set product id. */
	DBGMSG("p_canon_init_scanner() product = %X\n", dev->product_id);
	if ( canon_init_scanner( dev->product_id, dev->speed, NULL) < 0 ) {
		DBGMSG("ERROR : p_canon_init_scanner() product = %d\n", dev->product_id);
		return (CMT_STATUS_INVAL);
	}

	opened_handle = dev;
	memset(&canon_device, 0, sizeof(canon_device));
	
	s->scanMethod = 0;
	s->CIJSC_start_status = CMT_STATUS_NO_DOCS;
	
	return CMT_STATUS_GOOD;
}

/*-------------------------------------------------
	CIJSC_close()
-------------------------------------------------*/
void CIJSC_close( void )
{
	canon_terminate_scanner();
	
	canon_usb_close ();
	canon_network_close();
	canon_network2_close();

	opened_handle = NULL;
}


/*-------------------------------------------------
	CIJSC_start()
-------------------------------------------------*/
CMT_Status CIJSC_start( CANON_ScanParam *param )
{
	int				scanmode;
	int status;
	CANON_Scanner *s = &canon_device;
	CANON_SCANDATA	scandata;
	
	DBGMSG("\n");
	
	if ( s->CIJSC_start_status == CMT_STATUS_NO_DOCS ) {
		cmt_network_mutex_lock();
	}
	
	s->scanning = TRUE;
	s->scanFinished = FALSE;
	
	s->xres = param->XRes;
	s->yres = param->YRes;
	
	/* use pixels */
	s->ulx = ( param->Left * CANONMUD ) / s->xres;
	s->uly = ( param->Top  * CANONMUD ) / s->yres;
	
	s->width  = ( ( param->Right  - param->Left ) * CANONMUD ) / s->xres;
	s->length = ( ( param->Bottom - param->Top  ) * CANONMUD ) / s->yres;
	
	s->bpp = 8;
	
	s->scanMethod = param->ScanMethod;
	
	status = canon_set_parameter_ex(
		s->xres,			/* XRes */
		s->yres,			/* YRes */
		s->ulx,				/* Left */
		s->uly,				/* Top */
		s->width,			/* Width */
		s->length,			/* Length */
		param->ScanMode,	/* ScanMode */
		s->bpp,				/* BitsPerPixel */
		s->scanMethod,		/* ScanMethod */
		NULL,				/* gamma table */
		&scandata,			/* CANON_SCANDATA */
		&(param->opts)	/* CANON_SCANOPTS */
	);
	
	if ( status < 0 ) {
		DBGMSG("error in canon_set_parameter/canon_set_parameter_ex()\n");
		status = CMT_STATUS_INVAL;
		goto _EXIT;
	}
	else if ( status == CNMP_ST_NO_PAPER ) {	/* No paper */
		DBGMSG("CNMP_ST_NO_PAPER in canon_set_parameter/canon_set_parameter_ex()\n");
		status = CMT_STATUS_NO_DOCS;
		cmt_network_mutex_unlock();
		goto _EXIT;
	}
	
	if ( canon_start_scan() < 0 ) {
		DBGMSG("error in canon_start_scan()\n");
		status = CMT_STATUS_INVAL;
		goto _EXIT;
	}
	
	status = CMT_STATUS_GOOD;

_EXIT:
	s->CIJSC_start_status = status;
	
	return status;
}

/*-------------------------------------------------
	CIJSC_get_parameters()
-------------------------------------------------*/
CMT_Status CIJSC_get_parameters( void *callback )
{
	CANON_SCANDATA		scandata;
	
	if ( canon_get_parameters( &scandata, callback ) < 0 ) {
		return CMT_STATUS_INVAL;
	}
	else {
		return CMT_STATUS_GOOD;
	}
}


/*-------------------------------------------------
	CIJSC_set_backend_error_code()
-------------------------------------------------*/
void CIJSC_get_backend_error_code( int *errCode )
{
	canon_get_status((unsigned int *)errCode);
}

/*-------------------------------------------------
	CIJSC_read()
-------------------------------------------------*/
CMT_Status CIJSC_read(
	unsigned char	*buf,
	int				*len)
{
	CMT_Status status;
	int nread;
	CANON_Scanner *s = &canon_device;
	
	if ( s->scanFinished ) {
		s->scanning = FALSE;
		status = canon_end_scan();
		
		status = CMT_STATUS_EOF;
		goto _EXIT;
	}

	if (!s->scanning) {
		status = CMT_STATUS_CANCELLED;
		goto _EXIT;
	}
	nread = ( *len * 32 ) / 32;
	
	if ( ( *len = canon_read_scan ( buf, nread ) ) < 0 ) {
		status = CMT_STATUS_IO_ERROR;
		goto _EXIT;
	}
	
	if ( *len == 0 ) {
		s->scanFinished = TRUE;
		status = CMT_STATUS_EOF;
		goto _EXIT;
	}
	
	status = CMT_STATUS_GOOD;

_EXIT:
	return status;
}

/*-------------------------------------------------
	CIJSC_cancel()
-------------------------------------------------*/
void CIJSC_cancel( void )
{
	CANON_Scanner *s = &canon_device;
	int			skip_unlock = 0;
	
	DBGMSG("\n");
	
	s->scanning = FALSE;
	
	if ( s->scanFinished ) {
		/* end scan */
		if ( s->scanMethod ) {	/* ADF */
			if ( s->CIJSC_start_status == CMT_STATUS_NO_DOCS ) {
				canon_end_scan();
				skip_unlock = 1;
				DBGMSG("skip cmt_network_mutex_unlock()\n");
			}
			else {
				canon_do_cancel();
			}
		}
		else {
			canon_end_scan();
		}
	}
	else {
		/* user cancel */
		canon_do_cancel();
	}
	
	if( !skip_unlock ) {
		cmt_network_mutex_unlock();
	}
	
	/* for next CIJSC_start */
	s->CIJSC_start_status = CMT_STATUS_NO_DOCS;
}

#endif	/* _CANON_MFP_IO_C_ */
