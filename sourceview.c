#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include "litosappwin.h"

GtkWidget* MyNewSourceview()
{
	GtkWidget *source_view;

	GtkSourceBuffer *source_buffer = gtk_source_buffer_new (NULL);

	source_view = gtk_source_view_new_with_buffer (source_buffer);

	gtk_source_buffer_set_style_scheme (source_buffer,
	gtk_source_style_scheme_manager_get_scheme (
	gtk_source_style_scheme_manager_get_default (), "gtk-application-prefer-dark-theme"));

	gtk_source_view_set_show_line_numbers (GTK_SOURCE_VIEW((source_view)), TRUE);

	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(source_view), GTK_WRAP_WORD_CHAR);

	return source_view;
}
