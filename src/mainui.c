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

#ifndef	_MAINUI_C_
#define	_MAINUI_C_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include "support.h"
#include "callbacks.h"
#include "canon_mfp_tools.h"
#include "cnmsstrings.h"

#include "mainui.h"
#include "scanmain.h"

typedef struct {
	const int		id;
	const char		*str;
} CIJSC_MAINUI_ITEM_TABLE;

/* scanmode */
static CIJSC_MAINUI_ITEM_TABLE scanmode_table[] = {
	{ CIJSC_SCANMODE_PLATEN,	STR_CNMS_LS_002_01, },
	{ CIJSC_SCANMODE_ADF_S,		STR_CNMS_LS_002_02, },
	{ CIJSC_SCANMODE_ADF_D_L,	STR_CNMS_LS_002_03, },
	{ CIJSC_SCANMODE_ADF_D_S,	STR_CNMS_LS_002_04, },
	{ -1,						NULL, },
};

/* select source */
static CIJSC_MAINUI_ITEM_TABLE source_table[] = {
	{ CIJSC_SOURCE_DOCUMENT,	STR_CNMS_LS_003_01, },
	{ CIJSC_SOURCE_PHOTO,		STR_CNMS_LS_003_02, },
	{ -1,						NULL, },
};

/* resolution list */
static CIJSC_MAINUI_ITEM_TABLE resolution_table[] = {
       { 0,    "75", },
       { 1,    "150", },
       { 2,    "300", },
       { 3,    "600", },
       { 4,    "1200", },
       { -1,                                           NULL, },
};

/* color mode */
static CIJSC_MAINUI_ITEM_TABLE colormode_table[] = {
	{ CIJSC_COLOR_COLOR,		STR_CNMS_LS_004_01, },
	{ CIJSC_COLOR_GRAY,			STR_CNMS_LS_004_02, },
	{ -1,						NULL, },
};

/* size(Platen) */
static CIJSC_MAINUI_ITEM_TABLE size_platen_table[] = {
	{ CIJSC_SIZE_CARD,			STR_CNMS_LS_005_01, },
	{ CIJSC_SIZE_L_L,			STR_CNMS_LS_005_02, },
	{ CIJSC_SIZE_L_P,			STR_CNMS_LS_005_03, },
	{ CIJSC_SIZE_4X6_L,			STR_CNMS_LS_005_04, },
	{ CIJSC_SIZE_4X6_P,			STR_CNMS_LS_005_05, },
	{ CIJSC_SIZE_HAGAKI_L,		STR_CNMS_LS_005_06, },
	{ CIJSC_SIZE_HAGAKI_P,		STR_CNMS_LS_005_07, },
	{ CIJSC_SIZE_2L_L,			STR_CNMS_LS_005_08, },
	{ CIJSC_SIZE_2L_P,			STR_CNMS_LS_005_09, },
	{ CIJSC_SIZE_A5,			STR_CNMS_LS_005_10, },
	{ CIJSC_SIZE_B5,			STR_CNMS_LS_005_11, },
	{ CIJSC_SIZE_A4,			STR_CNMS_LS_005_12, },
	{ CIJSC_SIZE_LETTER,		STR_CNMS_LS_005_13, },
	{ -1,						NULL, },
};

/* size(ADF) */
static CIJSC_MAINUI_ITEM_TABLE size_adf_table[] = {
	{ CIJSC_SIZE_A4,			STR_CNMS_LS_005_12, },
	{ CIJSC_SIZE_LETTER,		STR_CNMS_LS_005_13, },
	{ -1,						NULL, },
};

int is_flatbed = 0;
int is_adf = 0;
int is_duplex = 0;

static int ui_main_is_en_US( void )
{
	char	*env_locale = NULL;

	DBGMSG("->\n");
	/* get locale string */
#ifdef USE_PO_LOCALE
	env_locale = _( "Locale" );
#else
	if( ( env_locale = (char *)getenv( "LC_PAPER" ) ) == NULL ){
		if( ( env_locale = (char *)getenv( "LANG" ) ) == NULL ){
			env_locale = "";
		}
	}
	env_locale = (char *)strsep( &env_locale, "." );
#endif
	/* return local */
	if( strcasecmp( env_locale, "en_US" ) == 0 ){
		return TRUE;
	}

	return	FALSE;
}

static void ui_main_combobox_scanmode_init( SGMP_Data *data, CANON_Device const *dev )
{
	DBGMSG("->\n");
	(void)dev;
        gtk_combo_box_text_remove_all ( GTK_COMBO_BOX_TEXT ( data->combobox_scanmode ) );
	
	if ( is_flatbed ) {
                gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (data->combobox_scanmode ), gettext( STR_CNMS_LS_002_01 ));
	}
	if ( is_adf ) {
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (data->combobox_scanmode ), gettext( STR_CNMS_LS_002_02 ));
	}
	if ( is_duplex ) {
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (data->combobox_scanmode ), gettext( STR_CNMS_LS_002_03 ));
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (data->combobox_scanmode ), gettext( STR_CNMS_LS_002_04 ));
	}
	
	gtk_combo_box_set_active( GTK_COMBO_BOX( data->combobox_scanmode ) , 0 );
}

static void ui_main_other_combobox_init( SGMP_Data *data, GtkWidget *combo, CIJSC_MAINUI_ITEM_TABLE *table )
{
	int					i;
	
	(void)data;
	DBGMSG("->\n");
        gtk_combo_box_text_remove_all ( GTK_COMBO_BOX_TEXT( combo ) );
	
	for( i = 0 ; table[i].id >= 0; i++ ) {
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT ( combo ), gettext( table[i].str ) );
	}
	gtk_combo_box_set_active( GTK_COMBO_BOX( combo ) , 0 );
}

static int ui_main_combobox_get_id( SGMP_Data *data, GtkWidget *combo, CIJSC_MAINUI_ITEM_TABLE *table )
{
	GtkListStore	*store;
	GtkTreeIter		iter;
	gint			select_no, i;
	gchar			*str = NULL;

	(void)data;
	DBGMSG("->\n");
	select_no = gtk_combo_box_get_active( GTK_COMBO_BOX( combo ) );
	store = GTK_LIST_STORE( gtk_combo_box_get_model( GTK_COMBO_BOX( combo ) ) );
	
	gtk_tree_model_iter_nth_child( GTK_TREE_MODEL( store ), &iter, NULL, select_no );
	gtk_tree_model_get( GTK_TREE_MODEL( store ), &iter, 0, &str, -1);
	
	for( i = 0 ; table[i].id >= 0; i++ ) {
		if( strcmp( str, gettext( table[i].str ) ) == 0 ){
			break;
		}
	}
	DBGMSG("selected item : %s(%d)\n", gettext( table[i].str ), select_no );
	
	return (int)table[i].id;
}

static int ui_main_combobox_set_id( SGMP_Data *data, GtkWidget *combo, CIJSC_MAINUI_ITEM_TABLE *table, int id )
{
	GtkListStore	*store;
	GtkTreeIter		iter;
	gint			i, table_index = -1, combo_index = -1;
	gchar			*str = NULL;
        (void) data;
	DBGMSG("->\n");
	for( i = 0 ; table[i].id >= 0; i++ ) {
		if( table[i].id == id ) {
			table_index = i;
			break;
		}
	}
	if ( table[i].id < 0 ) {
		return -1;
	}
	
	store = GTK_LIST_STORE( gtk_combo_box_get_model( GTK_COMBO_BOX( combo ) ) );
	for( i = 0 ; ; i++ ) {
		if ( gtk_tree_model_iter_nth_child( GTK_TREE_MODEL( store ), &iter, NULL, i ) ) {
			gtk_tree_model_get( GTK_TREE_MODEL( store ), &iter, 0, &str, -1);
			if( strcmp( str, gettext( table[table_index].str ) ) == 0 ){
				combo_index = i;
				break;
			}
		}
		else{
			return -1;
		}
	}
	
	gtk_combo_box_set_active( GTK_COMBO_BOX( combo ) , combo_index );
	
	return 0;
}

static void ui_main_combobox_set_default_size( SGMP_Data *data )
{
	DBGMSG("->\n");
	if ( ui_main_is_en_US() ) {
		ui_main_combobox_set_id( data, data->combobox_size, size_adf_table, CIJSC_SIZE_LETTER );
	}
	else {
		ui_main_combobox_set_id( data, data->combobox_size, size_adf_table, CIJSC_SIZE_A4 );
	}
}

static void ui_main_button_scan_main( SGMP_Data *data )
{
	DBGMSG("->\n");
	gtk_widget_set_sensitive( data->window_main, FALSE );
	/* set scan parameters. */
	data->scan_scanmode = ui_main_combobox_get_id( data, data->combobox_scanmode, (CIJSC_MAINUI_ITEM_TABLE *)scanmode_table );
	data->scan_source = ui_main_combobox_get_id( data, data->combobox_source, (CIJSC_MAINUI_ITEM_TABLE *)source_table );
	data->scan_resolution = ui_main_combobox_get_id( data, data->combobox_resolution, (CIJSC_MAINUI_ITEM_TABLE *)resolution_table );
	data->scan_color = ui_main_combobox_get_id( data, data->combobox_colormode, (CIJSC_MAINUI_ITEM_TABLE *)colormode_table );
	data->scan_size = ui_main_combobox_get_id( data, data->combobox_size, (CIJSC_MAINUI_ITEM_TABLE *)size_platen_table );
	
	/* scan and save scanned data. */
	CIJSC_Scan_And_Save( data );
	gtk_widget_set_sensitive( data->window_main, TRUE );
}

void CIJSC_UI_main_show( SGMP_Data	*data, CANON_Device const *dev )
{
	DBGMSG("->\n");
	/* set fullname of selected device. */
	gtk_label_set_text( GTK_LABEL ( data->label_devname ), dev->fullname );
	DBGMSG("selected device : id = 0x%04X, type = 0x%04X\n", dev->product_id, dev->type);
	
	/* init combobox. */
	data->ignore_combobox_changed = TRUE;
	ui_main_other_combobox_init( data, data->combobox_source, source_table );
	ui_main_other_combobox_init( data, data->combobox_resolution, resolution_table );
	ui_main_other_combobox_init( data, data->combobox_colormode, colormode_table );
	ui_main_other_combobox_init( data, data->combobox_size, size_platen_table );
	ui_main_combobox_scanmode_init( data, dev );
	/* set default size. */
	ui_main_combobox_set_default_size( data );
	/* save current scanmode. */
	data->prev_scanmode = ui_main_combobox_get_id( data, data->combobox_scanmode, (CIJSC_MAINUI_ITEM_TABLE *)scanmode_table );
	data->ignore_combobox_changed = FALSE;
	
	/* show main ui. */
	gtk_widget_show( data->window_main );
	gtk_main();
	/* return to main() */
}

void CIJSC_UI_main_combobox_scanmode_changed( SGMP_Data	*data )
{
	int				current_scanmode;
	int				current_size;

	DBGMSG("->\n");
	if ( !data->ignore_combobox_changed ) {
		current_scanmode = ui_main_combobox_get_id( data, data->combobox_scanmode, (CIJSC_MAINUI_ITEM_TABLE *)scanmode_table );
		DBGMSG("scanmode changed : %s -> %s\n", 
				gettext( scanmode_table[data->prev_scanmode].str ),
				gettext( scanmode_table[current_scanmode].str ) );
		
		/* update Scan(JPEG) button. */
		if ( current_scanmode == CIJSC_SCANMODE_PLATEN ) {
			gtk_widget_show( data->button_scanjpeg );
		}
		else {
			gtk_widget_hide( data->button_scanjpeg );
		}
		if ( data->prev_scanmode == CIJSC_SCANMODE_PLATEN ) {
			/* PLATEN -> ADF */
			/* get current size and set current size. */
			current_size = ui_main_combobox_get_id( data, data->combobox_size, (CIJSC_MAINUI_ITEM_TABLE *)size_platen_table );
			ui_main_other_combobox_init( data, data->combobox_size, size_adf_table );
			if ( ui_main_combobox_set_id( data, data->combobox_size, (CIJSC_MAINUI_ITEM_TABLE *)size_adf_table, current_size ) ) {
				/* set default size. */
				ui_main_combobox_set_default_size( data );
			}
		}
		else {
			if ( current_scanmode == CIJSC_SCANMODE_PLATEN ) {
				/* ADF -> PLATEN */
				/* get current size and set current size. */
				current_size = ui_main_combobox_get_id( data, data->combobox_size, (CIJSC_MAINUI_ITEM_TABLE *)size_adf_table );
				ui_main_other_combobox_init( data, data->combobox_size, size_platen_table );
				if ( ui_main_combobox_set_id( data, data->combobox_size, (CIJSC_MAINUI_ITEM_TABLE *)size_platen_table, current_size ) ) {
					/* set default size. */
					ui_main_combobox_set_default_size( data );
				}
			}
			/* ADF -> ADF : do nothing. */
		}
		/* update prev_scanmode. */
		data->prev_scanmode = current_scanmode;
	}
}

void CIJSC_UI_main_button_scan_clicked( SGMP_Data *data, int format )
{
	DBGMSG("->\n");
	/* set file format. */
	data->scan_format = format;
	
	/* set scan parameters, start scan and save scanned data. */
	ui_main_button_scan_main( data );
}


#endif	/* _MAINUI_C_ */




