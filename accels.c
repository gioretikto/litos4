#include <gtk/gtk.h>

#include "litosapp.h"
#include "litosappwin.h"
#include "litosappprefs.h"

void litos_app_window_open (LitosAppWindow *win, GFile *file);

static void open_cb (GtkWidget *dialog, gint response, gpointer win)
{
	if (response == GTK_RESPONSE_ACCEPT)
	{
		GFile *file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));

		litos_app_window_open(LITOS_APP_WINDOW(win), file);		
	}

	gtk_window_destroy (GTK_WINDOW (dialog));
}

static void
open_activated(GSimpleAction *action, GVariant *parameter, gpointer app)
{
	(void)action;
	(void)parameter;

	GtkWidget *dialog;

	GtkWindow *win = gtk_application_get_active_window (GTK_APPLICATION (app));

	dialog = gtk_file_chooser_dialog_new ("Open File",
		NULL,
		GTK_FILE_CHOOSER_ACTION_OPEN,
		"Cancel",
		GTK_RESPONSE_CANCEL,
		"Open",
		GTK_RESPONSE_ACCEPT,
		NULL);

	gtk_window_set_transient_for(GTK_WINDOW(dialog), win);

	gtk_widget_show(dialog);

	g_signal_connect (dialog, "response", G_CALLBACK (open_cb), win);
}

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

static void
quit_activated (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       app)
{
	g_application_quit (G_APPLICATION (app));
}

void setAppAccels (GApplication *app)
{
	long unsigned int i;

	/* map actions to callbacks */
	const GActionEntry app_entries[] = {
		{"preferences", preferences_activated, NULL, NULL, NULL },
		{"open", open_activated, NULL, NULL, NULL},
		{"quit", quit_activated, NULL, NULL, NULL }
	};

	/* define keyboard accelerators*/
	struct {
	  const gchar *action;
	  const gchar *accels[2];
	} action_accels[] = {
	  { "app.open", { "<Control>o", NULL} },
	  { "app.quit", { "<Control>q", NULL} }
	};

	g_action_map_add_action_entries(G_ACTION_MAP(app), app_entries, G_N_ELEMENTS(app_entries), app);

	for (i = 0; i < G_N_ELEMENTS(action_accels); i++)
		gtk_application_set_accels_for_action(GTK_APPLICATION(app), action_accels[i].action, action_accels[i].accels);
}
