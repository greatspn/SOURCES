/*
 *  Paris, January 25, 1990
 *  program: grg_prep.c
 *  purpose: precompute marking encodings and transition data structures
 *           for TRG construction or simulation of a GSPN without net-dependent
 *           files compilation.
 *  programmer: Giovanni Chiola
 *  notes:
 *   1) No general marking dependency is allowed for transition rates.
 *      Only enabling dependency for timed transitions is implemented.
 */


/*
#define DEBUG
*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

FILE *nfp, * bfp, * ccfp, * scfp, * ecsfp, *subfp, * ofp;

#include "const.h"

unsigned place_num, group_num, trans_num, nomp, sub_num, ecs_num;

struct Coding_descr {
    unsigned char shift;
    unsigned char mask;
    unsigned short num_byte;
    unsigned short link;
};

struct Place_descr {
    unsigned short notok;
    unsigned short lbound;
    unsigned short ubound;
    struct Coding_descr *cods;
    char *name;
};

/*  PLACE ARRAY  */ struct Place_descr *p_list;

/* CODING BYTES */ unsigned *no_cod;

unsigned short *MP;

struct Byte_descr {
    unsigned char free_bits;
    unsigned short max_conf;
    unsigned short first_elem;
};

struct Byte_descr **encoding;

struct Test_descr {
    unsigned short mask;
    unsigned short shift;
    unsigned short offset;
    unsigned short test;
    struct Test_descr *next;
};

struct Enabl_descr {
    unsigned short nobyte;
    struct Test_descr *test_ge;
    struct Test_descr *test_lt;
    struct Enabl_descr *next;
};

struct Change_descr {
    unsigned short place;
    short delta;
    struct Change_descr *next;
};

struct Trans_descr {
    unsigned char en_dep;
    unsigned char no_serv;
    unsigned short noecs;
    unsigned short nt;
    unsigned short nosub;
    struct Coding_descr ecs_cod;
    struct Enabl_descr *test_bytes;
    struct Change_descr *change_places;
    unsigned short *add_list;
    unsigned short *tst_list;
    unsigned short *i_arcs;
    unsigned short *o_arcs;
    unsigned short *h_arcs;
    char *name;
};

/* TRANSITION ARRAY */ struct Trans_descr *t_list;

/* predeclaration */
void getname(char   *name_pr);
void getarcs(unsigned short *arr, int	noar);
void find_byte(struct Trans_descr *trans_p, unsigned num, struct Enabl_descr **byte_pp);

unsigned int nobits(unsigned int n) {
    if (n < 1)
        return ((unsigned)0);
    if (n < 2)
        return ((unsigned)1);
    if (n < 4)
        return ((unsigned)2);
    if (n < 8)
        return ((unsigned)3);
    if (n < 16)
        return ((unsigned)4);
    if (n < 31)
        return ((unsigned)5);
    if (n < 64)
        return ((unsigned)6);
    if (n < 128)
        return ((unsigned)7);
    return ((unsigned)8);
}

char *
ecalloc(unsigned nitm, unsigned sz) {
    char               *callptr;

#ifdef Linux
    if (nitm == 0 || sz == 0)
        return NULL;
#endif

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(2);
    }
    return (callptr);
}


int  norp;

void read_noobjects() {
    char linebuf[LINEMAX];
    int  i, j;
    char cc;
#define VBAR '|'

    /* skip first line containing '|0|' */
    fgets(linebuf, LINEMAX, nfp);
    /* skip comment */
    for (; ;) {
        fgets(linebuf, LINEMAX, nfp);
        if (linebuf[0] == VBAR) break;
    }
    /* read number of objects in the net */
    fscanf(nfp, "f %d %d %d", &nomp, &place_num, &norp);
    fscanf(nfp, "%d %d %d %d\n", &trans_num, &group_num, &i, &j);
    fscanf(subfp, "%d\n", &sub_num);
    while ((cc = getc(subfp)) != '\n');
    for (ecs_num = 0, fscanf(ecsfp, "%d", &i) ; i ;) {
        while ((cc = getc(ecsfp)) != '\n'); fscanf(ecsfp, "%d", &i);
        ecs_num++;
    }

    p_list = (struct Place_descr *)ecalloc(place_num, sizeof(struct Place_descr));
    i = sub_num + ecs_num + 1;
    encoding = (struct Byte_descr **)ecalloc(i, sizeof(struct Byte_descr *));
    no_cod = (unsigned *)ecalloc(i, sizeof(unsigned));
    *(encoding) = (struct Byte_descr *)ecalloc(place_num,
                  sizeof(struct Byte_descr));
    MP = (unsigned short *)ecalloc(nomp, sizeof(unsigned short));
    t_list = (struct Trans_descr *)ecalloc(trans_num, sizeof(struct Trans_descr));
}


void load_places() {
    struct Place_descr   *place;
    char                linebuf[LINEMAX];
    float               ftemp, x, y;
    int                 i, mark;
#ifdef DEBUG
    printf("  Start of load_places\n");
    printf("    %d marking parameters\n", nomp);
#endif

    /* read marking parameters */
    {
        register int i = 0;
        unsigned int mark;
        while (i < nomp) {
            getname(linebuf);
            fscanf(nfp, "%d", &mark); while (getc(nfp) != '\n');
#ifdef DEBUG
            printf("    %d %s=%d\n", i, linebuf, mark);
#endif
            MP[i++] = mark;
        }
    }

    /* read places */
    i = 0; place = p_list;
    while (i < place_num) {
        getname(linebuf);
        place->name = ecalloc(strlen(linebuf) + 1, 1);
        sprintf(place->name, linebuf);
        fscanf(nfp, "%d ", &mark); while (getc(nfp) != '\n');
        place->cods = (struct Coding_descr *)ecalloc(sub_num,
                      sizeof(struct Coding_descr));
        place->cods->link = 0;
        if (mark >= 0)
            place->notok = mark;
        else
            place->notok = MP[-1 - mark];
#ifdef DEBUG
        printf("    place %s %d\n", linebuf, place->notok);
#endif
        ++place; ++i;
    }

    /* skip rate parameters */
    i = 0;
    while (i++ < norp) {
        getname(linebuf);
        fscanf(nfp, "%f", &ftemp); while (getc(nfp) != '\n');
    }

    /* skip groups */
    i = 1;
    while (i++ <= group_num) {
        getname(linebuf);
        fscanf(nfp, "%f %f", &x, &y); while (getc(nfp) != '\n');
    }
#ifdef DEBUG
    printf("  End of load_places\n");
#endif
}

void getname(char   *name_pr) {
#define BLANK ' '
#define EOLN  '\0'
    unsigned            i;

    for ((*name_pr) = fgetc(nfp) , i = 1 ;
            (*name_pr) != BLANK &&
            (*name_pr) != '\n' &&
            (*name_pr) != (char)0 &&
            i++ <= TAG_SIZE ;
            (*(++name_pr)) = fgetc(nfp));
    (*name_pr) = EOLN;
}

int cod_cmp(const struct Byte_descr *p1, const struct Byte_descr *p2) {
    register int m1, m2;

    m1 = (p1->max_conf);
    m2 = (p2->max_conf);
    if (m1 == m2)
        return (0);
    return ((m2 > m1) ? -1 : 1);
}



void read_bounds() {
    struct Place_descr *place_p;
    struct Byte_descr *byte_p;
    unsigned int ii, jj, kk, ll, ss;
    unsigned char mm;
    int new_enc;

#ifdef DEBUG
    printf("  Start of read_bounds\n");
#endif
    *no_cod = 0;
    for (place_p = p_list, ii = 0 ; ++ii <= place_num ; place_p++) {
        fscanf(bfp, "%d %d\n", &jj, &kk);
        place_p->lbound = jj;
        place_p->ubound = kk;
        kk -= jj;
        ll = nobits(kk); new_enc = TRUE;
        for (byte_p = *encoding, jj = *no_cod ;
                jj-- && new_enc ;) {
            if ((new_enc = (byte_p->free_bits < ll)))
                byte_p++;
        }
        if (new_enc) {
            (*no_cod)++;
            byte_p->free_bits = 8;
            byte_p->max_conf = 1;
            byte_p->first_elem = 0;
        }
        place_p->cods->link = byte_p->first_elem;
        ss = (8 - byte_p->free_bits);
        byte_p->free_bits -= ll;
        if (ll) {
            mm = (unsigned char)1;
            while (--ll)
                mm += mm + 1;
        }
        else  {
            mm = 0; kk = 0; ss = 0;
        }
        place_p->cods->shift = ss;
        place_p->cods->mask = (mm << ss);
        kk++;
        byte_p->max_conf *= kk;
        byte_p->first_elem = ii;
    }
#ifdef DEBUG
    printf("\n*** sorting %d codings\n", (*no_cod));
#endif
    qsort(*encoding, (*no_cod), sizeof(struct Byte_descr), (int (*)(const void *, const void *)) cod_cmp);
#ifdef DEBUG
    fprintf(stderr, "\n%d places have been coded in %d bytes as follows:\n",
            place_num, (*no_cod));
#endif
    for (byte_p = *encoding, ii = 0 ; ++ii <= (*no_cod) ; byte_p++) {
#ifdef DEBUG
        fprintf(stderr, "    coding #%d using %d bits (%d configurations):\n",
                ii, (8 - byte_p->free_bits), byte_p->max_conf);
#endif
        for (jj = byte_p->first_elem ; jj ; jj = place_p->cods->link) {
            place_p = p_list + (jj - 1);
            place_p->cods->num_byte = ii;
#ifdef DEBUG
            fprintf(stderr, "        place %d mask=%d shift=%d in byte #%d\n",
                    jj, place_p->cods->mask, place_p->cods->shift, place_p->cods->num_byte);
            fprintf(stderr, "            lbound=%d ubound=%d notok=%d\n",
                    place_p->lbound, place_p->ubound, place_p->notok);
#endif
        }
    }
#ifdef DEBUG
    printf("  End of read_bounds\n");
#endif
}


void compute_sub_enc() {
    struct Place_descr *place_p;
    struct Trans_descr *trans_p;
    struct Byte_descr **byte_pp;
    struct Byte_descr *byte_p;
    unsigned int nn, np, nc, ii, jj, kk, ll, ss;
    unsigned char mm;
    char cc;
    int new_enc;

#ifdef DEBUG
    printf("  Start of compute_sub_enc\n");
#endif
    for (ii = 0 ; ii < sub_num ; ++ii) {
        fscanf(subfp, "%d", &nn);
#ifdef DEBUG
        printf("   %d transitions in subnet # %d:\n        ", nn, ii);
#endif
        for (jj = 0 ; jj++ < nn ;) {
            fscanf(subfp, "%d", &kk);
#ifdef DEBUG
            printf(" t %d,", kk);
#endif
            trans_p = t_list + (kk - 1);
            trans_p->nosub = ii;
        }
#ifdef DEBUG
        printf("\n");
#endif
        while ((cc = getc(subfp)) != '\n');
        while ((cc = getc(subfp)) != '\n');
    }
    for (byte_pp = encoding + 1, nn = 0 ; ++nn < sub_num ; byte_pp++) {
#ifdef DEBUG
        printf(" subnet # %d\n", nn);
#endif
        no_cod[nn] = 0;
        fscanf(subfp, "%d", &np);
        *byte_pp = (struct Byte_descr *)ecalloc(np,
                                                sizeof(struct Byte_descr));
        for (ii = 0 ; ii++ < np ;) {
            fscanf(subfp, "%d", &jj);
            place_p = p_list + (jj - 1);
            kk = place_p->ubound - place_p->lbound;
            ll = nobits(kk); new_enc = TRUE;
            for (byte_p = *byte_pp, nc = no_cod[nn] ;
                    nc-- && new_enc ;) {
                if ((new_enc = (byte_p->free_bits < ll)))
                    byte_p++;
            }
            if (new_enc) {
                (no_cod[nn])++;
                byte_p->free_bits = 8;
                byte_p->max_conf = 1;
                byte_p->first_elem = 0;
            }
            place_p->cods[nn].link = byte_p->first_elem;
            ss = (8 - byte_p->free_bits);
            byte_p->free_bits -= ll;
            if (ll) {
                mm = (unsigned char)1;
                while (--ll)
                    mm += mm + 1;
            }
            else  {
                mm = 0; kk = 0; ss = 0;
            }
            place_p->cods[nn].shift = ss;
            place_p->cods[nn].mask = (mm << ss);
            kk++;
            byte_p->max_conf *= kk;
            byte_p->first_elem = jj;
        }
        while ((cc = getc(subfp)) != '\n');
#ifdef DEBUG
        printf("\n*** sorting %d codings\n", no_cod[nn]);
#endif
        qsort(*byte_pp, no_cod[nn], sizeof(struct Byte_descr), (int (*)(const void *, const void *)) cod_cmp);
#ifdef DEBUG
        fprintf(stderr, "\n%d places have been coded in %d bytes as follows:\n",
                np, no_cod[nn]);
#endif
        for (byte_p = *byte_pp, ii = 0 ; ++ii <= no_cod[nn] ; byte_p++) {
#ifdef DEBUG
            fprintf(stderr, "    coding #%d using %d bits (%d configurations):\n",
                    ii, (8 - byte_p->free_bits), byte_p->max_conf);
#endif
            for (jj = byte_p->first_elem ; jj ; jj = place_p->cods[nn].link) {
                place_p = p_list + (jj - 1);
                place_p->cods[nn].num_byte = ii;
#ifdef DEBUG
                fprintf(stderr, "        place %d mask=%d shift=%d in byte #%d\n", jj,
                        place_p->cods[nn].mask, place_p->cods[nn].shift, place_p->cods[nn].num_byte);
                fprintf(stderr, "            lbound=%d ubound=%d\n",
                        place_p->lbound, place_p->ubound);
#endif
            }
        }
    }
#ifdef DEBUG
    printf("  End of compute_sub_enc\n");
#endif
}


void write_marking_enc() {
    struct Place_descr *place_p;
    struct Byte_descr **byte_pp;
    struct Byte_descr *byte_p;
    unsigned int ii, jj, kk, ll, mm;

#ifdef DEBUG
    printf("  Start of write_marking_enc\n");
#endif
    fprintf(ofp, "%d %d %d %d\n", sub_num, place_num, ecs_num, trans_num);
    for (byte_pp = encoding, mm = 0 ; mm < sub_num ; mm++, byte_pp++) {
        fprintf(ofp, "%d %d\n", mm, no_cod[mm]);
        for (byte_p = *byte_pp, ii = 0 ; ++ii <= no_cod[mm] ; byte_p++) {
            fprintf(ofp, " %d %d %d ", ii, (8 - byte_p->free_bits), byte_p->max_conf - 1);
            for (kk = ll = 0, jj = byte_p->first_elem ;
                    jj ; kk++, jj = place_p->cods[mm].link) {
                place_p = p_list + (jj - 1);
                ll ^= (((unsigned int)((int)(place_p->notok) - (int)(place_p->lbound)))
                       << (place_p->cods[mm].shift));
            }
            fprintf(ofp, "%d %d\n", kk, ll);
            for (jj = byte_p->first_elem ; jj ; jj = place_p->cods[mm].link) {
                place_p = p_list + (jj - 1);
                fprintf(ofp, "   %d %d %d %d %d\n", jj, place_p->cods[mm].mask, place_p->cods[mm].shift,
                        place_p->lbound, place_p->ubound - place_p->lbound);
            }
        }
    }
    for (place_p = p_list, ii = 0 ; ++ii <= place_num ; place_p++) {
        fprintf(ofp, "%d %d %d %d %d %d %d %s\n",
                ii, place_p->cods->num_byte, place_p->cods->mask,
                place_p->cods->shift, place_p->lbound,
                place_p->ubound - place_p->lbound, place_p->notok,
                place_p->name);
    }
#ifdef DEBUG
    printf("  End of write_marking_enc\n");
#endif
}

unsigned load_vrs;

#define DETERM 127

void read_trans() {
    struct Trans_descr *trans_p;
    int nt, ii;
    char		linebuf[LINEMAX];
    float		ftemp;
    int                 knd, noar, load_d;

#ifdef DEBUG
    printf("  Start of read_trans\n");
#endif

    /* read transitions */
    for (trans_p = t_list, nt = 0 ; ++nt <= trans_num ; trans_p++) {
        trans_p->nt = nt;
        getname(linebuf);
        trans_p->name = ecalloc(strlen(linebuf) + 1, 1);
        sprintf(trans_p->name, linebuf);
        fscanf(nfp, "%f %d %d %d ", &ftemp, &load_d, &knd, &noar);
        while (getc(nfp) != '\n');
        if (load_d < 0) {
            ii = load_d = -load_d;
            while (--ii) {
                while (getc(nfp) != '\n');
            }
        }
        if (knd && (knd != DETERM))
            load_d = 1;
        trans_p->en_dep = (unsigned char)(load_d != 1);
        if (! load_d)
            load_d = 255;
        trans_p->no_serv = load_d;
        trans_p->noecs = knd;
        trans_p->i_arcs = (unsigned short *)ecalloc((noar + noar + 1),
                          sizeof(unsigned short));
        getarcs(trans_p->i_arcs, noar);
        fscanf(nfp, "%d\n", &noar);
        trans_p->o_arcs = (unsigned short *)ecalloc((noar + noar + 1),
                          sizeof(unsigned short));
        getarcs(trans_p->o_arcs, noar);
        fscanf(nfp, "%d\n", &noar);
        trans_p->h_arcs = (unsigned short *)ecalloc((noar + noar + 1),
                          sizeof(unsigned short));
        getarcs(trans_p->h_arcs, noar);
    }

#ifdef DEBUG
    printf("  End of read_trans\n");
#endif
}

void getarcs(unsigned short *arr, int	noar) {
    int			i, j, pl, mlt, ip;
    float		x, y;

    *(arr++) = noar;
    for (i = noar ; i-- ;) {
        fscanf(nfp, "%d %d %d", &mlt, &pl, &ip);
        while (getc(nfp) != '\n');
        for (j = 1; j++ <= ip; fscanf(nfp, "%f %f\n", &x, &y));
        *(arr++) = (ABS(mlt));
        *(arr++) = pl;
    }
}



int ecs_cmp(const struct Trans_descr *p1, const struct Trans_descr   *p2) {
    register int m1, m2;

    m1 = (p1->nosub);
    m2 = (p2->nosub);
    if ((m1 == m2)) {
        m1 = (p1->noecs);
        m2 = (p2->noecs);
    }
    if (m1 == m2)
        return (0);
    return ((m2 > m1) ? -1 : 1);
}

void read_ecs() {
    struct Trans_descr *trans_p, * trans2_p;
    struct Byte_descr **byte_pp;
    struct Byte_descr *byte_p;
    int nc, nt, nn, ii, jj, kk, ll, cur_pri, old_pri;
    unsigned mm, ss;
    int new_enc;

#ifdef DEBUG
    printf("  Start of read_ecs\n");
#endif
    for (nn = 0 ; nn++ < ecs_num ;) {
        fscanf(ecsfp, "%d", &nt);
        for (ii = 0 ; ii++ < nt ;) {
            fscanf(ecsfp, "%d", &jj);
            trans_p = t_list + (jj - 1);
            trans_p->noecs = nn;
        }
        while (getc(ecsfp) != '\n');
    }
    qsort(t_list, trans_num, sizeof(struct Trans_descr), (int (*)(const void *, const void *)) ecs_cmp);
    for (old_pri = -1, cur_pri = ecs_num, trans_p = t_list + (trans_num - 1),
            nn = sub_num + ecs_num, byte_pp = encoding + nn ;
            cur_pri ; cur_pri--, nn--, byte_pp--) {
        no_cod[nn] = 0;
        old_pri = trans_p->noecs;
        for (ii = 0 ; trans_p->noecs == old_pri ;
                ++ii, (trans_p--)->noecs = cur_pri);
        *byte_pp = byte_p = (struct Byte_descr *)ecalloc(ii,
                            sizeof(struct Byte_descr));
        jj = (trans_p - t_list) + 2;
        trans2_p = t_list + (jj - 1);
        for (; ii-- ; trans2_p++, jj++) {
            kk = 1;
            ll = 1;
            new_enc = TRUE;
            for (byte_p = *byte_pp, nc = no_cod[nn] ;
                    nc-- && new_enc ;) {
                if ((new_enc = (byte_p->free_bits < 1)))
                    byte_p++;
            }
            if (new_enc) {
                (no_cod[nn])++;
                byte_p->free_bits = 8;
                byte_p->max_conf = 1;
                byte_p->first_elem = 0;
            }
            trans2_p->ecs_cod.link = byte_p->first_elem;
            ss = (8 - byte_p->free_bits);
            (byte_p->free_bits)--;
            mm = (unsigned char)1;
            trans2_p->ecs_cod.shift = ss;
            trans2_p->ecs_cod.mask = (mm << ss);
            byte_p->max_conf += byte_p->max_conf;
            byte_p->first_elem = jj;
        }
#ifdef DEBUG
        printf("\n*** sorting %d codings\n", no_cod[nn]);
#endif
        qsort(*byte_pp, no_cod[nn], sizeof(struct Byte_descr), (int (*)(const void *, const void *)) cod_cmp);
#ifdef DEBUG
        fprintf(stderr, "\n ECS #%d has been coded in %d bytes as follows:\n",
                cur_pri, no_cod[nn]);
#endif
        for (byte_p = *byte_pp, ii = 0 ; ++ii <= no_cod[nn] ; byte_p++) {
#ifdef DEBUG
            fprintf(stderr, "    coding #%d using %d bits (%d configurations):\n",
                    ii, (8 - byte_p->free_bits), byte_p->max_conf);
#endif
            for (jj = byte_p->first_elem ; jj ; jj = trans2_p->ecs_cod.link) {
                trans2_p = t_list + (jj - 1);
                trans2_p->ecs_cod.num_byte = ii;
#ifdef DEBUG
                fprintf(stderr, "        trans %d mask=%d shift=%d in byte #%d\n", jj,
                        trans2_p->ecs_cod.mask, trans2_p->ecs_cod.shift, trans2_p->ecs_cod.num_byte);
#endif
            }
        }
    }
    for (nn = sub_num + 1, byte_pp = encoding + nn, mm = 0 ; mm++ < ecs_num ;
            byte_pp++, nn++) {
#ifdef DEBUG
        fprintf(stderr, "      writing ECS #%d\n", mm);
#endif
        fprintf(ofp, "%d %d\n", mm, no_cod[nn]);
        for (byte_p = *byte_pp, ii = 0 ; ++ii <= no_cod[nn] ; byte_p++) {
            fprintf(ofp, " %d %d %d ", ii, (8 - byte_p->free_bits),
                    byte_p->max_conf - 1);
            for (kk = 0, jj = byte_p->first_elem ;
                    jj ; kk++, jj = trans_p->ecs_cod.link) {
                trans_p = t_list + (jj - 1);
            }
#ifdef DEBUG
            fprintf(stderr, "        writing byte #%d encoding %d trans\n", ii, kk);
#endif
            fprintf(ofp, "%d\n", kk);
            for (jj = byte_p->first_elem ; jj ; jj = trans_p->ecs_cod.link) {
                trans_p = t_list + (jj - 1);
                fprintf(ofp, "   %d %d %d\n", jj,
                        trans_p->ecs_cod.mask, trans_p->ecs_cod.shift);
            }
        }
    }
#ifdef DEBUG
    printf("  End of read_ecs\n");
#endif
}


void compute_entest() {
    struct Trans_descr *trans_p;
    struct Place_descr *place_p;
    struct Enabl_descr *byte_p;
    struct Test_descr *tdp;
    int ii, nt, np, mult, na;
    unsigned short *usp;
    unsigned nobyte, tst, msk, shft, offst;

#ifdef DEBUG
    printf("  Start of compute_entest\n");
#endif
    for (trans_p = t_list, nt = 0 ; ++nt <= trans_num ; trans_p++) {
        trans_p->test_bytes = NULL;
#ifdef DEBUG
        printf("    trans #%d\n", nt);
#endif
        for (usp = trans_p->i_arcs, na = *(usp++), ii = 0 ; ++ii <= na ;) {
            mult = *(usp++); np = *(usp++);
            place_p = p_list + (np - 1);
            nobyte = place_p->cods->num_byte;
            msk = place_p->cods->mask;
            shft = place_p->cods->shift;
            offst = place_p->lbound;
            tst = mult;
            if (tst > offst) {
#ifdef DEBUG
                printf("       ge place=%d byte #%d, mask=%d shift=%d offset=%d test=%d\n",
                       np, nobyte, msk, shft, offst, tst);
#endif
                find_byte(trans_p, nobyte, &byte_p);
                tdp = (struct Test_descr *)ecalloc(1, sizeof(struct Test_descr));
                tdp->mask = msk; tdp->shift = shft;
                tdp->offset = offst; tdp->test = tst;
                tdp->next = byte_p->test_ge;
                byte_p->test_ge = tdp;
            }
        }
        for (usp = trans_p->h_arcs, na = *(usp++), ii = 0 ; ++ii <= na ;) {
            mult = *(usp++); np = *(usp++);
            place_p = p_list + (np - 1);
            nobyte = place_p->cods->num_byte;
            msk = place_p->cods->mask;
            shft = place_p->cods->shift;
            offst = place_p->lbound;
            tst = mult;
            if (tst <= place_p->ubound) {
#ifdef DEBUG
                printf("       lt place=%d byte #%d, mask=%d shift=%d offset=%d test=%d\n",
                       np, nobyte, msk, shft, offst, tst);
#endif
                find_byte(trans_p, nobyte, &byte_p);
                tdp = (struct Test_descr *)ecalloc(1, sizeof(struct Test_descr));
                tdp->mask = msk; tdp->shift = shft;
                tdp->offset = offst; tdp->test = tst;
                tdp->next = byte_p->test_lt;
                byte_p->test_lt = tdp;
            }
        }
    }
#ifdef DEBUG
    printf("  End of compute_entest\n");
#endif
}

void find_byte(struct Trans_descr *trans_p, unsigned num, struct Enabl_descr **byte_pp) {
    int new_byte, go_on;
    struct Enabl_descr *pbyte_p = trans_p->test_bytes;
    struct Enabl_descr *byte_p;

#ifdef DEBUG
    printf("   Start of find_byte\n");
#endif
    if (pbyte_p == NULL) {
        pbyte_p = (struct Enabl_descr *)ecalloc(1, sizeof(struct Enabl_descr));
        *byte_pp = trans_p->test_bytes = pbyte_p;
        pbyte_p->next = NULL;
        pbyte_p->nobyte = num;
        pbyte_p->test_ge = NULL;
        pbyte_p->test_lt = NULL;
        return;
    }
    for (go_on = TRUE; go_on ;) {
        go_on = new_byte = (pbyte_p->nobyte != num);
        if ((go_on &= ((byte_p = pbyte_p->next) != NULL)))
            if ((go_on = ((byte_p->nobyte <= num))))
                pbyte_p = byte_p;
    }
    if (new_byte) {
        byte_p = (struct Enabl_descr *)ecalloc(1, sizeof(struct Enabl_descr));
        *byte_pp = byte_p;
        byte_p->nobyte = num;
        byte_p->test_ge = NULL;
        byte_p->test_lt = NULL;
        if (num < pbyte_p->nobyte) {
            byte_p->next = pbyte_p;
            trans_p->test_bytes = byte_p;
        }
        else {
            byte_p->next = pbyte_p->next;
            pbyte_p->next = byte_p;
        }
        return;
    }
    *byte_pp = pbyte_p;
#ifdef DEBUG
    printf("   End of find_byte\n");
#endif
}

void compute_changes() {
    struct Trans_descr *trans_p;
    struct Change_descr *change_p;
    short *row_p, * pp;
    int ii, nt, np, mult, na;
    unsigned short *usp;

#ifdef DEBUG
    printf("  Start of compute_changes\n");
#endif
    row_p = (short *)ecalloc(place_num + 1, sizeof(short));
    for (pp = row_p, ii = 0 ; ++ii <= place_num ; * (++pp) = 0);
    for (trans_p = t_list, nt = 0 ; ++nt <= trans_num ; trans_p++) {
#ifdef DEBUG
        printf("    trans #%d\n", nt);
#endif
        trans_p->change_places = NULL;
        for (usp = trans_p->i_arcs, na = *(usp++), ii = 0 ; ++ii <= na ;) {
            mult = *(usp++); np = *(usp++);
            row_p[np] -= mult;
        }
        for (usp = trans_p->o_arcs, na = *(usp++), ii = 0 ; ++ii <= na ;) {
            mult = *(usp++); np = *(usp++);
            row_p[np] += mult;
        }
        for (pp = row_p + 1, np = 0 ; ++np <= place_num ; * (pp++) = 0) {
            if ((mult = *pp) != 0) {
                change_p = (struct Change_descr *)ecalloc(1,
                           sizeof(struct Change_descr));
                change_p->place = np;
                change_p->delta = mult;
                change_p->next = trans_p->change_places;
                trans_p->change_places = change_p;
            }
        }
    }
    free(row_p);
#ifdef DEBUG
    printf("  End of compute_changes\n");
#endif
}


int
ntrans_conv(unsigned nt) {
    int count_t;
    struct Trans_descr *trans_p;

    for (trans_p = t_list, count_t = 0 ; count_t++ < trans_num ; trans_p++) {
        if (trans_p->nt == nt)
            return (count_t);
    }
    return -1;
}

void compute_transrel() {
    struct Trans_descr *trans_p;
    int ii, nt, nad, nte;
    unsigned char *row_p, * pp;
    unsigned short *usp;

#ifdef DEBUG
    printf("  Start of compute_transrel\n");
#endif
    row_p = (unsigned char *)ecalloc(trans_num + 1, sizeof(unsigned char));
    for (pp = row_p, ii = 0 ; ++ii <= trans_num ; * (++pp) = FALSE);
    for (nt = 0 ; ++nt <= trans_num ;) {
        ii = ntrans_conv(nt);
        trans_p = t_list + (ii - 1);
        fscanf(ccfp, "%d", &nad);
        usp = (unsigned short *)ecalloc(nad + 1, sizeof(unsigned short));
        trans_p->add_list = usp;
        *(usp++) = nad;
#ifdef DEBUG
        fprintf(stderr, "   add_list for trans #%d with %d items:\n", ii, nad);
#endif
        for (ii = nad ; ii-- ;) {
            int nnn;
            fscanf(ccfp, "%d", &nnn);
            nnn = ntrans_conv(nnn);
            row_p[nnn] = TRUE;
        }
        fscanf(ccfp, "\n");
        for (pp = row_p + 1, ii = 0 ; ++ii <= trans_num ; * (pp++) = 0)
            if ((int)(*pp)) {
                *(usp++) = ii;
#ifdef DEBUG
                fprintf(stderr, "         trans #%d\n", ii);
#endif
            }
        fscanf(scfp, "%d", &nte);
#ifdef DEBUG
        fprintf(stderr, "   rem_list with %d items:\n", nte);
#endif
        for (ii = nte ; ii-- ;) {
            int nnn;
            fscanf(scfp, "%d", &nnn);
            nnn = ntrans_conv(nnn);
            row_p[nnn] = TRUE;
#ifdef DEBUG
            fprintf(stderr, "         trans #%d\n", nnn);
#endif
        }
        fscanf(scfp, "\n");
        for (pp = row_p, nte = ii = 0 ; ++ii <= trans_num ;)
            if ((int)(*(++pp)))
                nte++;
#ifdef DEBUG
        fprintf(stderr, "   tst_list for trans #%d with %d items:\n", nt, nte);
#endif
        usp = (unsigned short *)ecalloc(nte + 1, sizeof(unsigned short));
        trans_p->tst_list = usp;
        *(usp++) = nte;
        for (pp = row_p + 1, ii = 0 ; ++ii <= trans_num ; * (pp++) = FALSE)
            if ((int)(*pp)) {
                *(usp++) = ii;
#ifdef DEBUG
                fprintf(stderr, "         trans #%d\n", ii);
#endif
            }
    }
#ifdef DEBUG
    printf("  End of compute_transrel\n");
#endif
}

void write_trans() {
    struct Trans_descr *trans_p;
    struct Enabl_descr *byte_p;
    struct Test_descr *test_p;
    struct Change_descr *change_p;
    int ii, jj, nt, nb, nge, nlt, nc;
    unsigned short *usp;

#ifdef DEBUG
    printf("  Start of write_trans\n");
#endif
    for (trans_p = t_list, nt = 0 ; ++nt <= trans_num ; trans_p++) {
#ifdef DEBUG
        fprintf(stderr, "   writing transition #%d\n", nt);
#endif
        fprintf(ofp, "%d %d %d %d %d ", trans_p->nt, trans_p->noecs,
                trans_p->nosub, trans_p->en_dep, trans_p->no_serv);
        for (byte_p = trans_p->test_bytes, nb = 0 ; byte_p != NULL ;
                nb++, byte_p = byte_p->next);
        for (change_p = trans_p->change_places, nc = 0 ; change_p != NULL ;
                nc++, change_p = change_p->next);
        fprintf(ofp, "%d %d %s\n", nb, nc, trans_p->name);
        for (byte_p = trans_p->test_bytes, ii = 0 ; ++ii <= nb ;
                byte_p = byte_p->next) {
            fprintf(ofp, "  %d ", byte_p->nobyte);
            for (test_p = byte_p->test_ge, nge = 0 ; test_p != NULL ;
                    nge++, test_p = test_p->next);
            for (test_p = byte_p->test_lt, nlt = 0 ; test_p != NULL ;
                    nlt++, test_p = test_p->next);
            fprintf(ofp, "%d %d\n", nge, nlt);
            for (test_p = byte_p->test_ge, jj = 0 ; ++jj <= nge ;
                    test_p = test_p->next) {
                fprintf(ofp, "    %d %d %d %d\n", test_p->mask,
                        test_p->shift, test_p->offset, test_p->test);
            }
            for (test_p = byte_p->test_lt, jj = 0 ; ++jj <= nlt ;
                    test_p = test_p->next) {
                fprintf(ofp, "    %d %d %d %d\n", test_p->mask,
                        test_p->shift, test_p->offset, test_p->test);
            }
        }
        for (change_p = trans_p->change_places, ii = 0 ; ++ii <= nc ;
                change_p = change_p->next) {
            fprintf(ofp, "  %d %d\n", change_p->place, change_p->delta);
        }
#ifdef DEBUG
        fprintf(stderr, "    writing add_list\n");
#endif
        usp = trans_p->add_list, ii = *(usp++);
        fprintf(ofp, "  %d", ii);
        for (jj = 0 ; ++jj <= ii ; fprintf(ofp, " %d", *(usp++)));
#ifdef DEBUG
        fprintf(stderr, "    writing tst_list\n");
#endif
        usp = trans_p->tst_list, ii = *(usp++);
        fprintf(ofp, "\n  %d", ii);
        for (jj = 0 ; ++jj <= ii ; fprintf(ofp, " %d", *(usp++)));
        fprintf(ofp, "\n");
    }
#ifdef DEBUG
    printf("  End of write_trans\n");
#endif
}

int main(int argc, char **argv) {
    static char   *can_t_open = "Can't open file %s for %c\n";
    char  filename[100];

#ifdef DEBUG
    printf("Start\n");
#endif
    if (argc < 2) {
        printf("ERROR: no net name !\n");
        exit(1);
    }
    sprintf(filename, "%s.net", argv[1]);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.sub", argv[1]);
    if ((subfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.ecs", argv[1]);
    if ((ecsfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    read_noobjects();
    fclose(ecsfp);
    load_places();
    sprintf(filename, "%s.bnd", argv[1]);
    if ((bfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    read_bounds();
    fclose(bfp);
    compute_sub_enc();
    fclose(subfp);
    sprintf(filename, "%s.grg", argv[1]);
    if ((ofp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    write_marking_enc();
    read_trans();
    fclose(nfp);
    sprintf(filename, "%s.ecs", argv[1]);
    if ((ecsfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    read_ecs();
    fclose(ecsfp);
    compute_entest();
    compute_changes();
    sprintf(filename, "%s.sc", argv[1]);
    if ((scfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.cc", argv[1]);
    if ((ccfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    compute_transrel();
    fclose(scfp);
    fclose(ccfp);
    write_trans();
    fclose(ofp);
#ifdef DEBUG
    printf("End\n");
#endif
    return 0;
}


