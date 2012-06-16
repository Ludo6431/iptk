#ifndef _PROCESS_VIDEO_H
#define _PROCESS_VIDEO_H

#include "context.h"
#include "zones.h"

void process_video_init(context_t *ctx);
sZone *process_video(context_t *ctx, unsigned char *rw_data, unsigned int rw_w, unsigned int rw_h);

#endif

