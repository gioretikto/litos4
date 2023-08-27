#include <gtk/gtk.h>

#include "litosfile.h"

struct _LitosFile
{
	GObject parent;

	GFile* gfile;

	GtkWidget *scrolled;

	/*GtkSourceView*/

	GtkWidget *view;

	/* the text buffer of the file */
	GtkTextBuffer      *buffer;

	/*filename */
	gchar              *name;

	GtkTextTag *tag;

	_gboolean *saved;

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

	file->tag = NULL;
}

gchar * litos_file_basename(LitosFile *file)
{
	return g_file_get_basename(file->gfile);
}


static void
litos_app_window_dispose (GObject *object)
{
	LitosFile *file;

	file = LITOS_FILE (object);

	g_free (file->name);


	G_OBJECT_CLASS (litos_file_parent_class)->dispose (object);
}

static void
litos_file_class_init (LitosFileClass *class)
{
	G_OBJECT_CLASS (class)->dispose = litos_file_dispose;
}

LitosFile *
litos_file_new(LitosAppWindow *win)
{
	LitosFile *file = litos_file_new();

	char *basename;
	GtkTextBuffer *buffer;
	GtkTextTag *tag;

	GtkTextIter start_iter, end_iter;

	basename = litos_file_basename (file);

	file->scrolled = gtk_scrolled_window_new ();
	file->view = MyNewSourceview();

	gtk_widget_set_hexpand (file->scrolled, TRUE);
	gtk_widget_set_vexpand (file->scrolled, TRUE);

	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (file->scrolled), view);
	gtk_stack_add_titled (GTK_STACK (win->stack), file->scrolled, basename, basename);

	file->buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (file->view));

	file->tag = gtk_text_buffer_create_tag (file->buffer, NULL, NULL);

	g_settings_bind (win->settings, "font",
		file->tag, "font",
		G_SETTINGS_BIND_DEFAULT);

	gtk_text_buffer_get_start_iter (file->buffer, &start_iter);
	gtk_text_buffer_get_end_iter (file->buffer, &end_iter);
	gtk_text_buffer_apply_tag (file->buffer, tag, &start_iter, &end_iter);

	gtk_widget_set_sensitive (win->search, TRUE);
	
	return file;
}
