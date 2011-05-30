#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#include "bmpsave.h"

void write16(u16* address, u16 value) { // LE
    u8* first=(u8*)address;
    u8* second=first+1;

    *first=value&0xff;
    *second=value>>8;
}

void write32(u32* address, u32 value) { // LE
    u8* first=(u8*)address;
    u8* second=first+1;
    u8* third=first+2;
    u8* fourth=first+3;

    *first=value&0xff;
    *second=(value>>8)&0xff;
    *third=(value>>16)&0xff;
    *fourth=(value>>24)&0xff;
}

void bmpsave(const char* filename, char *data, unsigned w, unsigned h) {
    assert(filename); assert(data);

    if(!(w*h)) {
        unlink(filename);
        return;
    }

    FILE* file = fopen(filename, "wb");
    if(!file) return;

    unsigned adjust = 0;
    if(w & 0x3)
        adjust = 4 - (w & 0x3);
    // adjust = (4 - (w & 0x3))&0x3;

    u8 *temp = (u8 *)malloc((w+adjust)*h*3+sizeof(INFOHEADER)+sizeof(HEADER));
    if(!temp) {
        fclose(file);
        return;
    }

    HEADER* header = (HEADER*)temp;
    INFOHEADER* infoheader = (INFOHEADER*)(temp+sizeof(HEADER));

    write16(&header->type, 0x4D42);
    write32(&header->size, (w+adjust)*h*3+sizeof(INFOHEADER)+sizeof(HEADER));
    write32(&header->offset, sizeof(INFOHEADER)+sizeof(HEADER));
    write16(&header->reserved1, 0);
    write16(&header->reserved2, 0);

    write16(&infoheader->bits, 24);
    write32(&infoheader->size, sizeof(INFOHEADER));
    write32(&infoheader->compression, 0);
    write32(&infoheader->width, w);
    write32(&infoheader->height, h);
    write16(&infoheader->planes, 1);
    write32(&infoheader->imagesize, (w+adjust)*h*3);
    write32(&infoheader->xresolution, 0);
    write32(&infoheader->yresolution, 0);
    write32(&infoheader->importantcolours, 0);
    write32(&infoheader->ncolours, 0);

    unsigned x, y;
    unsigned char *p = &temp[sizeof(INFOHEADER)+sizeof(HEADER)];
    for(y = 0; y<h; y++) {
        for(x = 0; x<w; x++) {
            *p++ = data[(w*(h - y - 1) + x)*3 + 2]; // B
            *p++ = data[(w*(h - y - 1) + x)*3 + 1]; // G
            *p++ = data[(w*(h - y - 1) + x)*3 + 0]; // R
        }
        p += adjust;    // 4 bytes aligned
    }

    fwrite(temp, 1, (w+adjust)*h*3+sizeof(INFOHEADER)+sizeof(HEADER), file);
    fclose(file);
    free(temp);
}

