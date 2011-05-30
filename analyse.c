#include <stdlib.h>

#include "outils.h"

#include "analyse.h"

unsigned int THR0 = 60;
unsigned int THR1 = 25;

void analyse(unsigned char *data, unsigned int width, unsigned int height) {
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
            if(abs(M - m)>=THR0 && M>=0+THR0 && m<=255-THR0) {    // far enough from grey, white and black
                if(0);
                else if(360-THR1<t || t<0+THR1) { // red
                    R = 255;
                    G = 0;
                    B = 0;
                }
                else if(120-THR1<t && t<120+THR1) { // green
                    R = 0;
                    G = 255;
                    B = 0;
                }
                else if(240-THR1<t && t<240+THR1) { // blue
                    R = 0;
                    G = 0;
                    B = 255;
                }
            }

            data[i+0 + j*width*3] = R;
            data[i+1 + j*width*3] = G;
            data[i+2 + j*width*3] = B;
        }
}

