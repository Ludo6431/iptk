#ifndef _VIDEO_H
#define _VIDEO_H

#include <asm/types.h> 
#include <linux/videodev2.h>

int video_open(char *dev_name);
void *video_init(int fd, struct v4l2_format *fmt, unsigned int *size);
int video_read(int fd, void *buffer, unsigned int size);

#endif

