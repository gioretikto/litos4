#include <gtk/gtk.h>

#include "litosapp.h"
#include "litosappwin.h"
#include "litosfile.h"

GtkWidget* MyNewSourceview();

LitosFile * litos_file_new(LitosAppWindow *win);
GtkWidget * litos_file_get_scrolled(LitosFile *file);
GFile *litos_file_get_gfile(LitosFile* file);
gboolean litos_file_save(LitosFile *file, GError *error);
void litos_file_save_as(LitosFile* file, GFile *new_file);
gchar *litos_file_get_name(LitosFile *file);
GtkWidget * litos_file_get_view(LitosFile *file);
GtkTextBuffer *litos_file_get_buffer(LitosFile *file);
LitosFile * litos_file_set(char *filename, GFile *gf);

struct _LitosAppWindow
{
	GtkApplicationWindow parent;

	GSettings *settings;
	GtkWidget *stack;
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
			GParamSpec *pspec,
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
	win->litosFileList = g_ptr_array_new_full(0, g_object_unref);

	g_settings_bind (win->settings, "transition",
		win->stack, "transition-type",
		G_SETTINGS_BIND_DEFAULT);

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

GtkWidget * litos_app_window_get_child(LitosAppWindow *win)
{
	return gtk_stack_get_visible_child(GTK_STACK(win->stack));
}

void litos_app_window_remove_child(LitosAppWindow *win)
{
	GtkWidget * child = gtk_stack_get_visible_child(GTK_STACK(win->stack));

	if (child != NULL)
		gtk_stack_remove(GTK_STACK(win->stack), child);	
}

void litos_app_window_change_title(LitosAppWindow *win, char *filename)
{
	gtk_stack_page_set_title (gtk_stack_get_page(GTK_STACK(win->stack),litos_app_window_get_child(win)), filename);
	printf("filename is: %s", filename);
}

static gboolean func (gconstpointer array_element, gconstpointer scrolled_win)
{
	return litos_file_get_scrolled ((LITOS_FILE((void*)array_element))) == scrolled_win;
}

guint litos_app_window_search_file(LitosAppWindow *win)
{
	guint index;

	GtkWidget *scrolled_win = gtk_stack_get_visible_child(GTK_STACK(win->stack));

	g_ptr_array_find_with_equal_func(win->litosFileList, scrolled_win, func, &index);

	return index;
}

LitosFile * litos_app_window_current_file(LitosAppWindow *win)
{
	return g_ptr_array_index(win->litosFileList, litos_app_window_search_file(win));
}

LitosFile *litos_app_window_get_current_file(LitosAppWindow *win)
{
	return litos_app_window_current_file(win);
}

void litos_app_winddow_set_visible_child(LitosAppWindow *win, GtkWidget *scrolled)
{
	gtk_stack_set_visible_child(GTK_STACK (win->stack), scrolled);
}

void lito_app_window_save_finalize (GtkWidget *dialog, gint response, gpointer win)
{
	GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);

	if (response == GTK_RESPONSE_ACCEPT)
	{
		LitosFile *file = litos_app_window_get_current_file(win);
		g_autoptr (GFile) gfile = gtk_file_chooser_get_file(chooser);

		printf("GFile: %p\n", (void*)gfile); fflush(stdout);
		litos_file_save_as (file, gfile);

		litos_app_window_change_title(LITOS_APP_WINDOW(win), litos_file_get_name(file));
	}

	g_object_unref(dialog);
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

		litos_app_window_change_title(win, filename);
	}

	else
	{
		GError *error = NULL;
		if (litos_file_save(file,error))
		{
			GtkWidget *message_dialog;
			message_dialog = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "ERROR : Can't save %s.", filename);
			gtk_widget_show(message_dialog);
		}
	}
}

void litos_app_window_save_as(LitosAppWindow *win)
{
	litos_app_window_save_as_dialog(NULL, NULL, win);
}

LitosFile * litos_app_window_new_tab(LitosAppWindow *win, GFile *gf)
{
	static int file_index = 1;

	char *filename;

	if (gf == NULL) /* at Ctrl+N*/
	{
		filename = g_strdup_printf("Untitled %d", file_index);
		file_index++;
	}

	else /* we're loading a file */
		filename = g_file_get_basename(gf);

	GtkTextTag *tag;

	GtkTextIter start_iter, end_iter;

	LitosFile *file = litos_file_set(filename,gf);

	GtkWidget *scrolled = litos_file_get_scrolled(file);

	GtkWidget *view = litos_file_get_view(file);

	GtkTextBuffer *buffer = litos_file_get_buffer(file);

	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled), view);

	gtk_stack_add_titled (GTK_STACK (win->stack), scrolled, filename, filename);

	g_ptr_array_add(win->litosFileList, file);

	litos_app_winddow_set_visible_child(win, scrolled);

	litos_app_window_change_title(win, filename);

	gtk_widget_set_sensitive (win->search, TRUE);

	tag = gtk_text_buffer_create_tag (litos_file_get_buffer(file), NULL, NULL);

	gtk_text_buffer_get_start_iter (buffer, &start_iter);
	gtk_text_buffer_get_end_iter (buffer, &end_iter);
	gtk_text_buffer_apply_tag (buffer, tag, &start_iter, &end_iter);

	g_settings_bind (win->settings, "font",
			tag, "font",
			G_SETTINGS_BIND_DEFAULT);

	return file;
}
