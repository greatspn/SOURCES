/*
 *  Torino, April 20, 1993
 *  program: measure_pn.c
 *  purpose: Measurement of Performance Indices for a GSPN simulation.
 *  programmer: Giovanni Chiola
 *  notes:
 *    1) NOT YET COMPLETELY DEBUGGED
 */
#include <stdlib.h>

#include <stdlib.h>
#include <string.h>
#include "../WN/INCLUDE/compact.h"
#include "measure_pn.h"
#include "measure_checkpoint.h"


/*
#define DEBUG
*/

/*
#define DEBUGpipe
*/


#ifdef DEBUGpipe
#	define LOAD_COMPACT(a,f) load_compact(a,f); \
			fprintf(stderr,"measure: load_compact(%ld)\n",*(a))
#	define LOAD_DOUBLE(a,f) load_double(a,f); \
			fprintf(stderr,"measure: load_double(%lg)\n",*(a))
#else
#	define LOAD_COMPACT(a,f) load_compact(a,f)
#	define LOAD_DOUBLE(a,f) load_double(a,f)
#endif /* DEBUGpipe */


#include "measure_decl.h"

FILE *nfp, * mpdfp, * tpdfp;

unsigned place_num, group_num, trans_num;
unsigned sub_num;
/* CODING BYTES */ unsigned no_cod;
struct Byte_descr *encoding;
/* DECODED MARKING */ unsigned short *DM;
/*  PLACE ARRAY  */ Place_p p_list;
unsigned char *cmarks, * c_m_p;
struct p_bound_s *p_bound, * p_b_p;
double **t_prob, * *t_p_p;
unsigned maxprob = 0;
unsigned maxres = 0;
struct Prob_descr *Prob;
struct Res_descr *Res;
/* CONFIDENCE LEVEL */ int confidence;


char *ecalloc(unsigned nitm, unsigned sz) {
    char 		*callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(2);
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
        else
            *(++dmp) = 0;
    }
}


static char   *can_t_open = "Can't open file %s for %c\n";
char  netname[LINEMAX];
char  filename[LINEMAX];

int terminate_flag;

Trans_p t_list;

void init() {
    char cc;
    unsigned char *uc_p;
    int ii, nn, nt, jj, jj2, kk, ll, mm, np, n1, n2, n3, n4;
    struct Byte_descr *byte_p;
    Place_p place_p;
    Trans_p trans_p;
    struct Codbyt_descr *cods_p;
    char linebuf[LINEMAX];

#ifdef DEBUG
    fprintf(stderr, "  Start of init\n");
#endif

    sprintf(filename, "%s.grg", netname);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }

    /* read number of objects in the net */
    fscanf(nfp, "%d %d %d %d\n", &sub_num, &place_num, &group_num, &trans_num);

    DM = (unsigned short *)ecalloc(place_num + 1, sizeof(unsigned short));
    p_list = (struct Place_descr *)ecalloc(place_num, sizeof(struct Place_descr));
    t_list = (struct Trans_descr *)ecalloc(trans_num, sizeof(struct Trans_descr));


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
            cmarks = (unsigned char *)ecalloc(no_cod, sizeof(unsigned char));
        }
        for (byte_p = encoding, uc_p = cmarks, ii = 0 ;
                ++ii <= jj2 ; byte_p++) {
            fscanf(nfp, "%d %d %d %d %d\n", &jj, &kk, &ll, &np, &mm);
            if (nn == 0) {
                byte_p->max_conf = ll;
                byte_p->noitems = np;
                *(uc_p++) = mm;
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
        fscanf(nfp, "%d %d %d %d %d %d %d",
               &np, &jj, &kk, &ll, &nt, &nn, &mm);
        place_p->lbound = nt;
        place_p->alb = nn + nt;
        place_p->aub = nt;
        place_p->rubound = nn;
        place_p->tok_distr = (double *)ecalloc(nn + 1, sizeof(double));
        place_p->cum_distr = (double *)ecalloc(nn + 1, sizeof(double));
        while (getc(nfp) != '\n');
#ifdef DEBUG
        fprintf(stderr, "        place %d mask=%d shift=%d in byte #%d\n",
                ii, place_p->cod.mask, place_p->cod.shift, place_p->cod.num_byte);
        fprintf(stderr, "            lbound=%d rubound=%d notok=%d link=%d\n",
                place_p->lbound, place_p->rubound, mm, place_p->cod.link);
#endif
    }
    /* skip groups */
    for (ii = 0 ; ii++ < group_num;) {
        fscanf(nfp, "%*d %d\n", &kk);
        for (jj = 0 ; ++jj <= kk ;) {
            fscanf(nfp, " %*d %d", &n2);
            while (getc(nfp) != '\n');
            for (ll = 0 ; ll++ < n2 ;) {
                while (getc(nfp) != '\n');
            }
        }
    }
    /* read trans names and pri */
    for (ii = 0 ; ii++ < trans_num;) {
        char buf[256];
        fscanf(nfp, "%d %d %*d %*d %*d %d %d %s\n", &nt, &jj, &n1, &n2, buf);
        trans_p = t_list + --nt;
        trans_p->pri = jj;
        trans_p->name = ecalloc(strlen(buf) + 1, 1);
        sprintf(trans_p->name, buf);
        while (n1--) {
            fscanf(nfp, " %*d %d %d\n", &n3, &n4);
            for (n3 += n4 ; n3-- ;)
                while (getc(nfp) != '\n');
        }
        for (n2 += 2 ; n2-- ;)
            while (getc(nfp) != '\n');
    }
    (void) fclose(nfp);

    sprintf(filename, "%s.gst", netname);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    fscanf(nfp, "%d %d\n", &maxprob, &maxres);
#ifdef DEBUG
    fprintf(stderr, "    place_num=%d, no_cod=%d, maxprob=%d\n",
            place_num, no_cod, maxprob);
#endif
    Prob = (struct Prob_descr *)ecalloc(maxprob + 1,
                                        sizeof(struct Prob_descr));
    Res = (struct Res_descr *)ecalloc(maxres + 1,
                                      sizeof(struct Res_descr));
    for (ii = maxprob ; ii-- ;) {
        fscanf(nfp, "%d %d", &jj, &ll);
        while ((cc = getc(nfp)) == ' ');
        Prob[jj].val = 0.0;
        Prob[jj].type = cc;
        switch (cc) {
        case 'e' :
        case 'E' :
        case 'c' :
        case 'C' :
            fscanf(nfp, "%d\n", &kk);
            break;
        case 'p' :
        case 'P' :
            while (getc(nfp) != '\n');
            kk = 0;
            break;
        }
        Prob[jj].place = kk;
#ifdef DEBUG
        fprintf(stderr, "   prob #%d type=%c place=%d nocond=%d\n",
                jj, cc, kk, ll);
#endif
        if (ll) {
            struct Cond_descr *ccp;
            Prob[jj].cond = ccp = (struct Cond_descr *)
                                  ecalloc(ll, sizeof(struct Cond_descr));
            while (ll--) {
                while ((cc = getc(nfp)) == ' ');
                if ((ccp->op = cc) == 'c') {
                    while ((cc = getc(nfp)) == ' ');
                    ccp->type = cc;
                    while ((cc = getc(nfp)) == ' ');
                    ccp->top1 = cc;
                    fscanf(nfp, "%d", &kk);
                    ccp->op1 = kk;
                    while ((cc = getc(nfp)) == ' ');
                    ccp->top2 = cc;
                    fscanf(nfp, "%d\n", &kk);
                    ccp->op2 = kk;
#ifdef DEBUG
                    fprintf(stderr, "     cond (%c %d) %c (%c %d)\n",
                            ccp->top1, ccp->op1, ccp->type, ccp->top2, ccp->op2);
#endif
                }
                else {
#ifdef DEBUG
                    fprintf(stderr, "     op %c\n", cc);
#endif
                    while (getc(nfp) != '\n');
                }
                ++ccp;
            }
        }
        else
            Prob[jj].cond = NULL;
    }

    for (ii = maxres ; ii-- ;) {
        struct Op_descr *oop;
        fscanf(nfp, "%d %d %s\n", &jj, &ll, linebuf);
#ifdef DEBUG
        fprintf(stderr, "\nres %d (%d lines) name:%s\n", jj, ll, linebuf);
#endif
        Res[jj].name = ecalloc(strlen(linebuf) + 1, sizeof(char));
        sprintf(Res[jj].name, linebuf);
        Res[jj].oper = oop = (struct Op_descr *)
                             ecalloc(ll, sizeof(struct Op_descr));
        while (ll--) {
            while ((cc = getc(nfp)) == ' ');
            if ((oop->op = cc) == 'o') {
                while ((cc = getc(nfp)) == ' ');
                if ((oop->type = cc) == 'r') {
                    fscanf(nfp, "%lf\n", &(oop->val.rval));
#ifdef DEBUG
                    fprintf(stderr, "    const operand %lg\n", oop->val.rval);
#endif
                }
                else {
                    fscanf(nfp, "%d\n", &(oop->val.num));
#ifdef DEBUG
                    fprintf(stderr, "   operand type %c #%d\n", cc, oop->val.num);
#endif
                }
            }
            else {
                while ((getc(nfp)) != '\n');
            }
            oop++;
        }
    }
}


static struct Cond_descr *cond_stack;


int evaluate_next(unsigned short *mrk) {
    int val, val2;
    char op = (++cond_stack)->op;

#ifdef DEBUG
    fprintf(stderr, "      Start of evaluate_next %c\n", op);
#endif
    if (op == 'c') {
        val = cond_stack->op1;
        if (cond_stack->top1 == 'p')
            val = mrk[val];
        val2 = cond_stack->op2;
        if (cond_stack->top2 == 'p')
            val2 = mrk[val2];
#ifdef DEBUG
        fprintf(stderr, "      End of evaluate_next c %d %c %d\n",
                val, cond_stack->type, val2);
#endif
        switch (cond_stack->type) {
        case '=' :
            return (val == val2);
        case 'n' :
            return (val != val2);
        case 'g' :
            return (val >= val2);
        case '>' :
            return (val > val2);
        case 'l' :
            return (val <= val2);
        case '<' :
            return (val < val2);
        }
    }
    val = evaluate_next(mrk);
    if (op == '~') {
#ifdef DEBUG
        fprintf(stderr, "      End of evaluate_next ~ %d\n", val);
#endif
        return (! val);
    }
    val2 = evaluate_next(mrk);
#ifdef DEBUG
    fprintf(stderr, "      End of evaluate_next %d %c %d\n", val, op, val2);
#endif
    if (op == '|')
        return (val || val2);
    else
        return (val && val2);
}


int evaluate_cond(struct Cond_descr *cond, unsigned short *mrk) {
    int val, val2;

#ifdef DEBUG
    fprintf(stderr, "    Start of evaluate_cond %c\n", cond->op);
#endif
    if (cond->op == 'c') {
        val = cond->op1;
        if (cond->top1 == 'p')
            val = mrk[val];
        val2 = cond->op2;
        if (cond->top2 == 'p')
            val2 = mrk[val2];
#ifdef DEBUG
        fprintf(stderr, "    End of evaluate_cond c %d %c %d\n",
                val, cond->type, val2);
#endif
        switch (cond->type) {
        case '=' :
            return (val == val2);
        case 'n' :
            return (val != val2);
        case 'g' :
            return (val >= val2);
        case '>' :
            return (val > val2);
        case 'l' :
            return (val <= val2);
        case '<' :
            return (val < val2);
        }
    }
    cond_stack = cond;
    val = evaluate_next(mrk);
    if (cond->op == '~') {
#ifdef DEBUG
        fprintf(stderr, "    End of evaluate_cond ~ %d\n", val);
#endif
        return (! val);
    }
    if ((cond->op == '|') && val) {
#ifdef DEBUG
        fprintf(stderr, "    End of evaluate_cond fast-or TRUE\n");
#endif
        return (TRUE);
    }
    if ((cond->op == '&') && ! val) {
#ifdef DEBUG
        fprintf(stderr, "    End of evaluate_cond fast-and FALSE\n");
#endif
        return (FALSE);
    }
    val = evaluate_next(mrk);
#ifdef DEBUG
    fprintf(stderr, "    End of evaluate_cond %c val=%d\n", cond->op, val);
#endif
    return (val);
}



static struct Op_descr *op_stack;

double
evaluate_res(cvp, typ)
double *cvp;
int *typ;
{
    char cc = op_stack->op;
    double dd, dd2, val1, val2;
    int nn;
    Prob_p p_p;

#ifdef DEBUG
    fprintf(stderr, "  Start evaluate_res %c\n", cc);
#endif
    if (cc == 'o') {
        if (op_stack->type == 'r') {
            *cvp = dd = op_stack->val.rval;
#ifdef DEBUG
            fprintf(stderr, "   End evaluate_res r %f\n", dd);
#endif
            return (dd);
        }
        nn = op_stack->val.num;
        p_p = Prob + nn;
        dd = p_p->val;
#ifdef DEBUG
        fprintf(stderr, "       ... evaluate_res: prob %d, type=%c\n", nn, p_p->type);
#endif
        if (p_p->type <= 'Z') {
            *cvp = p_p->cumval;
            *typ = TRUE;
#ifdef DEBUG
            fprintf(stderr, "       ... evaluate_res: typ=TRUE val = %lg\n", dd);
#endif
        }
        else {
#ifdef DEBUG
            fprintf(stderr, "       ... evaluate_res: non cumulative prob\n");
#endif
            *cvp = dd;
        }
#ifdef DEBUG
        fprintf(stderr, "   End evaluate_res %c %d %f\n",
                op_stack->type, nn, dd);
#endif
        return (dd);
    }
    ++op_stack;
    val2 = evaluate_res(&dd, typ);
    ++op_stack;
    val1 = evaluate_res(&dd2, typ);
#ifdef DEBUG
    fprintf(stderr, "   End evaluate_res %f %c %f\n", val1, cc, val2);
#endif
    switch (cc) {
    case '+' :
        *cvp = dd + dd2;
        return (val1 + val2);
    case '-' :
        *cvp = dd - dd2;
        return (val1 - val2);
    case '*' :
        *cvp = dd * dd2;
        return (val1 * val2);
    case '/' :
        *cvp = dd / dd2;
        return (val1 / val2);
    default : return 0;
    }
}


void compute_res() {
    int ii;
    Res_p res_p;
    Prob_p prob_p;
    double cumval;
    int type;

#ifdef DEBUG
    fprintf(stderr, "Start compute_res\n");
#endif
    for (prob_p = Prob + 1, ii = 0 ; ii++ < maxprob ; prob_p++) {
        prob_p->cumval += prob_p->val;
    }
    for (ii = maxres, res_p = Res + 1 ; ii-- ; ++res_p) {
#ifdef DEBUG
        fprintf(stderr, "\n  result:%s =\n", res_p->name);
#endif
        type = FALSE;
        op_stack = res_p->oper;
        res_p->val = evaluate_res(&cumval, &type);
        if ((res_p->type = type)) {
#ifdef DEBUG
            fprintf(stderr, "       ...%s type=TRUE\n", res_p->name);
#endif
            res_p->cumval = cumval;
        }
        else {
#ifdef DEBUG
            fprintf(stderr, "       ...%s type=FALSE\n", res_p->name);
#endif
            res_p->cumval = 0.0;
        }
#ifdef DEBUG
        fprintf(stderr, "     %lg\n", res_p->val);
#endif
    }
    for (prob_p = Prob + 1, ii = 0 ; ii++ < maxprob ; prob_p++) {
        prob_p->val = 0.0;
    }
#ifdef DEBUG
    fprintf(stderr, "End compute_res\n");
#endif
}

void in_cur_marking(FILE *fp) {
    unsigned char *uc_p = cmarks;
    int nc = no_cod;

#ifdef DEBUGCALLS
    fprintf(stderr, "              Start of in_cur_marking\n");
#endif
    for (; nc-- ; ++uc_p) {
        *uc_p = getc(fp);;
#ifdef DEBUGpipe
        fprintf(stderr, "measure: in_cur_marking: %d\n", *uc_p);
#endif /* DEBUGpipe */
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "              End of in_cur_marking\n");
#endif
}


double target_time;

void firing(long nn) {
    int flag = FALSE;
    Trans_p trans_p;
    long nt;
    int state_flag = FALSE;

    do {
        if (nn > 0) {
            if ((nt = nn >> 1) > trans_num) {
                fprintf(stderr, "measure: ERROR! wrong transition number %ld\n",
                        nt);
                exit(1);
            }
            state_flag = nn & 1;
            trans_p = t_list + nt - 1;
#ifdef DEBUG
            fprintf(stderr, "       ... firing: +%s\n", trans_p->name);
#endif
            ++(trans_p->count);
        }
        else {
            nn = -nn;
            state_flag = nn & 1;
            if ((nt = nn >> 1) > trans_num) {
                fprintf(stderr, "measure: ERROR! wrong transition number -%ld\n",
                        nt);
                exit(1);
            }
            trans_p = t_list + nt - 1;
            nt = -nt;
#ifdef DEBUG
            fprintf(stderr, "       ... firing: -%s\n", trans_p->name);
#endif
            --(trans_p->count);
        }
        if (state_flag) {
            LOAD_DOUBLE(&target_time, stdin);
            if (nt > 0)   /* firing */
                do_state_transition(target_time);
            in_cur_marking(stdin);
            decode_mark(cmarks, DM);
            if (nt < 0)   /* unfiring */
                do_state_transition(target_time);
            if (flag) {
                flag = FALSE;
#ifdef DEBUG
                fprintf(stderr, "   ...measure: do checkpoint at time %lg\n", target_time);
#endif
                do_checkpoint();
            }
            return;
        }
#ifdef DEBUGpipe
        fprintf(stderr, "       ... firing 1: calling load_compact\n");
#endif

        LOAD_COMPACT((unsigned long *) &nn, stdin);
        if (nn > 0 && (nn >> 1) > trans_num) {
            if (nn == CMD_s) {
                LOAD_DOUBLE(&target_time, stdin);
                if (nt > 0)   /* firing */
                    do_state_transition(target_time);
                in_cur_marking(stdin);
                decode_mark(cmarks, DM);
                if (nt < 0)   /* unfiring */
                    do_state_transition(target_time);
                if (flag) {
                    flag = FALSE;
#ifdef DEBUG
                    fprintf(stderr, "   ...measure: do checkpoint at time %lg\n", target_time);
#endif
                    do_checkpoint();
                }
                return;
            }
            else if (nn == CMD_b) {
                LOAD_DOUBLE(&target_time, stdin);
                flag = TRUE;
#ifdef DEBUGpipe
                fprintf(stderr, "       ... firing 2: calling load_compact\n");
#endif
                LOAD_COMPACT((unsigned long *) &nn, stdin);
            }
            else {
                fprintf(stderr,
                        "Measure ERROR: firing unrecognized command %ld\n", nn);
                terminate_flag = TRUE;
                flag = FALSE;
                return;
            }
        }
        else { /* (UN)FIRING */
#ifdef DEBUG
            fprintf(stderr, "   ...measure: firing transition %ld\n", nt);
#endif
        }
    }
    while (TRUE);
}


int main(int argc, char **argv) {
    long nn;

#ifdef DEBUG
    fprintf(stderr, "Start\n");
#endif
    if (argc < 2) {
        fprintf(stderr, "ERROR: no net name !\n");
        exit(1);
    }
    sprintf(netname, "%s", argv[1]);
    confidence = 90;
    if (argc > 2) {
        char **a_p = &(argv[2]);
        char *s_p;
        unsigned ii = argc - 2;
        while (ii-- > 0) {
            s_p = *(a_p++);
            if (*s_p == '-') {
                switch (*++s_p) {
                case 'c' :
                    sscanf(s_p + 1, "%d", &confidence);
#ifdef DEBUG
                    fprintf(stderr, "  ... Measure: confidence level %d%%\n", confidence);
#endif
                    break;
                default :
#ifdef DEBUG
                    fprintf(stderr, "  ... Measure: other argument '%c'?\n", *s_p);
#endif
                    break;
                }
            }
        }
    }
    init();
    init_checkpoint();
    while (! terminate_flag) {
#ifdef DEBUGpipe
        fprintf(stderr, "    ... measure main() 1: calling load_compact\n");
#endif
        LOAD_COMPACT((unsigned long *) &nn, stdin);
        if (nn >= CMD_s) {
            if (nn == CMD_s) {
                LOAD_DOUBLE(&cur_time, stdin);
#ifdef DEBUG
                fprintf(stderr, "   ...measure: setting state at time %lg\n", cur_time);
#endif
                in_cur_marking(stdin);
                decode_mark(cmarks, DM);
            }
            else if (nn == CMD_R) {
#ifdef DEBUG
                fprintf(stderr, "   ...measure: computing results at time %lg\n", cur_time);
#endif
                compute_res();
                out_results();
            }
            else if (nn == CMD_b) {
                LOAD_DOUBLE(&target_time, stdin);
#ifdef DEBUG
                fprintf(stderr, "   ...measure: do checkpoint at time %lg\n", target_time);
#endif
                do_checkpoint();
            }
            else if (nn == CMD_c) {
                LOAD_DOUBLE(&target_time, stdin);
#ifdef DEBUG
                fprintf(stderr, "   ...measure: restore checkpoint at time %lg\n", target_time);
#endif
                restore_checkpoint(target_time);
            }
            else if (nn == CMD_p) {
                LOAD_DOUBLE(&target_time, stdin);
#ifdef DEBUG
                fprintf(stderr, "   ...measure: purge checkpoint at time %lg\n", target_time);
#endif
                s_purge_history(target_time);
                purge_history(target_time);
            }
            else if (nn == CMD_Q) {
#ifdef DEBUG
                fprintf(stderr, "   ...measure: Quitting\n");
#endif
                terminate_flag = TRUE;
            }
            else {
                fprintf(stderr,
                        "Measure ERROR: unrecognized command '%ld'\n", nn);
                terminate_flag = TRUE;
            }
        }
        else { /* (UN)FIRING */
#ifdef DEBUGpipe
            fprintf(stderr, "    ... measure main() 2: calling load_compact\n");
#endif
            /* LOAD_COMPACT( &ul, stdin ); nt = ul; */
#ifdef DEBUG
            fprintf(stderr, "   ...measure: firing transition %ld\n", nn);
#endif
            firing(nn);
        }
    }
    finish_checkpoint();
#ifdef DEBUG
    fprintf(stderr, "End\n");
#endif
    return 0;
}

