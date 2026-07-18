#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include <stddef.h>

enum {
	CIJSC_CURVE_LINEAR = 0,
	CIJSC_CURVE_S,
	CIJSC_CURVE_HIGH_KEY,
	CIJSC_CURVE_LOW_KEY,
	CIJSC_CURVE_INVERT
};

typedef struct {
	int brightness;
	int contrast;
	double gamma;
	int black_point;
	int white_point;
	int curve;
	int threshold_enabled;
	int threshold;
	int unsharp;
	int descreen;
} CIJSC_ImageSettings;

void CIJSC_image_settings_reset(CIJSC_ImageSettings *settings);
void CIJSC_image_build_lut(const CIJSC_ImageSettings *settings,
	unsigned char lut[256]);
int CIJSC_image_process_jpeg(const char *path,
	const CIJSC_ImageSettings *settings, unsigned long histogram[256],
	int *width, int *height);

#endif
