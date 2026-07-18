#include "advanced_ui.h"

#include <math.h>
#include <string.h>

static double clamp_coordinate(double value)
{
	return CLAMP(value, 0.0, 1.0);
}

static void preview_bounds(SGMP_Data *data, int widget_width, int widget_height,
	double *x, double *y, double *width, double *height)
{
	double image_ratio;
	double widget_ratio;
	if (data->scan_w <= 0 || data->scan_h <= 0) {
		*x = *y = 0.0;
		*width = widget_width;
		*height = widget_height;
		return;
	}
	image_ratio = data->scan_w / (double)data->scan_h;
	widget_ratio = widget_width / (double)widget_height;
	if (image_ratio > widget_ratio) {
		*width = widget_width;
		*height = widget_width / image_ratio;
		*x = 0.0;
		*y = (widget_height - *height) / 2.0;
	}
	else {
		*height = widget_height;
		*width = widget_height * image_ratio;
		*x = (widget_width - *width) / 2.0;
		*y = 0.0;
	}
}

static void preview_to_normalized(SGMP_Data *data, int widget_width,
	int widget_height, double pointer_x, double pointer_y, double *x, double *y)
{
	double image_x, image_y, image_width, image_height;
	preview_bounds(data, widget_width, widget_height, &image_x, &image_y,
		&image_width, &image_height);
	*x = clamp_coordinate((pointer_x - image_x) / image_width);
	*y = clamp_coordinate((pointer_y - image_y) / image_height);
}

static void settings_changed(GtkWidget *widget, SGMP_Data *data)
{
	(void)widget;
	data->image_settings.brightness = (int)gtk_range_get_value(GTK_RANGE(data->scale_brightness));
	data->image_settings.contrast = (int)gtk_range_get_value(GTK_RANGE(data->scale_contrast));
	data->image_settings.gamma = gtk_range_get_value(GTK_RANGE(data->scale_gamma));
	data->image_settings.black_point = (int)gtk_range_get_value(GTK_RANGE(data->scale_black_point));
	data->image_settings.white_point = (int)gtk_range_get_value(GTK_RANGE(data->scale_white_point));
	data->image_settings.curve = gtk_combo_box_get_active(GTK_COMBO_BOX(data->combobox_curve));
	data->image_settings.threshold_enabled = gtk_check_button_get_active(GTK_CHECK_BUTTON(data->check_threshold));
	data->image_settings.threshold = (int)gtk_range_get_value(GTK_RANGE(data->scale_threshold));
	data->image_settings.unsharp = gtk_check_button_get_active(GTK_CHECK_BUTTON(data->check_unsharp));
	data->image_settings.descreen = gtk_check_button_get_active(GTK_CHECK_BUTTON(data->check_descreen));
	gtk_widget_set_sensitive(data->scale_threshold, data->image_settings.threshold_enabled);
	gtk_widget_queue_draw(data->histogram_area);
}

static void reset_adjustments(GtkButton *button, SGMP_Data *data)
{
	(void)button;
	CIJSC_image_settings_reset(&data->image_settings);
	gtk_range_set_value(GTK_RANGE(data->scale_brightness), 0);
	gtk_range_set_value(GTK_RANGE(data->scale_contrast), 0);
	gtk_range_set_value(GTK_RANGE(data->scale_gamma), 1.0);
	gtk_range_set_value(GTK_RANGE(data->scale_black_point), 0);
	gtk_range_set_value(GTK_RANGE(data->scale_white_point), 255);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->combobox_curve), CIJSC_CURVE_LINEAR);
	gtk_check_button_set_active(GTK_CHECK_BUTTON(data->check_threshold), FALSE);
	gtk_range_set_value(GTK_RANGE(data->scale_threshold), 128);
	gtk_check_button_set_active(GTK_CHECK_BUTTON(data->check_unsharp), FALSE);
	gtk_check_button_set_active(GTK_CHECK_BUTTON(data->check_descreen), FALSE);
	settings_changed(NULL, data);
}

static void reset_crop(GtkButton *button, SGMP_Data *data)
{
	(void)button;
	data->crop_enabled = FALSE;
	data->crop_x = data->crop_y = 0.0;
	data->crop_width = data->crop_height = 1.0;
	gtk_widget_queue_draw(data->preview_crop_area);
}

static void crop_draw(GtkDrawingArea *area, cairo_t *cr, int width, int height,
	SGMP_Data *data)
{
	(void)area;
	if (!data->crop_enabled)
		return;
	{
		double image_x, image_y, image_width, image_height;
		preview_bounds(data, width, height, &image_x, &image_y,
			&image_width, &image_height);
	cairo_set_source_rgba(cr, 0.95, 0.2, 0.15, 0.95);
	cairo_set_line_width(cr, 2.0);
	cairo_rectangle(cr,
		image_x + (data->crop_x - data->preview_source_x) /
			data->preview_source_width * image_width,
		image_y + (data->crop_y - data->preview_source_y) /
			data->preview_source_height * image_height,
		data->crop_width / data->preview_source_width * image_width,
		data->crop_height / data->preview_source_height * image_height);
	cairo_stroke(cr);
	}
}

static void crop_drag_begin(GtkGestureDrag *gesture, double x, double y,
	SGMP_Data *data)
{
	GtkWidget *widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
	int width = gtk_widget_get_width(widget);
	int height = gtk_widget_get_height(widget);
	if (!gtk_widget_get_visible(data->preview_picture) || width <= 0 || height <= 0)
		return;
	preview_to_normalized(data, width, height, x, y,
		&data->crop_drag_x, &data->crop_drag_y);
	data->crop_drag_x = data->preview_source_x +
		data->crop_drag_x * data->preview_source_width;
	data->crop_drag_y = data->preview_source_y +
		data->crop_drag_y * data->preview_source_height;
	data->crop_x = data->crop_drag_x;
	data->crop_y = data->crop_drag_y;
	data->crop_width = data->crop_height = 0.0;
	data->crop_enabled = TRUE;
}

static void crop_drag_update(GtkGestureDrag *gesture, double offset_x,
	double offset_y, SGMP_Data *data)
{
	GtkWidget *widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
	int width = gtk_widget_get_width(widget);
	int height = gtk_widget_get_height(widget);
	double current_x, current_y, start_widget_x, start_widget_y;
	double image_x, image_y, image_width, image_height;
	if (!data->crop_enabled || width <= 0 || height <= 0)
		return;
	preview_bounds(data, width, height, &image_x, &image_y, &image_width, &image_height);
	start_widget_x = image_x + (data->crop_drag_x - data->preview_source_x) /
		data->preview_source_width * image_width;
	start_widget_y = image_y + (data->crop_drag_y - data->preview_source_y) /
		data->preview_source_height * image_height;
	preview_to_normalized(data, width, height, start_widget_x + offset_x,
		start_widget_y + offset_y, &current_x, &current_y);
	current_x = data->preview_source_x + current_x * data->preview_source_width;
	current_y = data->preview_source_y + current_y * data->preview_source_height;
	data->crop_x = MIN(data->crop_drag_x, current_x);
	data->crop_y = MIN(data->crop_drag_y, current_y);
	data->crop_width = fabs(current_x - data->crop_drag_x);
	data->crop_height = fabs(current_y - data->crop_drag_y);
	gtk_widget_queue_draw(widget);
}

static void crop_drag_end(GtkGestureDrag *gesture, double offset_x,
	double offset_y, SGMP_Data *data)
{
	(void)offset_x;
	(void)offset_y;
	if (data->crop_width < 0.02 || data->crop_height < 0.02)
		data->crop_enabled = FALSE;
	gtk_widget_queue_draw(gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture)));
}

static void histogram_draw(GtkDrawingArea *area, cairo_t *cr, int width,
	int height, SGMP_Data *data)
{
	unsigned long maximum = 0;
	unsigned char lut[256];
	int i;
	(void)area;
	cairo_set_source_rgb(cr, 0.12, 0.12, 0.12);
	cairo_paint(cr);
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.12);
	cairo_set_line_width(cr, 1.0);
	for (i = 1; i < 4; i++) {
		cairo_move_to(cr, width * i / 4.0, 0);
		cairo_line_to(cr, width * i / 4.0, height);
		cairo_move_to(cr, 0, height * i / 4.0);
		cairo_line_to(cr, width, height * i / 4.0);
	}
	cairo_stroke(cr);
	if (data->histogram_valid) {
		for (i = 0; i < 256; i++)
			maximum = MAX(maximum, data->histogram[i]);
		if (maximum > 0) {
			cairo_set_source_rgb(cr, 0.35, 0.75, 1.0);
			cairo_set_line_width(cr, MAX(1.0, width / 256.0));
			for (i = 0; i < 256; i++) {
				double x = i * (width - 1) / 255.0;
				double bar = data->histogram[i] * (height - 2) / (double)maximum;
				cairo_move_to(cr, x, height - 1);
				cairo_line_to(cr, x, height - 1 - bar);
			}
			cairo_stroke(cr);
		}
	}
	CIJSC_image_build_lut(&data->image_settings, lut);
	cairo_set_source_rgb(cr, 1.0, 0.55, 0.2);
	cairo_set_line_width(cr, 2.0);
	cairo_move_to(cr, 0, height - 1 - lut[0] * (height - 2) / 255.0);
	for (i = 1; i < 256; i++)
		cairo_line_to(cr, i * (width - 1) / 255.0,
			height - 1 - lut[i] * (height - 2) / 255.0);
	cairo_stroke(cr);
}

void CIJSC_advanced_ui_image_updated(SGMP_Data *data)
{
	gtk_widget_queue_draw(data->histogram_area);
	gtk_widget_queue_draw(data->preview_crop_area);
}

void CIJSC_advanced_ui_init(SGMP_Data *data)
{
	GtkGesture *drag;
	GtkWidget *ranges[] = {
		data->scale_brightness, data->scale_contrast, data->scale_gamma,
		data->scale_black_point, data->scale_white_point, data->scale_threshold
	};
	size_t i;

	CIJSC_image_settings_reset(&data->image_settings);
	data->histogram_valid = FALSE;
	data->crop_enabled = FALSE;
	data->crop_width = data->crop_height = 1.0;
	data->preview_source_width = data->preview_source_height = 1.0;
	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(data->preview_crop_area),
		(GtkDrawingAreaDrawFunc)crop_draw, data, NULL);
	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(data->histogram_area),
		(GtkDrawingAreaDrawFunc)histogram_draw, data, NULL);
	drag = gtk_gesture_drag_new();
	g_signal_connect(drag, "drag-begin", G_CALLBACK(crop_drag_begin), data);
	g_signal_connect(drag, "drag-update", G_CALLBACK(crop_drag_update), data);
	g_signal_connect(drag, "drag-end", G_CALLBACK(crop_drag_end), data);
	gtk_widget_add_controller(data->preview_crop_area, GTK_EVENT_CONTROLLER(drag));

	for (i = 0; i < G_N_ELEMENTS(ranges); i++)
		g_signal_connect(ranges[i], "value-changed", G_CALLBACK(settings_changed), data);
	g_signal_connect(data->combobox_curve, "changed", G_CALLBACK(settings_changed), data);
	g_signal_connect(data->check_threshold, "toggled", G_CALLBACK(settings_changed), data);
	g_signal_connect(data->check_unsharp, "toggled", G_CALLBACK(settings_changed), data);
	g_signal_connect(data->check_descreen, "toggled", G_CALLBACK(settings_changed), data);
	g_signal_connect(data->button_reset_adjustments, "clicked", G_CALLBACK(reset_adjustments), data);
	g_signal_connect(data->button_reset_crop, "clicked", G_CALLBACK(reset_crop), data);
	reset_adjustments(NULL, data);
}
