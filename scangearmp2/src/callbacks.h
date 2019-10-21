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

#ifndef	_CALLBACKS_H_
#define	_CALLBACKS_H_

#include <gtk/gtk.h>

#include "support.h"

/*
	main window
*/
G_MODULE_EXPORT gboolean
on_window_main_delete_event(	GtkWidget	*widget,
								GdkEvent	*event,
								SGMP_Data	*data );

G_MODULE_EXPORT gboolean
on_combobox_scanmode_changed(	GtkWidget	*widget,
								SGMP_Data	*data );

G_MODULE_EXPORT void
on_combobox_source_changed(	GtkWidget	*widget,
							SGMP_Data	*data );

G_MODULE_EXPORT void
on_combobox_colormode_changed(	GtkWidget	*widget,
								SGMP_Data	*data );

G_MODULE_EXPORT void
on_combobox_size_changed(	GtkWidget	*widget,
							SGMP_Data	*data );

G_MODULE_EXPORT void
on_button_scanjpeg_clicked(	GtkWidget	*widget,
							SGMP_Data	*data );

G_MODULE_EXPORT void
on_button_scanpdf_clicked(	GtkWidget	*widget,
							SGMP_Data	*data );

G_MODULE_EXPORT void
on_button_version_clicked(	GtkWidget	*widget,
							SGMP_Data	*data );


/*
	select device dialog
*/
G_MODULE_EXPORT gboolean
on_dialog_select_delete_event(	GtkWidget	*widget,
								GdkEvent	*event,
								SGMP_Data	*data );

G_MODULE_EXPORT void
on_button_select_update_clicked(	GtkWidget	*widget,
									SGMP_Data	*data );

G_MODULE_EXPORT void
on_button_select_cancel_clicked(	GtkWidget	*widget,
									SGMP_Data	*data );

G_MODULE_EXPORT void
on_button_select_ok_clicked(	GtkWidget	*widget,
								SGMP_Data	*data );


/*
	save dialog
*/
G_MODULE_EXPORT gboolean
on_dialog_save_delete_event(	GtkWidget	*widget,
								GdkEvent	*event,
								SGMP_Data	*data );

G_MODULE_EXPORT void
on_button_save_save_clicked(	GtkWidget	*widget,
								SGMP_Data	*data );

G_MODULE_EXPORT void
on_button_save_cancel_clicked(	GtkWidget	*widget,
								SGMP_Data	*data );


/*
	version dialog
*/
G_MODULE_EXPORT gboolean
on_dialog_version_delete_event(	GtkWidget	*widget,
								GdkEvent	*event,
								SGMP_Data	*data );

G_MODULE_EXPORT void
on_button_version_ok_clicked(	GtkWidget	*widget,
								SGMP_Data	*data );


/*
	notify dialog
*/
G_MODULE_EXPORT gboolean
on_dialog_notify_delete_event(	GtkWidget	*widget,
								GdkEvent	*event,
								SGMP_Data	*data );

G_MODULE_EXPORT void
on_dialog_notify_hide(	GtkWidget	*widget,
						SGMP_Data	*data );


/*
	progress bar dialog
*/
G_MODULE_EXPORT gboolean
on_dialog_progress_delete_event(	GtkWidget	*widget,
									GdkEvent	*event,
									SGMP_Data	*data );

G_MODULE_EXPORT void
on_button_prog_cancel_clicked(	GtkWidget	*widget,
								SGMP_Data	*data );


/*
	error dialog
*/
G_MODULE_EXPORT gboolean
on_dialog_error_delete_event(	GtkWidget	*widget,
								GdkEvent	*event,
								SGMP_Data	*data );

G_MODULE_EXPORT void
on_button_error_ok_clicked(	GtkWidget	*widget,
							SGMP_Data	*data );

G_MODULE_EXPORT void
on_button_error_cancel_clicked(	GtkWidget	*widget,
								SGMP_Data	*data );


#endif	/*_CALLBACKS_H_*/

