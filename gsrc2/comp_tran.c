/*
 *  Torino, December 28, 1987
 *  program: comp_tran.c
 *  purpose: prepare compilation module for transitions enabling and firing.
 *  programmer: Giovanni Chiola
 *  notes:
 *    1) the net description must be in GreatSPN format
 */


/*
#define DEBUG
*/


#include <stdio.h>

extern FILE *nfp, * ifp, * ccfp, * scfp, * ofp;


#include "const.h"

#include "comp.h"


extern int norp;
extern float 	*rparp;

extern struct place_obj *p_list;

extern struct C_mark *encoding, * cccp;
extern int no_byte;
extern int no_cod;

extern int	trans_num;

extern unsigned int nobits();
extern void start_generate_rate();
extern void getname();
extern void generate_t_rate();
extern void terminate_t_rate_gen();

void getarcs(ROWP arr, int noar);


void encode_trans() {
    struct C_mark *ccp;
    struct place_obj *pp;
    int ii, jj, cur_pri = -1;
    int nt = trans_num;
    ROWT pri[MAX_TRANS];
    ROWT en_dep[MAX_TRANS];
    ROWT first_pri[MAX_TRANS];
    ROWT addlist[MAX_TRANS];
    ROWT in_a[MAX_TRANS], out_a[MAX_TRANS], inh_a[MAX_TRANS], * a_p;
    char		linebuf[LINEMAX];
    float		ftemp;
    int                 knd, noar, load_d;
    int	                single_server_flag;

#ifdef DEBUG
    printf("  Start of encode_trans\n");
#endif
    start_generate_rate();
    fprintf(ofp, "\nstruct abil {\n unsigned long nome;\n int limite;\n");
    fprintf(ofp, " unsigned short tipe;\n int levcop;\n int next;\n };\n");
    fprintf(ofp, "typedef struct abil ABILI;\n");
    fprintf(ofp, "struct vett {\n float temp;\n int scatti;\n");
    fprintf(ofp, " int ultimo;\n int ncopie;\n double resto[10];\n };\n");
    fprintf(ofp, "typedef struct vett VETT;\n");
    fprintf(ofp, "\nABILI abi[%d];\n", trans_num + 2);
    fprintf(ofp, "ABILI possabi[%d];\n", trans_num + 2);
    fprintf(ofp, "VETT bptr[%d];\n", trans_num + 1);
    fprintf(ofp, "int imlist[%d];\n", trans_num + 1);


    fprintf(ofp, "\n/* TRANSITION DESCRIPTION */ struct T_descr {\n");
    fprintf(ofp, "    int (*e_f)();\n");
    fprintf(ofp, "    int (*f_f)();\n");
    fprintf(ofp, "    unsigned short * add_l;\n");
    fprintf(ofp, "    unsigned short * test_l;\n");
    fprintf(ofp, "    unsigned short en_dep;\n");
    fprintf(ofp, "    unsigned short pri;\n};\n\n");

    /* read transitions */
    for (; nt-- ;) {
        getname(linebuf);
        fscanf(nfp, "%f %d %d %d ", &ftemp, &load_d, &knd, &noar);
        while (getc(nfp) != '\n');
        if (load_d < 0) {
            int ii;
            ii = load_d = -load_d;
            while (--ii) {
                while (getc(nfp) != '\n');
            }
        }
        en_dep[trans_num - nt - 1] = (load_d != 1);
        generate_t_rate(ftemp, load_d, trans_num - nt, knd);
        if (! load_d)
            load_d = 255;
        pri[trans_num - nt] = knd;
        if (knd > cur_pri && knd < DETERM) {
            first_pri[knd] = trans_num - nt;
            cur_pri = knd;
        }
        getarcs(in_a, noar);
        fscanf(nfp, "%d\n", &noar);
        getarcs(out_a, noar);
        fscanf(nfp, "%d\n", &noar);
        getarcs(inh_a, noar);
        if (load_d > 1) {
            single_server_flag = FALSE;
            fprintf(ofp, "\n /* EN. TEST FUNCTION *");
            fprintf(ofp, "/\ninf_t%d( oMp, nMp)\n",
                    trans_num - nt);
            fprintf(ofp, "unsigned char * oMp, *nMp;\n");
            fprintf(ofp, "{ unsigned char * nmp;\n");
            fprintf(ofp, "  int ii, vv;\n\n");
            for (ccp = cccp, ii = no_byte ; ii-- ; ccp++)
                ccp->flag = FALSE;
            for (a_p = in_a, ii = *(a_p++) ; ii-- ; a_p += 2) {
                pp = p_list + (*(a_p + 1) - 1);
                ccp = encoding + (pp->covered - 1);
                if ((ccp->cod_type & 7) > 1)
                    ccp->flag = TRUE;
            }
            for (ccp = cccp, ii = no_byte ; ii-- ; ccp++)
                if (ccp->flag) {
                    fprintf(ofp, "  unsigned char dm%d[%d];\n",
                            (int)(ccp - cccp), (int)ccp->max_c + 1);
                }
            for (ccp = cccp, ii = no_byte ; ii-- ; ccp++)
                if (ccp->flag) {
                    int sum = ccp->displ;
                    int rep = (ccp->cod_type) & 7;
                    int repcod = 0, numrep, totrep = 0;
                    fprintf(ofp, "for ( ii=%d, nmp=dm%d; ii-- ; *(nmp++)=0 );\n",
                            (int)ccp->max_c + 1, (int)(ccp - cccp));
                    numrep = 8 / nobits(ccp->max_c);
                    while (++repcod <= (((ccp->cod_type) & 56) >> 3)) {
                        unsigned char *mp = ccp->mask, * op = ccp->offset + 1;
                        if (numrep > (((ccp->cod_type) & 7) - totrep))
                            numrep = (((ccp->cod_type) & 7) - totrep);
                        totrep += numrep;
                        rep = numrep;
                        if (sum)
                            fprintf(ofp, "vv = *(oMp+%d);\n", sum);
                        else
                            fprintf(ofp, "vv = *oMp;\n");
                        fprintf(ofp, "dm%d[(vv & %d)] ++;\n",
                                (int)(ccp - cccp), (int) * (mp++));
                        while (--rep) {
                            fprintf(ofp, "dm%d[((vv & %d)>>%d)] ++;\n",
                                    (int)(ccp - cccp),
                                    (int) * (mp++), (int) * (op++));
                        }
                        ++sum;
                    }
                }
            fprintf(ofp, "for ( ii = %d, nmp = nMp; ii-- ;\n", no_cod);
            fprintf(ofp, "      *(nmp++) = *(oMp++) );\n");
            for (a_p = in_a, ii = *(a_p++) ; ii-- ; a_p += 2) {
                pp = p_list + (*(a_p + 1) - 1);
                ccp = encoding + (pp->covered - 1);
                if ((int)ccp->cod_type & 64) {
                    int sum = (encoding + (ccp->covered - 1))->displ;
                    int dec;
                    if ((int)ccp->cod_type & 63) {
                        single_server_flag = TRUE;
                        dec = (pp->enc) << (ccp->offset[0]);
                    }
                    else
                        dec = (*a_p) << (ccp->offset[0]);
                    if (dec) {
                        if (sum)
                            fprintf(ofp, "*(nMp+%d) -= %d;\n", sum, dec);
                        else
                            fprintf(ofp, "*nMp -= %d;\n", dec);
                    }
                }
                else if ((int)ccp->cod_type) {
                    if (((ccp->cod_type) & 7) > 1) {
                        int dec = *a_p;
                        if (dec) {
                            int ncod = ccp - cccp;
                            int indx = pp->enc;
                            fprintf(ofp, "dm%d[%d] -= %d;\n", ncod, indx, dec);
                        }
                    }
                    else {
                        int dec = pp->enc;
                        single_server_flag = TRUE;
                        if (dec) {
                            int sum = ccp->displ;
                            if (sum)
                                fprintf(ofp, "*(nMp+%d) -= %d;\n", sum, dec);
                            else
                                fprintf(ofp, "*nMp -= %d;\n", dec);
                        }
                    }
                }
                else {
                    int dec = *a_p;
                    if (dec) {
                        int sum = ccp->displ;
                        if (sum)
                            fprintf(ofp, "*(nMp+%d) -= %d;\n", sum, dec);
                        else
                            fprintf(ofp, "*nMp -= %d;\n", dec);
                    }
                }
            }
            for (ccp = cccp, ii = no_byte ; ii-- ; ccp++)
                if (ccp->flag) {
                    int sum = ccp->displ;
                    int rep;
                    int repcod = 0, numrep, totrep = 0;
                    numrep = 8 / nobits(ccp->max_c);
                    while (++repcod <= (((ccp->cod_type) & 56) >> 3)) {
                        unsigned char *mp = ccp->mask, * op = ccp->offset + 1;
                        if (numrep > (((ccp->cod_type) & 7) - totrep))
                            numrep = (((ccp->cod_type) & 7) - totrep);
                        totrep += numrep;
                        rep = numrep;
                        if ((int)ccp->from0)
                            fprintf(ofp, "for ( ii=%d, nmp=dm%d; --ii && !(*nmp) ; nmp++ );\n",
                                    (int)ccp->max_c + 1, (int)(ccp - cccp));
                        else
                            fprintf(ofp, "for ( ii=%d, nmp=dm%d+1; --ii && !(*nmp) ; nmp++ );\n",
                                    (int)ccp->max_c, (int)(ccp - cccp));
                        fprintf(ofp, "if ( *nmp )\n  (*nmp)--;\n");
                        fprintf(ofp, "else\n  ii = %d;\n", (int)ccp->max_c);
                        fprintf(ofp, "vv = (%d-ii);\n",
                                (int)ccp->max_c);
                        while (--rep) {
                            if ((int)ccp->from0)
                                fprintf(ofp, "for ( ii=%d, nmp=dm%d; --ii && !(*nmp) ; nmp++ );\n",
                                        (int)ccp->max_c + 1, (int)(ccp - cccp));
                            else
                                fprintf(ofp, "for ( ii=%d, nmp=dm%d+1; --ii && !(*nmp) ; nmp++ );\n",
                                        (int)ccp->max_c, (int)(ccp - cccp));
                            fprintf(ofp, "if ( *nmp )\n  (*nmp)--;\n");
                            fprintf(ofp, "else\n  ii = %d;\n", (int)ccp->max_c);
                            fprintf(ofp, "vv |= ((%d-ii)<<%d);\n",
                                    (int)ccp->max_c, (int) * (op++));
                        }
                        if (sum)
                            fprintf(ofp, "*(nMp+%d) = (unsigned char)vv;\n", sum);
                        else
                            fprintf(ofp, "*nMp = (unsigned char)vv;\n");
                        ++sum;
                    }
                }
            fprintf(ofp, "}\n\n");
        }
        fprintf(ofp, "\n/* ENABLING FUNCTION *");
        fprintf(ofp, "/\nint en_t%d(Mp)\n", trans_num - nt);
        fprintf(ofp, "unsigned char * Mp;\n");
        fprintf(ofp, "{ unsigned enabl = 1;\n");
        if (load_d > 1)
            fprintf(ofp, "  unsigned char nextm[%d];\n", no_cod);
        jj = inh_a[0];
        for (a_p = in_a, ii = *(a_p++) ; ii-- ; a_p += 2) {
            pp = p_list + (*(a_p + 1) - 1);
            ccp = encoding + (pp->covered - 1);
            fprintf(ofp, "if ( enabl )\n");
            if ((int)ccp->cod_type & 64) {
                int sum = (encoding + (ccp->covered - 1))->displ;
                int oo = ccp->offset[0];
                fprintf(ofp, "  enabl = ");
                if ((int)ccp->cod_type & 63) {
                    if (sum) {
                        fprintf(ofp, "((((unsigned)*(Mp+%d))&%d)==%d);\n", sum,
                                ccp->mask[0], (((int)pp->enc) << oo));
                    }
                    else {
                        fprintf(ofp, "((((unsigned)*Mp)&%d)==%d);\n",
                                ccp->mask[0], (((int)pp->enc) << oo));
                    }
                }
                else {
                    if (sum) {
                        fprintf(ofp, "((((unsigned)*(Mp+%d))&%d)>=%d);\n", sum,
                                ccp->mask[0], (((int)*a_p) << oo));
                    }
                    else {
                        fprintf(ofp, "((((unsigned)*Mp)&%d)>=%d);\n",
                                ccp->mask[0], (((int)*a_p) << oo));
                    }
                }
            }
            else if ((int)ccp->cod_type) {
                if (((ccp->cod_type) & 7) > 1) {
                    int rep;
                    int sum = ccp->displ;
                    int repcod = 0, numrep, totrep;
                    unsigned char *mp = ccp->mask, * op = ccp->offset + 1;
                    numrep = 8 / nobits(ccp->max_c);
                    if (numrep > ((ccp->cod_type) & 7))
                        numrep = ((ccp->cod_type) & 7);
                    totrep = numrep;
                    rep = numrep;
                    if (sum)
                        fprintf(ofp, "{unsigned vv = (unsigned)*(Mp+%d);\n", sum);
                    else
                        fprintf(ofp, "{unsigned vv = (unsigned)*Mp;\n");
                    if (rep > 1)
                        fprintf(ofp, "  unsigned jj = %d;\n", rep);
                    else
                        fprintf(ofp, "  unsigned jj;\n");
                    fprintf(ofp, "  unsigned ntok = ((vv & %d) == %d);\n",
                            (int)*mp, (int)pp->enc);
                    if (rep > 1) {
                        fprintf(ofp, "  while ( ntok < %d && --jj ) {\n",
                                (int)*a_p);
                        fprintf(ofp, "    vv = vv>>%d;\n", (int)*op);
                        fprintf(ofp, "    if ( (vv & %d) == %d )  ntok++;\n  }\n",
                                (int)*mp, (int)pp->enc);
                    }
                    while (++repcod < (((ccp->cod_type) & 56) >> 3)) {
                        ++sum;
                        if (numrep > (((ccp->cod_type) & 7) - totrep))
                            numrep = (((ccp->cod_type) & 7) - totrep);
                        totrep += numrep;
                        rep = numrep;
                        mp = ccp->mask;  op = ccp->offset + 1;
                        fprintf(ofp, " vv = (unsigned)*(Mp+%d);\n", sum);
                        fprintf(ofp, " jj = %d;\n", rep + 1);
                        fprintf(ofp, "  while ( ntok < %d && --jj ) {\n",
                                (int)*a_p);
                        fprintf(ofp, "    if ( (vv & %d) == %d )  ntok++;\n",
                                (int)*mp, (int)pp->enc);
                        fprintf(ofp, "    vv = vv>>%d;\n  }\n", (int)*op);
                    }
                    fprintf(ofp, "  enabl = (ntok>=%d);\n}\n", (int)*a_p);
                }
                else {
                    int sum = ccp->displ;
                    fprintf(ofp, "  enabl = ");
                    if (sum)
                        fprintf(ofp, "((unsigned)*(Mp+%d)==%d);\n", sum, (int)pp->enc);
                    else
                        fprintf(ofp, "((unsigned)*Mp==%d);\n", (int)pp->enc);
                }
            }
            else {
                int sum = ccp->displ;
                fprintf(ofp, "  enabl = ");
                if (sum)
                    fprintf(ofp, "((unsigned)*(Mp+%d)>=%d);\n", sum, (int)*a_p);
                else
                    fprintf(ofp, "((unsigned)*Mp>=%d);\n", (int)*a_p);
            }
        }
        for (a_p = inh_a + 1 ; jj-- ; a_p += 2) {
            pp = p_list + (*(a_p + 1) - 1);
            ccp = encoding + (pp->covered - 1);
            fprintf(ofp, "if ( enabl )\n");
            if ((int)ccp->cod_type & 64) {
                int sum = (encoding + (ccp->covered - 1))->displ;
                fprintf(ofp, "  enabl = ");
                if ((int)ccp->cod_type & 63) {
                    int oo = ccp->offset[0];
                    if (sum)
                        fprintf(ofp, "((((unsigned)*(Mp+%d))&%d)!=%d);\n", sum,
                                ccp->mask[0], (((int)pp->enc) << oo));
                    else
                        fprintf(ofp, "((((unsigned)*Mp)&%d)!=%d);\n",
                                ccp->mask[0], (((int)pp->enc) << oo));
                }
                else {
                    int oo = ccp->offset[0];
                    if (sum)
                        fprintf(ofp, "((((unsigned)*(Mp+%d))&%d)<%d);\n", sum,
                                ccp->mask[0], (((int)*a_p) << oo));
                    else
                        fprintf(ofp, "((((unsigned)*Mp)&%d)<%d);\n",
                                ccp->mask[0], (((int)*a_p) << oo));
                }
            }
            else if ((int)ccp->cod_type) {
                if (((ccp->cod_type) & 7) > 1) {
                    int rep;
                    int sum = ccp->displ;
                    int repcod = 0, numrep, totrep;
                    unsigned char *mp = ccp->mask, * op = ccp->offset + 1;
                    numrep = 8 / nobits(ccp->max_c);
                    if (numrep > ((ccp->cod_type) & 7))
                        numrep = ((ccp->cod_type) & 7);
                    totrep = numrep;
                    rep = numrep;
                    if (sum)
                        fprintf(ofp, "{unsigned vv = (unsigned)*(Mp+%d);\n", sum);
                    else
                        fprintf(ofp, "{unsigned vv = (unsigned)*Mp;\n");
                    if (rep > 1)
                        fprintf(ofp, "  unsigned jj = %d;\n", rep);
                    else
                        fprintf(ofp, "  unsigned jj;\n");
                    fprintf(ofp, "  unsigned ntok = ((vv & %d) == %d);\n",
                            (int)*mp, (int)pp->enc);
                    if (rep > 1) {
                        fprintf(ofp, "  while ( ntok < %d && --jj ) {\n",
                                (int)*a_p);
                        fprintf(ofp, "    vv = vv>>%d;\n", (int)*op);
                        fprintf(ofp, "    if ( (vv & %d) == %d )  ntok++;\n  }\n",
                                (int)*mp, (int)pp->enc);
                    }
                    while (++repcod < (((ccp->cod_type) & 56) >> 3)) {
                        ++sum;
                        if (numrep > (((ccp->cod_type) & 7) - totrep))
                            numrep = (((ccp->cod_type) & 7) - totrep);
                        totrep += numrep;
                        rep = numrep;
                        mp = ccp->mask;  op = ccp->offset + 1;
                        fprintf(ofp, " vv = (unsigned)*(Mp+%d);\n", sum);
                        fprintf(ofp, " jj = %d;\n", rep + 1);
                        fprintf(ofp, "  while ( ntok < %d && --jj ) {\n",
                                (int)*a_p);
                        fprintf(ofp, "    if ( (vv & %d) == %d )  ntok++;\n",
                                (int)*mp, (int)pp->enc);
                        fprintf(ofp, "    vv = vv>>%d;\n  }\n", (int)*op);
                    }
                    fprintf(ofp, "  enabl = (ntok<%d);\n}\n", (int)*a_p);
                }
                else {
                    int sum = ccp->displ;
                    fprintf(ofp, "  enabl = ");
                    if (sum)
                        fprintf(ofp, "((unsigned)*(Mp+%d)!=%d);\n", sum, (int)pp->enc);
                    else
                        fprintf(ofp, "((unsigned)*Mp!=%d);\n", (int)pp->enc);
                }
            }
            else {
                int sum = ccp->displ;
                fprintf(ofp, "  enabl = ");
                if (sum)
                    fprintf(ofp, "((unsigned)*(Mp+%d)<%d);\n", sum, (int)*a_p);
                else
                    fprintf(ofp, "((unsigned)*Mp<%d);\n", (int)*a_p);
            }
        }
        if ((load_d > 1) && !single_server_flag) {
            fprintf(ofp, "if ( enabl ) {\n");
            fprintf(ofp, "  inf_t%d( Mp, nextm);\n", trans_num - nt);
            fprintf(ofp, "  return( 1+ en_t%d(nextm) );\n}\n", trans_num - nt);
            fprintf(ofp, "else\n  return( 0 );\n}\n\n");
        }
        else
            fprintf(ofp, "return( enabl );\n}\n\n");
        fprintf(ofp, "\n/* FIRING FUNCTION *");
        fprintf(ofp, "/\nfire_t%d( oMp, nMp)\n", trans_num - nt);
        fprintf(ofp, "unsigned char * oMp, *nMp;\n");
        fprintf(ofp, "{ unsigned char * nmp;\n");
        fprintf(ofp, "  unsigned ii, vv;\n\n");
        for (ccp = cccp, ii = no_byte ; ii-- ; ccp++)
            ccp->flag = FALSE;
        for (a_p = in_a, ii = *(a_p++) ; ii-- ; a_p += 2) {
            pp = p_list + (*(a_p + 1) - 1);
            ccp = encoding + (pp->covered - 1);
            if ((ccp->cod_type & 7) > 1)
                ccp->flag = TRUE;
        }
        for (a_p = out_a, ii = *(a_p++) ; ii-- ; a_p += 2) {
            pp = p_list + (*(a_p + 1) - 1);
            ccp = encoding + (pp->covered - 1);
            if ((ccp->cod_type & 7) > 1)
                ccp->flag = TRUE;
        }
        for (ccp = cccp, ii = no_byte ; ii-- ; ccp++)
            if (ccp->flag) {
                fprintf(ofp, "  unsigned char dm%d[%d];\n",
                        (int)(ccp - cccp), (int)ccp->max_c + 1);
            }
        for (ccp = cccp, ii = no_byte ; ii-- ; ccp++)
            if (ccp->flag) {
                int sum = ccp->displ;
                int rep = (ccp->cod_type) & 7;
                int repcod = 0, numrep, totrep = 0;
                fprintf(ofp, "for ( ii=%d, nmp=dm%d; ii-- ; *(nmp++)=0 );\n",
                        (int)ccp->max_c + 1, (int)(ccp - cccp));
                numrep = 8 / nobits(ccp->max_c);
                while (++repcod <= (((ccp->cod_type) & 56) >> 3)) {
                    unsigned char *mp = ccp->mask, * op = ccp->offset + 1;
                    if (numrep > (((ccp->cod_type) & 7) - totrep))
                        numrep = (((ccp->cod_type) & 7) - totrep);
                    totrep += numrep;
                    rep = numrep;
                    if (sum)
                        fprintf(ofp, "vv = *(oMp+%d);\n", sum);
                    else
                        fprintf(ofp, "vv = *oMp;\n");
                    fprintf(ofp, "dm%d[(vv & %d)] ++;\n",
                            (int)(ccp - cccp), (int) * (mp++));
                    while (--rep) {
                        fprintf(ofp, "dm%d[((vv & %d)>>%d)] ++;\n",
                                (int)(ccp - cccp),
                                (int) * (mp++), (int) * (op++));
                    }
                    ++sum;
                }
            }
        fprintf(ofp, "for ( ii = %d, nmp = nMp; ii-- ;\n", no_cod);
        fprintf(ofp, "      *(nmp++) = *(oMp++) );\n");
        for (a_p = in_a, ii = *(a_p++) ; ii-- ; a_p += 2) {
            pp = p_list + (*(a_p + 1) - 1);
            ccp = encoding + (pp->covered - 1);
            if ((int)ccp->cod_type & 64) {
                int sum = (encoding + (ccp->covered - 1))->displ;
                int dec;
                if ((int)ccp->cod_type & 63)
                    dec = (pp->enc) << (ccp->offset[0]);
                else
                    dec = (*a_p) << (ccp->offset[0]);
                if (dec) {
                    if (sum)
                        fprintf(ofp, "*(nMp+%d) -= (unsigned char)%d;\n", sum, dec);
                    else
                        fprintf(ofp, "*nMp -= (unsigned char)%d;\n", dec);
                }
            }
            else if ((int)ccp->cod_type) {
                if (((ccp->cod_type) & 7) > 1) {
                    int dec = *a_p;
                    if (dec) {
                        int ncod = ccp - cccp;
                        int indx = pp->enc;
                        fprintf(ofp, "dm%d[%d] -= (unsigned char)%d;\n",
                                ncod, indx, dec);
                    }
                }
                else {
                    int dec = pp->enc;
                    if (dec) {
                        int sum = ccp->displ;
                        if (sum)
                            fprintf(ofp, "*(nMp+%d) -= (unsigned char)%d;\n", sum, dec);
                        else
                            fprintf(ofp, "*nMp -= (unsigned char)%d;\n", dec);
                    }
                }
            }
            else {
                int dec = *a_p;
                if (dec) {
                    int sum = ccp->displ;
                    if (sum)
                        fprintf(ofp, "*(nMp+%d) -= (unsigned char)%d;\n", sum, dec);
                    else
                        fprintf(ofp, "*nMp -= (unsigned char)%d;\n", dec);
                }
            }
        }
        for (a_p = out_a, ii = *(a_p++) ; ii-- ; a_p += 2) {
            pp = p_list + (*(a_p + 1) - 1);
            ccp = encoding + (pp->covered - 1);
            if ((int)ccp->cod_type & 64) {
                int sum = (encoding + (ccp->covered - 1))->displ;
                int inc;
                if ((int)ccp->cod_type & 63)
                    inc = (pp->enc) << (ccp->offset[0]);
                else
                    inc = (*a_p) << (ccp->offset[0]);
                if (inc) {
                    if (sum)
                        fprintf(ofp, "*(nMp+%d) += (unsigned char)%d;\n", sum, inc);
                    else
                        fprintf(ofp, "*nMp += (unsigned char)%d;\n", inc);
                }
            }
            else if ((int)ccp->cod_type) {
                if (((ccp->cod_type) & 7) > 1) {
                    int inc = *a_p;
                    if (inc) {
                        int ncod = ccp - cccp;
                        int indx = pp->enc;
                        fprintf(ofp, "dm%d[%d] += (unsigned char)%d;\n",
                                ncod, indx, inc);
                    }
                }
                else {
                    int inc = pp->enc;
                    if (inc) {
                        int sum = ccp->displ;
                        if (sum)
                            fprintf(ofp, "*(nMp+%d) += (unsigned char)%d;\n", sum, inc);
                        else
                            fprintf(ofp, "*nMp += (unsigned char)%d;\n", inc);
                    }
                }
            }
            else {
                int inc = *a_p;
                if (inc) {
                    int sum = ccp->displ;
                    if (sum)
                        fprintf(ofp, "*(nMp+%d) += (unsigned char)%d;\n", sum, inc);
                    else
                        fprintf(ofp, "*nMp += (unsigned char)%d;\n", inc);
                }
            }
        }
        for (ccp = cccp, ii = no_byte ; ii-- ; ccp++)
            if (ccp->flag) {
                int sum = ccp->displ;
                int rep;
                int repcod = 0, numrep, totrep = 0;
                numrep = 8 / nobits(ccp->max_c);
                while (++repcod <= (((ccp->cod_type) & 56) >> 3)) {
                    unsigned char *mp = ccp->mask, * op = ccp->offset + 1;
                    if (numrep > (((ccp->cod_type) & 7) - totrep))
                        numrep = (((ccp->cod_type) & 7) - totrep);
                    totrep += numrep;
                    rep = numrep;
                    if ((int)ccp->from0)
                        fprintf(ofp, "for ( ii=%d, nmp=dm%d; --ii && !(*nmp) ; nmp++ );\n",
                                (int)ccp->max_c + 1, (int)(ccp - cccp));
                    else
                        fprintf(ofp, "for ( ii=%d, nmp=dm%d+1; --ii && !(*nmp) ; nmp++ );\n",
                                (int)ccp->max_c, (int)(ccp - cccp));
                    fprintf(ofp, "if ( *nmp )\n  (*nmp)--;\n");
                    fprintf(ofp, "else\n  ii = %d;\n", (int)ccp->max_c);
                    fprintf(ofp, "vv = (%d-ii);\n",
                            (int)ccp->max_c);
                    while (--rep) {
                        if ((int)ccp->from0)
                            fprintf(ofp, "for ( ii=%d, nmp=dm%d; --ii && !(*nmp) ; nmp++ );\n",
                                    (int)ccp->max_c + 1, (int)(ccp - cccp));
                        else
                            fprintf(ofp, "for ( ii=%d, nmp=dm%d+1; --ii && !(*nmp) ; nmp++ );\n",
                                    (int)ccp->max_c, (int)(ccp - cccp));
                        fprintf(ofp, "if ( *nmp )\n  (*nmp)--;\n");
                        fprintf(ofp, "else\n  ii = %d;\n", (int)ccp->max_c);
                        fprintf(ofp, "vv |= ((%d-ii)<<%d);\n",
                                (int)ccp->max_c, (int) * (op++));
                    }
                    if (sum)
                        fprintf(ofp, "*(nMp+%d) = (unsigned char)vv;\n", sum);
                    else
                        fprintf(ofp, "*nMp = (unsigned char)vv;\n");
                    ++sum;
                }
            }
        fprintf(ofp, "}\n\n");
        /* ADD LIST */ fscanf(ccfp, "%d", &noar);
        fprintf(ofp, "unsigned short al%d[] = {\n", trans_num - nt);
        for (ii = noar ; ii-- ;) {
            int nnn;
            fscanf(ccfp, "%d", &nnn);
            addlist[ii] = nnn;
            fprintf(ofp, " %d,", nnn);
        }
        fscanf(ccfp, "\n");
        fprintf(ofp, " 0\n};\n");
        /* TEST LIST */ fscanf(scfp, "%d", &knd);
        fprintf(ofp, "unsigned short tl%d[] = {\n", trans_num - nt);
        for (jj = noar - 1, ii = knd ; ii-- ;) {
            int nnn;
            fscanf(scfp, "%d", &nnn);
            while (jj >= 0 && addlist[jj] < nnn) {
                fprintf(ofp, " %d,", addlist[jj--]);
            }
            if (jj >= 0 && addlist[jj] == nnn)
                jj--;
            fprintf(ofp, " %d,", nnn);
        }
        while (jj >= 0) {
            fprintf(ofp, " %d,", addlist[jj--]);
        }
        fscanf(scfp, "\n");
        fprintf(ofp, " 0\n};\n");
    }


    terminate_t_rate_gen();
    fprintf(ofp, "\n/* TRANSITION ARRAY */ struct T_descr trans[%d] = {\n",
            trans_num + 1);
    fprintf(ofp, "0, 0, 0, 0, 0, 0,\n");
    for (nt = trans_num ; nt-- ;) {
        fprintf(ofp, "en_t%d, ", trans_num - nt);
        fprintf(ofp, "fire_t%d, ", trans_num - nt);
        fprintf(ofp, "al%d, ", trans_num - nt);
        fprintf(ofp, "tl%d, ", trans_num - nt);
        fprintf(ofp, "%d, ", (int)en_dep[trans_num - nt - 1]);
        fprintf(ofp, "%d,\n", (int)pri[trans_num - nt]);
    }
    fprintf(ofp, "};\n\n");
    fprintf(ofp, "unsigned short first_trans[%d] = {\n", cur_pri + 1);
    for (a_p = first_pri, nt = cur_pri + 1 ; nt-- ;)
        fprintf(ofp, " %d,", (int)(*(a_p++)));
    fprintf(ofp, "\n};\n\n");
    fprintf(ofp, "unsigned char elp[%d];\n", trans_num + 1);
#ifdef DEBUG
    printf("  End of encode_trans\n");
#endif
}

void
getarcs(arr, noar)
ROWP	arr;
int	noar;
{
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

