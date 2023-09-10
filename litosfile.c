#include <gtk/gtk.h>

#include "litosfile.h"

GtkWidget* MyNewSourceview();

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

LitosFile *litos_file_new()
{
	return g_object_new (LITOS_TYPE_FILE, NULL);
}

LitosFile * litos_file_set(char *filename, GFile *gf)
{
	LitosFile *file = litos_file_new();

	file->name = filename;
	file->gfile = gf;
	file->scrolled = gtk_scrolled_window_new ();
	file->view = MyNewSourceview();

	gtk_widget_set_hexpand (file->scrolled, TRUE);
	gtk_widget_set_vexpand (file->scrolled, TRUE);

	file->buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (file->view));

	return file;
}

void litos_file_load (LitosFile *file)
{
	char *contents;
	gsize length;
	GError *error = NULL;

	if (g_file_load_contents (file->gfile, NULL, &contents, &length, NULL, &error))
	{
		gtk_text_buffer_set_text (file->buffer, contents, length);
		g_free (contents);
	}

	else
	{
		g_error("%s\n", error->message);
		g_clear_error(&error);
	}
}

void litos_file_save(LitosFile *file)
{
	if (file->gfile != NULL)
	{
		GtkWidget *err_dialog;
		char *contents;
		GtkTextIter start_iter;
		GtkTextIter end_iter;
		GError *error = NULL;

		gtk_text_buffer_get_bounds(file->buffer, &start_iter, &end_iter);
		contents = gtk_text_buffer_get_text(file->buffer, &start_iter, &end_iter, TRUE);

		if (!g_file_replace_contents(file->gfile, contents, strlen(contents), NULL, TRUE, G_FILE_CREATE_NONE, NULL, NULL, &error))
		{
			g_error("%s\n", error->message);
			g_clear_error(&error);
		}

		g_free(contents);
	}
}

void litos_file_save_as(LitosFile* file, GFile *new_file)
{
	if (new_file != NULL)
		g_object_ref(new_file);

	file->gfile = new_file;
	g_free (file->name);
	file->name = g_file_get_basename(new_file);
	litos_file_save(file);
}

GtkWidget * litos_file_get_scrolled(LitosFile *file)
{
	return file->scrolled;
}

GtkWidget * litos_file_get_view(LitosFile *file)
{
	return file->view;
}

GFile *litos_file_get_gfile(LitosFile* file)
{
	return file->gfile;
}

gchar *litos_file_get_name(LitosFile *file)
{
	return file->name;
}

GtkTextBuffer *litos_file_get_buffer(LitosFile *file)
{
	return file->buffer;
}
