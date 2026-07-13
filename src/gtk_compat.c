/*
 * GTK4 main-loop compatibility helpers.
 */

#include "gtk_compat.h"

static GPtrArray *loop_stack;

void cijsc_ui_run(void)
{
	GMainLoop *loop;

	if (loop_stack == NULL) {
		loop_stack = g_ptr_array_new();
	}

	loop = g_main_loop_new(NULL, FALSE);
	g_ptr_array_add(loop_stack, loop);
	g_main_loop_run(loop);
	g_ptr_array_remove_index(loop_stack, loop_stack->len - 1);
	g_main_loop_unref(loop);
}

void cijsc_ui_quit(void)
{
	GMainLoop *loop;

	if (loop_stack == NULL || loop_stack->len == 0) {
		return;
	}

	loop = g_ptr_array_index(loop_stack, loop_stack->len - 1);
	g_main_loop_quit(loop);
}
