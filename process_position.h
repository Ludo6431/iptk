#ifndef _PROCESS_POSITION_H
#define _PROCESS_POSITION_H

#include "context.h"
#include "zones.h"

typedef struct {
    float x, y;
} sPos;

void process_position_init(context_t *ctx);
int process_position(context_t *ctx, sZone *vd_zl);

#endif

