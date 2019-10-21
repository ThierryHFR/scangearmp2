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

#ifndef _JPEG2PDF_H_
#define _JPEG2PDF_H_


/* CnmsPDF_StartPage - type */
enum {
	CNMS_PDF_IMAGE_COLOR = 0,	/* RGB24bit */
	CNMS_PDF_IMAGE_GRAY,		/* Gray8bit */
	CNMS_PDF_IMAGE_MONO,		/* Gray1bit */
	CNMS_PDF_IMAGE_NUM,
};

/* CnmsPDF_StartPage - rotate */
enum {
	CNMS_PDF_ROTATE_OFF = 0,	/* rotate off */
	CNMS_PDF_ROTATE_ON,			/* rotate 180 degrees */
};

CNMSInt32 CnmsPDF_Open( CNMSVoid **ppw, CNMSFd fd );
CNMSVoid CnmsPDF_Close( CNMSVoid *pw );

CNMSInt32 CnmsPDF_StartDoc( CNMSVoid *pw );
CNMSInt32 CnmsPDF_EndDoc( CNMSVoid *pw );

CNMSInt32 CnmsPDF_StartPage( CNMSVoid *pw, CNMSInt32 w, CNMSInt32 h, CNMSInt32 res, CNMSInt32 type, CNMSInt32 rotate );
CNMSInt32 CnmsPDF_EndPage( CNMSVoid *pw );

CNMSInt32 CnmsPDF_WriteJpegData( CNMSVoid *pw, LPCNMS_NODE node, int (*callback)(int,int,int) );

#endif /* _JPEG2PDF_H_ */
