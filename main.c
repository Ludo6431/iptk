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

#include "bmpsave.h"
#include "video.h"
#include "video_params.h"
#include "tools.h"
#include "process.h"
#include "param.h"
#include "gv.h"
#include "context.h"

// TODO: si on fourni un fichier de sauvegarde, il faut charger et sauvegarder les paramètres

int update(context_t *ctx) {
//printf(".\n");

    ctx->curbuffer ^= 1;    // change current buffer

    // get current frame
    video_read(&ctx->cam, ctx->buffers[ctx->curbuffer]);

    // update media
    analyse_update(ctx, ctx->buffers[ctx->curbuffer]);

    return TRUE;
}

int main(int argc, char *argv[]) {
    context_t ctx;

    bzero(&ctx, sizeof(ctx));

    // open device
    video_open(&ctx.cam, argc==2?argv[1]:"/dev/video0");

    // configure device according to the desired format
    {
        struct v4l2_format vid_fmt;
        memset(&vid_fmt, 0, sizeof(vid_fmt));

        vid_fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vid_fmt.fmt.pix.width       = 640;
        vid_fmt.fmt.pix.height      = 480;
        vid_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
        vid_fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

        video_config(&ctx.cam, &vid_fmt);

#ifdef DEBUG
        printf("pixfmt: %c%c%c%c %dx%d\n",
        vid_fmt.fmt.pix.pixelformat & 0xff,
        (vid_fmt.fmt.pix.pixelformat >> 8) & 0xff,
        (vid_fmt.fmt.pix.pixelformat >> 16) & 0xff,
        (vid_fmt.fmt.pix.pixelformat >> 24) & 0xff,
        vid_fmt.fmt.pix.width, vid_fmt.fmt.pix.height);
#endif

        ctx.width = vid_fmt.fmt.pix.width;
        ctx.height = vid_fmt.fmt.pix.height;
        ctx.sizeimage = vid_fmt.fmt.pix.sizeimage;
    }

    // allocate data for each buffer
    ctx.buffers[0] = malloc(ctx.sizeimage);
    ctx.buffers[1] = malloc(ctx.sizeimage);
    ctx.curbuffer = 0;  // current displayed buffer

    // init viewer
    gv_init(&argc, &argv, "Visu balise robot en temps réel", "Bienvenue ici!\n\nIl y aura une aide ici.");

    // init video pics handlers
    // add some video sources
    analyse_init(&ctx);

    // add parameters of the camera
    {
        int gid = gv_gparam_new("Camera", "Paramètres de la caméra");
        video_add_cam_params(&ctx.cam, gid);
    }

    // setup periodic update
#if 0
// in this case (user-fixed framerate), you would enable the select in video_read
    g_timeout_add(1000/5, (GSourceFunc)update, &ctx);
#else
// in this case (maximum framerate), you must remove the select in video_read
    int handle(GIOChannel *source, GIOCondition condition, context_t *ctx) {
        return update(ctx);
    }
    GIOChannel *ch = g_io_channel_unix_new(ctx.cam.fd);
    g_io_channel_set_encoding(ch, NULL, NULL);  // this is binary data
    g_io_add_watch(ch, G_IO_IN /*| G_IO_PRI*/, (GIOFunc)handle, &ctx);
#endif

    // run viewer
    gv_run();

/*    if(close(ctx.fd)<0)
        mexit (1, "close"); TODO video_close*/

    return EXIT_SUCCESS;
}

