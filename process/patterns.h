#ifndef _PATTERNS_H
#define _PATTERNS_H

unsigned int pat_box(unsigned int *intg, unsigned int iw, unsigned int ih, unsigned int y, int x, unsigned int w, unsigned int h);
int pat_vedge(unsigned int *intg, unsigned int iw, unsigned int ih, unsigned int y, int x, unsigned int w, unsigned int h);
int pat_hedge(unsigned int *intg, unsigned int iw, unsigned int ih, unsigned int y, int x, unsigned int w, unsigned int h);
int pat_hedge_tune(unsigned int *intg, unsigned int iw, unsigned int ih, unsigned int y, int x, unsigned int w, unsigned int h, unsigned int ph);
int pat_vhotdog(unsigned int *intg, unsigned int iw, unsigned int ih, unsigned int y, int x, unsigned int w, unsigned int h);

#endif

