#ifndef _INTEG_H
#define _INTEG_H

// computes the integral image of a component of an RGB image 
//    *intg : resulting array (<width> * <height> * sizeof(unsigned int)) bytes)
//    *rgb : raw rgb data (<width> x <height> pixels picture with <rowstride> bytes per row and 8 bits per component) 
//    width, height : ...
//    rowstride : lenght (in bytes) of a row
//    color : 0=R, 1=G, 2=B
unsigned int *integ_rgb(unsigned int *intg, unsigned char *rgb, int width, int height, int rowstride, char color);

#ifdef DEBUG_COMPUTE_INTEG_MAX
extern unsigned char integ_max;
#endif

#endif

