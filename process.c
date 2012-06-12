#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "tools.h"
#include "gv.h"
#include "integ.h"
#include "steps.h"
#include "zones.h"
#include "video_draw.h"

#include "process.h"

// ----------------------------------------------------------------------------
// raw
int rw_mid;

// ----------------------------------------------------------------------------
// undistort
int ud_mid;
int ud_gp;
param_t ud_pdiameter, ud_pcx, ud_pcy;
volatile unsigned int ud_diameter = 480, ud_cx = 640>>1, ud_cy = 480>>1;

// ----------------------------------------------------------------------------
// horizontal sweep
int hs_gp;
param_t hs_pthreshold;
volatile unsigned int hs_threshold = 5000;

// ----------------------------------------------------------------------------
// vertical edge
int ve_gp;
param_t ve_pwidth;
volatile unsigned int ve_width = 6;

// ----------------------------------------------------------------------------
// horizontal edge
int he_gp;
param_t he_pheight;
volatile unsigned int he_height = 6;

#ifdef STEP_COLOR
// ----------------------------------------------------------------------------
// color
int cl_mid;
int cl_gp;
param_t cl_pc, cl_pTHR0, cl_pTHR1;
volatile unsigned int cl_c = 240, cl_THR0 = 60, cl_THR1 = 25;
#endif

void analyse_init(context_t *ctx) {
// ----------------------------------------------------------------------------
// raw
    rw_mid = gv_media_new("raw", "Caméra sans traitement", ctx->width, ctx->height);

// ----------------------------------------------------------------------------
// undistort
    unsigned int dw, dh;
    dw=M_PI*ud_diameter;
    dh=ud_diameter>>1;

    ud_mid = gv_media_new("undistort", "Caméra redressée", dw, dh);
    ud_gp = gv_gparam_new("undistort", "Params to define how we undistort the pic");

    param_init(&ud_pdiameter, "diameter", "diameter of the circle", PT_INT, &ud_diameter, 1 /* min */, 480 /* max */, 1 /* step */);
    gv_param_add(ud_gp, &ud_pdiameter);
    param_init(&ud_pcx, "center x", "x of the center", PT_INT, &ud_cx, 0 /* min */, 639 /* max */, 1 /* step */);
    gv_param_add(ud_gp, &ud_pcx);
    param_init(&ud_pcy, "center y", "y of the center", PT_INT, &ud_cy, 0 /* min */, 479 /* max */, 1 /* step */);
    gv_param_add(ud_gp, &ud_pcy);

// ----------------------------------------------------------------------------
// horizontal sweep
    hs_gp = gv_gparam_new("hsweep", "Params to define how we run the horizontal sweep");

    param_init(&hs_pthreshold, "threshold", "smaller value", PT_INT, &hs_threshold, 0 /* min */, 50000 /* max */, 1 /* step */);
    gv_param_add(hs_gp, &hs_pthreshold);

// ----------------------------------------------------------------------------
// vertical edge
    ve_gp = gv_gparam_new("vedge", "Params to define how we run the vertical edge detector");

    param_init(&ve_pwidth, "pattern width", "width of the pattern", PT_INT, &ve_width, 2 /* min */, 32 /* max */, 2 /* step */);
    gv_param_add(ve_gp, &ve_pwidth);

// ----------------------------------------------------------------------------
// horizontal edge
    he_gp = gv_gparam_new("hedge", "Params to define how we run the horizontal edge detector");

    param_init(&he_pheight, "pattern height", "height of the pattern", PT_INT, &he_height, 2 /* min */, 64 /* max */, 2 /* step */);
    gv_param_add(he_gp, &he_pheight);

#ifdef STEP_COLOR
// ----------------------------------------------------------------------------
// color
    cl_mid = gv_media_new("color", "Caméra après traitement sur les couleurs", dw, dh);
    cl_gp = gv_gparam_new("color params", "how to filter the colors");

    param_init(&cl_pc, "couleur", "Choisir la couleur", PT_INT, &cl_c /* cf analyse.h */, 0 /* min */, 255 /* max */, 1 /* step */);
    gv_param_add(cl_gp, &cl_pc);

    param_init(&cl_pTHR0, "THR0", "Value threshold", PT_INT, &cl_THR0 /* cf analyse.h */, 0 /* min */, 255 /* max */, 1 /* step */);
    gv_param_add(cl_gp, &cl_pTHR0);
    param_init(&cl_pTHR1, "THR1", "Color threshold", PT_INT, &cl_THR1 /* cf analyse.h */, 0 /* min */, 360/6 /* max */, 1 /* step */);
    gv_param_add(cl_gp, &cl_pTHR1);
#endif
}

#ifdef DEBUG_DUMP_ZONES
void dump_rgb_b(unsigned char *tab, int iw, int ih, int y, int x, int w, int h) {
    int i, j;

    for (i=y; i<MIN(y+h, ih); i++) {
        for (j=x; j<MIN(x+w, iw); j++)
            printf("%d,",
//                tab[(i*iw + j)*3 + 0],    // R
//                tab[(i*iw + j)*3 + 1],    // G
                tab[(i*iw + j)*3 + 2]     // B
            );

        printf("\n");
    }
}
#endif

#ifndef OLD_UNDIS
#define D0 (13.5*360/(2*M_PI))
#define D1 (3.9*360/(2*M_PI))

float rho2y(float rho) {
    return D1*tan((asin(atan(rho/D0) + 0.001856) - 0.8294)/0.5161);
}

float y2rho(float y) {
    return D0*tan(sin(0.5161*atan(y/D1)+0.8294)-0.001856);
}
#endif

void analyse_update(context_t *ctx, unsigned char *rw_data) {
// do not destroy this <rw_data> argument

// ----------------------------------------------------------------------------
// undistort
    unsigned int ud_w, ud_h;
    unsigned char *ud_data;

    ud_w = M_PI*ud_diameter;
#ifdef OLD_UNDIS
    ud_h = ud_diameter>>1;
#else

    float v = rho2y(240)-rho2y(240-65);

    printf("ud_h = %f\n", v);

    ud_h = (int)v;

    printf("ud_h = %d\n", ud_h);
#endif

    ud_data = step_undis(rw_data, ctx->width, ctx->height, ud_diameter, ud_cx, ud_cy);

// ----------------------------------------------------------------------------
// integ
    unsigned int *intg = (unsigned int *)malloc(ud_w*ud_h*sizeof(unsigned int));

    integ_rgb(intg, ud_data, ud_w, ud_h, ud_w*3, 2);   // integral of blue component

// ----------------------------------------------------------------------------
// horizontal sweep
    sZone *hs_zl;
#ifdef OLD_UNDIS
    hs_zl = step_hsweep(intg, ud_w, ud_h, NULL, 175, 60, hs_threshold);
#else
    hs_zl = step_hsweep(intg, ud_w, ud_h, NULL, 20, 145, hs_threshold);
#endif

#ifdef DEBUG_HSWEEP
    printf("hsweep zones:\n");
    zone_print_all(hs_zl);
#endif

    {
        sZone * l = hs_zl;
        for(; l; l = l->next) {
#if 0
            // multiply by 1.5 the size of the zone
            l->x -= l->w>>2;
            l->w += l->w>>1;
#else
            // double the size of the zone
            l->x -= l->w>>1;
            l->w += l->w;
#endif
        }

        hs_zl = zone_hshrink(hs_zl, 1 /* sort */);
    }

    printf("hsweep zones (enlarge + hshrink):\n");
//    zone_print_all(hs_zl);

    {
#ifdef DEBUG_DUMP_ZONES
        printf("hsweep zones data:\n");
#endif
        sZone * l = hs_zl;
        for(; l; l = l->next) {
            printf("  "); zone_print(l); printf("\n");
#ifdef DEBUG_DUMP_ZONES
            dump_rgb_b(ud_data, dw, dh, l->y, l->x, l->w, l->h);
#endif

            zone_video_draw(ud_data, ud_w, ud_h, ud_w*3, l, 255, 0, 0);   // red boxes
        }
    }

// ----------------------------------------------------------------------------
// vertical edge
    sZone *ve_zl;
    ve_zl = step_vedge(intg, ud_w, ud_h, hs_zl, ve_width);

    printf("vedge zones:\n");
    zone_print_all(ve_zl);

    {
        sZone * l = ve_zl;
        for(; l; l = l->next)
            zone_video_draw(ud_data, ud_w, ud_h, ud_w*3, l, 0, 255, 0);   // green boxes
    }

// ----------------------------------------------------------------------------
// horizontal edge
    sZone *he_zl;
    he_zl = step_hedge(intg, ud_w, ud_h, ve_zl, he_height);

    printf("hedge zones:\n");
    zone_print_all(he_zl);

    {
        sZone * l = he_zl;
        for(; l; l = l->next)
            zone_video_draw(ud_data, ud_w, ud_h, ud_w*3, l, 255, 255, 0); // yellow boxes
    }

// ----------------------------------------------------------------------------
// detection
    printf("detect zones:\n");
    {
        sZone * l = he_zl;
        for(; l; l = l->next) {
            int v;

            // print zone
            zone_print(l); printf("\n");

            // print zone h/w ratio
            printf("  %.2f\n", (float)l->h/(float)l->w);

            v = zone_pat_hedge(intg, ud_w, ud_h, l);

            if(v > l->v/20) {
                // up
                printf("up\n");
            }
            else if(v < -l->v/20) {
                // down
                printf("down\n");
            }
            else {
                // center
                printf("center\n");
            }
        }
    }

// free temporary data
    zone_del_all(hs_zl);
    zone_del_all(ve_zl);
    zone_del_all(he_zl);
    free(intg);

// undistort update (with the zones displayed)
    gv_media_update(ud_mid, ud_data, ud_w, ud_h, (gv_destroy)free, NULL);

#ifdef STEP_COLOR
// ----------------------------------------------------------------------------
// color
    unsigned char *cl_data;
    cl_data = step_color(ud_data, ud_w, ud_h);
    gv_media_update(cl_mid, cl_data, ud_w, ud_h, (gv_destroy)free, NULL);
#endif

// ----------------------------------------------------------------------------
// raw
    video_draw_cross(rw_data, ctx->width*3, ctx->height, ud_cx, ud_cy, ud_diameter>>3, 255, 0, 0);
    video_draw_circle(rw_data, ctx->width*3, ctx->height, ud_cx, ud_cy, ud_diameter>>1, 255, 0, 0);

    gv_media_update(rw_mid, rw_data, ctx->width, ctx->height, (gv_destroy)NULL, NULL);
}

