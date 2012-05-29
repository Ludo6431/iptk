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
#include <glib.h>

#include "tinyjpeg.h"
#include "bmpsave.h"
#include "v4l2_wrapper.h"
#include "outils.h"
#include "analyse.h"
#include "cust_params.h"

#include "gtkviewer.h"

#include "main.h"

// TODO: si on fourni un fichier de sauvegarde, il faut charger et sauvegarder les paramètres

int update(context_t *ctx) {
    int ret = video_read(ctx->fd, ctx->buffer, ctx->size);
//printf("ret=%d\n", ret);
//usleep(10000);

    unsigned char *data = NULL;

    switch(ctx->fmt.fmt.pix.pixelformat) {
    case V4L2_PIX_FMT_SBGGR8: {
        data = (unsigned char *)malloc(640*480*3);
        sbggr8_to_bgr(ctx->buffer, data, ctx->width, ctx->height);

        gv_media_update(ctx->mid_rawcam, data, ctx->width, ctx->height, (gv_destroy)free, NULL);
        break;
    }
    case V4L2_PIX_FMT_JPEG: {
        struct jdec_private *jdec;
        unsigned char *components[3];

        // TODO: use hardware accelerated decomp routines
        jdec = tinyjpeg_init();
        if(!jdec) mexit(1, "fail tinyjpeg_init");
        if(tinyjpeg_parse_header(jdec, ctx->buffer, ret) < 0) mexit(1, tinyjpeg_get_errorstring(jdec));
        tinyjpeg_get_size(jdec, &ctx->width, &ctx->height);
        if(tinyjpeg_decode(jdec, TINYJPEG_FMT_RGB24) < 0) mexit(1, tinyjpeg_get_errorstring(jdec));
        tinyjpeg_get_components(jdec, components);
        data = components[0];

        void jpegfree(void *_data, struct jdec_private *jdec) {
            tinyjpeg_free(jdec);
        }
        gv_media_update(ctx->mid_rawcam, data, ctx->width, ctx->height, (gv_destroy)jpegfree, jdec);
        break;
    }
    default:
        mexit(1, "unknown format");
        break;
    }

    // update pictures
    analyse_update(ctx, data);

    return TRUE;
}

int main(int argc, char *argv[]) {
    context_t ctx;

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

    // init viewer
    gv_init(&argc, &argv, "Visu balise robot en temps réel", "Bienvenue ici!\n\nIl y aura une aide ici.");

    // add some video sources
    ctx.mid_rawcam = gv_media_new("Caméra pure", "Caméra sans traitement", ctx.width, ctx.height);

    // init video pics handlers
    analyse_init(&ctx);

    // periodic update
#if 1
    g_timeout_add(1000/5, (GSourceFunc)update, &ctx);
#else
    // FIXME... (the app freezes when enabled)
    int handle(GIOChannel *source, GIOCondition condition, context_t *ctx) {
        return update(ctx);
    }
    GIOChannel *ch = g_io_channel_unix_new(ctx.fd);
    g_io_channel_set_encoding(ch, NULL, NULL);  // this is binary data
    g_io_add_watch(ch, G_IO_IN /*| G_IO_PRI*/, (GIOFunc)handle, &ctx);
#endif

    // run viewer
    gv_run();

    if(close(ctx.fd)<0)
        mexit (1, "close");

    return EXIT_SUCCESS;
}

