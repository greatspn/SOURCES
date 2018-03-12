/*
 *  Torino, September 4, 1987
 *  program: convertc.c
 *  purpose: conversion of GSPN2 RG into GSPN1 format.
 *  programmer: Giovanni Chiola
 *  notes: this program won't be needed in the final version.
 */

#include <stdio.h>

FILE *infp, * outfp;

/*
#define DEBUG
*/

#include "const.h"

#include "grg.h"

/* TRANSITION ARRAY */ extern struct T_descr trans[];

MARKP rootm;


/* DECODED MARKING */ extern unsigned char DP[];

extern unsigned place_num;

int toptan = 0, topvan = 0;

/* CODING BYTES */ extern unsigned no_cod;

extern unsigned char M0[];


extern load_compact(/* pp, fp */);
/*
  unsigned long * pp;
  FILE * fp;
*/


writegraph(n1, n2, n3, n4)
int n1, n2, n3, n4;
{
    printf("%d\n%d\n%d\n%d\n", n1, n2, n3, n4);
}

load_rgr() {
    unsigned long frm, tmp;
    unsigned long lic, lnel, lthr, lnen;
    unsigned char ic, nel, thr, nen;

#ifdef DEBUG
    fprintf(stderr, "  Start of load_rgr\n");
#endif
    load_compact((&frm), infp);
    load_compact((&lnel), infp); nel = (unsigned char)((lnel & ~ 0x1) >> 1);
    writegraph((int)nel, 0, (frm > toptan), (int)frm);
    for (; nel-- ;) {
        load_compact((&lthr), infp); thr = (unsigned char)lthr;
        if (trans[lthr].en_dep)
            load_compact((&lnen), infp);
        else
            lnen = 1;
        nen = (unsigned char)lnen;
        load_compact((&tmp), infp);
        writegraph((int)thr, (int)nen, (int)(tmp > toptan), (int)tmp);
    }
#ifdef DEBUG
    fprintf(stderr, "  End of load_rgr\n");
#endif
}

main(argc, argv)
int argc;
char **argv;
{
    extern FILE *fopen();

    static char   *can_t_open = "Can't open file %s for %c\n";
    char  filename[100];
    unsigned char ic, * cp1;
    int ii;

#ifdef DEBUG
    fprintf(stderr, "Start\n");
#endif
    if (argc < 2) {
        fprintf(stderr, "ERROR: no net name !\n");
        exit(1);
    }
    sprintf(filename, "%s.ctrs", argv[1]);
    if ((infp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.trs", argv[1]);
    if ((outfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    while ((ic = getc(infp)) != EOF) {
        M0[0] = ic; toptan++;
        for (cp1 = M0, ii = no_cod ; --ii ;)
            *(++cp1) = getc(infp);
        decode_mark(M0);
        for (cp1 = DP + 1, ii = place_num ; ii-- ; cp1++)
            putc(*cp1, outfp);
    }
    (void) fclose(infp);
    (void) fclose(outfp);
    sprintf(filename, "%s.cvrs", argv[1]);
    if ((infp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.vrs", argv[1]);
    if ((outfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    while ((ic = getc(infp)) != EOF) {
        M0[0] = ic; topvan++;
        for (cp1 = M0, ii = no_cod ; --ii ;)
            *(++cp1) = getc(infp);
        decode_mark(M0);
        for (cp1 = DP + 1, ii = place_num ; ii-- ; cp1++)
            putc(*cp1, outfp);
    }
    (void) fclose(infp);
    (void) fclose(outfp);
    sprintf(filename, "%s.crgr", argv[1]);
    if ((infp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
#ifdef DEBUG
    fprintf(stderr, "  RG\n");
#endif
    for (ii = toptan + topvan ; ii-- ;) {
        load_rgr();
    }
    (void) fclose(infp);
#ifdef DEBUG
    fprintf(stderr, "End\n");
#endif
}
