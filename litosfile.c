#include <gtk/gtk.h>

#include "litosfile.h"

GtkWidget* MyNewSourceview();
void litos_app_window_set_file (LitosAppWindow *win, GtkTextTag *tag);
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
	return g_object_new (LITOS_TYPE_FILE, NULL);
}

LitosFile * litos_file_new_tab(LitosAppWindow *win)
{
	GtkTextTag *tag;

	LitosFile *file = litos_file_new(win);

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

	litos_app_window_set_file (win,tag);

	return file;
}

void litos_file_load (LitosAppWindow *win, GFile *gf)
{
	char *contents;
	gsize length;

	LitosFile *file = litos_file_new_tab(win);

	file->gfile = gf;

	g_free (file->name);
	file->name = g_file_get_basename(gf);

	if (g_file_load_contents (file->gfile, NULL, &contents, &length, NULL, NULL))
	{
		gtk_text_buffer_set_text (file->buffer, contents, length);
		g_free (contents);
	}
}

void litos_file_save(LitosAppWindow *win, GFile *gf)
{
	GtkWidget *err_dialog;
	char *contents;
	gchar *filename;
	GtkTextIter start_iter;
	GtkTextIter end_iter;

	g_free (file->name);
	file->name = g_file_get_basename(gf);
	gtk_text_buffer_get_bounds(file->buffer, &start_iter, &end_iter);
	contents = gtk_text_buffer_get_text(file->buffer, &start_iter, &end_iter, TRUE);

	if (g_file_replace_contents(file->gfile), contents, strlen(contents), NULL, TRUE, G_FILE_CREATE_NONE, NULL, NULL, NULL)) {
		file->save = TRUE;
		//gtk_window_set_title(GTK_WINDOW(win), filename);
	}else {
		err_dialog = gtk_err_dialog_new(GTK_WINDOW(win), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
		GTK_BUTTONS_CLOSE, "ERROR : Can't save %s.", file->name);
		gtk_dialog_run(GTK_DIALOG(err_dialog));
		gtk_widget_destroy(err_dialog);
	}

	g_free(filename);
	g_free(contents);
}
