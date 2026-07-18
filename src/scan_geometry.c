#include "scan_geometry.h"

#include <stddef.h>

typedef struct {
	int width_300dpi;
	int height_300dpi;
} CIJSC_PageGeometry;

static const CIJSC_PageGeometry page_geometry[] = {
	{ 1074,  649 }, /* Card */
	{ 1500, 1051 }, /* L Landscape */
	{ 1051, 1500 }, /* L Portrait */
	{ 1800, 1200 }, /* 4x6 Landscape */
	{ 1200, 1800 }, /* 4x6 Portrait */
	{ 1748, 1181 }, /* Hagaki Landscape */
	{ 1181, 1748 }, /* Hagaki Portrait */
	{ 2102, 1500 }, /* 2L Landscape */
	{ 1500, 2102 }, /* 2L Portrait */
	{ 1748, 2480 }, /* A5 */
	{ 2149, 3035 }, /* B5 */
	{ 2480, 3507 }, /* A4 */
	{ 2550, 3300 }, /* Letter */
	{ 1650, 2550 }, /* Statement */
};

int CIJSC_scan_geometry(int size_id, int resolution_index,
	int *width, int *height)
{
	static const double resolution_factor[] = {4.0, 2.0, 1.0, 0.5, 0.25};
	if (size_id < 0 || size_id >= (int)(sizeof(page_geometry) / sizeof(page_geometry[0])) ||
		resolution_index < 0 || resolution_index >= (int)(sizeof(resolution_factor) / sizeof(resolution_factor[0])) ||
		width == NULL || height == NULL)
		return -1;
	*width = (int)(page_geometry[size_id].width_300dpi /
		resolution_factor[resolution_index]);
	*height = (int)(page_geometry[size_id].height_300dpi /
		resolution_factor[resolution_index]);
	return 0;
}
