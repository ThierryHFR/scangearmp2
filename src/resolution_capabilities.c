#include "resolution_capabilities.h"

static int axis_supports_resolution(
	int resolution,
	const int values[CIJSC_RESOLUTION_LIST_SIZE],
	int minimum,
	int maximum)
{
	int i;
	int has_values = 0;

	for (i = 0; i < CIJSC_RESOLUTION_LIST_SIZE; i++) {
		if (values[i] <= 0) {
			continue;
		}
		has_values = 1;
		if (values[i] == resolution) {
			return 1;
		}
	}
	if (has_values) {
		return 0;
	}

	if (minimum > 0 || maximum > 0) {
		if (minimum > 0 && resolution < minimum) {
			return 0;
		}
		if (maximum > 0 && resolution > maximum) {
			return 0;
		}
		return 1;
	}

	return -1;
}

int cijsc_resolution_support_from_capabilities(
	int resolution,
	const int x_list[CIJSC_RESOLUTION_LIST_SIZE],
	int x_min,
	int x_max,
	const int y_list[CIJSC_RESOLUTION_LIST_SIZE],
	int y_min,
	int y_max)
{
	int x_support = axis_supports_resolution(resolution, x_list, x_min, x_max);
	int y_support = axis_supports_resolution(resolution, y_list, y_min, y_max);

	if (x_support == 0 || y_support == 0) {
		return 0;
	}
	if (x_support == 1 && y_support == 1) {
		return 1;
	}
	return -1;
}
