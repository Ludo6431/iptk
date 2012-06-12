#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "tools.h"

#include "zones.h"

#ifndef MAX
#define MAX(a, b) ((a)>(b)?(a):(b))
#endif

#ifndef MIN
#define MIN(a, b) ((a)>(b)?(b):(a))
#endif

sZone *zone_new(int y, int x, int w, int h, int v) {
    sZone *z = (sZone *)malloc(sizeof(sZone));
    if(!z)
        return NULL;

    z->next = NULL;
    z->x = x;
    z->y = y;
    z->w = w;
    z->h = h;
    z->v = v;

#ifdef DEBUG_ZONES
printf("z new (%d,%d;%dx%d;%d)\n", z->x, z->y, z->w, z->h, z->v);
#endif

    return z;
}

unsigned int zone_count(sZone *l) {
    unsigned int nb = 0;

    while(l) {
        l = l->next;
        nb++;
    }

    return nb;
}

sZone *zone_last(sZone *l) {
    if(!l)
        return NULL;

    while(l->next) l = l->next;

    return l;
}

sZone *zone_concat(sZone *l1, sZone *l2) {
    sZone *last1;

    if(!l1)
        return l2;
    if(!l2)
        return l1;

    last1 = zone_last(l1);
    last1->next = l2;

    return l1;
}

sZone *zone_prepend(sZone *l, sZone *z) {
    assert(z);

    z->next = l;

    return z;
}

sZone *zone_hshrink(sZone *l, int sort) {
    sZone *z, *next, *nl = 0;
    sZone *prev = NULL;
    int x_min;
    sZone *prev_min;

    if(sort) {
#ifdef DEBUG_ZONES
printf("before sort:\n");
zone_print_all(l);
#endif
        // sort list
        while(l) {
            // search minimum in the not sorted list
            prev = NULL;
            z = l;
            while(z) {
                if(!prev || z->x < x_min) {
                    x_min = z->x;
                    prev_min = prev;
                }

                prev = z;
                z = z->next;
            }

            // the minimum is at <x_min> and its previous element is <prev_min>
            if(prev_min) {
                next = prev_min->next->next;
                nl = zone_prepend(nl, prev_min->next);
                prev_min->next = next;
            }
            else {
                next = l->next;
                nl = zone_prepend(nl, l);
                l = next;
            }
        }
#ifdef DEBUG_ZONES
printf("after sort:\n");
zone_print_all(nl);
#endif
        z = nl;
    }
    else
        z = l;

    l = z;
    while(z && z->next) {
        next = z->next;

        if(MAX(z->x, next->x) <= MIN(z->x + z->w, next->x + next->w)) {   // non void intersection
            int x;

#ifdef DEBUG_ZONES
printf("z shrink (%d,%d;%dx%d;%d|%d,%d;%dx%d;%d) to ", z->x, z->y, z->w, z->h, z->v, next->x, next->y, next->w, next->h, next->v);
#endif

            assert(z->y == next->y && z->h == next->h);

            x = MIN(z->x, next->x);

            z->w = MAX(z->x + z->w, next->x + next->w) - x;
            z->x = x;

            z->v = z->v + next->v;   // FIXME, use a linear approximation which will give the right value in some particular (but common) cases

#ifdef DEBUG_ZONES
printf("(%d,%d;%dx%d;%d)\n", z->x, z->y, z->w, z->h, z->v);
#endif

            z->next = next->next;

            free(next);

            next = z;
        }

        z = next;
    }

    return l;
}

sZone *zone_del_all(sZone *l) {
    sZone *next;

    while(l) {
        next = l->next;

        free(l);

        l = next;
    }

    return NULL;
}

// ---- PATTERNS ----
#include "patterns.h"

sZone *zone_new_v(unsigned int *intg, unsigned int iw, unsigned int ih, int y, int x, int w, int h) {
    sZone *z = (sZone *)malloc(sizeof(sZone));
    if(!z)
        return NULL;

    z->next = NULL;
    z->x = x;
    z->y = y;
    z->w = w;
    z->h = h;
    z->v = zone_pat_box(intg, iw, ih, z);

#ifdef DEBUG_ZONES
printf("z new (%d,%d;%dx%d;%d)\n", z->x, z->y, z->w, z->h, z->v);
#endif

    return z;
}

unsigned int zone_pat_box(unsigned int *intg, unsigned int iw, unsigned int ih, sZone *z) {
    return pat_box(intg, iw, ih, z->y, z->x, z->w, z->h);
}

int zone_pat_hedge(unsigned int *intg, unsigned int iw, unsigned int ih, sZone *z) {
    return pat_hedge(intg, iw, ih, z->y, z->x, z->w, z->h);
}

int zone_pat_hedge_tune(unsigned int *intg, unsigned int iw, unsigned int ih, sZone *z, unsigned int ph) {
    return pat_hedge_tune(intg, iw, ih, z->y, z->x, z->w, z->h, ph);
}

// ---- VIDEO ----
#include "video_draw.h"

void zone_video_draw(unsigned char *rgb, unsigned int w, unsigned int h, unsigned int rowstride, sZone *z, unsigned char r, unsigned char g, unsigned char b) {
    // draw the box 1 pixel outside of the actual zone
    video_draw_box_ovr(rgb, w, h, rowstride, z->x-1, z->y-1, z->w+2, z->h+2, r, g, b);
}

// ---- DEBUG ----

void zone_print(sZone *z) {
    printf("(%d,%d;%dx%d;%d)", z->x, z->y, z->w, z->h, z->v);
}

void zone_print_all(sZone *l, char *prefix) {
    for(; l; l = l->next)
        printf("%s%03d,%03d (%03dx%03d) %d\n", prefix, l->x, l->y, l->w, l->h, l->v);
}

