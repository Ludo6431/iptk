#ifndef _VIDEO_H
#define _VIDEO_H

#include <asm/types.h> 
#include <linux/videodev2.h>

struct video_t {
    int fd;
    struct v4l2_format src_fmt; // raw data from device
    struct v4l2_format vid_fmt; // desired output data
    struct v4lconvert_data *convert_data;

    unsigned char *raw_buffer;
};

int video_open  (struct video_t *vid, char *dev_name);
int video_config(struct video_t *vid, struct v4l2_format *fmt);
int video_read  (struct video_t *vid, void *buffer);

// TODO add video_close
// TODO add g_io_watch setup

#endif

