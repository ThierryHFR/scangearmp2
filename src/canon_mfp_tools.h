/*
 *  ScanGear MP for Linux
 *  Copyright CANON INC. 2007-2023
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


#ifndef _canon_mfp_tools_h_
#define _canon_mfp_tools_h_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <inttypes.h>

#include "../usr/include/libcnnet.h"
#include "../usr/include/libcnnet2.h"
#include "../usr/include/libcnnet3.h"
#include "../usr/include/libcnnet3_url.h"
#include "../usr/include/cnmslld.h"

#define DEFTOSTR1(def) #def
#define DEFTOSTR(def) DEFTOSTR1(def)

#define CN_USB_VENDERID (0x04a9)

/*-------Error Code(USB)------------------*/
#define CN_USB_WRITE_OK (0)
#define CN_USB_WRITE_END (0)
#define CN_USB_WRITE_ERROR (-1)
#define CN_USB_WRITE_EP_SEARCH_ERROR (-101)
#define CN_USB_READ_EP_SEARCH_ERROR (-102)
#define CN_USB_READ_DATA_ERROR (-103)
#define CN_USB_WRITE_DATA_ERROR (-104)
#define CN_USB_FILE_READ_ERROR (-105)

/*-------Parameter Id(USB)----------------*/
#define CN_USB_PRINTER_WRITE_EP (1)
#define CN_USB_PRINTER_READ_EP (2)

/* CANON_Device macros.*/
#define	CIJSC_GET_PLATEN_A5(x)	( ( (x) >> 12 ) & 0x0001 )
#define	CIJSC_GET_LIB_VERSION(x)	( ( (x) >> 8 ) & 0x000F )
#define	CIJSC_GET_SUPPORT_USB(x)	( ( (x) >> 4 ) & 0x0001 )
#define	CIJSC_GET_SUPPORT_NET(x)	( ( (x) >> 4 ) & 0x0002 )
#define	CIJSC_GET_SUPPORT_NET2(x)	( ( (x) >> 4 ) & 0x0004 )
#define	CIJSC_GET_SUPPORT_PLATEN(x)	( (x) & 0x0001 )
#define	CIJSC_GET_SUPPORT_ADF_S(x)	( (x) & 0x0002 )
#define	CIJSC_GET_SUPPORT_ADF_D(x)	( (x) & 0x0004 )


typedef enum
  {
    CMT_STATUS_GOOD = 0,	/* everything A-OK */
    CMT_STATUS_UNSUPPORTED,	/* operation is not supported */
    CMT_STATUS_CANCELLED,	/* operation was cancelled */
    CMT_STATUS_DEVICE_BUSY,	/* device is busy; try again later */
    CMT_STATUS_INVAL,		/* data is invalid (includes no dev at open) */
    CMT_STATUS_EOF,		/* no more data available (end-of-file) */
    CMT_STATUS_JAMMED,		/* document feeder jammed */
    CMT_STATUS_NO_DOCS,	/* document feeder out of documents */
    CMT_STATUS_COVER_OPEN,	/* scanner cover is open */
    CMT_STATUS_IO_ERROR,	/* error during device I/O */
    CMT_STATUS_NO_MEM,		/* out of memory */
    CMT_STATUS_ACCESS_DENIED	/* access to resource has been denied */
  }
CMT_Status;

typedef struct CANON_Device {
	struct CANON_Device *next;

	const char	*name;
	char	*model;
	char	*fullname;
	int		product_id;
	int		type;
	int		speed;
	const char	*ipaddress;

} CANON_Device;

typedef struct {
	int				XRes;
	int				YRes;
	int				Left;
	int				Top;
	int				Right;
	int				Bottom;
	int				ScanMode;	/* gray : 2, color : 4 */
	int				ScanMethod;	/* Flatbed or TPU or ADF ... */
	CANON_SCANOPTS	opts;
} CANON_ScanParam;

FILE *cmt_conf_file_open(const char *conf);
int cmt_conf_file_read_line(char *line, int size, FILE *fp);
int cmt_get_device_info( char *line, int len, CANON_Device *c_dev );
int cmt_convert_macadress_to_array(char *str, CNNLNICINFO* infos);
int cmt_convert_ipadress_to_array(char *str, CNNLNICINFO* infos);
char *cmt_config_get_string (char *str, char **string_const);
char *cmt_config_skip_whitespace (char *str);

char *cmt_find_device_usb( CANON_Device *c_dev, int *index );
char *cmt_find_device_net( CANON_Device *c_dev, int *index );
char *cmt_find_device_net2( CANON_Device *c_dev, int *index );

int cmt_libusb_init(void);
void cmt_libusb_exit(void);
CMT_Status cmt_libusb_open(const char *devname, int *index);
void cmt_libusb_close(int index);

CMT_Status cmt_libusb_get_id( const char *devname, int *idVendor, int *idProduct, int *speed );

CMT_Status cmt_libusb_bulk_write( int index, unsigned char *buffer, unsigned long *size );
CMT_Status cmt_libusb_bulk_read( int index, unsigned char *buffer, unsigned long *size );


void cmt_network_init( void *cnnl_callback );
void cmt_network_exit( void );
CMT_Status cmt_network_update( void *cnnl_callback );
CMT_Status cmt_network_open(const char *macaddr, CNNLHANDLE *handle);
void cmt_network_close(CNNLHANDLE *handle);

CMT_Status cmt_network_write( CNNLHANDLE handle, unsigned char *buffer, unsigned long *size );
CMT_Status cmt_network_read( CNNLHANDLE handle, unsigned char *buffer, unsigned long *size );

void cmt_network_mutex_lock( void );
void cmt_network_mutex_unlock( void );


void cmt_network2_init( void *cnnl_callback );
void cmt_network2_exit( void );
CMT_Status cmt_network2_open( const char *devname, HCNNET3 *handle );
void cmt_network2_close(HCNNET3 *handle);

CMT_Status cmt_network2_write( HCNNET3 handle, unsigned char *buffer, unsigned long *size );
CMT_Status cmt_network2_read( HCNNET3 handle, unsigned char *buffer, unsigned long *size );


/* canon_mfp_io.c */
CMT_Status CIJSC_init( void *cnnl_callback );
void CIJSC_exit(void);
CMT_Status CIJSC_get_devices( const CANON_Device ***device_list );
CMT_Status CIJSC_open( const char *name );
void CIJSC_close( void );

CMT_Status CIJSC_start( CANON_ScanParam *param );
CMT_Status CIJSC_get_parameters( void *callback );
void CIJSC_get_backend_error_code( int *errCode );
CMT_Status CIJSC_read( unsigned char *buf, int *len );
void CIJSC_cancel( void );


#endif /*_canon_mfp_tools_h_ */
