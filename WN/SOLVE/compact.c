#include <stdio.h>
#include <stdlib.h>
#include "../INCLUDE/compact.h"

void store_compact(unsigned long nval, FILE *fp) {
    /* Init store_compact */
    register unsigned char cc;

    if (nval < 64) {
        cc = (unsigned char)(0x3F & nval);
        putc(cc, fp);
    }
    else if (nval < 16384) {
        cc = (unsigned char)(0x40 | ((0x3F00 & nval) >> 8));
        putc(cc, fp);
        cc = (unsigned char)(0xFF & nval);
        putc(cc, fp);
    }
    else if (nval < 4194304) {
        cc = (unsigned char)(0x80 | ((0x3F0000 & nval) >> 16));
        putc(cc, fp);
        cc = (unsigned char)((0xFF00 & nval) >> 8);
        putc(cc, fp);
        cc = (unsigned char)(0xFF & nval);
        putc(cc, fp);
    }
    else if (nval < 1073741824) {
        cc = (unsigned char)(0xC0 | ((0x3F000000 & nval) >> 24));
        putc(cc, fp);
        cc = (unsigned char)((0xFF0000 & nval) >> 16);
        putc(cc, fp);
        cc = (unsigned char)((0xFF00 & nval) >> 8);
        putc(cc, fp);
        cc = (unsigned char)(0xFF & nval);
        putc(cc, fp);
    }
    else {
        fprintf(stderr, "store compact error: value overflow\n");
        exit(1);
    }
}/* End store_compact */

void load_compact(unsigned long *pp, FILE *fp) {
    /* Init load_compact */
    register unsigned char cc0 = (unsigned char)getc(fp);
    register unsigned long uu = (unsigned long)(cc0 & 0x3F);
    register unsigned long ii = (unsigned long)(cc0 & 0xC0) >> 6;

    for (; ii ; ii--) {
        cc0 = getc(fp);
        uu = uu << 8;
        uu = uu + cc0;
    }
    *pp = uu;
}/* End load_compact */

#ifdef VECCHIA
void store_compact(nval, fp)
unsigned long nval;
FILE *fp;
{
    /* Init store_compact */
    register unsigned char cc;

    if (nval < 128) {
        cc = (unsigned char)(0x7F & nval);
        putc(cc, fp);
    }
    else if (nval < 16384) {
        cc = (unsigned char)(0x80 | ((0x3F00 & nval) >> 8));
        putc(cc, fp);
        cc = (unsigned char)(0xFF & nval);
        putc(cc, fp);
    }
    else {
        cc = (unsigned char)(0xC0 | ((0x3F0000 & nval) >> 16));
        putc(cc, fp);
        cc = (unsigned char)((0xFF00 & nval) >> 8);
        putc(cc, fp);
        cc = (unsigned char)(0xFF & nval);
        putc(cc, fp);
    }
}/* End store_compact */

void load_compact(pp, fp)
unsigned long *pp;
FILE *fp;
{
    /* Init load_compact */
    register unsigned char cc = getc(fp);
    register unsigned long lc = cc;

    if (0x80 & lc) {
        register unsigned long uu = (lc & 0x3F) << 8;
        cc = getc(fp);
        uu += cc;
        if (0x40 & lc) {
            uu = uu << 8;
            cc = getc(fp);
            uu += cc;
        }
        *pp = uu;
    }
    else
        *pp = lc;
}/* End load_compact */
#endif


void store_double(double *dval, FILE *fp) {
    unsigned char *dv = (unsigned char *)dval;
    /* Init store_double */
    register unsigned i = sizeof(double);

    for (; i-- ; dv++)
        putc(*dv, fp);
}/* End store_double */

void load_double(double *dval, FILE *fp) {
    unsigned char *dv = (unsigned char *)dval;
    /* Init load_double */
    register unsigned i = sizeof(double);

    for (; i-- ; dv++)
        *dv = getc(fp);
}/* End load_double */
