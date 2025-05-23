/*
 *  ScanGear MP for Linux
 *  Copyright CANON INC. 2007-2024
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

#ifndef	_SCANMAIN_H_
#define	_SCANMAIN_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "support.h"

void CIJSC_Scan_And_Save( SGMP_Data	*data );
void CIJSC_UI_save_button_save_clicked( SGMP_Data *data );
void CIJSC_UI_save_button_cancel_clicked( SGMP_Data *data );
void CIJSC_UI_notify_hide( SGMP_Data *data );


#endif	/* _SCANMAIN_H_ */




