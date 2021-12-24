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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <locale.h>

#define _GLOBALS_
#include "errors.h"

#include "support.h"
#include "callbacks.h"

#include "cnmsstrings.h"
#include "canon_mfp_tools.h"

#include "mainui.h"
#include "selectdevice.h"
#include "keep_setting.h"
#include "errordlg.h"

#define WAIT_SECOND				{usleep(1000000);}

static void create_combobox(
	SGMP_Data	*data,
	GtkWidget	*combobox )
{
        (void)data;
        (void)combobox;
/*
	GtkListStore		*store;
	GtkCellRenderer		*renderer;
	
	data->ignore_combobox_changed = TRUE;
	
	store = gtk_list_store_new( 1, G_TYPE_STRING );
	gtk_combo_box_set_model( GTK_COMBO_BOX( combobox ), GTK_TREE_MODEL( store ) );
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT( combobox ), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT( combobox ), renderer, "text", 0, NULL);
	g_object_unref( store );
	g_object_unref( renderer );
*/	
	data->ignore_combobox_changed = FALSE;
}


int main(int argc, char **argv )
{
	SGMP_Data		*data;
	GError			*error = NULL;
	char 			*home_dir = NULL;
	char			strbuf[PATH_MAX];
	CANON_Device const	*selected = NULL;

	char bin_path[PATH_MAX];
	ssize_t bin_path_len = readlink("/proc/self/exe", bin_path, PATH_MAX);
	bin_path[bin_path_len] = '\0';
	char *bin_dir_path = dirname(bin_path);
	char pkg_locale_dir[PATH_MAX];
	snprintf(pkg_locale_dir, PATH_MAX, "%s/../share/locale", bin_dir_path);
#ifdef ENABLE_NLS
        setlocale(LC_ALL, "");
        bindtextdomain( "scangearmp2", pkg_locale_dir );
        bind_textdomain_codeset( "scangearmp2", "UTF-8" );
        textdomain( "scangearmp2" );
#endif
	
	setlocale(LC_ALL,"");

	gtk_init( &argc, &argv );
	
	data = g_slice_new( SGMP_Data );
	data->builder = gtk_builder_new();
	
	char glade_path[PATH_MAX];
	snprintf(glade_path, PATH_MAX, "%s/%s/scangearmp2.glade", bin_dir_path, PACKAGE_DATA_DIR);
	if( ! gtk_builder_add_from_file( data->builder, glade_path, &error ) )
	{
		g_warning( "%s", error->message );
		g_slice_free(SGMP_Data, data);
		return 1;
	}
	
	/* main window */
	data->window_main = GTK_WIDGET( gtk_builder_get_object( data->builder, "window_main" ) );
	data->combobox_scanmode = GTK_WIDGET( gtk_builder_get_object( data->builder, "combobox_scanmode" ) );
	data->combobox_resolution = GTK_WIDGET( gtk_builder_get_object( data->builder, "combobox_resolution" ) );
	data->combobox_source = GTK_WIDGET( gtk_builder_get_object( data->builder, "combobox_source" ) );
	data->combobox_colormode = GTK_WIDGET( gtk_builder_get_object( data->builder, "combobox_colormode" ) );
	data->combobox_size = GTK_WIDGET( gtk_builder_get_object( data->builder, "combobox_size" ) );
	data->button_scanjpeg = GTK_WIDGET( gtk_builder_get_object( data->builder, "button_scanjpeg" ) );
	data->button_scanpdf = GTK_WIDGET( gtk_builder_get_object( data->builder, "button_scanpdf" ) );
	data->button_version = GTK_WIDGET( gtk_builder_get_object( data->builder, "button_version" ) );
	data->button_close = GTK_WIDGET( gtk_builder_get_object( data->builder, "button_close" ) );
	data->label_devname = GTK_WIDGET( gtk_builder_get_object( data->builder, "label_devname" ) );
	
	/* select device dialog */
	data->dialog_select = GTK_WIDGET( gtk_builder_get_object( data->builder, "dialog_select" ) );
	data->combobox_select_devlist = GTK_WIDGET( gtk_builder_get_object( data->builder, "combobox_select_devlist" ) );
	data->label_select_nodev = GTK_WIDGET( gtk_builder_get_object( data->builder, "label_select_nodev" ) );
	data->button_select_update = GTK_WIDGET( gtk_builder_get_object( data->builder, "button_select_update" ) );
	data->button_select_cancel = GTK_WIDGET( gtk_builder_get_object( data->builder, "button_select_cancel" ) );
	data->button_select_ok = GTK_WIDGET( gtk_builder_get_object( data->builder, "button_select_ok" ) );
	
	/* save dialog */
	data->dialog_save = GTK_WIDGET( gtk_builder_get_object( data->builder, "dialog_save" ) );
	data->filechooserwidget_save = GTK_WIDGET( gtk_builder_get_object( data->builder, "filechooserwidget_save" ) );
	data->button_save_cancel = GTK_WIDGET( gtk_builder_get_object( data->builder, "button_save_cancel" ) );
	data->button_save_save = GTK_WIDGET( gtk_builder_get_object( data->builder, "button_save_save" ) );
	
	/* version dialog */
	data->dialog_version = GTK_WIDGET( gtk_builder_get_object( data->builder, "dialog_version" ) );
	data->label_version_app = GTK_WIDGET( gtk_builder_get_object( data->builder, "label_version_app" ) );
	data->label_version_ver = GTK_WIDGET( gtk_builder_get_object( data->builder, "label_version_ver" ) );
	data->label_version_cpy = GTK_WIDGET( gtk_builder_get_object( data->builder, "label_version_cpy" ) );
	data->button_version_ok = GTK_WIDGET( gtk_builder_get_object( data->builder, "button_version_ok" ) );
	
	/* notify dialog */
	data->dialog_notify = GTK_WIDGET( gtk_builder_get_object( data->builder, "dialog_notify" ) );
	data->label_notify_save = GTK_WIDGET( gtk_builder_get_object( data->builder, "label_notify_save" ) );
	
	/* progress bar dialog */
	data->dialog_progress = GTK_WIDGET( gtk_builder_get_object( data->builder, "dialog_progress" ) );
	data->progressbar_prog = GTK_WIDGET( gtk_builder_get_object( data->builder, "progressbar_prog" ) );
	data->label_prog_msg = GTK_WIDGET( gtk_builder_get_object( data->builder, "label_prog_msg" ) );
	data->label_prog_percent = GTK_WIDGET( gtk_builder_get_object( data->builder, "label_prog_percent" ) );
	data->label_prog_pages = GTK_WIDGET( gtk_builder_get_object( data->builder, "label_prog_pages" ) );
	data->button_prog_cancel = GTK_WIDGET( gtk_builder_get_object( data->builder, "button_prog_cancel" ) );
	
	/* error dialog */
	data->dialog_error = GTK_WIDGET( gtk_builder_get_object( data->builder, "dialog_error" ) );
	data->label_error_msg = GTK_WIDGET( gtk_builder_get_object( data->builder, "label_error_msg" ) );
	data->button_error_cancel = GTK_WIDGET( gtk_builder_get_object( data->builder, "button_error_cancel" ) );
	data->button_error_ok = GTK_WIDGET( gtk_builder_get_object( data->builder, "button_error_ok" ) );
	
	/* connect signals */
	gtk_builder_connect_signals( data->builder, data );
	
	/* set version and year */
	{
		gtk_label_set_text( GTK_LABEL ( data->label_version_app ), STR_CNMS_LS_007_02 );
		snprintf( (char *)strbuf, sizeof(strbuf), gettext( STR_CNMS_LS_007_03 ), STR_CNMS_LS_007_VER_NUM );
		gtk_label_set_text( GTK_LABEL ( data->label_version_ver ), strbuf );
		snprintf( (char *)strbuf, sizeof(strbuf), gettext( STR_CNMS_LS_007_04 ), STR_CNMS_LS_007_YEAR_BEGIN, STR_CNMS_LS_007_YEAR_END );
		gtk_label_set_text( GTK_LABEL ( data->label_version_cpy ), strbuf );
	}
	
	/* create combobox */
	create_combobox( data, data->combobox_scanmode );
	create_combobox( data, data->combobox_resolution );
	create_combobox( data, data->combobox_source );
	create_combobox( data, data->combobox_colormode );
	create_combobox( data, data->combobox_size );
	create_combobox( data, data->combobox_select_devlist );
	
	/* set label(notify dialog). */
	gtk_label_set_text( GTK_LABEL ( data->label_notify_save ), gettext( STR_CNMS_LS_009_03 ) );
	
	if( ( home_dir = getenv("HOME") ) != NULL ) {
		gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER( data->filechooserwidget_save ), home_dir );
	}
	
	gtk_window_set_default_size( GTK_WINDOW( data->dialog_save ), gdk_screen_width() * 0.35, gdk_screen_height() * 0.35 );
	
	g_object_unref( G_OBJECT( data->builder ) );
	
	/* initialize device list. */
	if( CIJSC_init( (void *)NULL ) != CMT_STATUS_GOOD ) {
		/* show error dialog. */
		CIJSC_UI_error_show( data, NULL );
		goto EXIT;
	}
	
	/* initialize selected device cache. */
	if( KeepSettingCommonOpen() != CNMS_NO_ERR ){
		DBGMSG( "[CnmsScanInit]Error is occured in KeepSettingCommonOpen.\n" );
		/* show error dialog. */
		CIJSC_UI_error_show( data, NULL );
		goto EXIT;
	}
	
	/* show dialog_select. */
	selected = CIJSC_UI_select_show( data );
	if ( data->dialog_select_value == CIJSC_VALUE_OK && selected ) {
		/* open selected device. */
		if( CIJSC_open( selected->name ) == CMT_STATUS_GOOD ) {
			/* show main ui. */
			CIJSC_UI_main_show( data, selected );
			/* close selected device. */
			CIJSC_close();
		}
		else {
			/* error */
			lastBackendErrCode = BERRCODE_CONNECT_FAILED;
			/* show error dialog. */
			CIJSC_UI_error_show( data, NULL );
		}
	}
EXIT:
	
	CIJSC_exit();
	KeepSettingCommonClose();
	
	g_slice_free(SGMP_Data, data);
	
	return 0;
}
