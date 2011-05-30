#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "gtkviewer.h"

GtkWidget *_gv_window;
    GtkWidget *_gv_hbox;
        GtkWidget *_gv_nbmedia; // notebook
        GtkWidget *_gv_nbparams;    // notebook

void gv_init(int *pargc, char **pargv[], char *wtitle, char *help) {
    // init GUI
    gtk_init(pargc, pargv);

    // the main window
    _gv_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(_gv_window), wtitle);
    g_signal_connect(_gv_window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);

        // the main horizontal box
        _gv_hbox = gtk_hbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(_gv_window), _gv_hbox);

            // the notebook for the media
            _gv_nbmedia = gtk_notebook_new();
            gtk_container_add(GTK_CONTAINER(_gv_hbox), _gv_nbmedia);

                // the help first page
                GtkTextBuffer *bhwtxt = gtk_text_buffer_new(NULL);
                gtk_text_buffer_insert_at_cursor(bhwtxt, help, strlen(help));
                GtkWidget *whwtxt = gtk_text_view_new_with_buffer(bhwtxt);
                gtk_text_view_set_editable(GTK_TEXT_VIEW(whwtxt), FALSE);
                GtkWidget *lhwtxt = gtk_image_new_from_stock(GTK_STOCK_HOME, GTK_ICON_SIZE_BUTTON);
                gtk_notebook_prepend_page(GTK_NOTEBOOK(_gv_nbmedia), whwtxt, lhwtxt);
                gtk_widget_show(whwtxt);
                gtk_widget_show(lhwtxt);

            gtk_widget_show(_gv_nbmedia);

            // the notebook for the parameters
            _gv_nbparams = gtk_notebook_new();
            gtk_container_add(GTK_CONTAINER(_gv_hbox), _gv_nbparams);
            gtk_widget_show(_gv_nbparams);

        gtk_widget_show(_gv_hbox);

    gtk_widget_show(_gv_window);
}

// medias
int gv_media_new(char *name, char *desc, unsigned int width, unsigned int height) {
    GtkWidget *label = gtk_label_new(name);
    gtk_widget_set_tooltip_text(label, desc);
    GdkPixbuf *pb_video = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, width, height);
    GtkWidget *content = gtk_image_new_from_pixbuf(pb_video);
    g_object_unref(pb_video);

    int ret = gtk_notebook_append_page(GTK_NOTEBOOK(_gv_nbmedia), content, label);

    gtk_widget_show(content);
    gtk_widget_show(label);

    return ret;
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

// parameters
int gv_gparam_new(char *gname, char *gdesc) {
    
}

void gv_gparam_del(int gid) {
    
}

void gv_param_add(int gid, param_t *p) {
    
}

void gv_run() {
    gtk_main();
}

