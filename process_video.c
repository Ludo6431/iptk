#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "tools.h"
#include "gv.h"
#include "integ.h"
#include "steps.h"
#include "zones.h"
#include "video_draw.h"
#include "clock.h"

#include "process_video.h"

// ----------------------------------------------------------------------------
// raw
int rw_mid;

// ----------------------------------------------------------------------------
// undistort
int ud_mid;
int ud_gp;
param_t ud_pdiameter, ud_pcx, ud_pcy;
volatile unsigned int ud_diameter = 480, ud_cx = 300/*640>>1*/, ud_cy = 230/*480>>1*/;

// ----------------------------------------------------------------------------
// horizontal sweep
int hs_gp;
param_t hs_pthreshold;
volatile unsigned int hs_threshold = 4000;

// ----------------------------------------------------------------------------
// vertical edge
int ve_gp;
param_t ve_pwidth;
volatile unsigned int ve_width = 10;

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

void process_video_init(context_t *ctx) {
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

sZone *process_video(context_t *ctx, unsigned char *rw_data, unsigned int rw_w, unsigned int rw_h) {
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

    ud_h = (int)v;

#ifdef DEBUG_PROCESS_VIDEO
    printf("ud_h = %f\n", v);
    printf("ud_h = %d\n", ud_h);
#endif
#endif

    CLOCK_STEP(ctx->clock_ref, "###");

    ud_data = step_undis(rw_data, rw_w, rw_h, ud_diameter, ud_cx, ud_cy);

    CLOCK_STEP(ctx->clock_ref, "step_undis");

// ----------------------------------------------------------------------------
// integ
    unsigned int *intg = (unsigned int *)malloc(ud_w*ud_h*sizeof(unsigned int));

    integ_rgb(intg, ud_data, ud_w, ud_h, ud_w*3, 2);   // integral of blue component

    CLOCK_STEP(ctx->clock_ref, "step_integ");

#ifdef STEP_COLOR
// ----------------------------------------------------------------------------
// color
    unsigned char *cl_data;
    cl_data = step_color(ud_data, ud_w, ud_h);
    gv_media_update(cl_mid, cl_data, ud_w, ud_h, (gv_destroy)free, NULL);
#endif

    CLOCK_STEP(ctx->clock_ref, "###");

// "little" step to show the picture in grayscale
    {
        int i;
        unsigned char c, *p = ud_data;
        for(i=0; i<ud_h*ud_w; i++) {
                c = 255 - p[2];

                *p++ = c;
                *p++ = c;
                *p++ = c;
            }
    }

    CLOCK_STEP(ctx->clock_ref, "to grayscale");

// ----------------------------------------------------------------------------
// horizontal sweep
    sZone *hs_zl;
#ifdef OLD_UNDIS
    hs_zl = step_hsweep(intg, ud_w, ud_h, NULL, 175, 60, hs_threshold);
#else
    hs_zl = step_hsweep(intg, ud_w, ud_h, NULL, 20, 145, hs_threshold);
#endif

    CLOCK_STEP(ctx->clock_ref, "step_hsweep");

#ifdef DEBUG_PROCESS_VIDEO
    printf("hsweep zones:\n");
//    zone_print_all(hs_zl, "  ");
#endif

    {
#ifdef DEBUG_DUMP_ZONES
        printf("hsweep zones data:\n");
#endif
        sZone *l = hs_zl;
        for(; l; l = l->next) {
#ifdef DEBUG_PROCESS_VIDEO
            printf("  "); zone_print(l); printf("\n");
#ifdef DEBUG_DUMP_ZONES
            dump_rgb_b(ud_data, dw, dh, l->y, l->x, l->w, l->h);
#endif
#endif

            zone_video_draw(ud_data, ud_w, ud_h, ud_w*3, l, 255, 0, 0);   // red boxes
        }
    }

    CLOCK_STEP(ctx->clock_ref, "###");

// ----------------------------------------------------------------------------
// vertical edge
    sZone *ve_zl;
    ve_zl = step_vedge(intg, ud_w, ud_h, hs_zl, ve_width);

    CLOCK_STEP(ctx->clock_ref, "step_vedge");

#ifdef DEBUG_PROCESS_VIDEO
    printf("vedge zones:\n");
    zone_print_all(ve_zl, "  ");
#endif

    {
        sZone *l = ve_zl;
        for(; l; l = l->next)
            zone_video_draw(ud_data, ud_w, ud_h, ud_w*3, l, 0, 255, 0);   // green boxes
    }

    CLOCK_STEP(ctx->clock_ref, "###");

// ----------------------------------------------------------------------------
// horizontal edge
    sZone *he_zl;
    he_zl = step_hedge(intg, ud_w, ud_h, ve_zl, he_height);

#ifdef DEBUG_PROCESS_VIDEO
    printf("hedge zones:\n");
    zone_print_all(he_zl, "  ");
#endif

    {
        sZone *l = he_zl;
        for(; l; l = l->next)
            zone_video_draw(ud_data, ud_w, ud_h, ud_w*3, l, 0, 0, 255); // blue boxes
    }

    CLOCK_STEP(ctx->clock_ref, "step_hedge");

// ----------------------------------------------------------------------------
// detection
#ifdef DEBUG_PROCESS_VIDEO
    printf("detect zones:\n");
#endif
    {
        sZone *l = he_zl;
        for(; l; l = l->next) {
            int v;

            // print zone
#ifdef DEBUG_PROCESS_VIDEO
            zone_print(l); printf("\n");

            // print zone h/w ratio
            printf("  %.2f\n", (float)l->h/(float)l->w);
#endif

            v = zone_pat_hedge(intg, ud_w, ud_h, l);

            // print
#ifdef DEBUG_PROCESS_VIDEO
            printf("  %.2f\n", (float)v*100./(float)l->v);
#endif

            if(v > l->v/10) {
                // up
#ifdef DEBUG_PROCESS_VIDEO
                printf("up\n");
#endif
                l->v = 2;
            }
            else if(v < -l->v/10) {
                // down
#ifdef DEBUG_PROCESS_VIDEO
                printf("down\n");
#endif
                l->v = 0;
            }
            else {
                // center
#ifdef DEBUG_PROCESS_VIDEO
                printf("center\n");
#endif
                l->v = 1;
            }
        }
    }

// free temporary data
    zone_del_all(hs_zl);
    zone_del_all(ve_zl);
//    zone_del_all(he_zl);
    free(intg);

// undistort update (with the zones displayed)
    gv_media_update(ud_mid, ud_data, ud_w, ud_h, (gv_destroy)free, NULL);

// ----------------------------------------------------------------------------
// raw
    video_draw_cross(rw_data, rw_w, rw_h, rw_w*3, ud_cx, ud_cy, ud_diameter>>3, 255, 0, 0);
    video_draw_circle(rw_data, rw_w, rw_h, rw_w*3, ud_cx, ud_cy, ud_diameter>>1, 255, 0, 0);

    gv_media_update(rw_mid, rw_data, rw_w, rw_h, (gv_destroy)NULL, NULL);

    return he_zl;
}

