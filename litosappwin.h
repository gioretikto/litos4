#ifndef __LITOSAPPWIN_H
#define __LITOSAPPWIN_H

#include <gtk/gtk.h>
#include "litosapp.h"

struct _LitosAppWindow
{
	GtkApplicationWindow parent;

	GSettings *settings;
	GtkWidget *stack;
	GtkWidget *gears;
	GtkWidget *search;
	GtkWidget *searchbar;
};


#define LITOS_APP_WINDOW_TYPE (litos_app_window_get_type ())
G_DECLARE_FINAL_TYPE (LitosAppWindow, litos_app_window, LITOS, APP_WINDOW, GtkApplicationWindow)


LitosAppWindow       *litos_app_window_new          (LitosApp *app);
void                    litos_app_window_open         (LitosAppWindow *win,
                                                         GFile            *file);


#endif /* __LITOSAPPWIN_H */
