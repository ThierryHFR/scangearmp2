#include <assert.h>

#include "scan_geometry.h"

int main(void)
{
	int width = 0;
	int height = 0;

	/* A4 at 300 dpi. */
	assert(CIJSC_scan_geometry(11, 2, &width, &height) == 0);
	assert(width == 2480);
	assert(height == 3507);

	/* A4 at 75 and 1200 dpi keeps the same aspect ratio. */
	assert(CIJSC_scan_geometry(11, 0, &width, &height) == 0);
	assert(width == 620);
	assert(height == 876);
	assert(CIJSC_scan_geometry(11, 4, &width, &height) == 0);
	assert(width == 9920);
	assert(height == 14028);

	assert(CIJSC_scan_geometry(-1, 2, &width, &height) == -1);
	assert(CIJSC_scan_geometry(11, 5, &width, &height) == -1);
	return 0;
}
