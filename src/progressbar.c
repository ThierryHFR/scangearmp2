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

#ifndef	_PROGRESSBAR_C_
#define	_PROGRESSBAR_C_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "support.h"
#include "callbacks.h"
#include "cnmsstrings.h"

#include "progressbar.h"

typedef struct {
	const int		id;
	const char		*title;
	const char		*mess;
} CIJSC_PROGRESSBAR_TABLE;

static CIJSC_PROGRESSBAR_TABLE progressbar_table[] = {
	{ CIJSC_PROGRESS_SEARCHING,			STR_CNMS_LS_012_01,	STR_CNMS_LS_012_02, },
	{ CIJSC_PROGRESS_SCANNING_PLATEN,	STR_CNMS_LS_007_01,	STR_CNMS_LS_009_01, },
	{ CIJSC_PROGRESS_SCANNING_ADF,		STR_CNMS_LS_007_01,	STR_CNMS_LS_009_01, },
	{ CIJSC_PROGRESS_SAVING,			STR_CNMS_LS_007_01,	STR_CNMS_LS_009_02, },
	{ -1,	NULL, NULL, },
};

static int ui_progress_current_id = -1;
static SGMP_Data *ui_progress_data = NULL;

int CIJSC_UI_progress_show( SGMP_Data *data, int id )
{
	int			i;
	int			ret = -1;
	char		pagesStr[256];
	
	data->dialog_progress_value = CIJSC_VALUE_OK;
	ui_progress_data = data;
	for( i = 0; progressbar_table[i].id >= 0 ; i++ ) {
		if ( progressbar_table[i].id == id ) {
			break;
		}
	}
	if ( progressbar_table[i].id < 0 ) {
		goto _EXIT;
	}
	ui_progress_current_id = id;
	
	gtk_window_set_title( GTK_WINDOW( data->dialog_progress ), gettext( progressbar_table[i].title ) );
	gtk_label_set_text( GTK_LABEL ( data->label_prog_msg ), gettext( progressbar_table[i].mess )  );
	gtk_label_set_text( GTK_LABEL ( data->label_prog_pages ), ""  );
	gtk_label_set_text( GTK_LABEL ( data->label_prog_percent ), ""  );
	gtk_widget_set_sensitive( data->button_prog_cancel, FALSE );
	
	switch ( id ) {
		case CIJSC_PROGRESS_SEARCHING :
			break;
		case CIJSC_PROGRESS_SCANNING_PLATEN :
			break;
		case CIJSC_PROGRESS_SCANNING_ADF :
			snprintf( pagesStr, 256, gettext( STR_CNMS_LS_009_04 ), data->scanning_page );
			gtk_label_set_text( GTK_LABEL ( data->label_prog_pages ), pagesStr  );
			break;
		case CIJSC_PROGRESS_SAVING :
			gtk_label_set_text( GTK_LABEL ( data->label_prog_pages ), ""  );
			gtk_label_set_text( GTK_LABEL ( data->label_prog_percent ), ""  );
			break;
	}
	gtk_progress_bar_set_fraction( GTK_PROGRESS_BAR( data->progressbar_prog ), 0 );
	gtk_widget_show( data->dialog_progress );
	while( gtk_events_pending() ){
		usleep( 10000 );		/* wait 10msec */
		gtk_main_iteration();
	}
	
_EXIT:
	return ret;
}

int CIJSC_UI_progress_hide( SGMP_Data *data )
{
	gtk_widget_hide( data->dialog_progress );
	while( gtk_events_pending() ){
		gtk_main_iteration();
	}
	return 0;
}

int CIJSC_UI_progress_update_value( int pages, int total, int current )
{
	char		pagesStr[256];
	char		persentStr[256];
	int			ret = 0;
	int			percentVal = current * 100 / total;
	
	if( ui_progress_data->dialog_progress_value == CIJSC_VALUE_CANCEL ) {
#ifdef _SGMP_DEBUG_VERBOSE_
		DBGMSG("dialog_progress_value == CIJSC_VALUE_CANCEL\n");
#endif
		ret =  -1;
	}
	
	switch ( ui_progress_current_id ) {
		case CIJSC_PROGRESS_SEARCHING :
		case CIJSC_PROGRESS_SCANNING_PLATEN :
		case CIJSC_PROGRESS_SCANNING_ADF :
			break;
		case CIJSC_PROGRESS_SAVING :
			snprintf( pagesStr, 256, gettext( STR_CNMS_LS_009_04 ), pages );
			gtk_label_set_text( GTK_LABEL ( ui_progress_data->label_prog_pages ), pagesStr  );
			snprintf( persentStr, 256, "%d%%", percentVal );
			gtk_label_set_text( GTK_LABEL ( ui_progress_data->label_prog_percent ), persentStr  );
			break;
	}
	
	gtk_progress_bar_set_fraction( GTK_PROGRESS_BAR( ui_progress_data->progressbar_prog ), (double)current / total );
	while( gtk_events_pending() ){
		usleep( 10000 );		/* wait 10msec */
		gtk_main_iteration();
	}
	return ret;
}

int CIJSC_UI_progress_update_pulse( void )
{
	char		pagesStr[256];
	int			ret = 0;
	
#ifdef _SGMP_DEBUG_VERBOSE_
	DBGMSG("->\n");
#endif
	if( ui_progress_data->dialog_progress_value == CIJSC_VALUE_CANCEL ) {
#ifdef _SGMP_DEBUG_VERBOSE_
		DBGMSG("dialog_progress_value == CIJSC_VALUE_CANCEL\n");
#endif
		ret =  -1;
	}
	
	switch ( ui_progress_current_id ) {
		case CIJSC_PROGRESS_SEARCHING :
			break;
		case CIJSC_PROGRESS_SCANNING_PLATEN :
			break;
		case CIJSC_PROGRESS_SCANNING_ADF :
			snprintf( pagesStr, 256, gettext( STR_CNMS_LS_009_04 ), ui_progress_data->scanning_page );
			gtk_label_set_text( GTK_LABEL ( ui_progress_data->label_prog_pages ), pagesStr  );
			break;
		case CIJSC_PROGRESS_SAVING :
			break;
	}
	
	gtk_progress_bar_pulse( GTK_PROGRESS_BAR( ui_progress_data->progressbar_prog ) );
	while( gtk_events_pending() ){
		usleep( 10000 );		/* wait 10msec */
		gtk_main_iteration();
	}
	return ret;
}

void CIJSC_UI_progress_enable_cancel_button( SGMP_Data *data )
{
	gtk_widget_set_sensitive( data->button_prog_cancel, TRUE );
	gtk_widget_grab_focus( data->button_prog_cancel );
	while( gtk_events_pending() ){
		gtk_main_iteration();
	}
}

void CIJSC_UI_progress_disable_cancel_button( SGMP_Data *data )
{
	gtk_widget_set_sensitive( data->button_prog_cancel, FALSE );
	while( gtk_events_pending() ){
		gtk_main_iteration();
	}
}

void CIJSC_UI_progress_update_label( SGMP_Data *data )
{
	switch ( ui_progress_current_id ) {
		case CIJSC_PROGRESS_SEARCHING :
			gtk_label_set_text( GTK_LABEL ( data->label_prog_msg ), gettext( STR_CNMS_LS_012_03 ) );
			break;
		case CIJSC_PROGRESS_SCANNING_PLATEN :
		case CIJSC_PROGRESS_SCANNING_ADF :
			gtk_label_set_text( GTK_LABEL ( data->label_prog_msg ), gettext( STR_CNMS_LS_009_05 ) );
			break;
		case CIJSC_PROGRESS_SAVING :
			break;
	}
	while( gtk_events_pending() ){
		gtk_main_iteration();
	}
}

#endif	/* _PROGRESSBAR_C_ */

