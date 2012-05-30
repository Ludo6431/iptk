#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "tools.h"
#include "gv.h"

#include "process.h"


// ----------------------------------------------------------------------------
// raw
int mid_raw;

// ----------------------------------------------------------------------------
// undistort
unsigned char *undis(unsigned char *data, unsigned int sw, unsigned int sh, unsigned int dw, unsigned int dh);

int mid_und;
int u_gp;
param_t u_pdiameter, u_pcx, u_pcy;
volatile unsigned int u_diameter = 480, u_cx = 640>>1, u_cy = 480>>1;

// ----------------------------------------------------------------------------
// color pass
unsigned char *color_pass(unsigned char *data, unsigned int width, unsigned int height);

int mid_color;
int c_gp;
param_t c_pc, c_pTHR0, c_pTHR1;
volatile unsigned int c_c = 240, c_THR0 = 60, c_THR1 = 25;

void analyse_init(context_t *ctx) {
// ----------------------------------------------------------------------------
// raw
    mid_raw = gv_media_new("Caméra pure", "Caméra sans traitement", ctx->width, ctx->height);

// ----------------------------------------------------------------------------
// undistort
    unsigned int dw, dh;
    dw=M_PI*u_diameter;
    dh=u_diameter>>1;

    mid_und = gv_media_new("undistort", "Caméra redressée", dw, dh);
    u_gp = gv_gparam_new("undistort", "Params to define how we undistort the pic");

    param_init(&u_pdiameter, "diameter", "diameter of the circle", PT_INT, &u_diameter, 1 /* min */, 480 /* max */, 1 /* step */);
    gv_param_add(u_gp, &u_pdiameter);
    param_init(&u_pcx, "center x", "x of the center", PT_INT, &u_cx, 0 /* min */, 639 /* max */, 1 /* step */);
    gv_param_add(u_gp, &u_pcx);
    param_init(&u_pcy, "center y", "y of the center", PT_INT, &u_cy, 0 /* min */, 479 /* max */, 1 /* step */);
    gv_param_add(u_gp, &u_pcy);

// ----------------------------------------------------------------------------
// color pass
    mid_color = gv_media_new("Caméra filtrée", "Caméra après traitement", dw, dh);
    c_gp = gv_gparam_new("color params", "how to filter the colors");

    param_init(&c_pc, "couleur", "Choisir la couleur", PT_INT, &c_c /* cf analyse.h */, 0 /* min */, 255 /* max */, 1 /* step */);
    gv_param_add(c_gp, &c_pc);

    param_init(&c_pTHR0, "THR0", "Value threshold", PT_INT, &c_THR0 /* cf analyse.h */, 0 /* min */, 255 /* max */, 1 /* step */);
    gv_param_add(c_gp, &c_pTHR0);
    param_init(&c_pTHR1, "THR1", "Color threshold", PT_INT, &c_THR1 /* cf analyse.h */, 0 /* min */, 360/6 /* max */, 1 /* step */);
    gv_param_add(c_gp, &c_pTHR1);
}

void analyse_update(context_t *ctx, unsigned char *data) {
// do not destroy this data argument

// ----------------------------------------------------------------------------
// raw
    gv_media_update(mid_raw, data, ctx->width, ctx->height, (gv_destroy)NULL, NULL);

// ----------------------------------------------------------------------------
// undistort
    unsigned int dw, dh;
    unsigned char *u_data;

    dw=M_PI*u_diameter;
    dh=u_diameter>>1;

    u_data = undis(data, ctx->width, ctx->height, dw, dh);
    gv_media_update(mid_und, u_data, dw, dh, (gv_destroy)free, NULL);

// ----------------------------------------------------------------------------
// color pass
    unsigned char *c_data;
    c_data = color_pass(u_data, dw, dh);
    gv_media_update(mid_color, c_data, dw, dh, (gv_destroy)free, NULL);
}

// ----------------------------------------------------------------------------
// undistort
unsigned char *undis(unsigned char *data, unsigned int sw, unsigned int sh, unsigned int dw, unsigned int dh) {
    unsigned char *dst = malloc(dw*dh*3);

#if 0
    unsigned int i, j, sx, sy;

    for(i=0; i<dw; i++)
        for(j=0; j<dh; j++) {
            sx = (int)(u_cx + (double)j*cos(2 * M_PI * (double)i / (double)dw));
            sy = (int)(u_cy + (double)j*sin(2 * M_PI * (double)i / (double)dw));
            memcpy(&dst[(i + j*dw)*3], &data[(sx + sy*sw)*3], 3);
        }
#else
    static unsigned int *offset=NULL, _sw=0, _sh=0, _dw=0, _dh=0, _u_cx=0, _u_cy=0;
    unsigned int i, j, sx, sy;

    if(!offset || _sw!=sw || _sh!=sh || _dw!=dw || _dh!=dh || _u_cx!=u_cx || _u_cy!=u_cy) {
printf("offset_update\n");
        _sw=sw;
        _sh=sh;
        _dw=dw;
        _dh=dh;
        _u_cx=u_cx;
        _u_cy=u_cy;

        offset=realloc(offset, dw*dh*sizeof(*offset));

        for(i=0; i<dw; i++)
            for(j=0; j<dh; j++) {
                sx = (int)(u_cx + (double)j*cos(2 * M_PI * (double)i / (double)dw));
                sy = (int)(u_cy + (double)j*sin(2 * M_PI * (double)i / (double)dw));
                offset[i+j*dw]=(sx + sy*sw)*3;
            }
    }

    for(i=0; i<dw*dh; i++) {
        dst[i*3+0]=data[offset[i]+0];
        dst[i*3+1]=data[offset[i]+1];
        dst[i*3+2]=data[offset[i]+2];
    }
#endif

    return dst;
}


// ----------------------------------------------------------------------------
// color pass
unsigned char *color_pass(unsigned char *data, unsigned int width, unsigned int height) {
    data = memdup(data, width*height*3);

    unsigned char m, M, l, s, R, G, B;
    unsigned int t;

//printf("THR0=%d\nTHR1=%d\n", THR0, THR1);
    int i, j;
    for(j=0; j<height; j++)
        for(i=0; i<3*width; i+=3) {
            R = data[i+0 + j*width*3];
            G = data[i+1 + j*width*3];
            B = data[i+2 + j*width*3];

            m = MIN3(R, G, B);
            M = MAX3(R, G, B);

            // get T
            if(M==m)
                t = 0;
            else if(M==R)
                t = 60*(G - B)/(M - m) + 360;
            else if(M==G)
                t = 60*(B - R)/(M - m) + 120;
            else if(M==B)
                t = 60*(R - G)/(M - m) + 240;
            t = t%360;

            // get L
            l = (M + m)>>1;

            // get S
            if(M==m)
                s = 0;
            else if(l<=127)
                s = 255 * (M - m) / (M + m);
            else if(l>127)
                s = 255 * (M - m) / (2*255 - (M + m));

            R = G = B = 0;

            // do test
            if(abs(M - m)>=c_THR0 && M>=0+c_THR0 && m<=255-c_THR0) {    // far enough from grey, white and black
                if(0);
                else if(c_c-c_THR1<t && t<c_c+c_THR1) { // green
                    R = 0;
                    G = 255;
                    B = 0;
                }
/*                else if(360-c_THR1<t || t<0+c_THR1) { // red
                    R = 255;
                    G = 0;
                    B = 0;
                }
                else if(120-c_THR1<t && t<120+c_THR1) { // green
                    R = 0;
                    G = 255;
                    B = 0;
                }
                else if(240-c_THR1<t && t<240+c_THR1) { // blue
                    R = 0;
                    G = 0;
                    B = 255;
                }*/
            }

            data[i+0 + j*width*3] = R;
            data[i+1 + j*width*3] = G;
            data[i+2 + j*width*3] = B;
        }

    return data;
}

