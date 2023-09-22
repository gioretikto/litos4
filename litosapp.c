#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include "litosapp.h"
#include "litosappprefs.h"
#include "litosfile.h"

void setAccels (GApplication *app);
gboolean litos_file_load (LitosFile *file, GError *error);
LitosFile * litos_app_window_new_tab(LitosAppWindow *win, GFile *gf);

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
	LitosAppWindow *win;

	win = litos_app_window_new (LITOS_APP (app));

	gtk_window_set_title (GTK_WINDOW (win), "Litos");
	gtk_window_maximize (GTK_WINDOW (win));
	gtk_window_present (GTK_WINDOW (win));
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
		if (litos_file_load(litos_app_window_new_tab(win,files[i]),error))
		{
			GtkWidget *message_dialog;
			char *filename = g_file_get_basename(files[i]);
			message_dialog = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "ERROR : Can't load %s.\n %s", filename, error->message);
			gtk_widget_show(message_dialog);
			g_error_free(error);
		}
	}

	gtk_window_set_title (GTK_WINDOW (win), "Litos");
	gtk_window_maximize (GTK_WINDOW (win));
	gtk_window_present (GTK_WINDOW (win));
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
