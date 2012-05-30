#include <stdlib.h>
#include <assert.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "gv.h"

#include "gv_media.h"

int gv_media_new(char *name, char *desc, unsigned int width, unsigned int height) {
    GtkWidget *label = gtk_label_new(name);
    gtk_widget_set_tooltip_text(label, desc);
printf("__%dx%d__\n", width, height);

    GdkPixbuf *pb_video = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, width, height);
    GtkWidget *content = gtk_image_new_from_pixbuf(pb_video);
    g_object_unref(pb_video);

    int mid = gtk_notebook_append_page(GTK_NOTEBOOK(_gv_nbmedia), content, label);

    gtk_widget_show(content);
    gtk_widget_show(label);

    return mid;
}

void gv_media_update(int mid, unsigned char *data, unsigned int width, unsigned int height, gv_destroy destroy, void *destroy_data) {
    GtkWidget *content = gtk_notebook_get_nth_page(GTK_NOTEBOOK(_gv_nbmedia), mid);
    assert(content);

    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB, FALSE, 8, width, height, width*3, (GdkPixbufDestroyNotify)destroy, destroy_data);
    gtk_image_set_from_pixbuf(GTK_IMAGE(content), pixbuf);
//    gtk_widget_queue_draw(GTK_WIDGET(content));
    g_object_unref(pixbuf);
}

void gv_media_del(int mid) {
    // TODO
}

