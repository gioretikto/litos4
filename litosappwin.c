#include <gtk/gtk.h>

#include "litosapp.h"
#include "litosappwin.h"
#include "litosfile.h"

GtkWidget* MyNewSourceview();

LitosFile * litos_file_new(LitosAppWindow *win);

struct _LitosAppWindow
{
	GtkApplicationWindow parent;

	GSettings *settings;
	GtkWidget *stack;
	GtkWidget *gears;
	GtkWidget *search;
	GtkWidget *searchbar;
	GPtrArray litosFileList;
};

G_DEFINE_TYPE (LitosAppWindow, litos_app_window, GTK_TYPE_APPLICATION_WINDOW);

static void
close_activated (GSimpleAction *action, GVariant *parameter, gpointer userData)
{
	LitosAppWindow *win = LITOS_APP_WINDOW(userData);

	GtkWidget *child = gtk_stack_get_visible_child(GTK_STACK(win->stack));
	g_print("ctrl-w pressed\n");
	printf("ptr = %p\n", (void *)child);
	if (child != NULL)
		gtk_stack_remove(GTK_STACK(win->stack), child);
}

static void
search_text_changed (GtkEntry	*entry,
                     LitosAppWindow *win)
{
	const char *text;
	GtkWidget *tab;
	GtkWidget *view;
	GtkTextBuffer *buffer;
	GtkTextIter start, match_start, match_end;

	text = gtk_editable_get_text (GTK_EDITABLE (entry));

	if (text[0] == '\0')
		return;

	tab = gtk_stack_get_visible_child (GTK_STACK (win->stack));
	view = gtk_scrolled_window_get_child (GTK_SCROLLED_WINDOW (tab));
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	/* Very simple-minded search implementation */
	gtk_text_buffer_get_start_iter (buffer, &start);
	if (gtk_text_iter_forward_search (&start, text, GTK_TEXT_SEARCH_CASE_INSENSITIVE,
			&match_start, &match_end, NULL))
	{
		gtk_text_buffer_select_range (buffer, &match_start, &match_end);
		gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (view), &match_start,
				0.0, FALSE, 0.0, 0.0);
	}
}

static void
visible_child_changed (GObject	*stack,
			GParamSpec	*pspec,
			LitosAppWindow *win)
{
	if (gtk_widget_in_destruction (GTK_WIDGET (stack)))
		return;

	gtk_search_bar_set_search_mode (GTK_SEARCH_BAR (win->searchbar), FALSE);
}

static void
litos_app_window_init (LitosAppWindow *win)
{
	GtkBuilder *builder;
	GMenuModel *menu;

	gtk_widget_init_template (GTK_WIDGET (win));

	builder = gtk_builder_new_from_resource ("/org/gtk/litos/gears-menu.ui");
	menu = G_MENU_MODEL (gtk_builder_get_object (builder, "menu"));
	gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (win->gears), menu);
	g_object_unref (builder);

	win->settings = g_settings_new ("org.gtk.litos");

	g_settings_bind (win->settings, "transition",
		win->stack, "transition-type",
		G_SETTINGS_BIND_DEFAULT);

	g_object_bind_property (win->search, "active",
		win->searchbar, "search-mode-enabled",
		G_BINDING_BIDIRECTIONAL);

	g_object_unref(&win->litosFileList);
}

static void
litos_app_window_dispose (GObject *object)
{
	LitosAppWindow *win;

	win = LITOS_APP_WINDOW (object);

	g_clear_object (&win->settings);

	G_OBJECT_CLASS (litos_app_window_parent_class)->dispose (object);
}

static void
litos_app_window_class_init (LitosAppWindowClass *class)
{
	G_OBJECT_CLASS (class)->dispose = litos_app_window_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class),
						"/org/gtk/litos/window.ui");
	gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), LitosAppWindow, stack);
	gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), LitosAppWindow, gears);
	gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), LitosAppWindow, search);
	gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), LitosAppWindow, searchbar);

	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), search_text_changed);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), visible_child_changed);
}

LitosAppWindow *
litos_app_window_new (LitosApp *app)
{
	return g_object_new (LITOS_APP_WINDOW_TYPE, "application", app, NULL);
}

void litos_app_window_set_file (LitosAppWindow *win, GtkTextTag *tag)
{
	gtk_widget_set_sensitive (win->search, TRUE);

	g_settings_bind (win->settings, "font",
			tag, "font",
			G_SETTINGS_BIND_DEFAULT);
}

GtkWidget * litos_app_window_get_child(LitosAppWindow *win)
{

	return gtk_stack_get_visible_child(GTK_STACK(win->stack));
}

void litos_app_window_remove_child(LitosAppWindow *win)
{
	GtkWidget * child = gtk_stack_get_visible_child(GTK_STACK(win->stack));

	if(child != NULL)
		gtk_stack_remove(GTK_STACK(win->stack), child);	
}

void litos_app_window_add_title(LitosAppWindow *win, GtkWidget *scrolled, char *filename)
{
	gtk_stack_add_titled (GTK_STACK (win->stack), scrolled, filename, filename);
}

static gboolean func (gconstpointer a, gconstpointer scrolled_win)
{
	return LITOS_FILE(a)->scrolled == scrolled_win;
}

guint litos_app_window_search_file(LitosAppWindow *win)
{
	guint index;

	GtkWidget *scrolled_win = gtk_stack_get_visible_child(GTK_STACK(win->stack));

	g_ptr_array_find_with_equal_func(&win->litosFileList, scrolled_win, func, &index);

	return index;
}
