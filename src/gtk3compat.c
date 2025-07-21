#include "gtk3compat.h"

static GMainLoop *sgmp_loop = NULL;

void gtk3compat_init(void)
{
    /* gtk_init is still available in GTK4 */
    int argc = 0;
    char **argv = NULL;
    gtk_init();
    (void)argc; (void)argv;
}

void gtk3compat_main(void)
{
    sgmp_loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(sgmp_loop);
}

void gtk3compat_main_quit(void)
{
    if (sgmp_loop)
        g_main_loop_quit(sgmp_loop);
}

int gtk3compat_events_pending(void)
{
    return g_main_context_pending(NULL);
}

void gtk3compat_main_iteration(void)
{
    while (g_main_context_iteration(NULL, FALSE));
}
