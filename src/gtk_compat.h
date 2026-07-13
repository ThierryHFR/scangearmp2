/*
 * GTK4 main-loop compatibility helpers.
 *
 * ScanGear's workflow is synchronous and uses nested GTK3 main loops for
 * modal windows. GTK4 removed gtk_main(), so keep that control flow on top
 * of nestable GLib main loops while the dialogs are migrated incrementally.
 */
#ifndef GTK_COMPAT_H
#define GTK_COMPAT_H

#include <glib.h>

void cijsc_ui_run(void);
void cijsc_ui_quit(void);

#define gtk_main() cijsc_ui_run()
#define gtk_main_quit() cijsc_ui_quit()
#define gtk_events_pending() g_main_context_pending(NULL)
#define gtk_main_iteration() g_main_context_iteration(NULL, FALSE)

#endif
