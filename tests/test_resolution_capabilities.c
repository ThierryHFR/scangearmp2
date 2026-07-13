#include <assert.h>

#include "resolution_capabilities.h"

static void test_explicit_lists(void)
{
	int x[CIJSC_RESOLUTION_LIST_SIZE] = {75, 150, 300, 600, 1200};
	int y[CIJSC_RESOLUTION_LIST_SIZE] = {75, 150, 300, 600, 1200};

	assert(cijsc_resolution_support_from_capabilities(1200, x, 0, 0, y, 0, 0) == 1);
	assert(cijsc_resolution_support_from_capabilities(2400, x, 0, 0, y, 0, 0) == 0);
}

static void test_both_axes_must_support_resolution(void)
{
	int x[CIJSC_RESOLUTION_LIST_SIZE] = {75, 150, 300, 600, 1200};
	int y[CIJSC_RESOLUTION_LIST_SIZE] = {75, 150, 300, 600};

	assert(cijsc_resolution_support_from_capabilities(1200, x, 0, 0, y, 0, 0) == 0);
}

static void test_ranges(void)
{
	int empty[CIJSC_RESOLUTION_LIST_SIZE] = {0};

	assert(cijsc_resolution_support_from_capabilities(600, empty, 75, 1200, empty, 75, 1200) == 1);
	assert(cijsc_resolution_support_from_capabilities(2400, empty, 75, 1200, empty, 75, 1200) == 0);
}

static void test_missing_capabilities(void)
{
	int empty[CIJSC_RESOLUTION_LIST_SIZE] = {0};

	assert(cijsc_resolution_support_from_capabilities(1200, empty, 0, 0, empty, 0, 0) == -1);
}

int main(void)
{
	test_explicit_lists();
	test_both_axes_must_support_resolution();
	test_ranges();
	test_missing_capabilities();
	return 0;
}
