#ifndef RESOLUTION_CAPABILITIES_H
#define RESOLUTION_CAPABILITIES_H

#define CIJSC_RESOLUTION_LIST_SIZE 16

/* Returns 1 when supported, 0 when rejected, and -1 when unknown. */
int cijsc_resolution_support_from_capabilities(
	int resolution,
	const int x_list[CIJSC_RESOLUTION_LIST_SIZE],
	int x_min,
	int x_max,
	const int y_list[CIJSC_RESOLUTION_LIST_SIZE],
	int y_min,
	int y_max);

#endif
