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

#ifndef _cnmslld_h_
#define _cnmslld_h_

#ifdef __cplusplus
extern "C" {
#endif

/* Error and Status code */
#define CNMP_ST_OK					(0)
#define CNMP_ST_WAIT_CALIBRATION	(1)
#define CNMP_ST_NO_PAPER			(2)


/* for CANON_SCANOPTS */
#define CNMS_SCANOPTS_ON	(1)
#define CNMS_SCANOPTS_OFF	(0)


/* ADF status */
#define CNMS_ADF_SUPPORT		(1)
#define CNMS_DADF_SUPPORT		(2)
#define CNMS_ADF_EXIST_PAPER	(2)

/* device info struct */
typedef struct {
	int		mud;
	int		xres_default;
	int		yres_default;
	int		xres_max;
	int		yres_max;
	int		xres_min;
	int		yres_min;
	int		max_width;
	int		max_length;
	int		xres_list[16];
	int		yres_list[16];
} CANON_DEVICE_INFO;

/* scan data struct */
typedef struct {
	unsigned long	bytes_per_line;
	unsigned long	pixels_per_line;
	unsigned long	lines;
	unsigned long	available;
} CANON_SCANDATA;

/* scan option struct */
typedef struct {
	int		p1_0;
	int		p2_0;
	int		p3_3;
	int		DocumentType;
	int		p4_0;
	int		p5_0;
	int		p6_1;
	int		reserve[11];
} CANON_SCANOPTS;

typedef int ( *FPCANON_USB_READ ) ( unsigned char * buffer, unsigned long *size );
typedef int ( *FPCANON_USB_WRITE ) ( unsigned char * buffer, unsigned long size );

typedef int ( *FPCANON_INIT_DRIVER ) ( FPCANON_USB_READ, FPCANON_USB_WRITE );
typedef int ( *FPCANON_INIT_SCANNER ) ( int, int, CANON_DEVICE_INFO * );
typedef int ( *FPCANON_TERMINATE_SCANNER ) ( void );
typedef int ( *FPCANON_SET_PARAMETER ) ( int, int, int, int, int, int, int, int, int, unsigned short*, CANON_SCANDATA * );
typedef int ( *FPCANON_START_SCAN ) ( void );
typedef int ( *FPCANON_DO_CANCEL ) ( void );
typedef int ( *FPCANON_READ_SCAN ) ( unsigned char *, int );
typedef int ( *FPCANON_GET_STATUS ) ( unsigned int * );
typedef int ( *FPCANON_GET_CALIBLATION_STATUS ) ( unsigned int * );
typedef int ( *FPCANON_GET_PARAMETERS ) ( CANON_SCANDATA *, void * );
typedef int ( *FPCANON_END_SCAN ) ( void );
typedef int ( *FPCANON_SET_PARAMETER_EX ) ( int, int, int, int, int, int, int, int, int, unsigned short*, CANON_SCANDATA *, CANON_SCANOPTS * );

typedef int ( *FPCANON_GET_ADF_STATUS ) ( void );


/* cnmslld api struct */
typedef struct {
	void 							*handle;
	FPCANON_INIT_DRIVER				p_canon_init_driver;
	FPCANON_INIT_SCANNER			p_canon_init_scanner;
	FPCANON_TERMINATE_SCANNER		p_canon_terminate_scanner;
	FPCANON_SET_PARAMETER			p_canon_set_parameter;
	FPCANON_START_SCAN				p_canon_start_scan;
	FPCANON_DO_CANCEL				p_canon_do_cancel;
	FPCANON_READ_SCAN				p_canon_read_scan;
	FPCANON_GET_STATUS				p_canon_get_status;
	FPCANON_GET_CALIBLATION_STATUS	p_canon_get_calibration_status;
	FPCANON_GET_PARAMETERS			p_canon_get_parameters;
	FPCANON_END_SCAN				p_canon_end_scan;
	FPCANON_SET_PARAMETER_EX		p_canon_set_parameter_ex;
	FPCANON_GET_ADF_STATUS			p_canon_get_adf_status;
} CNMSLLDAPI;

int canon_init_driver ( FPCANON_USB_READ, FPCANON_USB_WRITE );
int canon_init_scanner ( int, int, CANON_DEVICE_INFO * );
int canon_terminate_scanner( void );
int canon_set_parameter ( int, int, int, int, int, int, int, int, int, unsigned short*, CANON_SCANDATA * );
int canon_start_scan ( void );
int canon_do_cancel ( void );
int canon_read_scan ( unsigned char *, int );
int canon_get_status ( unsigned int * );
int canon_get_calibration_status ( unsigned int * );
int canon_get_parameters( CANON_SCANDATA *, void *callback );
int canon_end_scan( void );
int canon_set_parameter_ex ( int, int, int, int, int, int, int, int, int, unsigned short*, CANON_SCANDATA *, CANON_SCANOPTS * );

int canon_get_adf_status( void );

#ifdef __cplusplus
}
#endif
#endif


