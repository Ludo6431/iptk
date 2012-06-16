#ifndef _CONTEXT_H
#define _CONTEXT_H

#include <asm/types.h> 
#include <linux/videodev2.h>
#include "clock.h"
#include "video.h"

typedef struct {
    // acquisition parameters
    struct video_t cam;
    unsigned int width, height;
    unsigned int sizeimage;
    unsigned char *buffers[2];
    int curbuffer;

    //time measurement ref
    CLOCK_TYPE clock_ref;
} context_t;

#endif

