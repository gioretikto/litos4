#include <gtk/gtk.h>

#include "litosapp.h"
#include "litosappwin.h"
#include "litosappprefs.h"
#include "litosfile.h"

void setAccels (GApplication *app);

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

	GdkDisplay *display = gdk_display_get_default ();
	GtkCssProvider *provider = gtk_css_provider_new ();
	gtk_css_provider_load_from_path (provider, "litos.css");

	gtk_style_context_add_provider_for_display (display,
				GTK_STYLE_PROVIDER (provider),
				GTK_STYLE_PROVIDER_PRIORITY_USER);
	g_object_unref (provider);
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
                  GFile        **files,
                  int            n_files,
                  const char    *hint)
{
	GList *windows;
	LitosAppWindow *win;
	int i;

	windows = gtk_application_get_windows (GTK_APPLICATION (app));

	if (windows)
		win = LITOS_APP_WINDOW (windows->data);
	else
		win = litos_app_window_new (LITOS_APP (app));

	for (i = 0; i < n_files; i++)
		litos_app_window_open (win, files[i]);

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
