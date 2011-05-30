#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <asm/types.h> 
#include <linux/videodev2.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "tinyjpeg.h"
#include "bmpsave.h"
#include "v4l2_wrapper.h"
#include "outils.h"
#include "analyse.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>


typedef struct {
    GtkWidget *window;
        GtkWidget *hbox;
            GtkWidget *im_video;
            GtkWidget *bt_run;

    int fd;
    struct v4l2_format fmt;
    unsigned int width, height;

    void *buffer;
    unsigned int size;
} context_t;

int handle(GIOChannel *source, GIOCondition condition, context_t *ctx);
int update(context_t *ctx);
void go(GtkWidget *widget, context_t *ctx) {
    g_timeout_add(1000/4, update, ctx); return;

    GIOChannel *ch = g_io_channel_unix_new(ctx->fd);
    g_io_channel_set_encoding(ch, NULL, NULL);  // this is binary data

    g_io_add_watch(ch, G_IO_IN | G_IO_PRI, (GIOFunc)handle, ctx);
}

int handle(GIOChannel *source, GIOCondition condition, context_t *ctx) {
    update(ctx);

    return TRUE;
}

int update(context_t *ctx) {
    GdkPixbuf *pb_video = NULL;
    int ret = video_read(ctx->fd, ctx->buffer, ctx->size);
printf("ret=%d\n", ret);
//usleep(10000);

    switch(ctx->fmt.fmt.pix.pixelformat) {
    case V4L2_PIX_FMT_SBGGR8: {
        unsigned char *ddata = (unsigned char *)malloc(640*480*3);
        sbggr8_to_bgr(ctx->buffer, ddata, ctx->width, ctx->height);

        analyse(ddata, ctx->width, ctx->height);

        pb_video = gdk_pixbuf_new_from_data(ddata, GDK_COLORSPACE_RGB, FALSE, 8, ctx->width, ctx->height, ctx->width*3, (GdkPixbufDestroyNotify)free, NULL);
        break;
    }
    case V4L2_PIX_FMT_JPEG: {
        struct jdec_private *jdec;
        unsigned char *components[3];

        jdec = tinyjpeg_init();
        if(!jdec) mexit(1, "fail tinyjpeg_init");
        if(tinyjpeg_parse_header(jdec, ctx->buffer, ret) < 0) mexit(1, tinyjpeg_get_errorstring(jdec));
        tinyjpeg_get_size(jdec, &ctx->width, &ctx->height);
        if(tinyjpeg_decode(jdec, TINYJPEG_FMT_RGB24) < 0) mexit(1, tinyjpeg_get_errorstring(jdec));
        tinyjpeg_get_components(jdec, components);

        analyse(components[0], ctx->width, ctx->height);

        void jpegfree(void *data, struct jdec_private *jdec) {
            tinyjpeg_free(jdec);
        }
        pb_video = gdk_pixbuf_new_from_data(components[0], GDK_COLORSPACE_RGB, FALSE, 8, ctx->width, ctx->height, ctx->width*3, (GdkPixbufDestroyNotify)jpegfree, jdec);
        break;
    }
    default:
        mexit(1, "unknown format");
        break;
    }

    gtk_image_set_from_pixbuf(GTK_IMAGE(ctx->im_video), pb_video);
    gtk_widget_queue_draw(GTK_WIDGET(ctx->im_video));
    g_object_unref(pb_video);

    return TRUE;
}

int main(int argc, char *argv[]) {
    context_t ctx;
    GdkPixbuf *pb_video;  // this is a GObject => we need to unref

    bzero(&ctx, sizeof(ctx));

    // open device
    ctx.fd = video_open(argc==2?argv[1]:"/dev/video0");

    // init device
    ctx.fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ctx.fmt.fmt.pix.width       = 640;
    ctx.fmt.fmt.pix.height      = 480;
    ctx.fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;//V4L2_PIX_FMT_JPEG;//V4L2_PIX_FMT_YUYV;
    ctx.fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

#define argsfourcc(f) (f)&0xff, ((f)>>8)&0xff, ((f)>>16)&0xff, ((f)>>24)&0xff

printf("asked:%dx%d %c%c%c%c\n", ctx.fmt.fmt.pix.width, ctx.fmt.fmt.pix.height, argsfourcc(ctx.fmt.fmt.pix.pixelformat));
    ctx.buffer = video_init(ctx.fd, &ctx.fmt, &ctx.size);
printf("got  :%dx%d %c%c%c%c\n", ctx.fmt.fmt.pix.width, ctx.fmt.fmt.pix.height, argsfourcc(ctx.fmt.fmt.pix.pixelformat));
    ctx.width = ctx.fmt.fmt.pix.width;
    ctx.height = ctx.fmt.fmt.pix.height;

printf("size=%d\n", ctx.size);

    // init GUI
    gtk_init(&argc, &argv);

    // the main window
    ctx.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(ctx.window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);

        // the main horizontal box
        ctx.hbox = gtk_hbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(ctx.window), ctx.hbox);

            // the video image
            pb_video = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, ctx.width, ctx.height);
            ctx.im_video = gtk_image_new_from_pixbuf(pb_video);
            g_object_unref(pb_video);
            gtk_container_add(GTK_CONTAINER(ctx.hbox), ctx.im_video);
            gtk_widget_show(ctx.im_video);

            // the run button
            ctx.bt_run = gtk_button_new_with_label("Run!");
            g_signal_connect(G_OBJECT(ctx.bt_run), "clicked", G_CALLBACK(go), &ctx);
            gtk_container_add(GTK_CONTAINER(ctx.hbox), ctx.bt_run);
            gtk_widget_show(ctx.bt_run);

        gtk_widget_show(ctx.hbox);

    gtk_widget_show(ctx.window);

    gtk_main();

    if(close(ctx.fd)<0)
        mexit (1, "close");

    return EXIT_SUCCESS;
}

