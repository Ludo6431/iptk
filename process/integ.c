#include "integ.h"

#ifndef MAX
#define MAX(a, b) ((a)>(b)?(a):(b))
#endif

/*
computes the integral image of a component of an RGB image 
*/
#ifdef DEBUG_COMPUTE_INTEG_MAX
unsigned char integ_max;
#endif

unsigned int *integ_rgb(unsigned int *intg, unsigned char *rgb, int width, int height, int rowstride, char color) {
    int i,j;
#ifdef DEBUG_COMPUTE_INTEG_MAX
    unsigned char v;
    register unsigned char max = 0;

    #define BI(i, j) (unsigned int)(v=rgb[(i)*rowstride + (j)*3 + color],max = MAX(max, v),v)
#else
    #define BI(i, j) (unsigned int)rgb[(i)*rowstride + (j)*3 + color]
#endif
    #define BO(i, j) intg[(i)*width + (j)]

    // fill first pixel
    BO(0, 0) = BI(0, 0);

    // fill first row
    for(j=1; j<width; j++)
        BO(0, j) = BI(0, j) + BO(0, j-1);

    // fill first column
    for(i=1; i<height; i++)
        BO(i, 0) = BI(i, 0) + BO(i-1, 0);

    // fill
    for(i=1; i<height; i++)
        for(j=1; j<width; j++)
            BO(i, j) = BI(i, j) + BO(i-1, j) + BO(i, j-1) - BO(i-1, j-1);

#ifdef DEBUG_COMPUTE_INTEG_MAX
    integ_max = max;
#endif

    #undef BO
    #undef BI

    return intg;
}

