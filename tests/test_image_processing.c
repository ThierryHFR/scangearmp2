#include <assert.h>
#include <stdio.h>
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

static void create_test_jpeg(const char *path)
{
	struct jpeg_compress_struct output;
	struct jpeg_error_mgr error;
	unsigned char pixels[] = {10, 10, 10, 240, 240, 240};
	FILE *file = g_fopen(path, "wb");
	JSAMPROW row = pixels;

	assert(file != NULL);
	output.err = jpeg_std_error(&error);
	jpeg_create_compress(&output);
	jpeg_stdio_dest(&output, file);
	output.image_width = 2;
	output.image_height = 1;
	output.input_components = 3;
	output.in_color_space = JCS_RGB;
	jpeg_set_defaults(&output);
	jpeg_set_quality(&output, 100, TRUE);
	jpeg_start_compress(&output, TRUE);
	jpeg_write_scanlines(&output, &row, 1);
	jpeg_finish_compress(&output);
	jpeg_destroy_compress(&output);
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
