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

#ifndef	_PROGRESSBAR_H_
#define	_PROGRESSBAR_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "support.h"

/* progress bar type */
enum{
	CIJSC_PROGRESS_SEARCHING = 0,
	CIJSC_PROGRESS_SCANNING_PLATEN,
	CIJSC_PROGRESS_SCANNING_ADF,
	CIJSC_PROGRESS_SAVING,
};


int CIJSC_UI_progress_show( SGMP_Data *data, int id );
int CIJSC_UI_progress_hide( SGMP_Data *data );
int CIJSC_UI_progress_update_value( int pages, int total, int current );
int CIJSC_UI_progress_update_pulse( void );
void CIJSC_UI_progress_enable_cancel_button( SGMP_Data *data );
void CIJSC_UI_progress_disable_cancel_button( SGMP_Data *data );
void CIJSC_UI_progress_update_label( SGMP_Data *data );


#endif	/* _PROGRESSBAR_H_ */




