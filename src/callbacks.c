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

#ifndef	_CALLBACKS_C_
#define	_CALLBACKS_C_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "support.h"
#include "callbacks.h"

#include "canon_mfp_tools.h"
#include "mainui.h"
#include "selectdevice.h"
#include "errordlg.h"
#include "scanmain.h"

/*
	main window
*/
G_MODULE_EXPORT gboolean
on_window_main_delete_event(	GtkWidget	*widget,
								GdkEvent	*event,
								SGMP_Data	*data )
{
	DBGMSG("->\n");
	(void)widget;
	(void)event;
	
	if( gtk_widget_get_sensitive ( data->window_main ) ){
		DBGMSG( "[x] sensitive true\n" );
		gtk_main_quit();
  		return TRUE;
	}
	else{
		DBGMSG( "[x] sensitive false\n" );
		return TRUE;
	}
}

G_MODULE_EXPORT gboolean
on_combobox_scanmode_changed(	GtkWidget	*widget,
								SGMP_Data	*data )
{
	(void)widget;
#ifdef _SGMP_DEBUG_VERBOSE_
	DBGMSG("->\n");
#endif
	CIJSC_UI_main_combobox_scanmode_changed( data );
	return FALSE;
}

G_MODULE_EXPORT void
on_combobox_source_changed(	GtkWidget	*widget,
							SGMP_Data	*data )
{
	(void)widget;
	(void)data;
#ifdef _SGMP_DEBUG_VERBOSE_
	DBGMSG("->\n");
#endif
}

G_MODULE_EXPORT void
on_combobox_resolution_changed(	GtkWidget	*widget,
							SGMP_Data	*data )
{
	(void)widget;
	(void)data;
#ifdef _SGMP_DEBUG_VERBOSE_
	DBGMSG("->\n");
#endif
}

G_MODULE_EXPORT void
on_combobox_colormode_changed(	GtkWidget	*widget,
								SGMP_Data	*data )
{
	(void)widget;
	(void)data;
#ifdef _SGMP_DEBUG_VERBOSE_
	DBGMSG("->\n");
#endif
}

G_MODULE_EXPORT void
on_combobox_size_changed(	GtkWidget	*widget,
							SGMP_Data	*data )
{
	(void)widget;
	(void)data;
#ifdef _SGMP_DEBUG_VERBOSE_
	DBGMSG("->\n");
#endif
}

G_MODULE_EXPORT void
on_button_scanjpeg_clicked(	GtkWidget	*widget,
							SGMP_Data	*data )
{
	(void)widget;
	DBGMSG("->\n");
	
	/* format->JPEG */
	CIJSC_UI_main_button_scan_clicked( data, CIJSC_FORMAT_JPEG );
}

G_MODULE_EXPORT void
on_button_scanpdf_clicked(	GtkWidget	*widget,
							SGMP_Data	*data )
{
	(void)widget;
	DBGMSG("->\n");
	
	/* format->PDF */
	CIJSC_UI_main_button_scan_clicked( data, CIJSC_FORMAT_PDF );
}

G_MODULE_EXPORT void
on_button_version_clicked(	GtkWidget	*widget,
							SGMP_Data	*data )
{
	(void)widget;
	DBGMSG("->\n");
	
	gtk_widget_set_sensitive( data->window_main, FALSE );
	gtk_widget_grab_focus( data->button_version_ok );
	gtk_widget_show( data->dialog_version );
	gtk_main();
}

G_MODULE_EXPORT void
on_button_close_clicked(	GtkWidget	*widget,
							SGMP_Data	*data )
{
	(void)widget;
	(void)data;
	DBGMSG("->\n");
	
	gtk_main_quit();
}


/*
	select device dialog
*/
G_MODULE_EXPORT gboolean
on_dialog_select_delete_event(	GtkWidget	*widget,
								GdkEvent	*event,
								SGMP_Data	*data )
{
	(void)widget;
	(void)event;
	DBGMSG("->\n");
	
	on_button_select_cancel_clicked( NULL, data );
	return TRUE;
}

G_MODULE_EXPORT void
on_button_select_update_clicked(	GtkWidget	*widget,
									SGMP_Data	*data )
{
	(void)widget;
	DBGMSG("->\n");
	if ( CIJSC_UI_select_update_list( data ) != CMT_STATUS_GOOD ) {
		data->dialog_select_value = CIJSC_VALUE_CANCEL;
		/* show error dialog. */
		CIJSC_UI_error_show( data, data->dialog_select );
		
		gtk_widget_hide( data->dialog_select );
		gtk_main_quit();	/* go next step */
	}
}

G_MODULE_EXPORT void
on_button_select_cancel_clicked(	GtkWidget	*widget,
									SGMP_Data	*data )
{
	(void)widget;
	DBGMSG("->\n");
	
	gtk_widget_hide( data->dialog_select );
	data->dialog_select_value = CIJSC_VALUE_CANCEL;
	gtk_main_quit();	/* go next step */
}

G_MODULE_EXPORT void
on_button_select_ok_clicked(	GtkWidget	*widget,
								SGMP_Data	*data )
{
	(void)widget;
	DBGMSG("->\n");
	
	gtk_widget_hide( data->dialog_select );
	data->dialog_select_value = CIJSC_VALUE_OK;
	gtk_main_quit();	/* go next step */
}

/*
	save dialog
*/
G_MODULE_EXPORT gboolean
on_dialog_save_delete_event(	GtkWidget	*widget,
								GdkEvent	*event,
								SGMP_Data	*data )
{
	(void)widget;
	(void)event;
	DBGMSG("->\n");

	on_button_save_cancel_clicked( NULL, data );
	
	return TRUE;
}

G_MODULE_EXPORT void
on_button_save_save_clicked(	GtkWidget	*widget,
								SGMP_Data	*data )
{
	(void)widget;
	DBGMSG("->\n");
	
	CIJSC_UI_save_button_save_clicked( data );
}

G_MODULE_EXPORT void
on_button_save_cancel_clicked(	GtkWidget	*widget,
								SGMP_Data	*data )
{
	(void)widget;
	DBGMSG("->\n");
	
	CIJSC_UI_save_button_cancel_clicked( data );
}


/*
	version dialog
*/
G_MODULE_EXPORT gboolean
on_dialog_version_delete_event(	GtkWidget	*widget,
								GdkEvent	*event,
								SGMP_Data	*data )
{
	(void)widget;
	(void)event;
	DBGMSG("->\n");
	
	on_button_version_ok_clicked( NULL, data );
	
	return TRUE;
}

G_MODULE_EXPORT void
on_button_version_ok_clicked(	GtkWidget	*widget,
								SGMP_Data	*data )
{
	(void)widget;
	DBGMSG("->\n");
	
	gtk_widget_hide( data->dialog_version );
	gtk_main_quit();
	gtk_widget_set_sensitive( data->window_main, TRUE );
}


/*
	notify dialog
*/
G_MODULE_EXPORT gboolean
on_dialog_notify_delete_event(	GtkWidget	*widget,
								GdkEvent	*event,
								SGMP_Data	*data )
{
	(void)widget;
	(void)event;
	DBGMSG("->\n");
	gtk_widget_hide( data->dialog_notify );
	
	return TRUE;
}

G_MODULE_EXPORT void
on_dialog_notify_hide(	GtkWidget	*widget,
						SGMP_Data	*data )
{
	(void)widget;
	DBGMSG("->\n");
	
	CIJSC_UI_notify_hide( data );
}


/*
	progress bar dialog
*/
G_MODULE_EXPORT gboolean
on_dialog_progress_delete_event(	GtkWidget	*widget,
									GdkEvent	*event,
									SGMP_Data	*data )
{
	(void)widget;
	(void)event;
	(void)data;
	DBGMSG("->\n");
	return TRUE;
}

G_MODULE_EXPORT void
on_button_prog_cancel_clicked(	GtkWidget	*widget,
								SGMP_Data	*data )
{
	(void)widget;
	DBGMSG("->\n");
	data->dialog_progress_value = CIJSC_VALUE_CANCEL;
	gtk_widget_set_sensitive( data->button_prog_cancel, FALSE );
	while( gtk_events_pending() ){
		gtk_main_iteration();
	}
}


/*
	error dialog
*/
G_MODULE_EXPORT gboolean
on_dialog_error_delete_event(	GtkWidget	*widget,
								GdkEvent	*event,
								SGMP_Data	*data )
{
	(void)widget;
	(void)event;
	DBGMSG("->\n");

	if ( gtk_widget_get_visible ( data->button_error_cancel ) ) {
		on_button_error_cancel_clicked( NULL, data);
	}
	else {
		on_button_error_ok_clicked( NULL, data);
	}

	return TRUE;
}

G_MODULE_EXPORT void
on_button_error_ok_clicked(	GtkWidget	*widget,
							SGMP_Data	*data )
{
	(void)widget;
	DBGMSG("->\n");
	
	gtk_widget_hide( data->dialog_error );
	data->dialog_error_value = CIJSC_VALUE_OK;
	gtk_main_quit();/* go next */
}

G_MODULE_EXPORT void
on_button_error_cancel_clicked(	GtkWidget	*widget,
								SGMP_Data	*data )
{
	(void)widget;
	DBGMSG("->\n");
	
	gtk_widget_hide( data->dialog_error );
	data->dialog_error_value = CIJSC_VALUE_CANCEL;
	gtk_main_quit();/* go next */
}

#endif	/* _CALLBACKS_C_ */




