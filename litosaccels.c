#include <gtk/gtk.h>

#include "litosapp.h"
#include "litosappprefs.h"
#include "litosfile.h"

void litos_file_load (LitosAppWindow *win, GFile *gf);
LitosFile * litos_file_new_tab(LitosAppWindow *win);
void litos_app_window_stack_remove(LitosAppWindow *win);
void litos_file_save(LitosAppWindow *win, GFile *gf);

static void open_cb (GtkWidget *dialog, gint response, gpointer win)
{
	if (response == GTK_RESPONSE_ACCEPT)
	{
		GFile *file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));

		litos_file_load(LITOS_APP_WINDOW(win), file);	
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

static void
new_file (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       app)
{
	GtkWindow *window = gtk_application_get_active_window (GTK_APPLICATION (app));

	LitosAppWindow *win = LITOS_APP_WINDOW(window);
	litos_file_new_tab(win);
}

static void
close_activated (GSimpleAction *action, GVariant *parameter, gpointer app)
{
	GtkWindow *window = gtk_application_get_active_window (GTK_APPLICATION (app));

	LitosAppWindow *win = LITOS_APP_WINDOW(window);

	litos_app_window_stack_remove(win);
}

static void save_dialog (GtkWidget *dialog, gint response, gpointer userData)
{
	GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);

	LitosAppWindow *win = LITOS_APP_WINDOW(userData);

	GtkWidget *child = gtk_stack_get_visible_child(GTK_STACK(win->stack));

	if (response == GTK_RESPONSE_ACCEPT)
	{
		g_autoptr (GFile) file = gtk_file_chooser_get_file(chooser);
		litos_file_save (file, litos);
	}

	g_object_unref(dialog);
}

static void save_as_dialog (GSimpleAction *action, GVariant *parameter, void* userData)
{
	GtkWidget *dialog = gtk_file_chooser_dialog_new ("Save File",
		                                  GTK_WINDOW(litos->window),
		                                  GTK_FILE_CHOOSER_ACTION_SAVE,
		                                  _("_Cancel"),
		                                  GTK_RESPONSE_CANCEL,
		                                  _("_Save"),
		                                  GTK_RESPONSE_ACCEPT,
		                                  NULL);

	g_signal_connect (dialog, "GtkDialog::response", G_CALLBACK (save_dialog), userData);
}

void setAccels (GApplication *app)
{
	long unsigned int i;

	/* map actions to callbacks */
	const GActionEntry app_entries[] = {
		{"preferences", preferences_activated, NULL, NULL, NULL },
		{"open", open_activated, NULL, NULL, NULL},
		{"new", new_file, NULL, NULL, NULL},
		{"save", save_dialog, NULL, NULL, NULL, {0,0,0}},
		{"save_as", save_as_dialog, NULL, NULL, NULL, {0,0,0}},
		{"close", close_activated, NULL, NULL, NULL},
		{"quit", quit_activated, NULL, NULL, NULL }
	};

	/* define keyboard accelerators*/
	struct {
	  const gchar *action;
	  const gchar *accels[2];
	} action_accels[] = {
	  { "app.open", { "<Control>o", NULL} },
	  { "app.new", { "<Control>n", NULL} },
	  { "app.save", { "<Control>s", NULL} },
	  { "app.save_as", { "<Shift><Control>s", NULL} },
	  { "app.close", { "<Control>w", NULL} },
	  { "app.quit", { "<Control>q", NULL} }
	};

	g_action_map_add_action_entries(G_ACTION_MAP(app), app_entries, G_N_ELEMENTS(app_entries), app);

	for (i = 0; i < G_N_ELEMENTS(action_accels); i++)
		gtk_application_set_accels_for_action(GTK_APPLICATION(app), action_accels[i].action, action_accels[i].accels);
}
