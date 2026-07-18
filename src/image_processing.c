#include "image_processing.h"

#include <errno.h>
#include <math.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <jpeglib.h>

typedef struct {
	struct jpeg_error_mgr base;
	jmp_buf jump;
} CIJSC_JpegError;

static void image_jpeg_error_exit(j_common_ptr info)
{
	CIJSC_JpegError *error = (CIJSC_JpegError *)info->err;
	longjmp(error->jump, 1);
}

static double clamp_unit(double value)
{
	if (value < 0.0)
		return 0.0;
	if (value > 1.0)
		return 1.0;
	return value;
}

void CIJSC_image_settings_reset(CIJSC_ImageSettings *settings)
{
	memset(settings, 0, sizeof(*settings));
	settings->gamma = 1.0;
	settings->white_point = 255;
	settings->threshold = 128;
}

void CIJSC_image_build_lut(const CIJSC_ImageSettings *settings,
	unsigned char lut[256])
{
	int i;
	int black = CLAMP(settings->black_point, 0, 254);
	int white = CLAMP(settings->white_point, black + 1, 255);
	double gamma = CLAMP(settings->gamma, 0.1, 5.0);
	double contrast = (100.0 + CLAMP(settings->contrast, -100, 100)) / 100.0;
	double brightness = CLAMP(settings->brightness, -100, 100) / 200.0;

	for (i = 0; i < 256; i++) {
		double value = clamp_unit((i - black) / (double)(white - black));

		switch (settings->curve) {
		case CIJSC_CURVE_S:
			value = value * value * (3.0 - 2.0 * value);
			break;
		case CIJSC_CURVE_HIGH_KEY:
			value = sqrt(value);
			break;
		case CIJSC_CURVE_LOW_KEY:
			value *= value;
			break;
		case CIJSC_CURVE_INVERT:
			value = 1.0 - value;
			break;
		default:
			break;
		}

		value = pow(clamp_unit(value), 1.0 / gamma);
		value = (value - 0.5) * contrast + 0.5 + brightness;
		lut[i] = (unsigned char)(clamp_unit(value) * 255.0 + 0.5);
	}
}

static unsigned char *box_blur(const unsigned char *pixels, int width, int height)
{
	size_t size = (size_t)width * height * 3;
	unsigned char *blurred = g_try_malloc(size);
	int x, y, channel;
	if (blurred == NULL)
		return NULL;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			for (channel = 0; channel < 3; channel++) {
				int dx, dy, sum = 0, count = 0;
				for (dy = -1; dy <= 1; dy++) {
					int source_y = CLAMP(y + dy, 0, height - 1);
					for (dx = -1; dx <= 1; dx++) {
						int source_x = CLAMP(x + dx, 0, width - 1);
						sum += pixels[((size_t)source_y * width + source_x) * 3 + channel];
						count++;
					}
				}
				blurred[((size_t)y * width + x) * 3 + channel] =
					(unsigned char)(sum / count);
			}
		}
	}
	return blurred;
}

static int apply_adjustments(unsigned char *pixels, int width, int height,
	const CIJSC_ImageSettings *settings, unsigned long histogram[256])
{
	size_t pixel_count = (size_t)width * height;
	size_t i;
	unsigned char lut[256];
	unsigned char *blurred = NULL;

	CIJSC_image_build_lut(settings, lut);
	if (settings->descreen || settings->unsharp)
		blurred = box_blur(pixels, width, height);
	if ((settings->descreen || settings->unsharp) && blurred == NULL)
		return -1;

	memset(histogram, 0, sizeof(unsigned long) * 256);
	for (i = 0; i < pixel_count; i++) {
		int channel;
		for (channel = 0; channel < 3; channel++) {
			int value;
			size_t offset = i * 3 + channel;
			int source = settings->descreen ? blurred[offset] : pixels[offset];
			if (settings->unsharp)
				source = CLAMP(source + (pixels[offset] - blurred[offset]), 0, 255);
			value = lut[source];
			pixels[offset] = (unsigned char)value;
		}
		if (settings->threshold_enabled) {
			int luminance = (pixels[i * 3] * 299 + pixels[i * 3 + 1] * 587 +
				pixels[i * 3 + 2] * 114) / 1000;
			unsigned char value = luminance >= settings->threshold ? 255 : 0;
			pixels[i * 3] = pixels[i * 3 + 1] = pixels[i * 3 + 2] = value;
		}
		{
			int luminance = (pixels[i * 3] * 299 + pixels[i * 3 + 1] * 587 +
				pixels[i * 3 + 2] * 114) / 1000;
			histogram[luminance]++;
		}
	}
	g_free(blurred);
	return 0;
}

static int adjustments_are_active(const CIJSC_ImageSettings *settings)
{
	return settings->brightness != 0 || settings->contrast != 0 ||
		fabs(settings->gamma - 1.0) > 0.0001 || settings->black_point != 0 ||
		settings->white_point != 255 || settings->curve != CIJSC_CURVE_LINEAR ||
		settings->threshold_enabled || settings->unsharp || settings->descreen;
}

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclobbered"
#endif
int CIJSC_image_process_jpeg(const char *path,
	const CIJSC_ImageSettings *settings, unsigned long histogram[256],
	int *width, int *height)
{
	struct jpeg_decompress_struct input;
	struct jpeg_compress_struct output;
	CIJSC_JpegError input_error, output_error;
	FILE *source = NULL, *destination = NULL;
	unsigned char *pixels = NULL;
	char *temporary = NULL;
	int temporary_fd = -1;
	int result = -1;

	memset(&input, 0, sizeof(input));
	memset(&output, 0, sizeof(output));
	source = g_fopen(path, "rb");
	if (source == NULL)
		goto cleanup;

	input.err = jpeg_std_error(&input_error.base);
	input_error.base.error_exit = image_jpeg_error_exit;
	if (setjmp(input_error.jump))
		goto cleanup_input;
	jpeg_create_decompress(&input);
	jpeg_stdio_src(&input, source);
	jpeg_read_header(&input, TRUE);
	input.out_color_space = JCS_RGB;
	jpeg_start_decompress(&input);
	*width = (int)input.output_width;
	*height = (int)input.output_height;
	pixels = g_try_malloc((size_t)*width * *height * 3);
	if (pixels == NULL)
		goto cleanup_input;
	while (input.output_scanline < input.output_height) {
		JSAMPROW row = pixels + (size_t)input.output_scanline * *width * 3;
		jpeg_read_scanlines(&input, &row, 1);
	}
	jpeg_finish_decompress(&input);
	jpeg_destroy_decompress(&input);
	fclose(source);
	source = NULL;

	if (apply_adjustments(pixels, *width, *height, settings, histogram) != 0)
		goto cleanup;
	if (!adjustments_are_active(settings)) {
		g_free(pixels);
		return 0;
	}
	temporary = g_strdup_printf("%s.XXXXXX", path);
	temporary_fd = g_mkstemp(temporary);
	if (temporary_fd < 0)
		goto cleanup;
	destination = fdopen(temporary_fd, "wb");
	if (destination == NULL)
		goto cleanup;
	temporary_fd = -1;

	output.err = jpeg_std_error(&output_error.base);
	output_error.base.error_exit = image_jpeg_error_exit;
	if (setjmp(output_error.jump))
		goto cleanup_output;
	jpeg_create_compress(&output);
	jpeg_stdio_dest(&output, destination);
	output.image_width = (JDIMENSION)*width;
	output.image_height = (JDIMENSION)*height;
	output.input_components = 3;
	output.in_color_space = JCS_RGB;
	jpeg_set_defaults(&output);
	jpeg_set_quality(&output, 92, TRUE);
	jpeg_start_compress(&output, TRUE);
	while (output.next_scanline < output.image_height) {
		JSAMPROW row = pixels + (size_t)output.next_scanline * *width * 3;
		jpeg_write_scanlines(&output, &row, 1);
	}
	jpeg_finish_compress(&output);
	jpeg_destroy_compress(&output);
	if (fclose(destination) != 0) {
		destination = NULL;
		goto cleanup;
	}
	destination = NULL;
	if (g_rename(temporary, path) != 0)
		goto cleanup;
	result = 0;
	g_free(temporary);
	temporary = NULL;
	g_free(pixels);
	return result;

cleanup_output:
	jpeg_destroy_compress(&output);
	goto cleanup;
cleanup_input:
	jpeg_destroy_decompress(&input);
cleanup:
	if (source != NULL)
		fclose(source);
	if (destination != NULL)
		fclose(destination);
	if (temporary_fd >= 0)
		close(temporary_fd);
	if (temporary != NULL) {
		g_unlink(temporary);
		g_free(temporary);
	}
	g_free(pixels);
	return result;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
