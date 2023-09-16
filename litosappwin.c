#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include "litosapp.h"
#include "litosappwin.h"
#include "litosfile.h"
#include "page.h"

GFile *litos_file_get_gfile(LitosFile* file);
gboolean litos_file_save(LitosFile *file, GError *error);
void litos_file_save_as(LitosFile* file, GFile *new_file);
gchar *litos_file_get_name(LitosFile *file);
GtkTextBuffer *litos_file_get_buffer(LitosFile *file);
LitosFile * litos_file_set(struct Page *page);
gboolean litos_file_get_saved(LitosFile *file);
GtkWidget * litos_file_get_view(LitosFile *file);
GtkWidget * litos_file_get_lbl(LitosFile *file);
GtkWidget * litos_file_get_tabbox(LitosFile *file);
void litos_file_set_saved(LitosFile *file, gboolean status);

static GtkSourceView* currentTabSourceView(LitosAppWindow *win);
GtkWidget* MyNewSourceview();

struct _LitosAppWindow
{
	GtkApplicationWindow parent;

	GSettings *settings;
	GtkNotebook *notebook;
	GtkWidget *gears;
	GtkWidget *search;
	GtkWidget *searchbar;
	GPtrArray *litosFileList;
};

G_DEFINE_TYPE (LitosAppWindow, litos_app_window, GTK_TYPE_APPLICATION_WINDOW);


static void
close_activated (GSimpleAction *action, GVariant *parameter, gpointer userData)
{
	LitosAppWindow *win = LITOS_APP_WINDOW(userData);

	gtk_notebook_remove_page(win->notebook, gtk_notebook_get_current_page (win->notebook));
}

static void
search_text_changed (GtkEntry	*entry,
                     LitosAppWindow *win)
{
	const char *text;
	GtkTextBuffer *buffer;
	GtkTextIter start, match_start, match_end;

	text = gtk_editable_get_text (GTK_EDITABLE (entry));

	if (text[0] == '\0')
		return;

	GtkSourceView *view = currentTabSourceView(win);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(view));

	/* Very simple-minded search implementation */

	gtk_text_buffer_get_start_iter (buffer, &start);

	if (gtk_text_iter_forward_search (&start, text, GTK_TEXT_SEARCH_CASE_INSENSITIVE,
			&match_start, &match_end, NULL))
	{
		gtk_text_buffer_select_range (buffer, &match_start, &match_end);
		gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW(view), &match_start,
				0.0, FALSE, 0.0, 0.0);
	}
}

static void
visible_child_changed (GObject	*notebook,
			GParamSpec *pspec,
			LitosAppWindow *win)
{
	if (gtk_widget_in_destruction (GTK_WIDGET (notebook)))
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
	gtk_widget_set_sensitive (win->search, TRUE);
	win->litosFileList = g_ptr_array_new_full(0, g_object_unref);

	g_object_bind_property (win->search, "active",
		win->searchbar, "search-mode-enabled",
		G_BINDING_BIDIRECTIONAL);
}

static void
litos_app_window_dispose (GObject *object)
{
	LitosAppWindow *win;

	win = LITOS_APP_WINDOW (object);

	g_clear_object (&win->settings);

	g_ptr_array_unref(win->litosFileList);

	G_OBJECT_CLASS (litos_app_window_parent_class)->dispose (object);
}

static void
litos_app_window_class_init (LitosAppWindowClass *class)
{
	G_OBJECT_CLASS (class)->dispose = litos_app_window_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class),
						"/org/gtk/litos/window.ui");
	gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), LitosAppWindow, notebook);
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

static gboolean func (gconstpointer array_element, gconstpointer tabbox)
{
	return litos_file_get_tabbox ((LITOS_FILE((void*)array_element))) == tabbox;
}

guint litos_app_window_search_file(LitosAppWindow *win)
{
	guint index;

	GtkWidget *tabbox = gtk_notebook_get_nth_page (win->notebook, gtk_notebook_get_current_page ((win->notebook)));

	g_ptr_array_find_with_equal_func(win->litosFileList, tabbox, func, &index);

	return index;
}

LitosFile * litos_app_window_current_file(LitosAppWindow *win)
{
	return g_ptr_array_index(win->litosFileList, litos_app_window_search_file(win));
}

GtkWidget * litos_app_window_get_child(LitosAppWindow *win)
{
	return gtk_notebook_get_nth_page (win->notebook, gtk_notebook_get_current_page ((win->notebook)));
}

gboolean litos_app_window_saveornot_at_close(GtkWidget *dialog, gint response, gpointer window)
{
	LitosAppWindow *win = LITOS_APP_WINDOW(window);

	LitosFile *file = litos_app_window_current_file(win);

	switch (response)
	{
		case GTK_RESPONSE_ACCEPT:
			litos_file_save (file, NULL);

		case GTK_RESPONSE_CANCEL:
			return TRUE;

		case GTK_RESPONSE_REJECT:
			gtk_notebook_remove_page (win->notebook,gtk_notebook_get_current_page(win->notebook));
			return false;
	}

	gtk_window_destroy (GTK_WINDOW (dialog));
}

void litos_app_window_remove_child(LitosAppWindow *win)
{
	GtkWidget *tabbox = gtk_notebook_get_nth_page (win->notebook, gtk_notebook_get_current_page ((win->notebook)));

	if (tabbox != NULL)
	{
		LitosFile *file = litos_app_window_current_file(win);

		if (litos_file_get_saved(file))
			gtk_notebook_remove_page (win->notebook,gtk_notebook_get_current_page(win->notebook));

		else
		{
			GtkWidget *dialog;

			dialog = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING,
				      GTK_BUTTONS_NONE, "Save changes to document %s before closing?", litos_file_get_name(file));

			gtk_dialog_add_buttons (GTK_DIALOG(dialog), "Close without Saving", GTK_RESPONSE_REJECT,
				                                      "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT,  NULL);

			gtk_widget_show(dialog);

			g_signal_connect (dialog, "response", G_CALLBACK (litos_app_window_saveornot_at_close), win);
		}
	}
}

LitosFile *litos_app_window_get_current_file(LitosAppWindow *win)
{
	return litos_app_window_current_file(win);
}

void lito_app_window_save_finalize (GtkWidget *dialog, gint response, gpointer win)
{
	GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);

	if (response == GTK_RESPONSE_ACCEPT)
	{
		LitosFile *file = litos_app_window_get_current_file(win);
		g_autoptr (GFile) gfile = gtk_file_chooser_get_file(chooser);

		litos_file_save_as (file, gfile);

		litos_file_set_saved(file, TRUE);

		gtk_label_set_text (GTK_LABEL(litos_file_get_lbl(file)),  litos_file_get_name(file));
	}

	gtk_window_destroy (GTK_WINDOW (dialog));
}

void litos_app_window_save_as_dialog (GSimpleAction *action, GVariant *parameter, gpointer win)
{
	GtkWidget *dialog = gtk_file_chooser_dialog_new ("Save File",
		                                  NULL,
		                                  GTK_FILE_CHOOSER_ACTION_SAVE,
		                                 ("_Cancel"),
		                                  GTK_RESPONSE_CANCEL,
		                                 ("_Save"),
		                                  GTK_RESPONSE_ACCEPT,
		                                  NULL);

	gtk_window_set_transient_for(GTK_WINDOW(dialog), win);

	gtk_widget_show(dialog);

	g_signal_connect (dialog, "response", G_CALLBACK (lito_app_window_save_finalize), win);
}

void litos_app_window_save(LitosAppWindow *win)
{
	LitosFile *file = litos_app_window_get_current_file(win);

	char *filename = litos_file_get_name(file);

	if (litos_file_get_gfile(file) == NULL)
	{
		litos_app_window_save_as_dialog(NULL, NULL, win);

		//litos_app_window_change_title(win, filename);
	}

	else
	{
		GError *error = NULL;

		if (!litos_file_save(file,error))
		{
			GtkWidget *message_dialog;
			message_dialog = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "ERROR : Can't save %s.\n %s", filename, error->message);
			gtk_widget_show(message_dialog);
			g_error_free(error);
		}
	}
}

void litos_app_window_save_as(LitosAppWindow *win)
{
	litos_app_window_save_as_dialog(NULL, NULL, win);
}

static GtkSourceView* currentTabSourceView(LitosAppWindow *win)
{
	LitosFile *file = litos_app_window_current_file(win);

	return GTK_SOURCE_VIEW(litos_file_get_view);
}

static void redLblColor(LitosAppWindow *win)
{
	LitosFile* file = litos_app_window_current_file(win);
	const char *format;

	format = "<span color='red'>\%s</span>";
	litos_file_set_saved(file, FALSE);

	/*else
		format = "<span color='black'>\%s</span>";*/

	const char *markup = g_markup_printf_escaped (format, litos_file_get_name(file));
	gtk_label_set_markup (GTK_LABEL(litos_file_get_lbl(file)), markup);
	gtk_notebook_set_tab_label (win->notebook, litos_file_get_tabbox(file), litos_file_get_lbl(file));
}

void monitor_change (GObject *gobject, GParamSpec *pspec, gpointer win)	/* Function called when the file gets modified */
{
	LitosFile* file = litos_app_window_current_file(win);
	
	if (litos_file_get_saved(file) == TRUE)
		redLblColor(win);
}

LitosFile * litos_app_window_set_page(LitosAppWindow *win, struct Page *page)
{
	GtkTextTag *tag;

	GtkTextIter start_iter, end_iter;

	page->lbl = gtk_label_new(page->name);
	page->tabbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	page->scrolled = gtk_scrolled_window_new ();
	page->view = MyNewSourceview();
	page->buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (page->view));

	gtk_widget_set_hexpand (page->scrolled, TRUE);
	gtk_widget_set_vexpand (page->scrolled, TRUE);

	LitosFile *file = litos_file_set(page);

	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (page->scrolled), page->view);
	gtk_box_append (GTK_BOX(page->tabbox), page->scrolled);
	gtk_notebook_append_page_menu (win->notebook, page->tabbox, page->lbl, page->lbl);
	g_ptr_array_add(win->litosFileList, file);

	tag = gtk_text_buffer_create_tag (litos_file_get_buffer(file), NULL, NULL);

	gtk_text_buffer_get_start_iter (page->buffer, &start_iter);
	gtk_text_buffer_get_end_iter (page->buffer, &end_iter);
	gtk_text_buffer_apply_tag (page->buffer, tag, &start_iter, &end_iter);

	g_settings_bind (win->settings, "font",
			tag, "font",
			G_SETTINGS_BIND_DEFAULT);

	gtk_widget_grab_focus(GTK_WIDGET(page->view));

	return file;
}

LitosFile * litos_app_window_open(LitosAppWindow *win, GFile *gf)
{
	struct Page page;

	page.name = g_file_get_basename(gf);
	page.gf = gf;

	LitosFile *file = litos_app_window_set_page(win,&page);

	return file;
}

LitosFile * litos_app_window_new_tab(LitosAppWindow *win, GFile *gf)
{
	static int file_index = 1;

	struct Page page;

	page.name = g_strdup_printf("Untitled %d", file_index);
	file_index++;
	page.gf = NULL;

	LitosFile *file = litos_app_window_set_page(win,&page);

	g_signal_connect (gtk_text_view_get_buffer (GTK_TEXT_VIEW(page.view)), "notify::text", G_CALLBACK (monitor_change), win);

	return file;
}
