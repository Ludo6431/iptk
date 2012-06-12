#include <assert.h>

#include "tools.h"

#include "patterns.h"

// |    |
// |    |
// |    |
// simple, non overflowing box
inline unsigned int _pat_box(unsigned int *intg, unsigned int iw, unsigned int ih, unsigned int y, unsigned int x, unsigned int w, unsigned int h) {
    return ((y>0 && x>0)?intg[(y-1)*iw+(x-1)]:0) + intg[(y+h-1)*iw+(x+w-1)] - (y>0?intg[(y-1)*iw+(x+w-1)]:0) - (x>0?intg[(y+h-1)*iw+(x-1)]:0);
}
// overflow proof box, used to get the integral of a part of the image
unsigned int pat_box(unsigned int *intg, unsigned int iw, unsigned int ih, int y, int x, unsigned int w, unsigned int h) {
    if(!h || !w)    // no data
        return 0;

    if(y + h <= 0)  // not in the image
        return 0;

    if(y < 0) { // partially in the image
        h += y;
        y = 0;
    }

    if(y >= ih) // not in the image
        return 0;

    if(y + h > ih)  // partially in the image
        h = ih - y;

#if 1
    if(x<iw && x+w>iw)   // overflowing at the end of the integral image
        return _pat_box(intg, iw, ih, y, x, iw-x, h) + _pat_box(intg, iw, ih, y, 0, w+x-iw, h);
    else if(x<0 && x+w>0)   // overflowing at the start of the integral image
        return _pat_box(intg, iw, ih, y, MOD(x, (int)iw), -x, h) + _pat_box(intg, iw, ih, y, 0, w+x, h);
#else
    if( (x<iw && x+w>iw) || (x<0 && x+w>0)) // overflowing
        return _pat_box(intg, iw, ih, y, MOD(x, (int)iw), MOD(-x, (int)iw), h) + _pat_box(intg, iw, ih, y, 0, MOD(w+x, (int)iw), h);
#endif
    else    // not overflowing
        return _pat_box(intg, iw, ih, y, MOD(x, (int)iw), w, h);
}

// vertical edge detector (maximize or minimize result)
// |  ##|
// |  ##|
// |  ##|
int pat_vedge(unsigned int *intg, unsigned int iw, unsigned int ih, int y, int x, unsigned int w, unsigned int h) {
    return (int)pat_box(intg, iw, ih, y, x, w>>1, h) - (int)pat_box(intg, iw, ih, y, x+(w>>1)+(w&1), w>>1, h);
}


// horizontal edge detector (maximize or minimize result)
// |    |
// |    |
// |####|
// |####|
int pat_hedge(unsigned int *intg, unsigned int iw, unsigned int ih, int y, int x, unsigned int w, unsigned int h) {
    return (int)pat_box(intg, iw, ih, y, x, w, h>>1) - (int)pat_box(intg, iw, ih, y+(h>>1)+(h&1), x, w, h>>1);
}


// |    |
// |####|
// |####|
int pat_hedge_tune(unsigned int *intg, unsigned int iw, unsigned int ih, int y, int x, unsigned int w, unsigned int h, unsigned int ph) {
    assert(ph<=h);

    return (int)pat_box(intg, iw, ih, y, x, w, ph) - (int)pat_box(intg, iw, ih, y+ph, x, w, h-ph);
}


// | ## |
// | ## |
// | ## |
int pat_vhotdog(unsigned int *intg, unsigned int iw, unsigned int ih, int y, int x, unsigned int w, unsigned int h) {
    assert(!(w&3));

    return (int)pat_box(intg, iw, ih, y, x, w>>2, h) + (int)pat_box(intg, iw, ih, y, x+3*(w>>2), w>>2, h) - (int)pat_box(intg, iw, ih, y, x+(w>>2), w>>1, h);
}

