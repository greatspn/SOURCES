/********************************************************************
 *
 * Date: Torino, April 20, 1993
 * Program: measure_checkpoint.c
 * Description: checkpoint routines for measurer module of GreatSPN 1.6
 * Programmer: Giovanni Chiola
 * notes:
 *   1) This program allows both normal (forward) and reversed
 *      (backward) simulation. Periodic checkpoints are stored on file
 *      in order to allow both extended backtracking and possibility
 *      of continuation and resume of previous runs.
 *
 ********************************************************************/

/*
#define DEBUG
*/


#ifdef DEBUG

#define DEBUGCALLS
/*
*/

#endif

#include "measure_decl.h"
#include "measure_pn.h"
#include "../WN/INCLUDE/compact.h"
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

double cur_time;

static double last_time = 0.0;



void do_state_transition(double tt) {
    double delta = tt - cur_time;
    Place_p place_p = p_list;
    Prob_p prob_p = Prob + 1;
    unsigned np, nn;
    char cc;

#ifdef DEBUGCALLS
    fprintf(stderr, "       Start of do_state_transition %g\n", tt);
#endif
    for (np = 0 ; np++ < place_num ; place_p++) {
        int ntok = DM[np];
        if (ntok > place_p->aub)
            place_p->aub = ntok;
        if (ntok < place_p->alb)
            place_p->alb = ntok;
        ntok -= place_p->lbound;
        *(place_p->tok_distr + ntok) += delta;
    }
    for (np = maxprob ; np-- ; prob_p++) {
        cc = prob_p->type;
        switch (cc) {
        case 'e' :
        case 'E' :
            nn = prob_p->place;
            if ((nn = DM[nn]))
                prob_p->val += delta * nn;
            break;
        case 'c' :
        case 'C' :
            nn = prob_p->place;
            if ((nn = DM[nn]) && (evaluate_cond(prob_p->cond, DM)))
                prob_p->val += delta * nn;
            break;
        case 'p' :
        case 'P' :
            if (evaluate_cond(prob_p->cond, DM))
                prob_p->val += delta;
            break;
        }
    }

    cur_time = tt;

#ifdef DEBUGCALLS
    fprintf(stderr, "       End of do_state_transition\n");
#endif
}


static FILE *ckpf = NULL;
char cur_checkpoint_name[256];
char tmp_checkpoint_name[256];
char s_tmp_checkpoint_name[256];
char history_name[256];
char s_history_name[256];
char cat_checkpoint_history[1600];
char s_transfer_history[1024];

void out_cur_checkpoint(FILE *ckpf) {
    int nn, ii;
    unsigned char *uc_p = cmarks;
    Place_p place_p = p_list;
    Trans_p trans_p = t_list;
    Prob_p prob_p = Prob + 1;

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of out_cur_checkpoint\n");
#endif
    fprintf(ckpf, "%lg %lg %d %d %d", cur_time, last_time,
            place_num, trans_num, maxprob);
    for (ii = no_cod ; ii-- ; ++uc_p) {
#ifdef DEBUG
        fprintf(stderr, "         ... out_cur_checkpoint: coding ii=%d\n", ii);
#endif
        fprintf(ckpf, " %d", *uc_p);
    }
#ifdef DEBUG
    fprintf(stderr, "      ... out_cur_checkpoint: marking saved\n");
#endif
    fprintf(ckpf, "\n");
    for (nn = 0 ; nn++ < place_num ;  place_p++) {
        fprintf(ckpf, " %d %d %d", nn, place_p->alb, place_p->aub);
#ifdef DEBUG
        fprintf(stderr, "      ... out_cur_checkpoint: place %d %d %d\n", nn,
                place_p->alb, place_p->aub);
#endif
        for (ii = place_p->alb - place_p->lbound ;
                ii <= place_p->aub - place_p->lbound ; ii++) {
#ifdef DEBUG
            fprintf(stderr, "         ... out_cur_checkpoint: ii=%d\n", ii);
#endif
            fprintf(ckpf, " %lg %lg", *(place_p->tok_distr + ii),
                    *(place_p->cum_distr + ii));
        }
        fprintf(ckpf, "\n");
    }
    for (nn = 0 ; nn++ < trans_num ;  trans_p++) {
#ifdef DEBUG
        fprintf(stderr, "      ... out_cur_checkpoint: trans %d %ld %ld\n", nn,
                trans_p->count, trans_p->cumcount);
#endif
        fprintf(ckpf, "  %d %ld %ld\n", nn, trans_p->count, trans_p->cumcount);
    }
    for (nn = 0 ; nn++ < maxprob ;  prob_p++) {
#ifdef DEBUG
        fprintf(stderr, "      ... out_cur_checkpoint: prob %d\n", nn);
#endif
        fprintf(ckpf, " %d %lg %lg\n", nn, prob_p->val, prob_p->cumval);
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of out_cur_checkpoint\n");
#endif
}


void do_checkpoint() {

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of do_checkpoint\n");
#endif

    if ((ckpf = fopen(cur_checkpoint_name, "w")) == NULL) {
        fprintf(stderr, "\n Measurer ERROR: couldn't open %s file\n",
                cur_checkpoint_name);
        return;
    }
    out_cur_checkpoint(ckpf);
    (void)fclose(ckpf); ckpf = NULL;
    system(cat_checkpoint_history);

#ifdef DEBUGCALLS
    fprintf(stderr, "    End of do_checkpoint\n");
#endif
}




double search_simulation_state(double time,
                               FILE *fp) {
    double ttt;
    int np, nt, npro;

#ifdef DEBUGCALLS
    fprintf(stderr, "        Start of search_simulation_state(%1.6g)\n", time);
#endif
    for (fscanf(fp, "%lg", &ttt) ; ttt > time ; fscanf(fp, "%lg", &ttt)) {
        fscanf(fp, "%*g %d %d %d", &np, &nt, &npro);
        for (np++ ; np-- ;)
            while (getc(fp) != '\n');
        for (; nt-- ;)
            while (getc(fp) != '\n');
        for (; npro-- ;)
            while (getc(fp) != '\n');
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "        End of search_simulation_state(%1.6g)\n", ttt);
#endif
    return (ttt);
}


void restore_checkpoint(double time) {
    FILE *fp;
    Place_p place_p = p_list;
    Trans_p trans_p = t_list;
    Prob_p prob_p = Prob + 1;
    unsigned char *uc_p = cmarks;
    int np, nt, npro;
    double dd;
    int nn, ii, jj;
    unsigned long uu;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of restore_checkpoint time=%1.6g\n", time);
#endif
    if ((fp = fopen(history_name, "r")) == NULL) {
        fprintf(stderr, "\n Measurer ERROR: couldn't open %s file\n",
                history_name);
        return;
    }
    (void) search_simulation_state(time, fp);
    cur_time = time;
    if (fscanf(fp, "%lg %d %d %d", &last_time, &np, &nt, &npro) < 4 ||
            nt != trans_num || np != place_num || npro != maxprob) {
        fprintf(stderr, "Measure ERROR: inconsistency in history file!\n");
        fprintf(stderr, "nt=%d trans_num=%d; np=%d place_num=%d;",
                nt, trans_num, np, place_num);
        fprintf(stderr, " npro=%d maxprob=%d\n", npro, maxprob);
        exit(1);
    }
#ifdef DEBUG
    fprintf(stderr, "    ... restore_checkpoint: reading marking\n");
#endif
    for (ii = no_cod ; ii-- ; ++uc_p) {
        fscanf(fp, "%d", &nn); *uc_p = (unsigned)nn;
    }
    while (getc(fp) != '\n');
    decode_mark(cmarks, DM);
#ifdef DEBUG
    fprintf(stderr, "    ... restore_checkpoint: reading token distr\n");
#endif
    for (nn = 0 ; nn++ < place_num ;  place_p++) {
        fscanf(fp, "%d %d %d", &np, &ii, &jj);
        place_p->alb = ii;
        place_p->aub = jj;
        for (ii -= place_p->lbound, jj -= place_p->lbound ; ii <= jj ; ii++) {
#ifdef DEBUG
            fprintf(stderr, "        ... restore_checkpoint: place %d tok %d\n", nn, ii);
#endif
            fscanf(fp, "%lg", &dd); *(place_p->tok_distr + ii) = dd;
            fscanf(fp, "%lg", &dd); *(place_p->cum_distr + ii) = dd;
        }
        while (getc(fp) != '\n');
    }
#ifdef DEBUG
    fprintf(stderr, "    ... restore_checkpoint: reading trans thru\n");
#endif
    for (nn = 0 ; nn++ < trans_num ;  trans_p++) {
        fscanf(fp, "%d %ld", &nt, &uu); trans_p->count = uu;
        fscanf(fp, "%ld", &uu); trans_p->cumcount = uu;
        while (getc(fp) != '\n');
    }
#ifdef DEBUG
    fprintf(stderr, "    ... restore_checkpoint: reading probs\n");
#endif
    for (nn = 0 ; nn++ < maxprob ;  prob_p++) {
        fscanf(fp, "%d %lg", &npro, &dd); prob_p->val = dd;
        fscanf(fp, "%d %lg", &npro, &dd); prob_p->cumval = dd;
        while (getc(fp) != '\n');
    }
    (void)fclose(fp);
#ifdef DEBUGCALLS
    fprintf(stderr, "    End of restore_checkpoint\n");
#endif
}


void purge_history(double at_time) {
    FILE *fp2;
    char ccc[600];
    char c;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of purge_history at time %1.6g\n", at_time);
#endif
    if ((ckpf = fopen(history_name, "r")) == NULL) {
        fprintf(stderr, "\n Measurer ERROR: couldn't open %s file\n",
                history_name);
        return;
    }
    at_time = search_simulation_state(at_time, ckpf);
    if ((fp2 = fopen(tmp_checkpoint_name, "w")) == NULL) {
        fprintf(stderr, "\n Measurer ERROR: couldn't open %s file\n",
                tmp_checkpoint_name);
        (void)fclose(ckpf); ckpf = NULL;
        return;
    }
    fprintf(fp2, "%1.6lg ", at_time);
    for (c = getc(ckpf) ; c != EOF ; putc(c, fp2), c = getc(ckpf));
    (void)fclose(ckpf); ckpf = NULL;
    (void)fclose(fp2);
    sprintf(ccc, "/bin/mv %s %s", tmp_checkpoint_name, history_name);
    system(ccc);

#ifdef DEBUGCALLS
    fprintf(stderr, "  End of purge_history\n");
#endif

}

void s_purge_history(double at_time) {
    FILE *fp;
    char c;
    int old_sample_n, sample_n, nn, nc;
    double xsum, tt, vv;
    int num_line = place_num + trans_num + maxres ;

#ifdef DEBUGCALLS
    fprintf(stderr, "  Start of statistic_purge_history at time %1.6g\n", at_time);
#endif

    if ((ckpf = fopen(s_history_name, "r")) == NULL) {
        fprintf(stderr, "\n Measurer ERROR: couldn't open %s file\n",
                s_history_name);
        return;
    }
    fscanf(ckpf, "%d\n", &nc);
    old_sample_n =  nc;
    if (nc == 0) {
        (void)fclose(ckpf); ckpf = NULL;
#ifdef DEBUGCALLS
        fprintf(stderr, "  End of statistic_purge_history: empty file\n");
#endif
        return ;
    }
    for (fscanf(ckpf, "%lg", &tt) ; nc-- > 0 && tt > at_time ; fscanf(ckpf, "%d %lg", &nc, &tt));
    fscanf(ckpf, "%d", &sample_n);

    if ((fp = fopen(s_tmp_checkpoint_name, "w")) == NULL) {
        fprintf(stderr, "\n Measurer ERROR: couldn't open %s file\n",
                s_tmp_checkpoint_name);
        (void)fclose(ckpf); ckpf = NULL;
        return;
    }
    fprintf(fp, "%d\n%lg %d ", sample_n, tt, sample_n);
    for (c = getc(ckpf), putc(c, fp) ; c != '\n' ; c = getc(ckpf), putc(c, fp));
    if (sample_n != 0) {
        for (nn = 0 ; nn < num_line ; nn++) {
            fscanf(ckpf, "%lg", &vv) ;
            xsum = vv ;
            for (nc = 0 ; nc < old_sample_n - sample_n ; nc++) {
                fscanf(ckpf, "%lg", &vv) ;
                xsum -= vv ;
            }
            fprintf(fp, "%lg ", xsum);
            for (c = getc(ckpf), putc(c, fp) ; c != '\n' ; c = getc(ckpf), putc(c, fp));
        }
    }

    (void)fclose(ckpf); ckpf = NULL;
    (void)fclose(fp);
    system(s_transfer_history);

#ifdef DEBUGCALLS
    fprintf(stderr, "  End of statistic_purge_history\n");
#endif
}

static unsigned max_struct;
static double *deltas_l;


void initialize_zero() {
    Place_p place_p = p_list;
    Trans_p trans_p = t_list;
    Prob_p prob_p = Prob + 1;
    int nn, ii;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of initialize_zero\n");
#endif
    cur_time = last_time = 0.0;
    decode_mark(cmarks, DM);
    for (nn = 0 ; nn++ < place_num ;  place_p++) {
        place_p->alb = DM[nn];
        place_p->aub = DM[nn];
        for (ii = 0 ; ii <= place_p->rubound ; ii++) {
            *(place_p->cum_distr + ii) = *(place_p->tok_distr + ii) = 0.0;
        }
    }
    for (nn = 0 ; nn++ < trans_num ;  trans_p++) {
        trans_p->cumcount = trans_p->count = 0;
    }
    for (nn = 0 ; nn++ < maxprob ;  prob_p++) {
        prob_p->val = prob_p->cumval = 0.0;
    }
#ifdef DEBUGCALLS
    fprintf(stderr, "    End of initialize_zero\n");
#endif
}


void init_checkpoint() {
    FILE *fp;
    int my_pid = getpid();
    char hostname[128];

#ifdef DEBUGCALLS
    fprintf(stderr, " Start of init_checkpoint\n");
#endif
    gethostname(hostname, 128);

    sprintf(history_name, "%s.mtrace", netname);
    if ((fp = fopen(history_name, "r")) == NULL) {
        fp = fopen(history_name, "w");
        fprintf(fp, "-1\n");
    }
    fclose(fp);

    sprintf(history_name, "%s.strace", netname);
    if ((fp = fopen(history_name, "r")) == NULL) {
        fp = fopen(history_name, "w");
        fprintf(fp, "0\n0 0\n");
    }
    fclose(fp);

    sprintf(history_name, "%s_%s_%d.mtrace", netname, hostname, my_pid);
    sprintf(cat_checkpoint_history, "/bin/cp %s.mtrace %s",
            netname, history_name);
    system(cat_checkpoint_history);


    sprintf(s_history_name, "%s_%s_%d.strace", netname, hostname, my_pid);
    sprintf(cat_checkpoint_history, "/bin/cp %s.strace %s",
            netname, s_history_name);
    system(cat_checkpoint_history);

    sprintf(cur_checkpoint_name, "%s_%s_%d.mckp", netname,
            hostname, my_pid);
    sprintf(tmp_checkpoint_name, "%s_%s_%d.mtmp", netname,
            hostname, my_pid);
    sprintf(s_tmp_checkpoint_name, "%s_%s_%d.stmp", netname,
            hostname, my_pid);
    sprintf(s_transfer_history, "/bin/mv %s %s",
            s_tmp_checkpoint_name , s_history_name);
    sprintf(cat_checkpoint_history,
            "cat %s %s > %s; /bin/mv %s %s ; /bin/rm -f %s",
            cur_checkpoint_name, history_name, tmp_checkpoint_name,
            tmp_checkpoint_name, history_name,
            cur_checkpoint_name);

    initialize_zero();

    if ((fp = fopen(history_name, "r")) == NULL) {
        fprintf(stderr,
                "\n Measurer ERROR: couldn't open %s file\n",
                history_name);
        exit(1);
    }
    fclose(fp);

    if ((fp = fopen(s_history_name, "r")) == NULL) {
        fprintf(stderr,
                "\n Measurer ERROR: couldn't open %s file\n",
                s_history_name);
        exit(1);
    }
    fclose(fp);
    max_struct = (place_num > trans_num) ? place_num : trans_num ;
    max_struct = (max_struct < maxres) ? maxres : max_struct ;
    deltas_l = (double *)ecalloc(max_struct, sizeof(double));
#ifdef DEBUGCALLS
    fprintf(stderr, " End of init_checkpoint\n");
#endif
}





void finish_checkpoint() {
    char copy_history[1024];

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of finish_checkpoint\n");
#endif

    sprintf(copy_history, "/bin/mv %s %s.mtrace", history_name, netname);
    system(copy_history);

    sprintf(copy_history, "/bin/mv %s %s.strace", s_history_name, netname);
    system(copy_history);

    free(deltas_l);
#ifdef DEBUGCALLS
    fprintf(stderr, "    End of finish_checkpoint\n");
#endif
}



float search_tab(int n, int alfa) {
    int i;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of search_tab\n");
#endif

    if (n <= 30) {
        for (i = 0 ; tabstud[n - 1][i][0] < alfa ; i++) ;
#ifdef DEBUGCALLS
        fprintf(stderr, "    End of search_tab\n");
#endif
        return (tabstud[n - 1][i][1]) ;
    }
    else {
        for (i = 0 ; tabnorm[i][0] < alfa ; i++) ;
#ifdef DEBUGCALLS
        fprintf(stderr, "    End of search_tab\n");
#endif
        return (tabnorm[i][1]) ;
    }


}

double sqr(double val) {
    double sqr;

    sqr = val * val;
    return (sqr);
}

static int write_flag;

double evaluate_delta(FILE *fp1, FILE *fp2,
                      int sn, int n_sn,
                      float z_t,
                      double sample) {
    int skip_n, ii;
    double xsum, mean, sigma, vv;
    double dv = (double)(n_sn + 1);

#ifdef DEBUGCALLS
    fprintf(stderr, "      Start of evaluate_delta n_sn=%d\n", n_sn);
#endif

    skip_n = sn - n_sn ;
    if (sn > 0)
        fscanf(fp1, "%lg", &xsum);
    else
        xsum = 0.0;
    for (ii = 0 ; ii++ < skip_n ;  fscanf(fp1, "%lg", &vv) , xsum -= vv);
    xsum += sample;
    if (write_flag)
        fprintf(fp2, "%lg %lg ", xsum, sample);
    mean = xsum / (n_sn + 1);
    xsum = sqr(sample - mean);
    for (ii = 0 ; ii < n_sn ; ii++) {
        fscanf(fp1, "%lg", &vv);
        if (write_flag)
            fprintf(fp2, "%lg ", vv);
        xsum += sqr(vv - mean);
    }
    if (write_flag)
        fprintf(fp2, "\n");
    if (n_sn > 0)
        vv = xsum / n_sn;
    else
        vv = 0;
    sigma = sqrt(vv);
    vv = sqrt(dv);
#ifdef DEBUGCALLS
    fprintf(stderr, "      End of evaluate_delta\n");
#endif
    return (z_t * (sigma / vv));
}



void out_results() {
    Place_p place_p;
    Trans_p trans_p;
    Res_p res_p;
    FILE *fp;
    FILE *sfp;
    FILE *tfp;
    char sss[1024], cc;
    int sample_n, new_sample_n, ii, nc, jj;
    float z_t;
    double p_sum = 0.0, tt, dd;
    double delta_t = cur_time - last_time;

#ifdef DEBUGCALLS
    fprintf(stderr, "    Start of out_results\n");
#endif

    last_time = cur_time;

    sprintf(sss, "%s.tpd", netname);
    if ((fp = fopen(sss, "w")) == NULL) {
        fprintf(stderr, "\n Measurer ERROR: couldn't open %s file for w\n",
                sss);
        return;
    }

    if ((sfp = fopen(s_history_name, "r")) == NULL) {
        fprintf(stderr, "\n Measurer ERROR: couldn't open %s file for r\n",
                s_history_name);
        return;
    }

    if ((tfp = fopen(s_tmp_checkpoint_name, "w")) == NULL) {
        fprintf(stderr, "\n Measurer ERROR: couldn't open %s file for w\n",
                s_tmp_checkpoint_name);
        return;
    }

    fscanf(sfp, "%d\n", &sample_n);
    if ((nc = sample_n) <= 0) {
        write_flag = TRUE;
        tt = 0.0;
        if (cur_time > 0.0)
            nc = 1;
        else
            nc = 0;
        fprintf(tfp, "%d\n %g %d\n", nc, cur_time, nc);
        new_sample_n = 0 ;
    }
    else {
        for (write_flag = FALSE, fscanf(sfp, "%lg %d", &tt, &nc) ; tt > cur_time ; fscanf(sfp, "%lg %d", &tt, &nc));
        if (tt < cur_time) {
            write_flag = TRUE;
            fprintf(tfp, "%d\n %g %d ", nc + 1, cur_time, nc + 1);
            fprintf(tfp, "%g %d ", tt, nc);
            for (cc = getc(sfp), putc(cc, tfp) ; cc != '\n' ; cc = getc(sfp), putc(cc, tfp));
            new_sample_n = nc ;
        }
        else {
            for (cc = getc(sfp) ; cc != '\n' ; cc = getc(sfp));
            new_sample_n = nc - 1 ;
            write_flag = FALSE;
        }
    }

    z_t = search_tab(sample_n + 1, confidence);

    /* Evaluate precision of token-expectation for PLACES */

    for (place_p = p_list, ii = 0 ; ii < place_num ; ii++, place_p++) {
        dd = place_p->alb; store_double(&dd, fp);
        dd = place_p->aub; store_double(&dd, fp);
        p_sum = 0.0;

        for (jj = 0 ; jj <= place_p->rubound ; jj++) {
            /******************************************************/
            if (delta_t != 0.0) {
                /******************************************************/
                p_sum += (*(place_p->tok_distr + jj) * jj);

                *(place_p->cum_distr + jj) += *(place_p->tok_distr + jj);
                /******************************************************/
            }
            /******************************************************/
            if (jj >= place_p->alb - place_p->lbound &&
                    jj <= place_p->aub - place_p->lbound) {
                if (cur_time > 0.0) {
                    dd = *(place_p->cum_distr + jj) / cur_time;
                }
                else {
                    dd = 1.0 / (place_p->aub - place_p->alb + 1);
                }
                store_double(&dd, fp);
            }
            *(place_p->tok_distr + jj) = 0.0;
        }
        /******************************************************/
        if (delta_t != 0.0) {
            /******************************************************/
            p_sum /= delta_t;
            /******************************************************/
        }
        /******************************************************/

        if (cur_time > 0.0)
            * (deltas_l + ii) = evaluate_delta(sfp, tfp, sample_n, new_sample_n, z_t, p_sum);
        else
            * (deltas_l + ii) = 0.0;
    }
    for (ii = 0 ; ii < place_num ; ii++) {
        dd = *(deltas_l + ii) ; store_double(&dd, fp); /* result precision */
    }
    (void)fclose(fp);

#ifdef DEBUG
    fprintf(stderr, "        ... opening .sta\n");
#endif
    sprintf(sss, "%s.sta", netname);
    if ((fp = fopen(sss, "w")) == NULL) {
        fprintf(stderr, "\n Measurer ERROR: couldn't open %s file for w\n",
                sss);
        return;
    }

    /* Evaluate precision of result values for RESULTS */

    for (res_p = Res + 1, ii = 0 ; ii < maxres ; ii++, res_p++) {
#ifdef DEBUG
        fprintf(stderr, "          ... result %s\n", res_p->name);
#endif
        if (res_p->val == 0)
            dd = 0.0;
        else
            dd = (double)(res_p->val / delta_t);

        if (cur_time > 0.0)
            * (deltas_l + ii) = evaluate_delta(sfp, tfp, sample_n, new_sample_n, z_t, dd);
        else
            * (deltas_l + ii) = 0.0;

        if (res_p->type)
            fprintf(fp, "%s = %lg +/- %lg\n", res_p->name, (cur_time > 0.0) ?
                    res_p->cumval / cur_time : 0 , *(deltas_l + ii));
        else
            fprintf(fp, "%s = %lg\n", res_p->name, (delta_t > 0.0) ?
                    res_p->val / delta_t : 0);
        res_p->val = 0.0;
    }

    /* Evaluate precision of troughtput results for TRANSITIONS */

    for (trans_p = t_list, ii = 0 ; ii < trans_num ; ii++, trans_p++) {
#ifdef DEBUG
        fprintf(stderr, "          ... thru of %s\n", trans_p->name);
#endif
        trans_p->cumcount += trans_p->count;
        if (trans_p->count == 0) dd = 0.0;
        else dd = (double)(trans_p->count / delta_t);

        if (cur_time > 0.0)
            * (deltas_l + ii) = evaluate_delta(sfp, tfp, sample_n, new_sample_n, z_t, dd);
        else
            * (deltas_l + ii) = 0.0;

        trans_p->count = 0;
    }
    for (trans_p = t_list, ii = 0 ; ii < trans_num ; ii++, trans_p++) {
        dd = (cur_time > 0.0) ? trans_p->cumcount / cur_time : 0;
        fprintf(fp, "Thru_%s = %lg +/- %lg\n", trans_p->name, dd , *(deltas_l + ii));
    }

    (void)fclose(fp); (void)fclose(sfp); (void)fclose(tfp);
    sprintf(sss, "/bin/rm -f %s.SIMCC", netname);
    system(sss);
#ifdef DEBUG
    fprintf(stderr, "Measure: removed SIMCC\n");
#endif // DEBUG
    if (write_flag)
        system(s_transfer_history);

#ifdef DEBUGCALLS
    fprintf(stderr, "    End of out_result\n");
#endif
}

