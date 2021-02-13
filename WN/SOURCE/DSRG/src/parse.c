#include "refDsrg.h"

#define MAXSTR 256

int nb_sr = 0;
int nb_sm = 0;
int nb_va = 0;
int nb_dead = 0;

FILE *dsrg = NULL ;
FILE *off_ev = NULL;
FILE *off_sr = NULL;
FILE *resultat = NULL;

char  net[MAXSTRING];

extern TO_MERGEP           *MERG_ALL;
extern STORE_STATICS_ARRAY  Sym_StaticConf;
extern char   *strtok();
extern double probs_for_a_node(pDtmcN nd, pDtmcA *p, int nb);
extern void local_free_temp_ord(Tree_Esrg_p  sr_ordinary, int sr_size_ordinary);
extern int get_index_from_group(pDtmcN nd, int gr);
extern void create_canonical_data_structure();
extern void get_canonical_marking();
extern TO_MERGEP   *TO_MERGE();
extern void   GROUPING_ALL_STATICS();
extern STORE_STATICS_ARRAY CREATE_STORE_STATIC_STRUCTURE();
extern void   NEW_SIM_STATIC_CONF();
extern void       FREE_ALL_LMS_ELEMS();
extern void compose_name(char  *name,  const char  *path,  const char  *postfix);
extern unsigned long load_event(FILE *fd, int pos, unsigned long fpos, unsigned long *sr, 
                    unsigned long *llength, unsigned long *ld_ptr, TYPE_P *** PM, TYPE_P   *NB_PM);
extern char * load_symbolic_marking(FILE *STD, FILE *fsr, FILE *fev, pDtmcN nd);

// adding a new event.
pDtmcA
add_ev(pDtmcN sr, int v,
       pDtmcN ds, int inc,
       int treated,
       double pr) {

    pDtmcA ev = (pDtmcA) malloc(sizeof(DtmcA));
    ev->sr = sr;
    ev->ds = ds;
    ev->pb = pr;
    ev->inc = inc;
    ev->treated = treated;
    ev->ev = v;


    return ev;
}

void
update_ev(pDtmcA ev,
          pDtmcN ds,
          double pr) {
    ev->ds = ds;
    ev->pb = pr;
}

void
event_add(pDtmcN nd, pDtmcA ev, int InOrOut) {
    int i, idx;

    if (InOrOut == 0) {
        int fr = false;

        for (i = 0; i < nd->nbin && !fr; i++)
            if (nd->in[i] == NULL) {
                fr = true;
                idx = i;
            }

        if (!fr) {
            nd->in = (pDtmcA *)realloc(nd->in,
                                       (nd->nbin + 1) *
                                       sizeof(pDtmcA **));
            idx = nd->nbin;
            nd->nbin++;
        }

        nd->in[idx] = ev;
    }

    if (InOrOut == 1) {

        int fr = false;

        for (i = 0; i < nd->nbout && !fr; i++)
            if (nd->out[i] == NULL) {
                fr = true;
                idx = i;
            }

        if (!fr) {
            nd->out = (pDtmcA *)realloc(nd->out,
                                        (nd->nbout + 1) *
                                        sizeof(pDtmcA **));
            idx = nd->nbout;
            nd->nbout++;
        }

        nd->out[idx] = ev;
    }

}

void
free_temp_in(pDtmcN nd) {
    int i, j;

    if (!(nd->nbinforcur)) return;

    for (i = 0; i < nd->nbcur; i++) {
        for (j = 0; j < nd->nbinforcur[i]; j++)
            free(nd->inforcur[i][j]);
        free(nd->inforcur[i]);
    }

    free(nd->inforcur);
    free(nd->nbinforcur);
    nd->inforcur = NULL;
    nd->nbinforcur = NULL;
}

void
free_temp_cur(pDtmcN nd) {
    local_free_temp_ord(nd->cur, nd->nbcur);

    int i;
    for (i = 0; i < nd->nbcur; i++)
        free(nd->cur[i].marking);

    free(nd->cur);
    nd->cur = NULL;
    nd->nbcur = 0;
}

void
free_node(pDtmcN nd) {
    int i;
    free_temp_in(nd);
    free_temp_cur(nd);
    for (i = 0; i < nd->nbin; i++)
        if (nd->in[i]) {free(nd->in[i]); nd->in[i] = NULL;}
    free(nd->in);
    for (i = 0; i < nd->nbout; i++)
        if (nd->out[i]) { free(nd->out[i]); nd->out[i] = NULL;}
    free(nd->out);
    if (nd->gl_out)
        free(nd->gl_out);
    nd->gl_out = NULL;

    free(nd);
}

void
eleminate_arcs(pDtmcN nd,
               pDtmcN end,
               int SrOrDs) {
    int k, nb;
    pDtmcA *ni;

    if (SrOrDs == 0) {
        ni = nd->in;
        nb = nd->nbin;
    }
    else {
        ni = nd->out;
        nb = nd->nbout;
    }


    for (k = 0; k < nb; k++) {
        if (SrOrDs == 0 &&
                ni[k] && ni[k]->sr == end)
            ni[k] = NULL;
        if (SrOrDs == 1 &&
                ni[k] && ni[k]->ds == end)
            ni[k] = NULL;
    }
}

// marking as treated
// a set of arcs.
// Ps : "srOrds != 0" => nd is a source
//      "srOrds == 0" => nd is a destination
void
marking_treated_ev(pDtmcA *head,
                   int start,
                   int end,
                   pDtmcN nd,
                   int srOrds) {
    int i;
    for (i = start + 1; i < end; i++) {
        if (srOrds == 0 && head[i] &&
                head[i]->sr == nd)
            head[i]->treated = 1;

        if (srOrds == 1 && head[i] &&
                head[i]->ds == nd)
            head[i]->treated = 1;
    }
}

void
free_a_ref_elements(pDtmcN nd) {
    free_temp_cur(nd);
    free(nd->gl_out);
    nd->gl_out = NULL;
}

pDtmcN
create_node(int st, unsigned long ofsr,
            int sr, unsigned long ofst,
            int nbord) {
    pDtmcN pt = (pDtmcN) malloc(sizeof(DtmcN));
    pt->nxt_list = NULL;
    pt->nxt_inc = NULL;
    pt->st = st;
    pt->sr = sr;
    pt->ord = nbord;
    pt->in = NULL;  pt->nbin = 0;
    pt->out = NULL; pt->nbout = 0;
    pt->cur = NULL; pt->nbcur = 0;
    pt->gl_out    = NULL;
    pt->inforcur  = NULL;
    pt->nbinforcur = NULL;
    pt->treated = 0;
    pt->fstpos = ofst;
    pt->fsrpos = ofsr;
    pt->first = 1;
    return pt;
}

void
dublicate_inst(pDtmcN nd, pDtmcN ndd,
               int idx) {

    int i;

    for (i = 0; i < nd->nbinforcur[idx]; i++) {
        if (nd->inforcur[idx][i] &&
                !nd->inforcur[idx][i]->treated &&
                nd->inforcur[idx][i]->sr->sr != nd->sr) {

            double prb = (double) probs_for_a_node(nd->inforcur[idx][i]->sr,
                                                   nd->inforcur[idx],
                                                   nd->nbinforcur[idx]) *
                         (double) ndd->ord /
                         (double) nd->inforcur[idx][i]->sr->ord;
            marking_treated_ev(nd->inforcur[idx],
                               0, nd->nbinforcur[idx],
                               nd->inforcur[idx][i]->sr ,
                               0);

            ndd->in = (pDtmcA *)realloc(ndd->in, (ndd->nbin + 1) * sizeof(pDtmcA));
            ndd->in[ndd->nbin] = add_ev(nd->inforcur[idx][i]->sr, -1,
                                        ndd, 0, 0,
                                        prb);

            event_add(nd->inforcur[idx][i]->sr,
                      ndd->in[ndd->nbin] , 1);
            ndd->nbin++;
        }
    }

    for (i = 0; i < nd->nbinforcur[idx]; i++)
        nd->inforcur[idx][i]->treated = 0;

}

pDtmcN
create_new_node(pDtmcN nd,
                MarkAsEventp li,
                int EVENTS,
                int gr) {
    pDtmcN pt = create_node(0, 0, 0, 0, 0);
    pt->sr  = nd->sr;
    pt->st  = EVENTS;
    pt->first = 0;
    int idx = get_index_from_group(nd, gr);
    pt->ord = li->ord;
    dublicate_inst(nd, pt, idx);

    return pt;
}



// Initialisation of global variables
void
init_global_variables(char *net_name) {

    create_canonical_data_structure();
    MERG = MERG_ALL = (TO_MERGEP *) TO_MERGE(ncl, tabc);
    GROUPING_ALL_STATICS(MERG_ALL, tabc, num);
    Sym_StaticConf =
        (STORE_STATICS_ARRAY)CREATE_STORE_STATIC_STRUCTURE();
    NEW_SIM_STATIC_CONF(&Sym_StaticConf);
    FreeMerg(MERG_ALL);
    FREE_ALL_LMS_ELEMS();

    // output params. of the
    // dsrg construction (with options: -o, -m)
    compose_name(net, net_name, "cmst");
    FILE *stat = fopen(net, "r");
    fscanf(stat, "%d %d %d %d", &nb_sr, &nb_sm,
           &nb_va, &nb_dead);
    fclose(stat);

    compose_name(net, net_name, "prob");
    resultat = fopen(net, "w+");

    compose_name(net, net_name, "esrg");
    dsrg = fopen(net, "r+");

    compose_name(net, net_name, "off_sr");
    off_sr = fopen(net, "r");

    compose_name(net, net_name, "off_ev");
    off_ev = fopen(net, "a+");

    fclose(EVENT_MARK);
    compose_name(net, net_name, "event");

    EVENT_MARK  = fopen(net, "a+");

}

// closes all files at the end
// of a session.
void
finalize_session() {
    fclose(dsrg);
    fclose(off_sr);
    fclose(resultat);

}

void
print_arcs(FILE *fd, pDtmcA pt) {

    fprintf(fd, "%d %d %d %d %d %d %g %d %d %d %d %d \n",
            pt->sr->sr,
            pt->sr->fsrpos,
            pt->sr->st,
            pt->sr->fstpos,
            pt->sr->ord,
            pt->inc, pt->pb,
            pt->ds->sr, pt->ds->fsrpos,
            pt->ds->st, pt->ds->fstpos,
            pt->ds->ord);

}

void
store_dtmc(char     *net_name,
           pDtmcN  **mtx_el,
           int      *nb_mtx_el) {

    int i, j, k, nb_smar = 0;
    unsigned long sr;
    unsigned long llength;
    unsigned long ld_ptr;
    unsigned long mark_pos;
    TYPE_P      **PM = NULL;
    TYPE_P        NB_PM = NULL;

    char  *tmp     = "./tmp";
    FILE  *mark1   = fopen(tmp, "w+");

    fclose(dsrg);
    compose_name(net, net_name, "dsrg");
    dsrg = fopen(net, "w+");

    for (i = 0; i < nb_sr; i++)
        for (j = 0; j < nb_mtx_el[i]; j++) {
            if (!mtx_el[i][j]->treated) {
                (void)load_event(off_ev,
                                 mtx_el[i][j]->st,
                                 mtx_el[i][j]->fstpos,
                                 &sr, &llength, &ld_ptr,
                                 &PM, &NB_PM);
                FreePartMAt(PM, NB_PM);
                mtx_el[i][j]->fstpos = ftell(mark1);
                mtx_el[i][j]->st = nb_smar;
                mtx_el[i][j]->treated = 1;
                fwrite(CACHE_STRING_ESRG, 1,
                       LP_ESRG - CACHE_STRING_ESRG, mark1);
                nb_smar++;
            }

            for (k = 0; k < mtx_el[i][j]->nbout; k++) {
                if (mtx_el[i][j]->out[k] &&
                        ! mtx_el[i][j]->out[k]->ds->treated) {
                    (void)load_event(off_ev, mtx_el[i][j]->out[k]->ds->st,
                                     mtx_el[i][j]->out[k]->ds->fstpos,
                                     &sr, &llength, &ld_ptr,
                                     &PM, &NB_PM);
                    FreePartMAt(PM, NB_PM);
                    mtx_el[i][j]->out[k]->ds->st = nb_smar;
                    mtx_el[i][j]->out[k]->ds->treated = 1;
                    mtx_el[i][j]->out[k]->ds->fstpos = ftell(mark1);
                    fwrite(CACHE_STRING_ESRG, 1,
                           LP_ESRG - CACHE_STRING_ESRG, mark1);
                    nb_smar++;
                }

                if (mtx_el[i][j]->out[k])
                    print_arcs(dsrg, mtx_el[i][j]->out[k]);
            }
        }

    fclose(mark1);
    fclose(off_ev);


    compose_name(net, net_name, "cmst");
    FILE *stat = fopen(net, "w+");
    fprintf(stat, "%d %d %d %d", nb_sr, nb_smar + 1,
            nb_va, nb_dead);
    fclose(stat);

    char *cm1 = (char *)calloc(MAXSTRING, sizeof(char));
    compose_name(net, net_name, "off_ev");
    sprintf(cm1, "/bin/mv -f %s %s", tmp, net);
    system(cm1);
    free(cm1);
}

void
print_dtmc(char     *net_name,
           pDtmcN  **mtx_el,
           int      *nb_mtx_el) {

    int i, j, k;
    FILE *dsrga;

    compose_name(net, net_name, "asciidsrg");
    dsrga = fopen(net, "w+");

    compose_name(net, net_name, "off_ev");
    off_ev = fopen(net, "r");

    fprintf(dsrga, "digraph dtmc{\n");

    for (i = 0; i < nb_sr; i++)
        for (j = 0; j < nb_mtx_el[i]; j++) {
            char *st;
            st = load_symbolic_marking(dsrga,
                                       off_sr,
                                       off_ev ,
                                       mtx_el[i][j]);

            fprintf(dsrga, "S%d [fontsize=7, label=\"S%d\[%d\]:%s \"];\n",
                    mtx_el[i][j]->st, mtx_el[i][j]->st, i, st);
            free(st);
        }

    for (i = 0; i < nb_sr; i++)
        for (j = 0; j < nb_mtx_el[i]; j++) {
            for (k = 0; k < mtx_el[i][j]->nbout; k++)
                if (mtx_el[i][j]->out[k]) {
                    fprintf(dsrga, "S%d -> S%d [fontsize=7,label=\"%g\"];\n",
                            mtx_el[i][j]->st,
                            mtx_el[i][j]->out[k]->ds->st,
                            mtx_el[i][j]->out[k]->pb);
                }
        }

    fprintf(dsrga, "}");
    fclose(off_ev);
    fclose(dsrga);
}

// Parse of  params. (in order) :
// (1) sym. rep. position of source fclose(   EVENT_MARK   );
// (2) source
// (3) nb ordinary of the source
// (4) type of source (tangible or vanishing)
// (5) out put prob.
// (6) sym. rep. position of destination
// (7) destination
// (8) nb ordinary of the destination
int
parse_binding(char *string,
              int *sr_source,
              unsigned long *ofsr,
              int *source ,
              unsigned long *ofst,
              int *sord,
              int *type,
              int *sr_dest,
              unsigned long *ofsrd,
              int *dest ,
              unsigned long *ofstd,
              int *dord,
              double *prob) {

    char *tok;
    char *delim = " ";
    int i;

    tok = strtok(string , delim);
    sscanf(tok, "%d", sr_source);
    tok = strtok(NULL , delim);
    sscanf(tok, "%d", ofsr);
    tok = strtok(NULL , delim);
    sscanf(tok, "%d", source);
    tok = strtok(NULL , delim);
    sscanf(tok, "%d", ofst);
    tok = strtok(NULL , delim);
    sscanf(tok, "%d", sord);
    tok = strtok(NULL , delim);
    sscanf(tok , "%d", type);

    if ((*type) != 0) {
        tok = strtok(NULL, delim);
        sscanf(tok, "%lf", prob);
        tok = strtok(NULL, delim);
        sscanf(tok, "%d", sr_dest);
        tok = strtok(NULL, delim);
        sscanf(tok, "%d", ofsrd);
        tok = strtok(NULL , delim);
        sscanf(tok, "%d", dest);
        tok = strtok(NULL, delim);
        sscanf(tok, "%d", ofstd);
        tok = strtok(NULL , delim);
        sscanf(tok, "%d", dord);
    }
    else
        return false;

    return true;
}

pDtmcN
add_incl(pDtmcN head,
         pDtmcN node) {
    pDtmcN hd = head;

    while (hd) {

        if (hd->sr == node->sr) {
            return head;
        }

        hd = hd->nxt_inc;
    }

    node->nxt_inc = head;
    return node;
}

// Store of symbolic states
// in the arr_sr_sm array structure
pDtmcN
add_dtmc(pDtmcN **mtx_el, int *nb_mtx_el,
         int sr_source, unsigned long ofsr,
         int source, unsigned long ofst,
         int sord,
         int sr_dest, unsigned long ofsrd,
         int dest, unsigned long ofstd,
         int dord,
         int type, double prob,
         pDtmcN head) {

    int i, j;

    for (i = 0;
            (i < nb_mtx_el[sr_source]) &&
            (mtx_el[sr_source][i]->st != source) ;
            i++);

    if (i == nb_mtx_el[sr_source]) {
        mtx_el[sr_source] =
            (pDtmcN *)realloc(mtx_el[sr_source],
                              (i + 1) * sizeof(pDtmcN));
        mtx_el[sr_source][i] =
            create_node(source, ofsr, sr_source, ofst, sord);
        nb_mtx_el[sr_source]++;
    }

    for (j = 0;
            (j < nb_mtx_el[sr_dest]) &&
            (mtx_el[sr_dest][j]->st != dest) ;
            j++);

    if (j == nb_mtx_el[sr_dest]) {
        mtx_el[sr_dest] =
            (pDtmcN *)realloc(mtx_el[sr_dest],
                              (j + 1) * sizeof(pDtmcN));
        mtx_el[sr_dest][j] =
            create_node(dest, ofsrd, sr_dest, ofstd, dord);
        nb_mtx_el[sr_dest]++;
    }


    pDtmcA ev = NULL;

    if (type == 3)
        ev = add_ev(mtx_el[sr_source][i], -1,
                    mtx_el[sr_dest][j],
                    1, 0, prob);
    else
        ev = add_ev(mtx_el[sr_source][i], -1,
                    mtx_el[sr_dest][j],
                    0, 0, prob);

    event_add(mtx_el[sr_dest][j], ev, 0);
    event_add(mtx_el[sr_source][i], ev, 1);

    if (type == 3)
        head = add_incl(head, mtx_el[sr_dest][j]);

    return head;
}

void
load_partial_dtmc(unsigned long nb_sm,
                  FILE *dsrg,
                  pDtmcN *head,
                  pDtmcN *headinc,
                  pDtmcN **mtx_el,
                  int *nb_mtx_el) {

    int sr_source, source, sord,
        sr_dest, dest, dord, type;
    double prob;
    char tmp[MAXSTR];
    unsigned long ofsr, ofst, ofsrd, ofstd;


    while (fgets(tmp, MAXSTR - 1, dsrg)) {

        (void) parse_binding(tmp,
                             &sr_source, &ofsr,
                             &source, &ofst,
                             &sord,
                             &type,
                             &sr_dest, &ofsrd,
                             &dest, &ofstd,
                             &dord, &prob);

        (*headinc) = add_dtmc(mtx_el, nb_mtx_el,
                              sr_source, ofsr, source, ofst , sord,
                              sr_dest, ofsrd, dest, ofstd , dord,
                              type, prob,
                              *headinc);

    }

    (*head) = mtx_el[0][0];
    // int i;
    // for(i=0;i<nb_sr;i++)
    //  free( mtx_el[i]);
    // free(mtx_el);
    // free(nb_mtx_el);

}

