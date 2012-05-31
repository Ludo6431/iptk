#include <stdlib.h>
#include <assert.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "gv.h"

#include "gv_params.h"

int gv_gparam_new(char *gname, char *gdesc) {
    GtkWidget *glabel = gtk_label_new(gname);
    gtk_widget_set_tooltip_text(glabel, gdesc);
    GtkWidget *gvbox = gtk_vbox_new(FALSE /* homogeneous */, 0);

    int ret = gtk_notebook_append_page(GTK_NOTEBOOK(_gv_nbparams), gvbox, glabel);

    gtk_widget_show(gvbox);
    gtk_widget_show(glabel);

    return ret;
}

void gv_gparam_del(int gid) {
    // TODO
}

void gv_param_add(int gid, param_t *p) {
    GtkWidget *w = gv_param_get_widget(p);

    gv_param_add_widget(gid, w);

    gtk_widget_show(w);
}


void pt_int_modified(GtkWidget *button, param_t *p) {
    int val = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(button));

    *(volatile int *)p->val = val;
}

GtkWidget *gv_param_get_widget(param_t *p) {
    GtkWidget *frame, *button = NULL;

    assert(p);

    frame = gtk_frame_new(p->name);
    if(p->desc)
        gtk_widget_set_tooltip_text(frame, p->desc);

    switch(p->type) {
    case PT_BOOL:
        // TODO
        break;
    case PT_INT:
        button = gtk_spin_button_new_with_range((gdouble)p->pt_int.min, (gdouble)p->pt_int.max, (gdouble)p->pt_int.step);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(button), (gdouble)*(int *)p->val);

        g_signal_connect(G_OBJECT(button), "value-changed", G_CALLBACK(pt_int_modified), p);
        break;
    default:
        // TODO erreur... button=...
        break;
    }

    gtk_container_add(GTK_CONTAINER(frame), button);

    gtk_widget_show(button);

    return frame;
}

void gv_param_add_widget(int gid, GtkWidget *w) {
    GtkWidget *gvbox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(_gv_nbparams), gid);
    assert(gvbox);

    gtk_box_pack_start(GTK_BOX(gvbox), w, FALSE /* expand */, FALSE /* fill */, 0 /* padding */);

//    gtk_widget_show(w);
//    gtk_widget_show(gvbox);
}

