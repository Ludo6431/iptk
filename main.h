#ifndef _MAIN_H
#define _MAIN_H

#include <asm/types.h> 
#include <linux/videodev2.h>
#include "v4l2_wrapper.h"

typedef struct {
    // acquisition parameters
    struct video_t cam;
    unsigned int width, height;
    unsigned int sizeimage;
    unsigned char *buffers[2];
    int curbuffer;
} context_t;

#endif

