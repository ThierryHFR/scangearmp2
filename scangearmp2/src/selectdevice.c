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

#ifndef	_SELECTDEVICE_C_
#define	_SELECTDEVICE_C_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "support.h"
#include "callbacks.h"
#include "canon_mfp_tools.h"

#include "cnmsfunc.h"

#include "errors.h"

#include "errordlg.h"
#include "progressbar.h"
#include "keep_setting.h"

static const CANON_Device **select_device_list = NULL;

static void ui_combobox_select_devlist_init( SGMP_Data *data )
{
	GtkListStore		*store;
	GtkCellRenderer		*renderer;
	GtkTreeIter			iter;
	int					devnum;
	int					cache_num = 0;
	char				dev_list_name[256];
	char				*cache_name = KeepSettingCommonGetString( KEEPSETTING_COMMON_ID_MACADDRESS );
	
	store = GTK_LIST_STORE( gtk_combo_box_get_model( GTK_COMBO_BOX( data->combobox_select_devlist ) ) );
	gtk_list_store_clear( store );
	
	for( devnum = 0 ; ; devnum ++ ){
		if( select_device_list[ devnum ] == NULL ){
			break;
		}
		else {
			gtk_list_store_append (store, &iter);
			gtk_list_store_set( store, &iter, 0, select_device_list[ devnum ]->fullname, -1 );
			if ( strcmp( select_device_list[ devnum ]->name, cache_name ) == 0 ) {
				cache_num = devnum;
			}
		}
	}
	gtk_combo_box_set_active( GTK_COMBO_BOX( data->combobox_select_devlist ), cache_num );
}

static int ui_dialog_select_init( SGMP_Data *data )
{
	CMT_Status		ret;
	int				devnum = 0;
	
	/* get device list */
	if ( ( ret = CIJSC_get_devices( &select_device_list ) ) != CMT_STATUS_GOOD ) {
		goto _EXIT;
	}
	for( devnum = 0 ; ; devnum ++ ){
		if( select_device_list[ devnum ] == NULL ){
			break;
		}
	}
	DBGMSG("devnum = %d\n", devnum);
	if ( devnum == 0 ) {
		gtk_widget_hide( data->combobox_select_devlist );
		gtk_widget_show( data->label_select_nodev );
		gtk_widget_set_sensitive( data->button_select_ok, FALSE );
		gtk_widget_grab_focus( data->button_select_cancel );
		/* show "device not found" */
		lastBackendErrCode = BERRCODE_CONNECT_NO_DEVICE;
		CIJSC_UI_error_show( data, data->dialog_select );
	}
	else {
		gtk_widget_show( data->combobox_select_devlist );
		gtk_widget_hide( data->label_select_nodev );
		gtk_widget_set_sensitive( data->button_select_ok, TRUE );
		gtk_widget_grab_focus( data->button_select_ok );
		ui_combobox_select_devlist_init( data );
	}
_EXIT:
	return devnum;
}

CANON_Device const *CIJSC_UI_select_show( SGMP_Data	*data )
{
	CMT_Status			ret;
	CANON_Device const	*selected = NULL;
	int					select_no;
	CNMSByte			address[256];
	
	ui_dialog_select_init( data );
	gtk_widget_show( data->dialog_select );
	gtk_main();
	/* set selected device */
	if ( data->dialog_select_value == CIJSC_VALUE_OK ) {
		select_no = gtk_combo_box_get_active( GTK_COMBO_BOX( data->combobox_select_devlist ) );
		DBGMSG( "select_no = %d\n",select_no );
		selected = select_device_list[ select_no ];
		DBGMSG( "selected->name = %s\n",selected->name );
		
		if( selected->speed > 0 ){
			if( CnmsStrCopy( KEEPSETTING_MAC_ADDRESS_USB, address, sizeof(address) ) == CNMS_ERR ){
				DBGMSG( "[CnmsScanOpen]Error is occured in CnmsStrCopy.\n" );
				goto _EXIT;
			}
		}
		else {
			if( CnmsStrCopy( (CNMSLPSTR)selected->name, address, sizeof(address) ) == CNMS_ERR ){
				DBGMSG( "[CnmsScanOpen]Error is occured in CnmsStrCopy.\n" );
				goto _EXIT;
			}
		}
		DBGMSG( "address = %s\n",address );
		if( KeepSettingCommonSetString( KEEPSETTING_COMMON_ID_MACADDRESS, address ) != CNMS_NO_ERR ){
			DBGMSG( "[CnmsScanOpen]Error is occured in KeepSettingCommonSetString.\n" );
			goto _EXIT;
		}
	}
_EXIT:
	return selected;
}

int CIJSC_UI_select_update_list( SGMP_Data	*data )
{
	int				devnum;
	
	/* disable dialog_select */
	gtk_widget_set_sensitive( data->dialog_select, FALSE );
	/* clear device list. */
	CIJSC_exit();
	
	/* searching progress start... */
	CIJSC_UI_progress_show( data, CIJSC_PROGRESS_SEARCHING );
	CIJSC_UI_progress_enable_cancel_button( data );
	
	/* update device list cache. */
	cmt_network_update( (void *)CIJSC_UI_progress_update_pulse );
	
	/* disable progress cancel button. */
	CIJSC_UI_progress_update_label( data );
	CIJSC_UI_progress_disable_cancel_button( data );
	
	/* initialize device list. */
	if( CIJSC_init( (void *)CIJSC_UI_progress_update_pulse ) != CMT_STATUS_GOOD ) {
		CIJSC_UI_progress_hide( data );
		return CMT_STATUS_INVAL;
	}
	
	CIJSC_UI_progress_hide( data );
	devnum = ui_dialog_select_init( data );
	
	/* enable dialog_select */
	gtk_widget_set_sensitive( data->dialog_select, TRUE );
	if ( devnum == 0 ) {
		gtk_widget_grab_focus( data->button_select_cancel );
	}
	else {
		gtk_widget_grab_focus( data->button_select_ok );
	}
	
	return CMT_STATUS_GOOD;
}



#endif	/* _SELECTDEVICE_C_ */




