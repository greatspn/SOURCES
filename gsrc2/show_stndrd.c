/*
 *  Paris, January 24, 1990
 *  program: show_stndrd.c
 *  purpose: display of the TRG of a GSPN without net-dependent files compilation.
 *  programmer: Giovanni Chiola
 *  notes:
 *   1) No general marking dependency is allowed for transition rates.
 *      Only enabling dependency for timed transitions is implemented.
 */

#include <string.h>
#include <stdlib.h>
#include "../WN/INCLUDE/compact.h"
/*
#define DEBUG
*/

#ifdef DEBUG

#define DEBUGCALLS
/*
*/

#endif


#include "const.h"

#include "grg.h"

int show_TRG, show_TRS, show_RTRG;

/* void (*terminate)(); */
void terminate() { exit(1); }

unsigned place_num, group_num, trans_num;

#include <stdio.h>

FILE *nfp, * rgfp, * tmfp, * aefp;


struct Codbyt_descr {
    unsigned char shift;
    unsigned char mask;
    unsigned short num_byte;
    unsigned short link;
};

struct Place_descr {
    unsigned short lbound;
    unsigned short rubound;
    struct Codbyt_descr cod;
    char *name;
};

/*  PLACE ARRAY  */ struct Place_descr *p_list;

unsigned sub_num;

/* CODING BYTES */ unsigned no_cod;

/* CODED MARKING */ unsigned char **CM;

/* MARKING REACHABILITY */ unsigned char *MR;
int reach_M1, new_reach;

union RG_descr {
    unsigned *from;
    unsigned num;
};

/* REVERSE GRAPH */ union RG_descr *RRG;
union RG_descr *rrg_p;
unsigned *from_p;


struct ECS_descr {
    unsigned short number;
    unsigned short **set;
};

struct ECS_descr *aecs, * aecs_p;
int Naecs;

struct Byte_descr {
    unsigned char noitems;
    unsigned char max_conf;
    unsigned short first_item;
};

struct Byte_descr *encoding;

struct Enabl_descr {
    unsigned char notestge;
    unsigned char notestlt;
    unsigned short nobyte;
    struct Test_descr *test_list;
};

struct Trans_descr {
    unsigned char noecs;
    char *name;
};

/* TRANSITION ARRAY */ struct Trans_descr *t_list;

/* DECODED MARKING */ unsigned short *DM;

unsigned lastan, toptan = 0, loadedtan = 0;
unsigned maxmark, topvan;


char *ecalloc(unsigned nitm, unsigned sz) {
    //extern char *       calloc();
    char               *callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        /*        (*terminate)(); */
        terminate();
    }
    return (callptr);
}


void decode_mark(unsigned char *Mp, unsigned short *dmp) {
    struct Place_descr *place_p = p_list;
    unsigned np, nb, val;

    for (np = place_num ; np-- ; place_p++) {
        if ((nb = place_p->cod.num_byte)) {
            nb--;
            val = Mp[nb] & place_p->cod.mask;
            val = val >> (place_p->cod.shift);
            val += place_p->lbound;
            *(++dmp) = val;
        }
    }
}



void init() {
    int ii, jj, kk, ll, mm, nn, np, ns, nt, ss;
    struct Byte_descr *byte_p;
    struct Place_descr *place_p;
    unsigned char *ucp;
    unsigned short *usp;
    struct Trans_descr *trans_p;
    int na, nb, nge, nlt, nc, n1, n2, n3, n4;
    char linebuf[LINEMAX];
    int jj2;
    struct Codbyt_descr *cods_p;
    unsigned short **uspp;

#ifdef DEBUG
    fprintf(stderr, "  Start of init\n");
#endif

    /* read aecs */
    for (ii = Naecs ; ii-- ;) {
        fscanf(aefp, "%d %d %d", &jj, &kk, &ll);
#ifdef DEBUG
        fprintf(stderr, "      ECS=%d AECS=%d nt=%d\n", jj, kk, ll);
#endif
        aecs_p = aecs + jj;
        (aecs_p->set)[kk] = usp = (unsigned short *)ecalloc(ll + 1,
                                  sizeof(unsigned short));
        *(usp++) = ll;
        while (ll--) {
            fscanf(aefp, "%d", &mm);
            *(usp++) = mm;
        }
        while (getc(aefp) != '\n');
    }

    /* read number of objects in the net */
    fscanf(nfp, "%d %d %d %d\n", &sub_num, &place_num, &group_num, &trans_num);

    CM = (unsigned char **)ecalloc(toptan + 1, sizeof(unsigned char *));
    MR = (unsigned char *)ecalloc(toptan + 1, sizeof(unsigned char));
    RRG = (union RG_descr *)ecalloc(toptan + 1, sizeof(union RG_descr));
    for (rrg_p = RRG, ucp = MR, ii = toptan ; ii-- ;
            (++rrg_p)->num = 0, *(++ucp) = 0);
    DM = (unsigned short *)ecalloc(place_num + 1, sizeof(unsigned short));
    p_list = (struct Place_descr *)ecalloc(place_num, sizeof(struct Place_descr));
    t_list = (struct Trans_descr *)ecalloc(trans_num, sizeof(struct Trans_descr));

    for (ii = 0, place_p = p_list; ++ii <= place_num; place_p++) {
        place_p->cod.num_byte = 0;
    }

    /* read codings */
    for (nn = 0 ; nn < sub_num ; nn++) {
        fscanf(nfp, "%d %d\n", &ii, &jj2);
#ifdef DEBUG
        fprintf(stderr, "  subnet #%d (%d bytes):\n",
                nn, jj2);
#endif
        if (nn == 0) {
            encoding = (struct Byte_descr *)ecalloc(jj2, sizeof(struct Byte_descr));
            no_cod = jj2;
        }
        for (byte_p = encoding, ii = 0 ;
                ++ii <= jj2 ; byte_p++) {
            fscanf(nfp, "%d %d %d %d %d\n", &jj, &kk, &ll, &np, &mm);
            if (nn == 0) {
                byte_p->max_conf = ll;
                byte_p->noitems = np;
            }
            fscanf(nfp, "%d %d %d %d %d\n", &jj, &n1, &n2, &n3, &n4);
            if (nn == 0)
                byte_p->first_item = jj;
            for (kk = 1 ; ++kk <= np ;) {
                fscanf(nfp, "%d", &ll);
                if (nn == 0) {
                    place_p = p_list + (jj - 1);
                    cods_p = &(place_p->cod);
                    cods_p->link = ll;
                    jj = ll;
                    cods_p->num_byte = ii;
                    cods_p->mask = n1;
                    cods_p->shift = n2;
                }
                fscanf(nfp, "%d %d %d %d\n", &n1, &n2, &n3, &n4);
            }
            if (nn == 0) {
                place_p = p_list + (jj - 1);
                cods_p = &(place_p->cod);
                cods_p->link = 0;
                cods_p->num_byte = ii;
                cods_p->mask = n1;
                cods_p->shift = n2;
            }
#ifdef DEBUG
            fprintf(stderr, "    coding #%d (%d configurations)\n",
                    ii, jj2);
#endif
        }
    }

    /* read places */
#ifdef DEBUG
    fprintf(stderr, "   places:\n");
#endif
    for (ii = 0, place_p = p_list; ++ii <= place_num; place_p++) {
        fscanf(nfp, "%d %d %d %d %d %d %d %s\n",
               &np, &jj, &kk, &ll, &nt, &nn, &mm, linebuf);
        place_p->lbound = nt;
        place_p->rubound = nn - nt;
        place_p->name = ecalloc(strlen(linebuf) + 1, 1);
        sprintf(place_p->name, linebuf);
#ifdef DEBUG
        fprintf(stderr, "        place %d mask=%d shift=%d in byte #%d\n",
                ii, place_p->cod.mask, place_p->cod.shift, place_p->cod.num_byte);
        fprintf(stderr, "            lbound=%d rubound=%d notok=%d link=%d\n",
                place_p->lbound, place_p->rubound, mm, place_p->cod.link);
#endif
    }

    /* read groups */
    for (nn = 1, ii = 0 ; ii++ < group_num; nn++) {
        fscanf(nfp, "%d %d\n", &jj, &kk);
        for (jj = 0 ; ++jj <= kk ;) {
            fscanf(nfp, " %d %d %d %d\n", &n1, &n2, &n3, &n4);
            for (ll = 0 ; ll++ < n2 ;) {
                fscanf(nfp, " %d %d %d\n", &nt, &mm, &ss);
            }
#ifdef DEBUG
            fprintf(stderr, "    ECS #%d, Byte #%d (%d configurations) nn=%d\n",
                    ii, jj, n2, nn);
#endif
        }
    }

    /* read transitions */
    for (trans_p = t_list, nt = 0 ; nt++ < trans_num ; trans_p++) {
        struct Trans_descr *tp;
        fscanf(nfp, "%d %d %d %d %d %d %d %s\n", &ii, &jj, &ns, &kk,
               &ll, &nb, &nc, linebuf);
        tp = t_list + (ii - 1);
        tp->name = ecalloc(strlen(linebuf) + 1, 1);
        sprintf(tp->name, linebuf);
        tp->noecs = jj;
        for (ii = 0 ; ++ii <= nb ;) {
            fscanf(nfp, "  %d %d %d\n", &jj, &nge, &nlt);
            nge += nlt;
            for (jj = 0 ; ++jj <= nge ;) {
                fscanf(nfp, "    %d %d %d %d\n", &mm, &kk, &ll, &nn);
            }
        }
        for (ii = 0 ; ++ii <= nc ;) {
            fscanf(nfp, " %d %d\n", &np, &jj);
        }
        fscanf(nfp, " %d", &na);
        for (ii = 0 ; ++ii <= na ;) {
            fscanf(nfp, "%d", &jj);
        }
        fscanf(nfp, "\n %d", &nc);
        for (ii = 0 ; ++ii <= nc ;) {
            fscanf(nfp, "%d", &jj);
        }
        while (getc(nfp) != '\n');
    }

    printf("\nActual Conflict Sets:\n");
    for (aecs_p = aecs + 1, ii = group_num ; ii-- ; aecs_p++) {
#ifdef DEBUG
        fprintf(stderr, "    ECS %d (%d AECS)\n", group_num - ii, (int)(aecs_p->number));
#endif
        for (jj = (aecs_p->number), uspp = (aecs_p->set + 1) ; jj-- ;
                uspp++) {
#ifdef DEBUG
            fprintf(stderr, "      AECS %d\n", (aecs_p->number) - jj);
#endif
            if ((kk = (*(usp = (*uspp)))) > 0) {
                printf("  {");
                while (kk--) {
                    ll = *(++usp);
                    printf(" %s", t_list[ll - 1].name);
                    if (kk)
                        printf(",");
                }
                printf(" }\n");
            }
        }
    }
    printf("\n\n");


#ifdef DEBUG
    fprintf(stderr, "  End of init\n");
#endif
}




void show_marking(int nm) {
    unsigned char *ucp;
    int nb, np, prev;
    struct Place_descr *place_p;

    while (nm > loadedtan) {
        ucp = CM[++loadedtan] = (unsigned char *)ecalloc(no_cod,
                                sizeof(unsigned char));
        for (nb = no_cod ; nb-- ; * (ucp++) = getc(tmfp));
    }
    decode_mark(CM[nm], DM);
    printf(" [ ");
    for (prev = 0, np = 0, place_p = p_list ; np++ < place_num ; place_p++) {
        if ((nb = DM[np])) {
            if (prev)
                printf(", ");
            prev = 1;
            if (nb == 1)
                printf(place_p->name);
            else
                printf("%d @ %s", nb, place_p->name);
        }
    }
    if (! prev)
        printf("... no tokens ...");
    printf(" ]");
}


void show_immpath() {
    unsigned long ul;
    int plength, nt, ecs, aecsn;
    unsigned short *usp;

    load_compact(&ul, rgfp); plength = ul;
    while (plength--) {
        load_compact(&ul, rgfp); nt = ul;
        if (show_TRG)
            printf("%s", t_list[nt - 1].name);
        load_compact(&ul, rgfp); aecsn = ul;
        if (show_TRG) {
            if (aecsn) {
                ecs = t_list[nt - 1].noecs;
                aecs_p = aecs + ecs;
                usp = (aecs_p->set)[aecsn];
                if ((aecsn = (*usp++)) > 1) {
                    printf("{");
                    while (aecsn--) {
                        nt = *(usp++);
                        printf(t_list[nt - 1].name);
                        if (aecsn)
                            printf(",");
                    }
                    printf("}");
                }
            }
            if (plength)
                printf(" + ");
        }
    }
}


static char   *can_t_open = "Can't open file %s for %c\n";
char  filename[100];

int main(int argc, char **argv, char **envp) {
    extern FILE *fopen();
    int no_trans;
    unsigned long ul;
    int nm, nt;

#ifdef DEBUG
    fprintf(stderr, "Start of grg_stndrd\n");
#endif
    /*    if ( 0 )
    	goto end_main;
        terminate = (void (*)())end_main; */
    if (argc < 2) {
        fprintf(stderr, "ERROR: no net name !\n");
        exit(1);
    }
    show_TRG = show_TRS = show_RTRG = 0;
    for (nm = 2 ; nm < argc ; nm++) {
        if (argv[nm][0] == '-')
            switch (argv[nm][1]) {
            case 's' : show_TRS = 1;
                break;
            case 't' : show_TRG = 1;
                break;
            case 'r' : show_RTRG = 1;
                break;
            default :
                break;
            }
    }
    sprintf(filename, "%s.rgr_aux", argv[1]);
    if ((rgfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    fscanf(rgfp, "toptan= %d\n", &toptan);
    printf("\n*** Tangible Reachability Graph of net %s ***\n\n",
           argv[1]);
    printf("%d Tangible Markings\n\n", toptan);
    fscanf(rgfp, "topvan= %d\n", &topvan);
    fscanf(rgfp, "maxmark= %d\n", &maxmark);
    topvan = maxmark - topvan;
    printf("%d Vanishing Markings\n\n", topvan);
    fscanf(rgfp, "aecs_conf[%d]=", &group_num);
    printf("%d ECS\n\n", group_num);
    aecs = (struct ECS_descr *)ecalloc(group_num + 1,
                                       sizeof(struct ECS_descr));
    for (aecs_p = aecs + 1, Naecs = 0, nm = group_num ; nm-- ; aecs_p++) {
        fscanf(rgfp, " %d", &nt); aecs_p->number = nt;
#ifdef DEBUG
        fprintf(stderr, "      ECS #%d  contains %d AECS\n", group_num - nm, nt);
#endif
        Naecs += nt;
        aecs_p->set = (unsigned short **)ecalloc(nt + 1, sizeof(unsigned short *));
    }
    (void) fclose(rgfp);
    sprintf(filename, "%s.grg", argv[1]);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.aecs", argv[1]);
    if ((aefp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    init();
    (void)fclose(nfp);
    sprintf(filename, "%s.crgr", argv[1]);
    if ((rgfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.ctrs", argv[1]);
    if ((tmfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }

    /* LOOK FOR INITIAL MARKING */
    load_compact(&ul, rgfp);
    if ((nm = ul)) {
        printf("Initial marking is #%d :\n  ", nm);
        show_marking(nm);
        printf("\n");
    }
    else {
        printf("The initial marking was Vanishing.");
        printf("  Initial Tangibles are:\n");
        for (load_compact(&ul, rgfp), nm = ul ; nm ;
                load_compact(&ul, rgfp), nm = ul) {
            show_immpath();
            printf("  -->  #%d :   ", nm);
            show_marking(nm);
            printf("\n");
        }
    }
#ifdef DEBUG
    fprintf(stderr, "End of Initial Marking\n");
#endif
    if (show_TRG)
        printf("\n\n*** Start of TRG ***\n");
    else if (show_TRS && ! show_RTRG)
        printf("\n\n*** Start of TRS ***\n");
    for (reach_M1 = 0, load_compact(&ul, rgfp), lastan = ul ; lastan ;
            load_compact(&ul, rgfp), lastan = ul) {
        load_compact(&ul, rgfp); no_trans = ul;
        if (show_TRG)
            printf("\nFrom #%d (%d timed trans)  ", lastan, no_trans);
        else if (show_TRS && ! show_RTRG)
            printf("\nMark #%d   :   ", lastan);
        if (show_TRG || (show_TRS && ! show_RTRG)) {
            show_marking(lastan);
            printf("\n");
        }
        while (no_trans--) {
            load_compact(&ul, rgfp); nt = ul;
            if (show_TRG)
                printf("  -Timed-> %s ", t_list[nt - 1].name);
            load_compact(&ul, rgfp); nt = ul;
            if (show_TRG && (nt > 1))
                printf("(enabl=%d)", nt);
            load_compact(&ul, rgfp);
            if ((nm = ul)) {
                (RRG[nm].num)++;
                if ((nm == 1) && !(MR[lastan])) {
                    MR[lastan] = 1;
                    reach_M1++;
                }
                if (show_TRG) {
                    printf(" -to_tang->  #%d", nm);
                    if (show_TRS) {
                        printf("   ");
                        show_marking(nm);
                    }
                    printf("\n");
                }
            }
            else {
                if (show_TRG)
                    printf(" start vanishing paths:\n");
                for (load_compact(&ul, rgfp), nm = ul ; nm ;
                        load_compact(&ul, rgfp), nm = ul) {
                    (RRG[nm].num)++;
                    if ((nm == 1) && !(MR[lastan])) {
                        MR[lastan] = 1;
                        reach_M1++;
                    }
                    if (show_TRG)
                        printf("        -VP-> ");
                    show_immpath();
                    if (show_TRG) {
                        printf("  -to_tang->  #%d", nm);
                        if (show_TRS) {
                            printf("   ");
                            show_marking(nm);
                        }
                        printf("\n");
                    }
                }
            }
        }
    }
    if (show_TRG)
        printf("\n*** End of TRG ***\n");
    else if (show_TRS && ! show_RTRG)
        printf("\n*** End of TRS ***\n");
    if (! reach_M1) {
        printf("\n\n!!!!!! The initial marking #1");
        printf(" is not reachable from any other !!!!!!\n\n");
    }
    if (reach_M1 || show_RTRG) {
        (void) fclose(rgfp);
        sprintf(filename, "%s.crgr", argv[1]);
        if ((rgfp = fopen(filename, "r")) == NULL) {
            fprintf(stderr, can_t_open, filename, 'r');
            exit(1);
        }
        for (rrg_p = RRG, lastan = toptan ; lastan-- ;) {
            nt = (++rrg_p)->num;
            rrg_p->from = (unsigned *)ecalloc(nt + 1, sizeof(unsigned));
            *(rrg_p->from) = 0;
        }
        load_compact(&ul, rgfp);
        show_TRG = 0;
        if (!(nm = ul)) {
            for (load_compact(&ul, rgfp), nm = ul ; nm ;
                    load_compact(&ul, rgfp), nm = ul) {
                show_immpath();
            }
        }
        for (new_reach = 0, load_compact(&ul, rgfp), lastan = ul ; lastan ;
                load_compact(&ul, rgfp), lastan = ul) {
            load_compact(&ul, rgfp); no_trans = ul;
            while (no_trans--) {
                load_compact(&ul, rgfp); nt = ul;
                load_compact(&ul, rgfp); nt = ul;
                load_compact(&ul, rgfp);
                if ((nm = ul)) {
                    nt = ++(*(RRG[nm].from));
                    RRG[nm].from[nt] = lastan;
                    if ((MR[nm]) && !(MR[lastan])) {
                        MR[lastan] = 1;
                        reach_M1++;
                        new_reach++;
                    }
                }
                else {
                    for (load_compact(&ul, rgfp), nm = ul ; nm ;
                            load_compact(&ul, rgfp), nm = ul) {
                        nt = ++(*(RRG[nm].from));
                        RRG[nm].from[nt] = lastan;
                        if ((MR[nm]) && !(MR[lastan])) {
                            MR[lastan] = 1;
                            reach_M1++;
                            new_reach++;
                        }
                        show_immpath();
                    }
                }
            }
        }
        if (reach_M1)
            while (reach_M1 < toptan && new_reach) {
                for (lastan = toptan, new_reach = 0 ;
                        lastan > 0 && reach_M1 < toptan ; lastan--) {
                    if (MR[lastan]) {
                        for (from_p = RRG[lastan].from, nt = *(from_p++) ;
                                nt-- ;) {
                            nm = *(from_p++);
                            if (!(MR[nm])) {
                                MR[nm] = 1;
                                reach_M1++;
                                new_reach++;
                            }
                        }
                    }
                }
            }
    }
    if (reach_M1 == toptan)
        printf("\n***   The RG is strongly connected   ***\n\n");
    else if (reach_M1) {
        printf("\n!!!!!!   The Initial Marking #1 is transient   !!!!!!\n");
        printf("     *** Only %d markings out of %d reach it.\n\n",
               reach_M1, toptan);
    }
    if (show_RTRG) {
        printf("\n\n***   Start of Reverse TRG   ***\n\n");
        for (lastan = 0, rrg_p = RRG ; lastan++ < toptan ;) {
            printf("\nTo #%d    :  ", lastan);
            show_marking(lastan);
            printf("\n");
            for (from_p = (++rrg_p)->from, nt = *(from_p++) ; nt-- ;) {
                nm = *(from_p++);
                printf("    <--  #%d", nm);
                if (show_TRS) {
                    printf("  :  ");
                    show_marking(nm);
                }
                printf("\n");
            }
        }
        printf("\n\n***   End of Reverse TRG   ***\n\n");
    }

//end_main:
    (void) fclose(aefp);
    (void) fclose(rgfp);
    (void) fclose(tmfp);
    return 0;
}

