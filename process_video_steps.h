#ifndef _STEPS_H
#define _STEPS_H

#include "zones.h"

unsigned char *step_undis(unsigned char *data, unsigned int sw, unsigned int sh, unsigned int dd, unsigned int dcx, unsigned int dcy);
#ifdef STEP_COLOR
unsigned char *step_color(unsigned char *data, unsigned int width, unsigned int height);
#endif

// zones steps
sZone *step_hsweep(unsigned int *intg, unsigned int iw, unsigned int ih, sZone *zl, unsigned int y, unsigned int h, unsigned int th);
sZone *step_vedge(unsigned int *intg, unsigned int iw, unsigned int ih, sZone *zl, unsigned int w);
sZone *step_hedge(unsigned int *intg, unsigned int iw, unsigned int ih, sZone *zl, unsigned int h);

#endif

