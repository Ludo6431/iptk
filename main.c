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
#include <time.h>

#include "video.h"
#include "video_params.h"
#include "tools.h"
#include "process_video.h"
#include "process_position.h"
#include "param.h"
#include "gv.h"
#include "context.h"

// TODO: si on fourni un fichier de sauvegarde, il faut charger et sauvegarder les paramètres

#ifdef FROM_FILE
char buf_[640*480*3];
#endif

int update(context_t *ctx) {
    sZone *l;

//printf(".\n");

    ctx->curbuffer ^= 1;    // change current buffer
    
    CLOCK_STEP(ctx->clock_ref, "debut");

    // get current frame
    video_read(&ctx->cam, ctx->buffers[ctx->curbuffer]);


    CLOCK_STEP(ctx->clock_ref, "video_read");

#ifdef FROM_FILE
memcpy(ctx->buffers[ctx->curbuffer], buf_, 640*480*3);
ctx->width = 640;
ctx->height = 480;
#endif

    // update media
    l = process_video(ctx, ctx->buffers[ctx->curbuffer], ctx->width, ctx->height);
    
    CLOCK_STEP(ctx->clock_ref, "process video");

    // update position
    process_position(ctx, l);

    CLOCK_STEP(ctx->clock_ref, "process position");

    zone_del_all(l);

    return TRUE;    // handled
}

int main(int argc, char *argv[]) {
    context_t ctx;
    
    CLOCK_INIT(ctx.clock_ref);

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
    process_video_init(&ctx);

    // init position handlers
    // add some video sources
    process_position_init(&ctx);

    // add parameters of the camera
    {
        int gid = gv_gparam_new("Camera", "Paramètres de la caméra");
        video_add_cam_params(&ctx.cam, gid);
    }

#ifdef FROM_FILE
    do {
        FILE *f = fopen(argv[2], "rb+");
        if(!f) {
            printf("!f (%s)\n", argv[2]);
            getchar();
            break;
        }

        fread(buf_, 640*3, 480, f);

        fclose(f);
    } while(0);
#endif

    // setup periodic update
#if 0 || defined(FROM_FILE)
// in this case (user-fixed framerate), you would enable the select in video_read
    g_timeout_add(1000/1, (GSourceFunc)update, &ctx);
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

