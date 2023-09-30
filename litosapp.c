#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include "litosapp.h"
#include "litosappprefs.h"
#include "litosfile.h"

void setAccels (GApplication *app);
gboolean litos_file_load (LitosFile *file, GError **error);
GFile *litos_file_get_gfile(LitosFile* file);

LitosFile * litos_app_window_new_tab(LitosAppWindow *win, GFile *gf);
LitosFile * litos_app_window_open(LitosAppWindow *win, GFile *gf);
guint litos_app_window_get_array_len(LitosAppWindow *win);
LitosFile * litos_app_window_get_file(LitosAppWindow *win, int *i);
GtkNotebook * litos_app_window_get_nb(LitosAppWindow *win);

void quit_activated (GSimpleAction *action, GVariant *parameter, gpointer app);

struct _LitosApp
{
	GtkApplication parent;
};

G_DEFINE_TYPE(LitosApp, litos_app, GTK_TYPE_APPLICATION);

static void
litos_app_init (LitosApp *app)
{
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
litos_app_startup (GApplication *app)
{
	setAccels(app);

	G_APPLICATION_CLASS (litos_app_parent_class)->startup (app);

	gtk_source_init();

	g_object_set(gtk_settings_get_default(), "gtk-application-prefer-dark-theme", TRUE, NULL);
}

static void
litos_app_activate (GApplication *app)
{
	LitosAppWindow *win = litos_app_window_new (LITOS_APP (app));

	GtkWindow *window = GTK_WINDOW (win);

	gtk_window_set_title (window, "Litos");
	gtk_window_maximize (window);
	gtk_window_present (window);
}

void litos_app_error_dialog(GtkWindow *window, GError *error, char *filename)
{
	GtkWidget *message_dialog;
	message_dialog = gtk_message_dialog_new(window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
	GTK_BUTTONS_CLOSE, "ERROR : Can't load %s.\n %s", filename, error->message);
	gtk_widget_show(message_dialog);
	g_error_free(error);
}

gboolean litos_app_check_duplicate(char *filename, LitosAppWindow *win)
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

static void
litos_app_open (GApplication  *app,
                  GFile **files,
                  int            n_files,
                  const char    *hint)
{
	GList *windows;
	LitosAppWindow *win;
	GError *error = NULL;
	int i;

	windows = gtk_application_get_windows (GTK_APPLICATION (app));

	if (windows)
		win = LITOS_APP_WINDOW (windows->data);
	else
		win = litos_app_window_new (LITOS_APP (app));

	for (i = 0; i < n_files; i++)
	{
		if (files[i] != NULL)
		{
			char *filename = g_file_get_path(files[i]);

			if (!litos_app_check_duplicate(filename,win))
			{
				LitosFile *litos_file = litos_app_window_open(win,files[i]);
				if (!litos_file_load(litos_file,&error))
					litos_app_error_dialog(GTK_WINDOW(win), error, filename);
			}

			g_free(filename);
		}
	}

	gtk_window_set_title (GTK_WINDOW(win), "Litos");
	gtk_window_maximize (GTK_WINDOW(win));
	gtk_window_present (GTK_WINDOW(win));
}

static void
litos_app_class_init (LitosAppClass *class)
{
	G_APPLICATION_CLASS (class)->activate = litos_app_activate;
	G_APPLICATION_CLASS (class)->startup = litos_app_startup;
	G_APPLICATION_CLASS (class)->open = litos_app_open;
}

LitosApp *
litos_app_new (void)
{
	return g_object_new (LITOS_APP_TYPE,
			"application-id", "org.gtk.litos",
			"flags", G_APPLICATION_HANDLES_OPEN,
			NULL);
}
