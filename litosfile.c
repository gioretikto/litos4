#include <gtk/gtk.h>

#include "litosfile.h"

GtkWidget* MyNewSourceview();
void litos_app_window_setter (LitosAppWindow *win, GtkTextTag *tag);
void litos_app_window_add_title(LitosAppWindow *win, GtkWidget *scrolled, char *filename);

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
	return g_object_new (LITOS_FILE_TYPE, NULL);
}

void litos_file_new_tab(LitosAppWindow *win, LitosFile *file)
{
	GtkTextTag *tag;

	file->name = g_strdup("Untitled");

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

	litos_app_window_setter (win,tag);
}

void litos_file_load (LitosAppWindow *win, GFile *gf)
{
	char *contents;
	gsize length;

	LitosFile *file = litos_file_new(win);

	litos_file_new_tab(win,file);

	file->gfile = gf;

	file->name = g_file_get_basename(gf);

	if (g_file_load_contents (file->gfile, NULL, &contents, &length, NULL, NULL))
	{
		gtk_text_buffer_set_text (file->buffer, contents, length);
		g_free (contents);
	}
}
