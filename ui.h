#include "gphoto_api/api.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <glib/gprintf.h>

typedef struct _setting_node
{
	GtkWidget *label,*combo_box;
	struct _setting_node *next;
}setting_node;

