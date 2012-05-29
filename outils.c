#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "outils.h"

void _mexit(int code, char *format, ...) {
    if(format) {
        va_list ap;
        va_start(ap, format);
        vfprintf(stderr, format, ap);
        fprintf(stderr, "\n");
        va_end(ap);
    }

    exit(code);
}

inline void *memdup(void *data, unsigned int size) {
    void *ret = malloc(size);
    if(ret)
        memcpy(ret, data, size);

    return ret;
}

void sbggr8_to_bgr (const void* src, void* dst, int width, int height) {
    typedef unsigned char Uint8;

    Uint8 *rawpt, *d8;
    Uint8 r, g, b;
    int i = width * height;
    rawpt = (Uint8*) src;

    d8 = (Uint8 *) dst;

    while (i--) {
        if ( (i/width) % 2 == 0 ) {
            /* even row (BGBGBGBG)*/
            if ( (i % 2) == 0 ) {
            /* B */
            if ( (i > width) && ((i % width) > 0) ) {
                b = *rawpt;            /* B */
                g = (*(rawpt-1)+*(rawpt+1)+
                *(rawpt+width)+*(rawpt-width))/4;      /* G */
                r = (*(rawpt-width-1)+*(rawpt-width+1)+
                *(rawpt+width-1)+*(rawpt+width+1))/4;  /* R */
            } else {
                /* first line or left column */
                b = *rawpt;                 /* B */
                g = (*(rawpt+1)+*(rawpt+width))/2;      /* G */
                r = *(rawpt+width+1);           /* R */
            }
            } else {
            /* (B)G */
            if ( (i > width) && ((i % width) < (width-1)) ) {
                b = (*(rawpt-1)+*(rawpt+1))/2;      /* B */
                g = *rawpt;                 /* G */
                r = (*(rawpt+width)+*(rawpt-width))/2;  /* R */
            } else {
                /* first line or right column */
                b = *(rawpt-1);     /* B */
                g = *rawpt;         /* G */
                r = *(rawpt+width);     /* R */
            }
            }
        } else {
            /* odd row (GRGRGRGR) */
            if ( (i % 2) == 0 ) {
            /* G(R) */
            if ( (i < (width*(height-1))) && ((i % width) > 0) ) {
                b = (*(rawpt+width)+*(rawpt-width))/2;  /* B */
                g = *rawpt;                 /* G */
                r = (*(rawpt-1)+*(rawpt+1))/2;      /* R */
            } else {
                /* bottom line or left column */
                b = *(rawpt-width);     /* B */
                g = *rawpt;         /* G */
                r = *(rawpt+1);     /* R */
            }
            } else {
            /* R */
            if ( i < (width*(height-1)) && ((i % width) < (width-1)) ) {
                b = (*(rawpt-width-1)+*(rawpt-width+1)+
                *(rawpt+width-1)+*(rawpt+width+1))/4;  /* B */
                g = (*(rawpt-1)+*(rawpt+1)+
                *(rawpt-width)+*(rawpt+width))/4;      /* G */
                r = *rawpt;            /* R */
            } else {
                /* bottom line or right column */
                b = *(rawpt-width-1);           /* B */
                g = (*(rawpt-1)+*(rawpt-width))/2;      /* G */
                r = *rawpt;                 /* R */
            }
            }
        }
        rawpt++;
        *d8++ = b;
        *d8++ = g;
        *d8++ = r;
    }
}

