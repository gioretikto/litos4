#include <gtk/gtk.h>

#include "litosfile.h"

GtkWidget* MyNewSourceview();
void litos_app_window_set_file (LitosAppWindow *win, GtkTextTag *tag);
void litos_app_window_add_title(LitosAppWindow *win, GtkWidget *scrolled, char *filename);
int litos_app_window_search_file(LitosAppWindow *win);
void litos_app_winddow_fileadd(LitosAppWindow *win, LitosFile *file);
LitosFile * litos_app_window_current_file(LitosAppWindow *win);
GtkWidget * litos_app_window_get_child(LitosAppWindow *win);
void litos_app_window_change_title(LitosAppWindow *win, char *filename);
void litos_app_winddow_set_visible_child(LitosAppWindow *win, GtkWidget *scrolled);

struct _LitosFile
{
	GObject parent;

	GFile* gfile;

	GtkWidget *scrolled;

	/*GtkSourceView*/

	GtkWidget *view;

	/* the text buffer of the file */
	GtkTextBuffer *buffer;

	/*filename */
	gchar *name;

	_Bool saved;
};

G_DEFINE_TYPE (LitosFile, litos_file, G_TYPE_OBJECT)

static void
litos_file_init (LitosFile *file)
{
	file->gfile = NULL;

	file->buffer = NULL;

	file->name = NULL;

	file->saved = TRUE;

	file->scrolled = NULL;
}

static void
litos_file_dispose (GObject *object)
{
	LitosFile *file = LITOS_FILE (object);

	g_free (file->name);

	G_OBJECT_CLASS (litos_file_parent_class)->dispose (object);
}

static void
litos_file_class_init (LitosFileClass *class)
{
	G_OBJECT_CLASS (class)->dispose = litos_file_dispose;
}

LitosFile *litos_file_new(LitosAppWindow *win)
{
	return g_object_new (LITOS_TYPE_FILE, NULL);
}

LitosFile * litos_file_new_tab(LitosAppWindow *win)
{
	GtkTextTag *tag;

	LitosFile *file = litos_file_new(win);

	static int file_index = 1;

	file->name = g_strdup_printf("Untitled %d", file_index);

	GtkTextIter start_iter, end_iter;

	file->scrolled = gtk_scrolled_window_new ();

	file->view = MyNewSourceview();

	gtk_widget_set_hexpand (file->scrolled, TRUE);
	gtk_widget_set_vexpand (file->scrolled, TRUE);

	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (file->scrolled), file->view);

	litos_app_window_add_title(win, file->scrolled, file->name);

	file->buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (file->view));

	tag = gtk_text_buffer_create_tag (file->buffer, NULL, NULL);

	gtk_text_buffer_get_start_iter (file->buffer, &start_iter);
	gtk_text_buffer_get_end_iter (file->buffer, &end_iter);
	gtk_text_buffer_apply_tag (file->buffer, tag, &start_iter, &end_iter);

	litos_app_winddow_fileadd (win,file);

	litos_app_window_set_file (win,tag);

	file_index++;

	return file;
}

void litos_file_load (LitosAppWindow *win, GFile *gf)
{
	char *contents;
	gsize length;
	GError *error = NULL;

	LitosFile *file = litos_file_new_tab(win);

	file->gfile = gf;

	g_free (file->name);
	file->name = g_file_get_basename(gf);

	if (g_file_load_contents (file->gfile, NULL, &contents, &length, NULL, &error))
	{
		gtk_text_buffer_set_text (file->buffer, contents, length);
		litos_app_winddow_set_visible_child(win, file->scrolled);
		litos_app_window_change_title(win, file->name);
		g_free (contents);
	}

	else
	{
		g_error("%s\n", error->message);
		g_clear_error(&error);
	}
}

void litos_file_save(LitosAppWindow *win, GFile *gf)
{
	GtkWidget *err_dialog;
	char *contents;
	GtkTextIter start_iter;
	GtkTextIter end_iter;
	GError *error = NULL;

	LitosFile *current_file = litos_app_window_current_file(win);

	if (gf != NULL)
	{
		current_file->name = g_file_get_basename(gf);
		current_file->gfile = gf;
	}		

	gtk_text_buffer_get_bounds(current_file->buffer, &start_iter, &end_iter);
	contents = gtk_text_buffer_get_text(current_file->buffer, &start_iter, &end_iter, TRUE);

	if (g_file_replace_contents(current_file->gfile, contents, strlen(contents), NULL, TRUE, G_FILE_CREATE_NONE, NULL, NULL, &error))
		litos_app_window_change_title(win, current_file->name);

	else
	{
		g_error("%s\n", error->message);
		g_clear_error(&error);
	}

	g_free(contents);
}

GtkWidget * litos_file_get_scrolled(LitosFile *file)
{
	return file->scrolled;
}
