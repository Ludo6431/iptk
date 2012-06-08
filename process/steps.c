#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "tools.h"
#include "zones.h"
#include "patterns.h"

#include "steps.h"

unsigned char *step_undis(unsigned char *data, unsigned int sw, unsigned int sh, unsigned int dd, unsigned int dcx, unsigned int dcy) {
    unsigned int dw = dd*M_PI;
    unsigned int dh;

#ifdef OLD_UNDIS
    dh = dd>>1;
#else
    float rho2y(float rho);
    float y2rho(float y);

    dh = (int)(rho2y(240)-rho2y(240-65));
#endif

    unsigned char *dst = malloc(dw*dh*3);
    if(!dst)
        exit(1);

#if 0
    unsigned int i, j, sx, sy;

    for(i=0; i<dw; i++)
        for(j=0; j<dh; j++) {
            sx = (int)(u_cx + (double)j*cos(2 * M_PI * (double)i / (double)dw));
            sy = (int)(u_cy + (double)j*sin(2 * M_PI * (double)i / (double)dw));
            memcpy(&dst[(i + j*dw)*3], &data[(sx + sy*sw)*3], 3);
        }
#else
    static unsigned int *offset=NULL, _sw=0, _sh=0, _dd=0, _dcx=0, _dcy=0;
    unsigned int i, j, sx, sy;
    double rho;

    if(!offset || _sw!=sw || _sh!=sh || _dd!=dd || _dcx!=dcx || _dcy!=dcy) {
        _sw=sw;
        _sh=sh;
        _dd=dd;
        _dcx=dcx;
        _dcy=dcy;

        offset=realloc(offset, dw*dh*sizeof(*offset));

        for(i=0; i<dw; i++)
            for(j=0; j<dh; j++) {
#ifdef OLD_UNDIS
                rho = (double)j;
#else
                rho = y2rho(rho2y(240)-(dh-1-j));

//                printf("  %d,%d\n", j, rho);
#endif

                sx = CLAMP(0, (int)(dcx + rho*cos(2 * M_PI * (double)i / (double)dw)), sw-1);
                sy = CLAMP(0, (int)(dcy + rho*sin(2 * M_PI * (double)i / (double)dw)), sh-1);

//                printf("    %d,%d\n", sx, sy);

                offset[i+j*dw]=(sx + sy*sw)*3;
            }
    }

    unsigned char *p = dst, *d;
    unsigned int *off = offset;
    for(i=0; i<dw*dh; i++) {
/*        dst[3*i+0] = data[offset[i]+0];
        dst[3*i+1] = data[offset[i]+1];
        dst[3*i+2] = data[offset[i]+2];*/

        d = data + *off++;
        *p++ = *d++;
        *p++ = *d++;
        *p++ = *d++;
    }
#endif

    return dst;
}

#ifdef STEP_COLOR
unsigned char *step_color(unsigned char *data, unsigned int width, unsigned int height) {
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
            else // if(M==B)
                t = 60*(R - G)/(M - m) + 240;
            t = t%360;

            // get L
            l = (M + m)>>1;

            // get S
            if(M==m)
                s = 0;
            else if(l<=127)
                s = 255 * (M - m) / (M + m);
            else // if(l>127)
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
#endif

#define SZ_MAX 128
#define SZ_MIN 16

sZone *step_hsweep(unsigned int *intg, unsigned int iw, unsigned int ih, sZone *zl, unsigned int y, unsigned int h, unsigned int th) {
    sZone *nzl, *z;
    unsigned int sz, v, _th;
    int j;

    assert(intg);

    if(!zl)
        zl = zone_new(y, 0, iw+SZ_MAX, h, 0);   // TODO : à créer dans process, on peut enlever y et h

    for(sz = SZ_MAX; sz>=SZ_MIN; sz>>=1) {
        nzl = NULL;

        _th = th; // * sz * h;
#ifdef DEBUG_HSWEEP
printf("sz=%d | th=%d\n", sz, _th);
#endif

        for(z = zl; z; z = z->next) {
#ifdef DEBUG_HSWEEP
printf("  "); zone_print(z); printf("\n");
#endif
            for(j = z->x; j + sz <= z->x + z->w; j+=sz) {
                v = pat_box(intg, iw, ih, y, j, sz, h);
#ifdef DEBUG_HSWEEP
printf("    j=%d | v=%d\n", j, v);
#endif
                if(v >= _th)
                    nzl = zone_prepend(nzl, zone_new(y, j, sz, h, v));
            }

            if(j - sz + sz != z->x + z->w) {
                j = z->x + z->w - sz;

                v = pat_box(intg, iw, ih, y, j, sz, h);
#ifdef DEBUG_HSWEEP
printf("   ~j=%d | v=%d\n", j, v);
#endif
                if(v >= _th)
                    nzl = zone_prepend(nzl, zone_new(y, j, sz, h, v));
            }

            nzl = zone_hshrink(nzl, 0); // no need to sort
        }

        zone_del_all(zl);

        zl = nzl;
    }

    return zl;
}

// horizontal sweep to detect vertical edges
sZone *step_vedge(unsigned int *intg, unsigned int iw, unsigned int ih, sZone *zl, unsigned int w) {
    sZone *nzl = NULL, *z;
    int j, v, min, max, jmin, jmax;

/*
 * first, we search a minimum, then a maximum and again a minimum and so on
 */
    int etat;

/*
 * sligthly modified Hill Climbing algorithm
 *  - continue while the extremum is not in a specific range
 *  - sliding data (not on a buffer)
 */
    #define NEIGHBORS_NB 5

    int j_neighbors[NEIGHBORS_NB], v_neighbors[NEIGHBORS_NB];
    int neighbors_i=0, neighbors_j=0;
    int j_current, v_current;
    int j_next, v_next;
    int x;

#ifdef DEBUG_VEDGE
    printf("vedge\n");
#endif

    for(z = zl; z; z = z->next) {
#ifdef DEBUG_VEDGE
        zone_print(z); printf("\n");
#endif

        etat = -1;  // we start by searching a minimum
        neighbors_i = 0;
        neighbors_j = 0;

        for(j = z->x; j + w < z->x + z->w; j++) {
            v = pat_vedge(intg, iw, ih, z->y, j, w, z->h);

            if(j == z->x) { // first node
                j_current = z->x;
                v_current = v;
            }

            // get data
#ifdef DEBUG_VEDGE
            printf("%d,%d\n", j, v);
#endif

            j_neighbors[neighbors_i%NEIGHBORS_NB] = j;
            v_neighbors[neighbors_i%NEIGHBORS_NB] = v;
            neighbors_i++;

            if((neighbors_i>=NEIGHBORS_NB && neighbors_i>neighbors_j+(NEIGHBORS_NB/2)) || j + w == z->x + z->w -1 /* last loop run */) { // enough data, run a step of the algorithm
                // is there any interesting neighbor ? ...
                for(x = 0; x<NEIGHBORS_NB; x++) {
                    if(!x || etat*v_neighbors[x] > etat*v_next) {
                        j_next = j_neighbors[x];
                        v_next = v_neighbors[x];
                    }
                }

#ifdef DEBUG_VEDGE
                printf("%s, next %d@%d\n", etat==-1?"min":"max", v_next, j_next);
#endif

                if(etat*v_next <= etat*v_current && abs(v_next) >= z->v/20) {  // ... yes, found local extremum
                    if(etat == -1) {
                        jmin = j_current;
                        min = v_current;

#ifdef DEBUG_VEDGE
                        printf("min detected %d@%d\n", min, jmin);
#endif
                    }
                    else {
                        jmax = j_current;
                        max = v_current;

#ifdef DEBUG_VEDGE
                        printf("max detected %d@%d\n", max, jmax);
#endif

                        nzl = zone_prepend(nzl, zone_new_v(intg, iw, ih, z->y, jmin + w/2, jmax - jmin + 1, z->h));
                    }

                    etat*=-1;
                }

                j_current = j_next;
                v_current = v_next;

                neighbors_j = neighbors_i;
            }
        }
    }

    return nzl;
}

// vertical sweep to detect horizontal edges
sZone *step_hedge(unsigned int *intg, unsigned int iw, unsigned int ih, sZone *zl, unsigned int h) {
    sZone *nzl = NULL, *z;
    int i, v, min, max, imin, imax;

/*
 * first, we search a minimum, then a maximum and again a minimum and so on
 */
    int etat;

/*
 * sligthly modified Hill Climbing algorithm
 *  - continue while the extremum is not in a specific range
 *  - sliding data (not on a buffer)
 */
    #define NEIGHBORS_NB 5

    int i_neighbors[NEIGHBORS_NB], v_neighbors[NEIGHBORS_NB];
    int neighbors_i=0, neighbors_j=0;
    int i_current, v_current;
    int i_next, v_next;
    int x;

#ifdef DEBUG_HEDGE
    printf("hedge\n");
#endif

    for(z = zl; z; z = z->next) {
#ifdef DEBUG_HEDGE
        zone_print(z); printf("\n");
#endif

        etat = -1;  // we start by searching a minimum
        neighbors_i = 0;
        neighbors_j = 0;

        for(i = z->y; i + h < z->y + z->h; i++) {
            v = pat_hedge(intg, iw, ih, i, z->x, z->w, h);

            if(i == z->y) { // first node
                i_current = z->y;
                v_current = v;
            }

            // get data
#ifdef DEBUG_HEDGE
            printf("%d,%d\n", i, v);
#endif

            i_neighbors[neighbors_i%NEIGHBORS_NB] = i;
            v_neighbors[neighbors_i%NEIGHBORS_NB] = v;
            neighbors_i++;

            if((neighbors_i>=NEIGHBORS_NB && neighbors_i>neighbors_j+(NEIGHBORS_NB/2)) || i + h == z->y + z->h -1 /* last loop run */) { // enough data, run a step of the algorithm
                // is there any interesting neighbor ? ...
                for(x = 0; x<NEIGHBORS_NB; x++) {
                    if(!x || etat*v_neighbors[x] > etat*v_next) {
                        i_next = i_neighbors[x];
                        v_next = v_neighbors[x];
                    }
                }

#ifdef DEBUG_HEDGE
                printf("%s, next %d@%d\n", etat==-1?"min":"max", v_next, i_next);
#endif

                if(etat*v_next <= etat*v_current && abs(v_next) >= z->v/20) {  // ... yes, found local extremum
                    if(etat == -1) {
                        imin = i_current;
                        min = v_current;

#ifdef DEBUG_HEDGE
                        printf("min detected %d@%d\n", min, imin);
#endif
                    }
                    else {
                        imax = i_current;
                        max = v_current;

#ifdef DEBUG_HEDGE
                        printf("max detected %d@%d\n", max, imax);
#endif

                        nzl = zone_prepend(nzl, zone_new_v(intg, iw, ih, imin + h/2, z->x, z->w, imax - imin + 1));
                    }

                    etat*=-1;
                }

                i_current = i_next;
                v_current = v_next;

                neighbors_j = neighbors_i;
            }
        }
    }

    return nzl;
}

