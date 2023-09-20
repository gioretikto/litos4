#include <gtk/gtk.h>

#include "litosfile.h"
#include "page.h"

GtkWidget* MyNewSourceview();
void highlight_buffer(GtkTextBuffer *buffer, char *filename);
void litos_file_set_unsaved(LitosFile *file);

struct _LitosFile
{
	GObject parent;

	GFile* gfile;

	GtkWidget *scrolled;

	GtkWidget *lbl;

	GtkWidget* tabbox;

	/*GtkSourceView*/

	GtkWidget *view;

	/* the text buffer of the file */
	GtkTextBuffer *buffer;

	/*filename */
	gchar *name;

	gboolean saved;
};

G_DEFINE_TYPE (LitosFile, litos_file, G_TYPE_OBJECT)

static void
litos_file_init (LitosFile *file)
{
	file->gfile = NULL;

	file->buffer = NULL;

	file->name = NULL;

	file->lbl = NULL;

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

GtkWidget * litos_file_get_lbl(LitosFile *file)
{
	return file->lbl;
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

gboolean litos_file_get_saved(LitosFile *file)
{
	return file->saved;
}

void litos_file_set_saved(LitosFile *file)
{
	file->saved = TRUE;
}

GtkWidget * litos_file_get_tabbox(LitosFile *file)
{
	return file->tabbox;
}

typedef enum
{
	PROP_SAVED = 1,
	N_PROPERTIES

} LitosFileProperty;

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
litos_file_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
	LitosFile *self = LITOS_FILE (object);

	switch ((LitosFileProperty) property_id)
	{
		case PROP_SAVED:
			self->saved = g_value_get_boolean (value);
			break;

		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static void
litos_file_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
	LitosFile *self = LITOS_FILE (object);

	switch ((LitosFileProperty) property_id)
	{
		case PROP_SAVED:
			g_value_set_boolean (value, TRUE);
			break;

		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

void litos_file_set_unsaved(LitosFile *file)
{
	file->saved = FALSE;
	g_object_notify_by_pspec (G_OBJECT (file), obj_properties[PROP_SAVED]);
}

LitosFile * litos_file_set(struct Page *page)
{
	LitosFile *file = litos_file_new();

	file->name = page->name;
	file->gfile = page->gf;
	file->scrolled = page->scrolled;
	file->tabbox = page->tabbox;
	file->view = page->view;
	file->lbl = page->lbl;
	file->buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (file->view));

	return file;
}

gboolean litos_file_load (LitosFile *file, GError *error)
{
	char *contents;
	gsize length;

	if (g_file_load_contents (file->gfile, NULL, &contents, &length, NULL, &error))
	{
		gtk_text_buffer_set_text (file->buffer, contents, length);
		highlight_buffer(file->buffer, file->name);
		g_free (contents);
		return TRUE;
	}

	else
		return FALSE;
}

gboolean litos_file_save(LitosFile *file, GError *error)
{
	if (file->gfile != NULL)
	{
		char *contents;
		GtkTextIter start_iter;
		GtkTextIter end_iter;

		gtk_text_buffer_get_bounds(file->buffer, &start_iter, &end_iter);
		contents = gtk_text_buffer_get_text(file->buffer, &start_iter, &end_iter, TRUE);

		if (!g_file_replace_contents(file->gfile, contents, strlen(contents), NULL, TRUE, G_FILE_CREATE_NONE, NULL, NULL, &error))
		{
			g_free(contents);
			return FALSE;
		}

		else
		{
			file->saved = TRUE;
			g_free(contents);
		}
	}

	return TRUE;
}

void litos_file_save_as(LitosFile* file, GFile *new_file)
{
	if (new_file != NULL)
		g_object_ref(new_file);

	file->gfile = new_file;

	g_free (file->name);
	file->name = g_file_get_basename(new_file);

	litos_file_save(file, NULL);
}
