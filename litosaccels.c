#include <gtk/gtk.h>

#include "litosapp.h"
#include "litosappprefs.h"
#include "litosfile.h"

gboolean litos_file_load (LitosFile *file, GError **error);
GtkWidget * litos_file_get_view(LitosFile *file);
GFile *litos_file_get_gfile(LitosFile* file);

gboolean litos_app_window_remove_child(LitosAppWindow *win);
void litos_app_window_save(LitosAppWindow *win, LitosFile *file);
void litos_app_window_save_as(LitosAppWindow *app);

LitosFile * litos_app_window_get_file(LitosAppWindow *win, int *i);

LitosFile * litos_app_window_new_tab(LitosAppWindow *win, GFile *gf);
LitosFile * litos_app_window_open(LitosAppWindow *win, GFile *gf);
void monitor_change (GObject *gobject, GParamSpec *pspec, gpointer win);
LitosFile * litos_app_window_current_file(LitosAppWindow *win);

guint litos_app_window_get_array_len(LitosAppWindow *win);

gboolean litos_app_window_quit (GtkWindow *window, gpointer user_data);
GtkNotebook * litos_app_window_get_nb(LitosAppWindow *win);

void show_error_dialog(GtkWindow *window, GError *error, char *filename)
{
	GtkWidget *message_dialog;
	message_dialog = gtk_message_dialog_new(window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
	GTK_BUTTONS_CLOSE, "ERROR : Can't load %s.\n %s", filename, error->message);
	gtk_widget_show(message_dialog);
	g_error_free(error);
}

gboolean check_duplicate_file(char *filename, LitosAppWindow *win)
{
	GFile *current_gfile;
	int i, n_pages;

	n_pages = litos_app_window_get_array_len(win);

	if (n_pages == 0)
		return FALSE;

	for (i = 0; i < n_pages; i++)
	{
		current_gfile = litos_file_get_gfile(litos_app_window_get_file(win,&i));

		if (current_gfile != NULL)
		{
			char *current_file_name = g_file_get_path(current_gfile);

			if (strcmp(current_file_name, filename) == 0)
			{
				g_free(current_file_name);
				gtk_notebook_set_current_page(litos_app_window_get_nb(win), i);
				return TRUE;
			}

			g_free(current_file_name);
		}
	}

	return FALSE;
}

static void open_cb (GtkWidget *dialog, gint response, gpointer win)
{
	if (response == GTK_RESPONSE_ACCEPT)
	{
		GError *error = NULL;
		GFile *gfile = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));

		LitosAppWindow *lwin = LITOS_APP_WINDOW(win);

		if (gfile != NULL)
		{
			char *gfile_name = g_file_get_path(gfile);

			if (!check_duplicate_file(gfile_name,lwin))
			{
				LitosFile *file = litos_app_window_open(lwin,gfile);
				if (!litos_file_load(file,&error))
					show_error_dialog(GTK_WINDOW(win), error, gfile_name);
			}

			g_free(gfile_name);
		}
	}

	gtk_window_destroy (GTK_WINDOW (dialog));
}

static void
open_activated(GSimpleAction *action, GVariant *parameter, gpointer app)
{
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
save(GSimpleAction *action, GVariant *parameter, gpointer app)
{
	GtkWindow *window = gtk_application_get_active_window (GTK_APPLICATION (app));
	LitosAppWindow *win = LITOS_APP_WINDOW(window);

	LitosFile *file = litos_app_window_current_file(win);
	litos_app_window_save(win, file);
}

static void
save_as_dialog (GSimpleAction *action, GVariant *parameter, gpointer app)
{
	GtkWindow *win = gtk_application_get_active_window (GTK_APPLICATION (app));
	litos_app_window_save_as(LITOS_APP_WINDOW(win));
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
close_activated (GSimpleAction *action, GVariant *parameter, gpointer app)
{
	GtkWindow *window = gtk_application_get_active_window (GTK_APPLICATION (app));

	LitosAppWindow *win = LITOS_APP_WINDOW(window);

	litos_app_window_remove_child(win);
}

static void quit_activated (GSimpleAction *action, GVariant *parameter, gpointer app)
{
	GtkWindow *window = gtk_application_get_active_window (GTK_APPLICATION (app));
	LitosAppWindow *win = LITOS_APP_WINDOW(window);

	litos_app_window_quit(window, win);
}

static void
new_file (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       app)
{
	GtkWindow *win = gtk_application_get_active_window (GTK_APPLICATION (app));
	litos_app_window_new_tab(LITOS_APP_WINDOW(win), NULL);
}

void setAccels (GApplication *app)
{
	long unsigned int i;

	/* map actions to callbacks */
	const GActionEntry app_entries[] = {
		{"preferences", preferences_activated, NULL, NULL, NULL },
		{"open", open_activated, NULL, NULL, NULL},
		{"new", new_file, NULL, NULL, NULL},
		{"save", save, NULL, NULL, NULL, {0,0,0}},
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
