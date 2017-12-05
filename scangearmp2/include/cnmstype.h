/*
 *  ScanGear MP for Linux
 *  Copyright CANON INC. 2007-2017
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

#ifndef _CNMSTYPE_H_
#define _CNMSTYPE_H_

#if defined(__cplusplus)
#define CNMSNULL         0
#else
#define CNMSNULL         (NULL)
#endif

#define	CNMS_NO_ERR			(0)
#define	CNMS_NO_ERR_CANCLED	(1)

#define	CNMS_ERR			(-128)
#define	CNMS_FILE_ERR		(-1)

#define	CNMS_TRUE			(1)
#define	CNMS_FALSE			(0)

typedef void				CNMSVoid;

typedef char				CNMSInt8;
typedef unsigned char		CNMSUInt8;
typedef short				CNMSInt16;
typedef unsigned short		CNMSUInt16;
#if __LP64__
typedef int					CNMSInt32;
typedef unsigned int		CNMSUInt32;
#else
typedef long				CNMSInt32;
typedef unsigned long		CNMSUInt32;
#endif
typedef long long 			CNMSInt64;
typedef unsigned long long 	CNMSUInt64;
typedef int					CNMSInt;

typedef char				CNMSByte;
typedef CNMSByte *			CNMSLPSTR;

typedef double				CNMSDec32;

typedef int					CNMSFd;
typedef CNMSInt32			CNMSBool;

enum{
	CNMS_DIM_H = 0,
	CNMS_DIM_V,
	CNMS_DIM_MAX,
};

enum{
	CNMS_KEY_OFF = 0,
	CNMS_KEY_ON,
	CNMS_KEY_MAX,
};

#define CNMS_GET_ABSOLUTE( in )		( ( ( in ) < 0 ) ? ( 0 - ( in ) ) : ( in ) )
#define CNMS_GET_MAX_2( a, b )		( ( ( a ) < ( b ) ) ? ( b ) : ( a ) )
#define CNMS_GET_MIN_2( a, b )		( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#define CNMS_GET_MAX_3( a, b, c )	( ( ( a ) < ( b ) ) ? ( CNMS_GET_MAX_2( b, c ) ) : ( CNMS_GET_MAX_2( a, c ) ) )
#define CNMS_GET_MIN_3( a, b, c )	( ( ( a ) < ( b ) ) ? ( CNMS_GET_MIN_2( a, c ) ) : ( CNMS_GET_MIN_2( b, c ) ) )

#define CNMS_CLIP_VAL( in, min, max )	( ( ( in ) < ( min ) ) ? ( min ) : ( ( ( max ) < ( in ) ) ? ( max ) : ( in ) ) )


#ifndef EXTERN_C
#if defined(__cplusplus)
#define EXTERN_C extern "C"
#else
#define EXTERN_C 
#endif 
#endif /* EXTERN_C */


#endif /* _CNMSTYPE_H_ */

