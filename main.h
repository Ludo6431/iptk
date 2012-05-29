#ifndef _MAIN_H
#define _MAIN_H

#include <asm/types.h> 
#include <linux/videodev2.h>

typedef struct {
    int fd;
    struct v4l2_format fmt;
    unsigned int width, height;

    void *buffer;
    unsigned int size;

    int mid_rawcam; // raw camera image
} context_t;

#endif

