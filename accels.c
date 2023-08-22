#include <gtk/gtk.h>

#include "litosapp.h"
#include "litosappwin.h"
#include "litosappprefs.h"

static void
preferences_activated (GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       app)
{
	LitosAppPrefs *prefs;
	GtkWindow *win;

	win = gtk_application_get_active_window (GTK_APPLICATION (app));
	prefs = litos_app_prefs_new (LITOS_APP_WINDOW (win));
	gtk_window_present (GTK_WINDOW (prefs));
}

void
quit_activated (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       app)
{
	g_application_quit (G_APPLICATION (app));
}

static GActionEntry app_entries[] =
{
	{ "preferences", preferences_activated, NULL, NULL, NULL },
	{ "quit", quit_activated, NULL, NULL, NULL }
};


void setAccels (GApplication *app)
{
	const char *quit_accels[2] = { "<Ctrl>Q", NULL };

	g_action_map_add_action_entries (G_ACTION_MAP (app),
                                   app_entries, G_N_ELEMENTS (app_entries),
                                   app);
	gtk_application_set_accels_for_action (GTK_APPLICATION (app),
				"app.quit",
				quit_accels);
}
