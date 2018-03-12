/*
 *  Torino, September 17, 1987
 *  program: comp_mark.c
 *  purpose: prepare marking handling functions compilation module
 *           for RG construction of a GSPN
 *           using topology and structural net properties information.
 *  programmer: Giovanni Chiola
 *  notes:
 *    1) the net description must be in GreatSPN format
 */


/*
#define DEBUG
*/



#define MAX_arr 32
/*
#define MAX_arr 256
*/
#define MAX_mark 1048575



#include <stdio.h>

extern FILE *nfp, * ifp, * ccfp, * scfp, * ofp;


#include "const.h"

#include "comp.h"

extern struct place_obj *p_list;

extern struct C_mark *encoding, * cccp;
extern int no_byte;
extern int no_cod;

extern int	place_num, trans_num;

extern unsigned int nobits();


gen_mark() {
    struct C_mark *ccp;
    struct place_obj *pp;
    int ii, jj, slot;
    unsigned long maxnum;
    unsigned long nb_code[100];
    int pval_flag = FALSE;
    int repcod = 0, numrep, totrep;

#ifdef DEBUG
    printf("\nStart of gen_mark\n");
#endif
    fprintf(ofp, "#include <stdio.h>\n");
    fprintf(ofp, "#include \"../grg.h\"\n\n");
    fprintf(ofp, "unsigned trans_num = %d;\n", trans_num);
    fprintf(ofp, "unsigned place_num = %d;\n\n", place_num);
    fprintf(ofp, "/* DECODED MARKING */  unsigned char DP[%d];\n\n",
            place_num + 1);
    fprintf(ofp, "/* CODING BYTES */  unsigned no_cod = %d;\n\n", no_cod);
    maxnum = 1;
    for (ccp = cccp, ii = no_cod, repcod = 0 ; ii-- ;) {
        nb_code[ii] = 1;
        if ((int)ccp->max_val >= MAX_arr)
            maxnum *= (int)(ccp->max_val + 1);
        if (maxnum > 255)
            nb_code[ii] = (maxnum > 65535) ? 3 : 2;
        if ((int)ccp->max_val < MAX_arr)
            maxnum *= (int)(ccp->max_val + 1);
        if (maxnum > 65536)
            maxnum = 65536;
        if ((((ccp->cod_type) & 56) >> 3) <= ++repcod) {
            ++ccp;  repcod = 0;
        }
    }
    fprintf(ofp, "extern MARKP rootm;\n\n");
    fprintf(ofp, "static MARKP%d xxnm;\n\n", nb_code[0]);
    fprintf(ofp, "c_err()\n{\nfprintf(stderr,");
    fprintf(ofp, "\"calloc: coudn't get enough memory\\n\");\n");
    fprintf(ofp, "exit(50);\n}\n\n");
    maxnum = 1;
    fprintf(ofp, "\n/* ENCODED MARKING NODES */\n\n");
#ifdef DEBUG
    printf("  for ccp ii=%d; ENCODED MARKING NODES\n", no_byte);
#endif
    for (ccp = cccp, ii = no_cod, repcod = 0 ; ii-- ;) {
        int nextval_flag = FALSE;
#ifdef DEBUG
        printf("    node # %d (%d)\n", no_cod - ii, (ccp - encoding) + 1);
#endif
        fprintf(ofp, "struct node_l%d {\n", no_cod - ii);
        if (pval_flag)
            fprintf(ofp, "   unsigned char pval;\n");
        if (ii < (no_cod - 1))
            fprintf(ofp, "   MARKP%d prev;\n", nb_code[ii + 1]);
        if ((int)ccp->max_val < MAX_arr) {
            nextval_flag = TRUE;
            if (ii)
                fprintf(ofp, "   MARKP%d next[%d];\n",
                        nb_code[ii - 1], (int)ccp->max_val + 1);
            else
                fprintf(ofp, "   MARKP3 next[%d];\n",
                        (int)ccp->max_val + 1);
        }
        else {
            maxnum *= (int)(ccp->max_val + 1);
            fprintf(ofp, "   unsigned char val;\n");
            fprintf(ofp, "   unsigned char balance;\n");
            fprintf(ofp, "   MARKP%d right;\n", nb_code[ii]);
            fprintf(ofp, "   MARKP%d left;\n", nb_code[ii]);
            if (ii)
                fprintf(ofp, "   MARKP%d next;\n", nb_code[ii - 1]);
            else
                fprintf(ofp, "   MARKP3 next;\n");
        }
        fprintf(ofp, "};\ntypedef struct node_l%d N_L%d;\n",
                no_cod - ii, no_cod - ii);
        fprintf(ofp, "#define S_N%d sizeof(N_L%d)\n\n",
                no_cod - ii, no_cod - ii);
        if (maxnum > MAX_mark)
            maxnum = MAX_mark;
        slot = (maxnum > 256) ? 256 : maxnum;
        fprintf(ofp, "N_L%d * addrt_%d[%d];\n",
                no_cod - ii, no_cod - ii, (maxnum >> 8) + 1);
        fprintf(ofp, "unsigned laddr_%d = 0;\n", no_cod - ii);
        fprintf(ofp, "unsigned naddr_%d = 0;\n", no_cod - ii);
        fprintf(ofp, "unsigned iaddr_%d = 0;\n\n", no_cod - ii);
        if ((int)ccp->max_val < MAX_arr) {
            maxnum *= (int)(ccp->max_val + 1);
            if (maxnum > MAX_mark)
                maxnum = MAX_mark;
        }
        if ((int)(ccp->cod_type & 128)) {
            struct C_mark *cccc = ccp->link;
            for (; cccc != NULL ; cccc = cccc->link) {
#ifdef DEBUG
                printf("        subcoding %d\n", (cccc - encoding) + 1);
#endif
                if ((int)(cccc->cod_type & 63)) {
                    pp = cccc->p_l; jj = cccc->max_c;
#ifdef DEBUG
                    printf("          place %d\n", (pp - p_list) + 1);
#endif
                    fprintf(ofp, "unsigned short tb_%d_%d[%d] = {\n",
                            no_cod - ii, (cccc - encoding) + 1, jj + 1);
                    if (pp->enc == 0)
                        jj++;
                    else
                        fprintf(ofp, "0, ");
                    for (; jj-- ; pp = pp->next)
                        fprintf(ofp, "%d, ", (int)(pp - p_list) + 1);
                    fprintf(ofp, "\n};\n");
                }
                else
                    fprintf(ofp, "#define tab_%d_%d 0\n", no_cod - ii, (cccc - encoding) + 1);
            }
#ifdef DEBUG
            printf("          NULL\n");
#endif
            fprintf(ofp, "#define tab_%d 0\n\n", no_cod - ii);
        }
        else if ((int)(ccp->cod_type & 63)) {
#ifdef DEBUG
            printf("        no subcoding\n");
#endif
            pp = ccp->p_l; jj = ccp->max_c;
            fprintf(ofp, "unsigned short tab_%d[%d] = {\n",
                    no_cod - ii, jj + 1);
            if (pp->enc == 0)
                jj++;
            else
                fprintf(ofp, "0, ");
            for (; jj-- ; pp = pp->next)
                fprintf(ofp, "%d, ", (int)(pp - p_list) + 1);
            fprintf(ofp, "\n};\n\n");
        }
        else
            fprintf(ofp, "#define tab_%d 0\n\n", no_cod - ii);
#ifdef DEBUG
        printf("      %d ALLOC FUNCTION\n", no_cod - ii);
#endif
        /* ALLOC FUNCTION */
        fprintf(ofp, "new_m%d( Mp, pnm, nm )\n", no_cod - ii);
        fprintf(ofp, "unsigned char * Mp;\n");
        fprintf(ofp, "MARKP pnm, * nm;\n");
        switch (nb_code[ii]) {
        case 1 :
            fprintf(ofp, "{ unsigned char n0;\n");
            break;
        case 2 :
            fprintf(ofp, "{ unsigned char n0, n1;\n");
            break;
        case 3 :
            fprintf(ofp, "{ unsigned char n0, n1, n2;\n");
            break;
        default :
            break;
        }
        fprintf(ofp, "  unsigned ii = laddr_%d;\n", no_cod - ii);
        fprintf(ofp, "  N_L%d * np;\n", no_cod - ii);
        if (ii)
            fprintf(ofp, "  MARKP%d * nmpp;\n", nb_code[ii - 1]);
        else
            fprintf(ofp, "  MARKP3 * nmpp;\n");
        fprintf(ofp, "if ( ++laddr_%d > naddr_%d ) {\n", no_cod - ii, no_cod - ii);
        if (maxnum >= MAX_mark) {
            fprintf(ofp, "  if ( laddr_%d > %d ) {\n", no_cod - ii, MAX_mark);
            fprintf(ofp, "    exit(100);\n   }\n");
        }
        fprintf(ofp, "  naddr_%d += %d;\n", no_cod - ii, slot);
        fprintf(ofp, "  if ( (addrt_%d[iaddr_%d++] =",
                no_cod - ii, no_cod - ii);
        fprintf(ofp, " (N_L%d*)calloc(%d,S_N%d)) == 0 )\n",
                no_cod - ii, slot, no_cod - ii);
        fprintf(ofp, "    c_err();\n  }\n");
        switch (nb_code[ii]) {
        case 3 :
            fprintf(ofp, "  n2 = (unsigned char)(laddr_%d>>16); ",
                    no_cod - ii);
        case 2 :
            fprintf(ofp, "n1 = (unsigned char)((laddr_%d>>8) & 0xFF); ",
                    no_cod - ii);
        case 1 :
            fprintf(ofp, "n0 = (unsigned char)(laddr_%d & 0xFF);\n",
                    no_cod - ii);
            break;
        default :
            break;
        }
        if (nb_code[ii] > 1)
            fprintf(ofp, "np = *(addrt_%d+((ii)>>8)) + (0xFF & ii);\n",
                    no_cod - ii , no_cod - ii);
        else
            fprintf(ofp, "np = *addrt_%d + n0-1;\n", no_cod - ii);
        if (ii < (no_cod - 1))
            switch (nb_code[ii + 1]) {
            case 3 :
                fprintf(ofp, "np->prev.b2 = pnm.c3.b2; ");
                fprintf(ofp, "np->prev.b1 = pnm.c3.b1; ");
                fprintf(ofp, "np->prev.b0 = pnm.c3.b0;\n");
                break;
            case 2 :
                fprintf(ofp, "np->prev.b1 = pnm.c2.b1; ");
                fprintf(ofp, "np->prev.b0 = pnm.c2.b0;\n");
                break;
            case 1 :
                fprintf(ofp, "np->prev = pnm.c1;\n");
                break;
            default :
                break;
            }
        if (pval_flag)
            fprintf(ofp, "np->pval = *(Mp-1);\n");
        if ((int)ccp->max_val < MAX_arr) {
            fprintf(ofp, "for ( nmpp = np->next, ii = %d ; ii-- ; )\n  ",
                    ccp->max_val + 1);
            if (! ii || nb_code[ii - 1] == 3)
                fprintf(ofp, "{ nmpp->b2 = '\\0'; nmpp->b1 = '\\0'; (nmpp++)->b0 = '\\0'; }\n");
            else
                switch (nb_code[ii - 1]) {
                case 2 :
                    fprintf(ofp, "{ nmpp->b1 = '\\0'; (nmpp++)->b0 = '\\0'; }\n");
                    break;
                case 1 :
                    fprintf(ofp, "*(nmpp++) = '\\0';\n");
                    break;
                default :
                    break;
                }
        }
        else {
            fprintf(ofp, "np->val = *Mp;\n");
            if ((! ii) || (nb_code[ii - 1] == 3))
                fprintf(ofp, "np->next.b2 = '\\0'; np->next.b1 = '\\0'; np->next.b0 = '\\0';\n");
            else if (nb_code[ii - 1] == 2)
                fprintf(ofp, "np->next.b1 = '\\0'; np->next.b0 = '\\0';\n");
            else
                fprintf(ofp, "np->next = '\\0';\n");
            switch (nb_code[ii]) {
            case 3 :
                fprintf(ofp, "np->right.b2 = '\\0'; np->right.b1 = '\\0'; np->right.b0 = '\\0';\n");
                fprintf(ofp, "np->left.b2 = '\\0'; np->left.b1 = '\\0'; np->left.b0 = '\\0';\n");
                break;
            case 2 :
                fprintf(ofp, "np->right.b1 = '\\0'; np->right.b0 = '\\0';\n");
                fprintf(ofp, "np->left.b1 = '\\0'; np->left.b0 = '\\0';\n");
                break;
            case 1 :
                fprintf(ofp, "np->right = '\\0';\n");
                fprintf(ofp, "np->left = '\\0';\n");
                break;
            default :
                break;
            }
        }
        switch (nb_code[ii]) {
        case 3 :
            fprintf(ofp, "nm->c3.b0 = n0; nm->c3.b1 = n1; nm->c3.b2 = n2;\n");
            break;
        case 2 :
            fprintf(ofp, "nm->c2.b0 = n0; nm->c2.b1 = n1;\n");
            break;
        case 1 :
            fprintf(ofp, "nm->c1 = n0;\n");
            break;
        default :
            break;
        }
        fprintf(ofp, "}\n\n");
#ifdef DEBUG
        printf("      %d FIND FUNCTION\n", no_cod - ii);
#endif
        /* FIND FUNCTION */
        fprintf(ofp, "MARKP * mrk_f%d( pnm, Mp )\n", no_cod - ii);
        fprintf(ofp, "MARKP%d pnm;\n", nb_code[ii]);
        fprintf(ofp, "unsigned char * Mp;\n");
        fprintf(ofp, "{ unsigned nup, nlo;\n");
        fprintf(ofp, "  N_L%d * np;\n", no_cod - ii);
        if (ii)
            fprintf(ofp, "  MARKP%d * nmpp;\n  int ii;\n", nb_code[ii - 1]);
        else
            fprintf(ofp, "  MARKP3 * nmpp;\n  int ii;\n");
        switch (nb_code[ii]) {
        case 3 :
        case 2 :
            fprintf(ofp, "nup = (INT%d(pnm))-1; ", nb_code[ii]);
            fprintf(ofp, "nlo = nup & 0xFF; ");
            fprintf(ofp, "nup = (nup)>>8;\n");
            fprintf(ofp, "np = *(addrt_%d+nup) + nlo;\n", no_cod - ii);
            break;
        case 1 :
            fprintf(ofp, "nlo = pnm-1;\n");
            fprintf(ofp, "np = (*addrt_%d) + nlo;\n", no_cod - ii);
            break;
        default :
            break;
        }
        if (! nextval_flag) {
            fprintf(ofp, "*Mp = np->val;\n");
        }
        if (pval_flag)
            fprintf(ofp, "*(Mp-1) = np->pval;\n");
        if (ii < (no_cod - 1))
            fprintf(ofp, "return( (MARKP*)(&(np->prev)) );\n");
        else
            fprintf(ofp, "return( (MARKP*)(nmpp) );\n");
        fprintf(ofp, "}\n\n");
#ifdef DEBUG
        printf("      %d TEST/INSERT FUNCTION\n", no_cod - ii);
#endif
        /* TEST/INSERT FUNCTION */
        fprintf(ofp, "MARKP * tsti_f%d( nm, Mp )\n", no_cod - ii);
        fprintf(ofp, "MARKP%d nm;\n", nb_code[ii]);
        fprintf(ofp, "unsigned char * Mp; {\n");
        fprintf(ofp, "  unsigned nup, nlo;\n");
        fprintf(ofp, "  MARKP * retnmpp;\n");
        fprintf(ofp, "  N_L%d * np;\n", no_cod - ii);
        if ((int)ccp->max_val >= MAX_arr) {
            fprintf(ofp, "  MARKP%d * mpp;\n", nb_code[ii]);
            if ((no_cod - ii) == 1)
                fprintf(ofp, "  MARKP%d pnm;\n", nb_code[ii]);
            else
                fprintf(ofp, "  MARKP%d pnm;\n", nb_code[ii + 1]);
        }
        if (ii) {
            fprintf(ofp, "  MARKP%d * nmpp;\n", nb_code[ii - 1]);
            fprintf(ofp, "  extern new_m%d();\n", no_cod - ii + 1);
        }
        else {
            fprintf(ofp, "  MARKP3 * nmpp;\n");
            if ((int)ccp->max_val < MAX_arr)
                fprintf(ofp, "xxnm = nm;\n");
        }
        switch (nb_code[ii]) {
        case 3 :
        case 2 :
            fprintf(ofp, "nup = (INT%d(nm))-1; ", nb_code[ii]);
            fprintf(ofp, "nlo = nup & 0xFF; ");
            fprintf(ofp, "nup = (nup)>>8;\n");
            fprintf(ofp, "np = *(addrt_%d+nup) + nlo;\n", no_cod - ii);
            break;
        case 1 :
            fprintf(ofp, "nlo = nm-1;\n");
            fprintf(ofp, "np = (*addrt_%d) + nlo;\n", no_cod - ii);
            break;
        default :
            break;
        }
        if ((int)ccp->max_val < MAX_arr) {    /*  ARRAY  */
            unsigned c_t = ccp->cod_type & 63;
            if (c_t && (c_t & 7) > 1) {
                unsigned char *op = ccp->offset + 1;
                unsigned char *mp = ccp->mask;
                if (! repcod) {
                    numrep = 8 / nobits(ccp->max_c);
                    totrep = 0;
                }
                fprintf(ofp, "{int jj, kk, mm, vv = *Mp;\n");
                fprintf(ofp, "  kk = vv & %d;\n", (int)*mp);
                if (numrep > ((c_t & 7) - totrep))
                    numrep = ((c_t & 7) - totrep);
                totrep += numrep;
                if ((numrep - 1)) {
                    fprintf(ofp, "  for ( jj = 0, mm = 1 ; ++jj < %d ; ) {\n",
                            numrep);
                    fprintf(ofp, "    vv = (vv >> %d); ", (int)*op);
                    fprintf(ofp, " mm *= %d;\n", (int)ccp->max_c + 1);
                    fprintf(ofp, "    kk += ((vv & %d)*mm);\n  }\n", (int)*mp);
                }
                fprintf(ofp, "nmpp = np->next+kk;\n}\n");
            }
            else if (ccp->cod_type == 128) {
                struct C_mark *cccc = ccp->link;
                fprintf(ofp, "{int jj, kk, mm=1, vv = *Mp;\n");
                for (; cccc != NULL ; cccc = cccc->link) {
                    fprintf(ofp, "  jj = (vv & %d)>>%d;\n",
                            (int)cccc->mask[0], (int)cccc->offset[0]);
                    if (cccc == ccp->link)
                        fprintf(ofp, "  kk = jj;\n");
                    else
                        fprintf(ofp, "  kk += jj*mm;\n");
                    if (cccc->link != NULL)
                        fprintf(ofp, " mm *= %d;\n", (int)cccc->max_c + 1);
                }
                fprintf(ofp, "nmpp = np->next+kk;\n}\n");
            }
            else
                fprintf(ofp, "nmpp = np->next+*Mp;\n");
            if (ii) {
                fprintf(ofp, "if ( ZERO%d(*nmpp) ) {\n", nb_code[ii - 1]);
                fprintf(ofp, "  retnmpp = (MARKP*)(&nm);\n");
                fprintf(ofp, "  new_m%d( (Mp+1), *retnmpp, nmpp );\n}\n", no_cod - ii + 1);
            }
            fprintf(ofp, "retnmpp = (MARKP*)nmpp;\n");
            fprintf(ofp, "return( retnmpp );\n}\n\n");
        }
        else { /* BINARY TREE  */
            if ((no_cod - ii) == 1)
                fprintf(ofp, "pnm = rootm.c%d;\n", nb_code[ii]);
            else
                fprintf(ofp, "pnm = np->prev;\n", no_cod - ii);
            fprintf(ofp, "for ( ; ; nm = *mpp, ");
            switch (nb_code[ii]) {
            case 3 :
            case 2 :
                fprintf(ofp, "nup = (INT%d(nm))-1, ", nb_code[ii]);
                fprintf(ofp, "nlo = nup & 0xFF, ");
                fprintf(ofp, "nup = (nup)>>8, ");
                fprintf(ofp, "np = *(addrt_%d+nup) + nlo", no_cod - ii);
                break;
            case 1 :
                fprintf(ofp, "nlo = nm-1, ");
                fprintf(ofp, "np = (*addrt_%d) + nlo", no_cod - ii);
                break;
            default :
                break;
            }
            fprintf(ofp, ") {\n");
            fprintf(ofp, "  if ( np->val == *Mp ) {\n");
            fprintf(ofp, "    nmpp =  &(np->next);\n");
            if (ii) {
                fprintf(ofp, "    if ( ZERO%d(*nmpp) ) {\n", nb_code[ii - 1]);
                fprintf(ofp, "      retnmpp = (MARKP*)(&nm);\n");
                fprintf(ofp, "      new_m%d( (Mp+1), *retnmpp, nmpp );\n     }\n",
                        no_cod - ii + 1);
            }
            else
                fprintf(ofp, "    xxnm =  nm;\n");
            fprintf(ofp, "    retnmpp = (MARKP*)nmpp;\n");
            fprintf(ofp, "    return( retnmpp );\n   }\n");
            fprintf(ofp, "  mpp = (np->val<*Mp) ? &(np->right) : &(np->left);",
                    no_cod - ii);
            fprintf(ofp, "\n  if ( ZERO%d(*mpp) ) {\n", nb_code[ii]);
            fprintf(ofp, "    retnmpp = (MARKP*)(&pnm);\n");
            fprintf(ofp, "    new_m%d( Mp, *retnmpp, mpp );\n   }\n",
                    no_cod - ii);
            fprintf(ofp, " }\n}\n\n");

        }
        pval_flag = nextval_flag;
        if ((((ccp->cod_type) & 56) >> 3) <= ++repcod) {
            ++ccp;  repcod = 0;
        }
    }
#ifdef DEBUG
    printf("  end of for\n");
#endif
    fprintf(ofp, "\n/* LOAD FUNCTION */ loadm_f( Mp, mdrp )\n");
    fprintf(ofp, "unsigned char * Mp;\n");
    fprintf(ofp, "MARK_P * mdrp;\n");
    fprintf(ofp, "{ MARKP pnm;\n");
    ii = no_cod;
    if (pval_flag)
        fprintf(ofp, "*(Mp+%d) = mdrp->pval;\n", ii - 1);
    fprintf(ofp, "pnm = mdrp->link;\n");
    for (; ii-- ;) {
        if (ii)
            fprintf(ofp, "pnm = *mrk_f%d( pnm.c%d, (Mp+%d) );\n",
                    ii + 1, nb_code[no_cod - ii - 1], ii);
        else
            fprintf(ofp, "(void) mrk_f%d( pnm.c%d, Mp );\n",
                    ii + 1, nb_code[no_cod - ii - 1]);
    }
    fprintf(ofp, "}\n\n");
    fprintf(ofp, "\n/* TEST/INSERT FUNCTION */ MARKP3 * tstins_f( Mp, xnm, val )\n");
    fprintf(ofp, "unsigned char * Mp, *val;\n");
    fprintf(ofp, "MARKP * xnm;\n");
    fprintf(ofp, "{ MARKP pnm, * nnm;\n");
    ii = no_cod;
    fprintf(ofp, "pnm = rootm;\n");
    for (; ii-- ;) {
        fprintf(ofp, "nnm = (MARKP*)tsti_f%d( pnm.c%d, Mp );\n",
                no_cod - ii, nb_code[ii]);
        if (ii)
            fprintf(ofp, "pnm = *nnm; Mp++;\n");
    }
    fprintf(ofp, "*val = *Mp; xnm->c%d = xxnm;\n", nb_code[0]);
    fprintf(ofp, "return( (MARKP3*)nnm );\n}\n\n");

    fprintf(ofp, "unsigned short * tabs[%d] = {\n", no_cod);
    for (ii = no_cod ; ii-- ;)
        fprintf(ofp, " tab_%d, ", no_cod - ii);
    fprintf(ofp, "\n};\n\n");

    fprintf(ofp, "\n/* DECODING FUNCTION */ decode_mark(Mp)\n");
    fprintf(ofp, "unsigned char * Mp;\n");
    fprintf(ofp, "{ int ii; unsigned char *ip;\n");
    fprintf(ofp, "  unsigned short **tt = tabs;\n");
    fprintf(ofp, "for ( ip = DP, ii = %d ; ii--; *(++ip) = 0 );\n", place_num);
#ifdef DEBUG
    printf("  for ccp, ii; DECODING FUNCTION\n");
#endif
    for (ccp = cccp, ii = no_cod, repcod = 0 ; ii-- ;) {
        if ((int)ccp->cod_type == 128) {
            struct C_mark *cccc = ccp->link;
            for (; cccc != NULL ; cccc = cccc->link) {
                unsigned c_t = cccc->cod_type & 63;
                if (c_t) {
                    fprintf(ofp, "DP[*(tb_%d_%d+(((*Mp)&%d)>>%d))] ++;\n",
                            no_cod - ii, (cccc - encoding) + 1,
                            (int)(cccc->mask[0]),
                            (int)(cccc->offset[0]));
                }
                else {
                    fprintf(ofp, "DP[%d] = (*(Mp) & %d)>>%d;\n",
                            (int)(cccc->p_l - p_list) + 1,
                            (int)(cccc->mask[0]),
                            (int)(cccc->offset[0]));
                }
            }
            if (ii)
                fprintf(ofp, "Mp++;\n");
        }
        else if ((int)ccp->cod_type) {
            if (((ccp->cod_type) & 7) > 1) {
                int rep = ccp->cod_type;
                unsigned char *mp = ccp->mask, * op = ccp->offset + 1;
                int sum = no_cod - ii - 1;
                if (! repcod) {
                    numrep = 8 / nobits(ccp->max_c);
                    totrep = 0;
                }
                if (numrep > ((rep & 7) - totrep))
                    numrep = ((rep & 7) - totrep);
                totrep += numrep;
                rep = numrep;
                if (sum)
                    fprintf(ofp, "DP[*(*(tabs+%d) + (*Mp & %d))] ++; ",
                            sum, (int) * (mp++));
                else
                    fprintf(ofp, "DP[*(*tabs + (*Mp & %d))] ++; ",
                            (int) * (mp++));
                while (--rep) {
                    if (sum)
                        fprintf(ofp, "DP[*(*(tabs+%d)+((*Mp & %d)>>%d))] ++; ",
                                sum, (int) * (mp++), (int) * (op++));
                    else
                        fprintf(ofp, "DP[*(*tabs+((*Mp & %d)>>%d))] ++; ",
                                (int) * (mp++), (int) * (op++));
                }
                if (ii)
                    fprintf(ofp, "Mp++;\n");
            }
            else {
                int sum = no_cod - ii - 1;
                if (ii) {
                    if (sum)
                        fprintf(ofp, "DP[*(*(tabs+%d) + *(Mp++))] ++;\n", sum);
                    else
                        fprintf(ofp, "DP[*(*tabs + *(Mp++))] ++;\n");
                }
                else {
                    if (sum)
                        fprintf(ofp, "DP[*(*(tabs+%d) + *Mp)] ++;\n", sum);
                    else
                        fprintf(ofp, "DP[*(*tabs + *Mp)] ++;\n");
                }
            }
        }
        else {
            if (ii)
                fprintf(ofp, "DP[%d] = *(Mp++);\n", (int)(ccp->p_l - p_list) + 1);
            else
                fprintf(ofp, "DP[%d] = *Mp;\n", (int)(ccp->p_l - p_list) + 1);
        }
        if ((((ccp->cod_type) & 56) >> 3) <= ++repcod) {
            ++ccp;  repcod = 0;
        }
    }
#ifdef DEBUG
    printf("  end of for\n");
#endif
    fprintf(ofp, "}\n\n");

    fprintf(ofp, "\n/* ENCODING FUNCTION */ encode_mark(Mp)\n");
    fprintf(ofp, "unsigned char * Mp;\n");
    fprintf(ofp, "{ int ii; unsigned char *ip;\n");
    fprintf(ofp, "  unsigned cc, ccc;\n");
#ifdef DEBUG
    printf("  for ccp, ii; ENCODING FUNCTION\n");
#endif
    for (ccp = cccp, ii = no_cod, repcod = 0 ; ii-- ;) {
        if ((int)ccp->cod_type == 128) {
            struct C_mark *cccc = ccp->link;
            fprintf(ofp, "cc = 0;\n");
            for (; cccc != NULL ; cccc = cccc->link) {
                unsigned c_t = cccc->cod_type & 63;
                if (c_t) {
                    for (pp = cccc->p_l ; pp != NULL ; pp = pp->next) {
                        int nplace = (pp - p_list) + 1;
                        fprintf(ofp, "if ( DP[%d] ) {\n", nplace);
                        fprintf(ofp, "DP[%d]--;", nplace);
                        fprintf(ofp, "ccc=%d;\n} else ", (int)pp->enc);
                    }
                    fprintf(ofp, "ccc=0;\n");
                    fprintf(ofp, "cc |= ccc<<%d;\n", (cccc->offset[0]));
                }
                else {
                    fprintf(ofp, "cc |= (DP[%d])<<%d;\n",
                            ((cccc->p_l - p_list) + 1), (cccc->offset[0]));
                }
            }
            fprintf(ofp, "*(Mp++) =  cc;\n");
        }
        else if ((int)ccp->cod_type) {
            int num = ((ccp->cod_type) & 7);
            int cc = 0, ss = 0;
            fprintf(ofp, "cc = 0;\n");
            if (num > 1) {
                if (! repcod) {
                    numrep = 8 / nobits(ccp->max_c);
                    totrep = 0;
                }
                if (numrep > (num - totrep))
                    numrep = (num - totrep);
                totrep += numrep;
                num = numrep;
            }
            while (num--) {
                for (pp = ccp->p_l ; pp != NULL ; pp = pp->next) {
                    int nplace = (pp - p_list) + 1;
                    fprintf(ofp, "if ( DP[%d] ) {\n", nplace);
                    fprintf(ofp, "DP[%d]--;", nplace);
                    fprintf(ofp, "cc |= %d;\n}",
                            (int)(((int)(pp->enc)) << (ccp->offset[ss])));
                    if (pp->next != NULL)
                        fprintf(ofp, " else ");
                }
                fprintf(ofp, "\n");
                ss++;
            }
            fprintf(ofp, "*(Mp++) =  cc;\n");
        }
        else {
            fprintf(ofp, "*(Mp++) =  DP[%d];\n", ((ccp->p_l - p_list) + 1));
        }
        if ((((ccp->cod_type) & 56) >> 3) <= ++repcod) {
            ++ccp;  repcod = 0;
        }
    }
#ifdef DEBUG
    printf("  end of for\n");
#endif
    fprintf(ofp, "}\n\n");

    fprintf(ofp, "/* REGENERATION MARKING */ unsigned char MRIG[%d];\n\n", no_cod);
    fprintf(ofp, "/* INITIAL MARKING */ unsigned char MINIZ[%d];\n\n", no_cod);
    fprintf(ofp, "/* INITIAL MARKING */ unsigned char M0[%d] = {\n", no_cod);
#ifdef DEBUG
    printf("  for ccp ii INITIAL MARKING\n");
#endif
    for (ccp = cccp, ii = no_cod, repcod = 0 ; ii-- ;) {
        if ((int)ccp->cod_type == 128) {
            struct C_mark *cccc = ccp->link;
            int cc = 0;
            for (; cccc != NULL ; cccc = cccc->link) {
                unsigned c_t = cccc->cod_type & 63;
                if (c_t) {
                    int ccc = 0;
                    for (pp = cccc->p_l ; pp != NULL && !(pp->mark) ;
                            pp = pp->next);
                    if (pp != NULL) {
                        ccc = (int)(pp->enc);
                        (pp->mark)--;
                    }
                    cc |= ccc << (cccc->offset[0]);
                }
                else {
                    cc |= (cccc->p_l->mark) << (cccc->offset[0]);
                }
            }
            fprintf(ofp, " %d,", cc);
        }
        else if ((int)ccp->cod_type) {
            int num = ((ccp->cod_type) & 7);
            int cc = 0, ss = 0;
            if (num > 1) {
                if (! repcod) {
                    numrep = 8 / nobits(ccp->max_c);
                    totrep = 0;
                }
                if (numrep > (num - totrep))
                    numrep = (num - totrep);
                totrep += numrep;
                num = numrep;
            }
            while (num--) {
                for (pp = ccp->p_l ; pp != NULL && !(pp->mark) ;
                        pp = pp->next);
                if (pp != NULL) {
                    cc |= ((int)(pp->enc)) << (ccp->offset[ss]);
                    (pp->mark)--;
                }
                ss++;
            }
            fprintf(ofp, " %d,", cc);
        }
        else {
            fprintf(ofp, " %d,", (int)(ccp->p_l->mark));
        }
        if ((((ccp->cod_type) & 56) >> 3) <= ++repcod) {
            ++ccp;  repcod = 0;
        }
    }
#ifdef DEBUG
    printf("  end of for\n");
#endif
    fprintf(ofp, "\n};\n\n");
#ifdef DEBUG
    printf("\nEnd of gen_mark\n");
#endif
}

