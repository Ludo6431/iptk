#ifndef _ZONES_H
#define _ZONES_H

struct sZone {
    struct sZone *next;

    int x, y;
    int w, h;

    int v;
};
typedef struct sZone sZone;

sZone *         zone_new        (int y, int x, int w, int h, int v);
unsigned int    zone_count      (sZone *l);
sZone *         zone_last       (sZone *l);
sZone *         zone_concat     (sZone *l1, sZone *l2);
sZone *         zone_prepend    (sZone *l, sZone *z);
sZone *         zone_hshrink    (sZone *l, int sort);
sZone *         zone_del_all    (sZone *l);

// ---- PATTERNS ----

sZone *         zone_new_v          (unsigned int *intg, unsigned int iw, unsigned int ih, int y, int x, int w, int h);
unsigned int    zone_pat_box        (unsigned int *intg, unsigned int iw, unsigned int ih, sZone *z);
int             zone_pat_hedge      (unsigned int *intg, unsigned int iw, unsigned int ih, sZone *z);
int             zone_pat_hedge_tune (unsigned int *intg, unsigned int iw, unsigned int ih, sZone *z, unsigned int ph);

// ---- VIDEO ----

void zone_video_draw(unsigned char *rgb, unsigned int w, unsigned int h, unsigned int rowstride, sZone *z, unsigned char r, unsigned char g, unsigned char b);

// ---- DEBUG ----

void zone_print(sZone *z);
void zone_print_all(sZone *l, char *prefix);

#endif

