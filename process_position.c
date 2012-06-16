#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "tools.h"
#include "gv.h"
#include "zones.h"
#include "video_draw.h"

#include "process_position.h"

// ----------------------------------------------------------------------------
// playground
int pg_mid;
int pg_cur = 0;
unsigned char *pg_data[2] = {NULL};

void process_position_init(context_t *ctx) {
// ----------------------------------------------------------------------------
// playground
    pg_mid = gv_media_new("playground", "Aire de jeu", 300, 200);
    pg_data[0] = malloc(300*200*3);
    pg_data[1] = malloc(300*200*3);
    pg_cur = 0;
}

int process_position(context_t *ctx, sZone *vd_zl) {
    sZone *b[3] = {NULL};
    sPos B1 = {0 ,0}, /* down */
        B2 = {3.11, 1.06}, /* center */
        B3 = {0, 2.09}, /* up */
        O1, O2, O3,
        M12, M13, M;
    float alpha1, alpha2, alpha3, a;

    {
        sZone *l;
        int i;

        for(l  = vd_zl; l; l = l->next) {
            if(l->v<0 || l->v>2)
                continue;

//            if(b[l->v] && b[l->v]->x!=l->x)
//                return 1;

#ifdef DEBUG_PROCESS_POSITION
            printf("b[%d]->x = %d\n", l->v, l->x);
#endif
            b[l->v] = l;
        }

        for(i=0; i<3; i++)
            if(!b[i]) {
                pg_cur ^= 1;

                memset(pg_data[pg_cur], 255, 300*200*3);

                gv_media_update(pg_mid, pg_data[pg_cur], 300, 200, (gv_destroy)NULL, NULL);

                return 1;
            }
    }

    #define C(i) ((float)b[i]->x + (float)b[i]->w/2.)
    alpha1 = (float)MIN(fabs(C(2) - C(0)), MIN(fabs(C(2) - C(0) - 1507), fabs(C(2) - C(0) + 1507)))*2*M_PI/1507.0;
    alpha2 = (float)MIN(fabs(C(1) - C(0)), MIN(fabs(C(1) - C(0) - 1507), fabs(C(1) - C(0) + 1507)))*2*M_PI/1507.0;
    alpha3 = (float)MIN(fabs(C(2) - C(1)), MIN(fabs(C(2) - C(1) - 1507), fabs(C(2) - C(1) + 1507)))*2*M_PI/1507.0;
    #undef C

#ifdef DEBUG_PROCESS_POSITION
    printf("alpha1=%.2f°\n", alpha1*180/M_PI);
    printf("alpha2=%.2f°\n", alpha2*180/M_PI);
    printf("alpha3=%.2f°\n", alpha3*180/M_PI);
#endif
    printf("%.3f;%.3f;%.3f;", alpha1*180/M_PI, alpha2*180/M_PI, alpha3*180/M_PI);

    O1.x = B3.y/(2*tan(alpha1));
    O1.y = B3.y/2;

    O2.x = B2.x/2 - B2.y/(2*tan(alpha2));
    O2.y = B2.y/2 + B2.x/(2*tan(alpha2));

    O3.x = B2.x/2 - (B3.y - B2.y)/(2*tan(alpha3));
    O3.y = (B2.y + B3.y)/2 - B2.x/(2*tan(alpha3));

#ifdef DEBUG_PROCESS_POSITION
    printf("O1(%.2f,%.2f)\n", O1.x, O1.y);
    printf("O2(%.2f,%.2f)\n", O2.x, O2.y);
    printf("O3(%.2f,%.2f)\n", O3.x, O3.y);
#endif

// méthode 12
    a = 2*(O2.x*O1.y - O1.x*O2.y) / (pow(O2.x - O1.x, 2) + pow(O2.y - O1.y, 2));
    M12.x = a*(O1.y - O2.y);
    M12.y = a*(O2.x - O1.x);

#ifdef DEBUG_PROCESS_POSITION
    printf("M12(%.3f,%.3f)\n", M12.x, M12.y);
#endif
    printf("%.3f;%.3f;", M12.x, M12.y);

// méthode 13
    a = 2*(O3.x*(B3.y - O1.y) - O1.x*(B3.y - O3.y)) / (pow(O3.x - O1.x, 2) + pow(O3.y - O1.y, 2));
    M13.x = a*(O3.y - O1.y);
    M13.y = B3.y - a*(O3.x - O1.x);

#ifdef DEBUG_PROCESS_POSITION
    printf("M13(%.3f,%.3f)\n", M13.x, M13.y);
#endif
    printf("%.3f;%.3f;", M13.x, M13.y);

// "fusion"
    if(M12.y < B3.y/2 && M13.y < B3.y/2) {
        M.x = M13.x;
        M.y = M13.y;
    }
    else if(M12.y >= B3.y/2 && M13.y >= B3.y/2) {
        M.x = M12.x;
        M.y = M12.y;
    }
    else {
        M.x = M12.x;
        M.y = M12.y;
    }

    M.x -= 0.06;
    M.y -= 0.06;

#ifdef DEBUG_PROCESS_POSITION
    printf("M(%.3f,%.3f)\n", M.x, M.y);
#endif
    printf("%.3f;%.3f;\n", M.x, M.y);

// ----------------------------------------------------------------------------
// playground
    pg_cur ^= 1;

    memset(pg_data[pg_cur], 255, 300*200*3);

    video_draw_cross(pg_data[pg_cur], 300, 200, 300*3, (int)(M.x*100), (int)(M.y*100), 10, 255, 0, 0);
    video_draw_circle(pg_data[pg_cur], 300, 200, 300*3, (int)(M.x*100), (int)(M.y*100), 10, 255, 0, 0);

    gv_media_update(pg_mid, pg_data[pg_cur], 300, 200, (gv_destroy)NULL, NULL);

    return 0;
}

