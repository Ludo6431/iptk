#include <assert.h>

#include "patterns.h"

#define MOD(a, n) ((((a)%(n))+(n))%(n))

// |    |
// |    |
// |    |
// simple, non overflowing box
inline unsigned int _pat_box(unsigned int *intg, unsigned int iw, unsigned int ih, unsigned int y, unsigned int x, unsigned int w, unsigned int h) {
    return ((y>0 && x>0)?intg[(y-1)*iw+(x-1)]:0) + intg[(y+h-1)*iw+(x+w-1)] - (y>0?intg[(y-1)*iw+(x+w-1)]:0) - (x>0?intg[(y+h-1)*iw+(x-1)]:0);
}
// overflow proof box
unsigned int pat_box(unsigned int *intg, unsigned int iw, unsigned int ih, unsigned int y, int x, unsigned int w, unsigned int h) {
    assert(h>=0 && h<=ih);
    assert(w>=0 && w<=iw);
    assert(y>=0 && y+h<=ih);    // the image is circular in x but not in y

    if(!h || !w)
        return 0;

#if 1
    if(x<iw && x+w>iw)   // overflowing at the end of the integral image
        return _pat_box(intg, iw, ih, y, x, iw-x, h) + _pat_box(intg, iw, ih, y, 0, w+x-iw, h);
    else if(x<0 && x+w>0)   // overflowing at the start of the integral image
        return _pat_box(intg, iw, ih, y, MOD(x, iw), -x, h) + _pat_box(intg, iw, ih, y, 0, w+x, h);
#else
    if( (x<iw && x+w>iw) || (x<0 && x+w>0)) // overflowing
        return _pat_box(intg, iw, ih, y, MOD(x, iw), MOD(-x, iw), h) + _pat_box(intg, iw, ih, y, 0, MOD(w+x, iw), h);
#endif
    else    // not overflowing
        return _pat_box(intg, iw, ih, y, MOD(x, iw), w, h);
}

// vertical edge detector (maximize or minimize result)
// |  ##|
// |  ##|
// |  ##|
int pat_vedge(unsigned int *intg, unsigned int iw, unsigned int ih, unsigned int y, int x, unsigned int w, unsigned int h) {
    assert(!(w&1));

    return (int)pat_box(intg, iw, ih, y, x, w>>1, h) - (int)pat_box(intg, iw, ih, y, x+(w>>1), w>>1, h);
}


// horizontal edge detector (maximize or minimize result)
// |    |
// |    |
// |####|
// |####|
int pat_hedge(unsigned int *intg, unsigned int iw, unsigned int ih, unsigned int y, int x, unsigned int w, unsigned int h) {
    assert(!(h&1));

    return (int)pat_box(intg, iw, ih, y, x, w, h>>1) - (int)pat_box(intg, iw, ih, y+(h>>1), x, w, h>>1);
}


// |    |
// |####|
// |####|
int pat_hedge_tune(unsigned int *intg, unsigned int iw, unsigned int ih, unsigned int y, int x, unsigned int w, unsigned int h, unsigned int ph) {
    assert(ph>0 && ph<h);

    return (int)pat_box(intg, iw, ih, y, x, w, ph) - (int)pat_box(intg, iw, ih, y+ph, x, w, h-ph);
}


// | ## |
// | ## |
// | ## |
int pat_vhotdog(unsigned int *intg, unsigned int iw, unsigned int ih, unsigned int y, int x, unsigned int w, unsigned int h) {
    assert(!(w&3));

    return (int)pat_box(intg, iw, ih, y, x, w>>2, h) + (int)pat_box(intg, iw, ih, y, x+3*(w>>2), w>>2, h) - (int)pat_box(intg, iw, ih, y, x+(w>>2), w>>1, h);
}

