#ifndef GTK3COMPAT_H
#define GTK3COMPAT_H

#include <gtk/gtk.h>
#include <glib.h>

void gtk3compat_init(void);
void gtk3compat_main(void);
void gtk3compat_main_quit(void);
int gtk3compat_events_pending(void);
void gtk3compat_main_iteration(void);

#define gtk_main          gtk3compat_main
#define gtk_main_quit     gtk3compat_main_quit
#define gtk_events_pending gtk3compat_events_pending
#define gtk_main_iteration gtk3compat_main_iteration

#endif /* GTK3COMPAT_H */
