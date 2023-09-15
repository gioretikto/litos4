struct Page
{
	GtkWidget *tabbox;
	GtkWidget *scrolled;
	GtkTextBuffer *buffer;
	char *name;
	GFile *gf;
	GtkWidget *view;
	GtkWidget *lbl;
};
