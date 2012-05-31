#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>  // ioctl
#include <gtk/gtk.h>

#include "video.h"
#include "gv.h"

#include "video_params.h"

struct ctrl_data {
    int fd;
    long id;
    GtkWidget *w;
};

static void set_ctrl(GtkWidget *widget, struct ctrl_data *ctrl_d)
{
	struct v4l2_control ctrl;

	ctrl.id = ctrl_d->id;
	ctrl.value = gtk_range_get_value(GTK_RANGE(widget));
	if (ioctl(ctrl_d->fd, VIDIOC_S_CTRL, &ctrl) < 0)
		fprintf(stderr, "set control error %d, %s\n", errno, strerror(errno));
}

static void toggle_ctrl(GtkWidget *widget, struct ctrl_data *ctrl_d)
{
	struct v4l2_control ctrl;

	ctrl.id = ctrl_d->id;
	ctrl.value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	if (ioctl(ctrl_d->fd, VIDIOC_S_CTRL, &ctrl) < 0)
		fprintf(stderr, "set control error %d, %s\n", errno, strerror(errno));
}

static void reset_ctrl(GtkButton *b, struct ctrl_data *ctrl_d)
{
	struct v4l2_queryctrl qctrl;
	struct v4l2_control ctrl;

	qctrl.id = ctrl.id = ctrl_d->id;
	if (ioctl(ctrl_d->fd, VIDIOC_QUERYCTRL, &qctrl) != 0) {
		fprintf(stderr, "query control error %d, %s\n", errno, strerror(errno));
		return;
	}
	ctrl.value = qctrl.default_value;
	if (ioctl(ctrl_d->fd, VIDIOC_S_CTRL, &ctrl) < 0)
		fprintf(stderr, "set control error %d, %s\n", errno, strerror(errno));

	gtk_range_set_value(GTK_RANGE(ctrl_d->w), ctrl.value);
//	gtk_adjustment_value_changed(GTK_ADJUSTMENT(val));
}

void video_add_cam_params(struct video_t *vid, int gid) {
    GtkWidget *frame, *hbox, *c_val, *c_reset;
    struct v4l2_queryctrl qctrl = { V4L2_CTRL_FLAG_NEXT_CTRL };
    struct v4l2_control ctrl;
//    struct v4l2_jpegcompression jc;
//    struct v4l2_streamparm parm;
    int setval;
    struct ctrl_data *ctrl_d;

    while (ioctl(vid->fd, VIDIOC_QUERYCTRL, &qctrl) == 0) {
//test
//fprintf(stderr, "%32.32s type:%d flags:0x%04x\n", qctrl.name, qctrl.type, qctrl.flags);

        frame = gtk_frame_new(strdup((char *)qctrl.name));

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(frame), hbox);

        setval = 1;
        switch (qctrl.type) {
        case V4L2_CTRL_TYPE_INTEGER:
        case V4L2_CTRL_TYPE_MENU:
            c_val = gtk_hscale_new_with_range(qctrl.minimum, qctrl.maximum, qctrl.step);
            gtk_scale_set_value_pos(GTK_SCALE(c_val), GTK_POS_LEFT);
            gtk_scale_set_draw_value(GTK_SCALE(c_val), TRUE);
            gtk_scale_set_digits(GTK_SCALE(c_val), 0);
//            gtk_range_set_update_policy(GTK_RANGE(c_val), GTK_UPDATE_DELAYED);
            break;
        case V4L2_CTRL_TYPE_BOOLEAN:
            c_val = gtk_check_button_new();
            break;
        default:
            c_val = gtk_label_new(strdup("(not treated yet"));
            setval = 0;
            break;
        }

        gtk_widget_set_size_request(GTK_WIDGET(c_val), 200, -1);
        gtk_box_pack_start(GTK_BOX(hbox), c_val, FALSE, FALSE, 0);

        if (setval) {
            ctrl_d = calloc(1, sizeof(*ctrl_d));
            ctrl_d->fd = vid->fd;
            ctrl_d->id = qctrl.id;
            ctrl_d->w = c_val;

            ctrl.id = qctrl.id;
            if (ioctl(vid->fd, VIDIOC_G_CTRL, &ctrl) < 0)
                fprintf(stderr, "get control error %d, %s\n", errno, strerror(errno));
            switch (qctrl.type) {
            case V4L2_CTRL_TYPE_INTEGER:
            case V4L2_CTRL_TYPE_MENU:
                gtk_range_set_value(GTK_RANGE(c_val), ctrl.value);
                g_signal_connect(G_OBJECT(c_val), "value_changed", G_CALLBACK(set_ctrl), (gpointer)ctrl_d);
                break;
            case V4L2_CTRL_TYPE_BOOLEAN:
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(c_val), ctrl.value);
                g_signal_connect(G_OBJECT(c_val), "toggled", G_CALLBACK(toggle_ctrl), (gpointer)ctrl_d);
                break;
            default:
                break;
            }

            c_reset = gtk_button_new_with_label("Reset");
            gtk_box_pack_end(GTK_BOX(hbox), c_reset, FALSE, FALSE, 0);
//            gtk_widget_set_size_request(GTK_WIDGET(c_reset), 60, 20);
            g_signal_connect(G_OBJECT(c_reset), "released", G_CALLBACK(reset_ctrl), (gpointer)ctrl_d);

            if (qctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
                gtk_widget_set_sensitive(c_val, FALSE);
                gtk_widget_set_sensitive(c_reset, FALSE);
            }

            gtk_widget_show(c_val);
            gtk_widget_show(c_reset);
            gtk_widget_show(hbox);

            gv_param_add_widget(gid, frame);

            gtk_widget_show(frame);
        }
        qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }
}

