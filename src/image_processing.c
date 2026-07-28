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
	settings->scale_percent = 100;
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

static void sort_nine(unsigned char values[9])
{
	int i;
	for (i = 1; i < 9; i++) {
		unsigned char value = values[i];
		int position = i;
		while (position > 0 && values[position - 1] > value) {
			values[position] = values[position - 1];
			position--;
		}
		values[position] = value;
	}
}

static unsigned char *dust_filter(const unsigned char *pixels, int width,
	int height, int strength)
{
	size_t size = (size_t)width * height * 3;
	unsigned char *filtered = g_try_malloc(size);
	int threshold[] = {256, 80, 50, 25};
	int x, y, channel;
	if (filtered == NULL)
		return NULL;
	memcpy(filtered, pixels, size);
	for (y = 1; y < height - 1; y++) {
		for (x = 1; x < width - 1; x++) {
			for (channel = 0; channel < 3; channel++) {
				unsigned char values[9];
				int dx, dy, index = 0;
				size_t offset = ((size_t)y * width + x) * 3 + channel;
				for (dy = -1; dy <= 1; dy++)
					for (dx = -1; dx <= 1; dx++)
						values[index++] = pixels[((size_t)(y + dy) * width + x + dx) * 3 + channel];
				sort_nine(values);
				if (abs((int)pixels[offset] - values[4]) > threshold[CLAMP(strength, 1, 3)])
					filtered[offset] = values[4];
			}
		}
	}
	return filtered;
}

static void auto_levels(const unsigned char *pixels, size_t pixel_count,
	int *black, int *white)
{
	unsigned long histogram[256] = {0};
	unsigned long tail = (unsigned long)MAX((size_t)1, pixel_count / 200);
	unsigned long low_target = tail;
	unsigned long high_target = (unsigned long)(pixel_count - tail + 1);
	unsigned long total = 0;
	int i;
	for (i = 0; i < (int)pixel_count; i++) {
		int luminance = (pixels[i * 3] * 299 + pixels[i * 3 + 1] * 587 +
			pixels[i * 3 + 2] * 114) / 1000;
		histogram[luminance]++;
	}
	*black = 0;
	*white = 255;
	for (i = 0; i < 256; i++) {
		total += histogram[i];
		if (total >= low_target) {
			*black = i;
			break;
		}
	}
	total = 0;
	for (i = 0; i < 256; i++) {
		total += histogram[i];
		if (total >= high_target) {
			*white = i;
			break;
		}
	}
	if (*white <= *black + 8) {
		*black = 0;
		*white = 255;
	}
}

static void calculate_histogram(const unsigned char *pixels, size_t pixel_count,
	unsigned long histogram[256])
{
	size_t i;
	memset(histogram, 0, sizeof(unsigned long) * 256);
	for (i = 0; i < pixel_count; i++) {
		int luminance = (pixels[i * 3] * 299 + pixels[i * 3 + 1] * 587 +
			pixels[i * 3 + 2] * 114) / 1000;
		histogram[luminance]++;
	}
}

static int apply_adjustments(unsigned char *pixels, int width, int height,
	const CIJSC_ImageSettings *settings, unsigned long histogram[256])
{
	size_t pixel_count = (size_t)width * height;
	size_t i;
	unsigned char lut[256];
	unsigned char *blurred = NULL;
	unsigned char *filtered = NULL;
	CIJSC_ImageSettings effective = *settings;
	double channel_gain[3] = {1.0, 1.0, 1.0};
	int saturation = settings->saturation;

	if (settings->auto_tone)
		auto_levels(pixels, pixel_count, &effective.black_point, &effective.white_point);
	if (settings->fading_correction) {
		effective.contrast = CLAMP(effective.contrast + settings->fading_correction * 5, -100, 100);
		saturation = CLAMP(saturation + settings->fading_correction * 15, -100, 100);
	}
	if (settings->dust_reduction) {
		filtered = dust_filter(pixels, width, height, settings->dust_reduction);
		if (filtered == NULL)
			return -1;
		memcpy(pixels, filtered, pixel_count * 3);
		g_free(filtered);
	}
	if (settings->descreen || settings->grain_reduction) {
		blurred = box_blur(pixels, width, height);
		if (blurred == NULL)
			return -1;
		if (settings->descreen)
			memcpy(pixels, blurred, pixel_count * 3);
		else {
			double amount = settings->grain_reduction / 4.0;
			for (i = 0; i < pixel_count * 3; i++)
				pixels[i] = (unsigned char)(pixels[i] * (1.0 - amount) + blurred[i] * amount + 0.5);
		}
		g_free(blurred);
		blurred = NULL;
	}
	if (settings->color_balance || settings->fading_correction) {
		double average[3] = {0.0, 0.0, 0.0};
		double target;
		int channel;
		for (i = 0; i < pixel_count; i++)
			for (channel = 0; channel < 3; channel++)
				average[channel] += pixels[i * 3 + channel];
		target = (average[0] + average[1] + average[2]) / 3.0;
		for (channel = 0; channel < 3; channel++)
			if (average[channel] > 0.0)
				channel_gain[channel] = CLAMP(target / average[channel], 0.5, 2.0);
	}
	CIJSC_image_build_lut(&effective, lut);
	if (settings->unsharp)
		blurred = box_blur(pixels, width, height);
	if (settings->unsharp && blurred == NULL)
		return -1;

	for (i = 0; i < pixel_count; i++) {
		int original[3];
		int luminance;
		int channel;
		for (channel = 0; channel < 3; channel++)
			original[channel] = CLAMP((int)(pixels[i * 3 + channel] * channel_gain[channel] + 0.5), 0, 255);
		luminance = (original[0] * 299 + original[1] * 587 + original[2] * 114) / 1000;
		for (channel = 0; channel < 3; channel++) {
			int source = original[channel];
			int value;
			if (saturation != 0)
				source = CLAMP(luminance + (source - luminance) * (100 + saturation) / 100, 0, 255);
			if (settings->backlight_correction && luminance < 192) {
				double shadow = (192 - luminance) / 192.0;
				source = CLAMP(source + (int)((255 - source) * shadow *
					settings->backlight_correction * 0.12), 0, 255);
			}
			if (settings->unsharp)
				source = CLAMP(source + (pixels[i * 3 + channel] - blurred[i * 3 + channel]), 0, 255);
			value = lut[source];
			pixels[i * 3 + channel] = (unsigned char)value;
		}
		if (settings->threshold_enabled) {
			int luminance = (pixels[i * 3] * 299 + pixels[i * 3 + 1] * 587 +
				pixels[i * 3 + 2] * 114) / 1000;
			unsigned char value = luminance >= settings->threshold ? 255 : 0;
			pixels[i * 3] = pixels[i * 3 + 1] = pixels[i * 3 + 2] = value;
		}
	}
	g_free(blurred);
	calculate_histogram(pixels, pixel_count, histogram);
	return 0;
}

static int adjustments_are_active(const CIJSC_ImageSettings *settings)
{
	return settings->brightness != 0 || settings->contrast != 0 ||
		fabs(settings->gamma - 1.0) > 0.0001 || settings->black_point != 0 ||
		settings->white_point != 255 || settings->curve != CIJSC_CURVE_LINEAR ||
		settings->threshold_enabled || settings->unsharp || settings->descreen ||
		settings->auto_tone || settings->color_balance || settings->dust_reduction ||
		settings->fading_correction || settings->grain_reduction ||
		settings->backlight_correction || settings->saturation != 0 ||
		settings->scale_percent != 100;
}

static unsigned char *resize_bilinear(const unsigned char *pixels, int width,
	int height, int output_width, int output_height)
{
	unsigned char *output = g_try_malloc((size_t)output_width * output_height * 3);
	int x, y, channel;
	if (output == NULL)
		return NULL;
	for (y = 0; y < output_height; y++) {
		double source_y = output_height == 1 ? 0.0 : y * (height - 1.0) / (output_height - 1.0);
		int y0 = (int)source_y;
		int y1 = MIN(y0 + 1, height - 1);
		double fy = source_y - y0;
		for (x = 0; x < output_width; x++) {
			double source_x = output_width == 1 ? 0.0 : x * (width - 1.0) / (output_width - 1.0);
			int x0 = (int)source_x;
			int x1 = MIN(x0 + 1, width - 1);
			double fx = source_x - x0;
			for (channel = 0; channel < 3; channel++) {
				double top = pixels[((size_t)y0 * width + x0) * 3 + channel] * (1.0 - fx) +
					pixels[((size_t)y0 * width + x1) * 3 + channel] * fx;
				double bottom = pixels[((size_t)y1 * width + x0) * 3 + channel] * (1.0 - fx) +
					pixels[((size_t)y1 * width + x1) * 3 + channel] * fx;
				output[((size_t)y * output_width + x) * 3 + channel] =
					(unsigned char)(top * (1.0 - fy) + bottom * fy + 0.5);
			}
		}
	}
	return output;
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
	if (!adjustments_are_active(settings)) {
		size_t row_size;

		if (input.output_width > G_MAXSIZE / input.output_components)
			goto cleanup_input;
		row_size = (size_t)input.output_width * input.output_components;
		pixels = g_try_malloc(row_size);
		if (pixels == NULL)
			goto cleanup_input;
		memset(histogram, 0, sizeof(unsigned long) * 256);
		while (input.output_scanline < input.output_height) {
			JSAMPROW row = pixels;
			size_t x;

			jpeg_read_scanlines(&input, &row, 1);
			for (x = 0; x < input.output_width; x++) {
				int luminance = (pixels[x * 3] * 299 +
					pixels[x * 3 + 1] * 587 +
					pixels[x * 3 + 2] * 114) / 1000;
				histogram[luminance]++;
			}
		}
		jpeg_finish_decompress(&input);
		jpeg_destroy_decompress(&input);
		fclose(source);
		g_free(pixels);
		return 0;
	}
	if ((size_t)*width > G_MAXSIZE / (size_t)*height ||
		(size_t)*width * (size_t)*height > G_MAXSIZE / 3)
		goto cleanup_input;
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
	if (settings->scale_percent != 100) {
		long requested_width = (long)*width * settings->scale_percent / 100;
		long requested_height = (long)*height * settings->scale_percent / 100;
		int output_width;
		int output_height;
		if (requested_width < 1 || requested_height < 1 ||
			requested_width > 32767 || requested_height > 32767 ||
			(size_t)requested_width * requested_height > 100000000)
			goto cleanup;
		output_width = (int)requested_width;
		output_height = (int)requested_height;
		unsigned char *resized = resize_bilinear(pixels, *width, *height,
			output_width, output_height);
		if (resized == NULL)
			goto cleanup;
		g_free(pixels);
		pixels = resized;
		*width = output_width;
		*height = output_height;
		calculate_histogram(pixels, (size_t)*width * *height, histogram);
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
