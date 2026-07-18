#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <jpeglib.h>

#include "image_processing.h"

static void test_identity_curve(void)
{
	CIJSC_ImageSettings settings;
	unsigned char lut[256];
	int i;

	CIJSC_image_settings_reset(&settings);
	CIJSC_image_build_lut(&settings, lut);
	for (i = 0; i < 256; i++)
		assert(lut[i] == i);
}

static void test_levels_and_invert(void)
{
	CIJSC_ImageSettings settings;
	unsigned char lut[256];

	CIJSC_image_settings_reset(&settings);
	settings.black_point = 64;
	settings.white_point = 192;
	CIJSC_image_build_lut(&settings, lut);
	assert(lut[63] == 0);
	assert(lut[64] == 0);
	assert(lut[192] == 255);
	assert(lut[193] == 255);

	CIJSC_image_settings_reset(&settings);
	settings.curve = CIJSC_CURVE_INVERT;
	CIJSC_image_build_lut(&settings, lut);
	assert(lut[0] == 255);
	assert(lut[255] == 0);
}

static void test_brightness_and_contrast(void)
{
	CIJSC_ImageSettings settings;
	unsigned char lut[256];

	CIJSC_image_settings_reset(&settings);
	settings.brightness = 50;
	CIJSC_image_build_lut(&settings, lut);
	assert(lut[128] > 128);

	CIJSC_image_settings_reset(&settings);
	settings.contrast = 50;
	CIJSC_image_build_lut(&settings, lut);
	assert(lut[64] < 64);
	assert(lut[192] > 192);
}

static void create_jpeg(const char *path, unsigned char *pixels, int width, int height)
{
	struct jpeg_compress_struct output;
	struct jpeg_error_mgr error;
	FILE *file = g_fopen(path, "wb");

	assert(file != NULL);
	output.err = jpeg_std_error(&error);
	jpeg_create_compress(&output);
	jpeg_stdio_dest(&output, file);
	output.image_width = width;
	output.image_height = height;
	output.input_components = 3;
	output.in_color_space = JCS_RGB;
	jpeg_set_defaults(&output);
	jpeg_set_quality(&output, 100, TRUE);
	jpeg_start_compress(&output, TRUE);
	while (output.next_scanline < output.image_height) {
		JSAMPROW row = pixels + (size_t)output.next_scanline * width * 3;
		jpeg_write_scanlines(&output, &row, 1);
	}
	jpeg_finish_compress(&output);
	jpeg_destroy_compress(&output);
	fclose(file);
}

static void create_test_jpeg(const char *path)
{
	unsigned char pixels[] = {10, 10, 10, 240, 240, 240};
	create_jpeg(path, pixels, 2, 1);
}

static void read_first_pixel(const char *path, unsigned char pixel[3])
{
	struct jpeg_decompress_struct input;
	struct jpeg_error_mgr error;
	FILE *file = g_fopen(path, "rb");
	unsigned char *row;
	JSAMPROW row_pointer;

	assert(file != NULL);
	input.err = jpeg_std_error(&error);
	jpeg_create_decompress(&input);
	jpeg_stdio_src(&input, file);
	jpeg_read_header(&input, TRUE);
	input.out_color_space = JCS_RGB;
	jpeg_start_decompress(&input);
	row = g_malloc((size_t)input.output_width * 3);
	row_pointer = row;
	jpeg_read_scanlines(&input, &row_pointer, 1);
	pixel[0] = row[0];
	pixel[1] = row[1];
	pixel[2] = row[2];
	g_free(row);
	jpeg_abort_decompress(&input);
	jpeg_destroy_decompress(&input);
	fclose(file);
}

static void test_jpeg_threshold_and_histogram(void)
{
	CIJSC_ImageSettings settings;
	unsigned long histogram[256];
	char *path = NULL;
	int fd = g_file_open_tmp("scangearmp2-image-XXXXXX", &path, NULL);
	int width = 0, height = 0;

	assert(fd >= 0);
	close(fd);
	create_test_jpeg(path);
	CIJSC_image_settings_reset(&settings);
	settings.threshold_enabled = 1;
	settings.threshold = 128;
	assert(CIJSC_image_process_jpeg(path, &settings, histogram, &width, &height) == 0);
	assert(width == 2);
	assert(height == 1);
	assert(histogram[0] == 1);
	assert(histogram[255] == 1);

	create_test_jpeg(path);
	CIJSC_image_settings_reset(&settings);
	settings.auto_tone = 1;
	assert(CIJSC_image_process_jpeg(path, &settings, histogram, &width, &height) == 0);
	assert(histogram[0] == 1);
	assert(histogram[255] == 1);

	create_test_jpeg(path);
	CIJSC_image_settings_reset(&settings);
	settings.scale_percent = 200;
	assert(CIJSC_image_process_jpeg(path, &settings, histogram, &width, &height) == 0);
	assert(width == 4);
	assert(height == 2);
	{
		unsigned long total = 0;
		int i;
		for (i = 0; i < 256; i++)
			total += histogram[i];
		assert(total == 8);
	}

	{
		unsigned char cast_pixels[] = {180, 60, 60, 180, 60, 60};
		unsigned char corrected[3];
		create_jpeg(path, cast_pixels, 2, 1);
		CIJSC_image_settings_reset(&settings);
		settings.color_balance = 1;
		assert(CIJSC_image_process_jpeg(path, &settings, histogram, &width, &height) == 0);
		read_first_pixel(path, corrected);
		assert(abs((int)corrected[0] - corrected[1]) < 10);
		assert(abs((int)corrected[1] - corrected[2]) < 10);
	}

	create_test_jpeg(path);
	CIJSC_image_settings_reset(&settings);
	settings.dust_reduction = 2;
	settings.grain_reduction = 2;
	settings.backlight_correction = 2;
	assert(CIJSC_image_process_jpeg(path, &settings, histogram, &width, &height) == 0);
	g_unlink(path);
	g_free(path);
}

int main(void)
{
	test_identity_curve();
	test_levels_and_invert();
	test_brightness_and_contrast();
	test_jpeg_threshold_and_histogram();
	return 0;
}
