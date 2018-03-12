/*
 * indprest.c
 */

/*
#define DEBUG
*/

#include "global.h"
#include "draw.h"
#include "res.h"
#include "showgdi.h"
#include "zoom.h"

struct ind_prest_list {
    DrawingSurfacePun       pw;
    int             dim_x;	/* dim fin. */
    int             dim_y;	/* dim fin. */
    float           sc_x;	/* scala */
    float           sc_y;	/* scala */
    float           min_y;	/* minimo y */
    float           max_y;	/* massimo y */
    float           tempo;	/* prec temp */
    float           valore;	/* prec val */
    int             diff_y;	/* min-max (px) */
    int             cont_x;	/* contatore  */
    int             prec_x;	/* prec. punto */
    int             prec_y;	/* prec. punto */
    int             dove_x;	/* togliere x */
    int             dove_y;	/* riferim. y */
    int             ind_p_flag;	/* inizializzaz. */
    struct mem_ind_prest *p_first;	/* punt. primo */
    struct mem_ind_prest *p_last;	/* punt. ultimo */
    struct res_object *res;	/* punt. risult. */
    struct ind_prest_list *next;
};


struct mem_ind_prest {
    float           val;
    float           tem;
    struct mem_ind_prest *next;
};


Pixmap  i_p_px = (Pixmap)NULL;


static struct ind_prest_list *head = NULL;
int             show_result_flag = FALSE;
int             pulisci_flag = FALSE;

#define IND_P_LIM 14
#define IND_P_X_LIM 19
#define ARR 0.5


/* predeclaration */
static void ind_prestazione_list(struct ind_prest_list *pp);
static void remove_mem_list(struct ind_prest_list *pp);

static void repaint_indprest(Widget canvas,
                             XtPointer closure,
                             XtPointer call_data) {
    DrawingSurfacePun paint_window = (DrawingSurfacePun)closure;

    struct ind_prest_list *pp = head;

    while (pp->pw != paint_window) pp = pp->next;
    ind_prestazione_list(pp);
}


void write_pt(struct ind_prest_list *pp,
              int            *pt_x,
              int            *pt_y) {
#ifdef DEBUG
    fprintf(stderr, "      Start write_pt\n");
#endif // DEBUG
    if (!pp->ind_p_flag) {
        DrawIntVector(pp->prec_x, *pt_y, *pt_x, *pt_y, SET, pp->pw);
        pp->prec_x = *pt_x;
        pp->prec_y = *pt_y;
    }
    else {
        DrawIntVector(pp->prec_x, pp->prec_y, *pt_x, *pt_y, SET, pp->pw);
        pp->prec_x = *pt_x;
        pp->prec_y = *pt_y;
    }
#ifdef DEBUG
    fprintf(stderr, "      End write_pt\n");
#endif // DEBUG
}



void memorizza(struct ind_prest_list *pp) {
    struct mem_ind_prest *new;

#ifdef DEBUG
    fprintf(stderr, "      Start memorizza\n");
#endif // DEBUG
    if (!pp->ind_p_flag) {
        pp->p_first->val = pp->res->value1;
        pp->p_first->tem = sim_time;
    }
    else {
        new = (struct mem_ind_prest *) emalloc(sizeof(struct mem_ind_prest));
        new->val = pp->res->value1;
        new->tem = sim_time;
        new->next = NULL;
        pp->p_last->next = new;
        pp->p_last = pp->p_last->next;
    }

#ifdef DEBUG
    fprintf(stderr, "      End memorizza\n");
#endif // DEBUG
}


void cambia_scala(struct ind_prest_list *pp) {
    struct mem_ind_prest *old;
    float           x, y;
    int             p_x, p_y, pr_x, pr_y;


#ifdef DEBUG
    fprintf(stderr, "        Start cambia_scala\n");
#endif // DEBUG
    gdiClearRect(pp->pw, 0, 0, pp->dim_x, pp->dim_y);
    old = pp->p_first;
    y = ((old->val - pp->min_y) / pp->sc_y) - ARR;
    p_y = pp->dove_y - y;
    pr_y = p_y;
    pr_x = IND_P_X_LIM;
    x = (old->tem / pp->sc_x) + ARR;
    p_x = pp->dove_x + x;
    while (old->next != NULL) {
        DrawIntVector(pr_x, pr_y, p_x, p_y, SET, pp->pw);
        old = old->next;
        pr_y = p_y;
        y = ((old->val - pp->min_y) / pp->sc_y) - ARR;
        p_y = pp->dim_y - 10 - y;
        p_y = pp->dove_y - y;
        pr_x = p_x;
        x = (old->tem / pp->sc_x) + ARR;
        p_x = pp->dove_x + x;
    }
    DrawIntVector(pr_x, pr_y, p_x, p_y, SET, pp->pw);
    pp->prec_x = p_x;
    pp->prec_y = p_y;
#ifdef DEBUG
    fprintf(stderr, "        End cambia_scala\n");
#endif // DEBUG
}


void cambia_blocco(struct ind_prest_list *pp,
                   int            *pt_x) {
    int             diff, dimx, dimy, init_x, init_y, xd, yd, fine_x, fine_y;
    struct mem_ind_prest *out;
    float           x;
    int             aux;

#ifdef DEBUG
    fprintf(stderr, "         Start cambia_blocco\n");
#endif // DEBUG
    diff = (*pt_x) - pp->dim_x + 1;
    dimx = (pp->dim_x - IND_P_X_LIM) - diff;
    dimy = pp->dim_y - 23;
    init_x = IND_P_X_LIM;
    init_y = IND_P_LIM;
    xd = init_x + diff;
    yd = init_y;
    fine_x = pp->dim_x - diff;
    fine_y = pp->dim_y - 10;
    i_p_px = (Pixmap)NULL;
    GetPixmap(&i_p_px, pp->pw, xd, yd, dimx, dimy);
    gdiClearRect(pp->pw, 0, 0, pp->dim_x, pp->dim_y);
    xv_scrivo(pp->pw, init_x, init_y, fine_x, fine_y, GXcopy, i_p_px, 0, 0);
    {
        /* Display * display = (Display*)XtDisplay(pp->pw); */
        Display *display = pp->pw->display;
        XFreePixmap(display, i_p_px);
    }
    pp->dove_x = pp->dove_x - diff;
    *pt_x = (*pt_x) - diff;
    pp->prec_x = pp->prec_x - diff;
    x = (pp->p_first->tem / pp->sc_x) + ARR;
    aux = pp->dove_x + x;
    while (aux <= IND_P_X_LIM) {
        out = pp->p_first;
        pp->p_first->next = out->next;
        pp->p_first = pp->p_first->next;
        free(out);
        x = (pp->p_first->tem / pp->sc_x) + ARR;
        aux = pp->dove_x + x;
    }
#ifdef DEBUG
    fprintf(stderr, "         End cambia_blocco\n");
#endif // DEBUG
}


void conv_num(struct ind_prest_list *pp,
              int            *nn,
              float          *nm, float *nM) {
    char            string[LINEMAX];
    char            ss[LINEMAX];
    char           *cp;
    float           aux;

#ifdef DEBUG
    fprintf(stderr, "                 Start conv_num\n");
#endif // DEBUG
    sprintf(string, "%e", pp->res->value1);
    for (cp = string; (*cp++) != 'e';);
    sscanf(cp, "%d", nn);
    sprintf(ss, "%c.%ce%d", string[0], string[2], *nn);
    sscanf(ss, "%f", nm);
    sprintf(ss, "0.1e%d", *nn);
    sscanf(ss, "%f", &aux);
    *nM = aux + *nm;
#ifdef DEBUG
    fprintf(stderr, "                 End conv_num\n");
#endif // DEBUG
}


void disegno_y(struct ind_prest_list *pp, int *pt_y) {
    int             aux2, aux, nn;
    float           delta, y, nm, nM;
    int             cambia_scala_flag;

#ifdef DEBUG
    fprintf(stderr, "      Start disegno_y\n");
#endif // DEBUG
    cambia_scala_flag = FALSE;
    if (!pp->ind_p_flag) {
        pp->valore = pp->res->value1;
        conv_num(pp, &nn, &nm, &nM);
        pp->min_y = nm;
        pp->max_y = nM;
        pp->sc_y = pp->max_y - pp->min_y;
        y = ((pp->res->value1 - pp->min_y) / pp->sc_y) - ARR;
    }
    else {
        nm = pp->min_y;
        nM = pp->max_y;
        delta = pp->res->value1 - pp->valore;
        if (delta < 0)
            delta = -delta;
        if ((delta != 0) && (delta < pp->sc_y)) {	/* test sul delta */
            pp->sc_y = delta;
            cambia_scala_flag = TRUE;
        }
        if (pp->min_y > pp->res->value1) {	/* diminuire min */
            conv_num(pp, &nn, &nm, &nM);
            nM = pp->max_y;
        }
        if (pp->max_y < pp->res->value1) {	/* aumentare max */
            conv_num(pp, &nn, &nm, &nM);
            nm = pp->min_y;
        }
        if ((nm - pp->min_y) < 0) {	/* minimo minore */
            aux = ((pp->min_y - nm) / pp->sc_y) - ARR;
            if ((pp->dove_y + aux) <= pp->dim_y - 10) {
                pp->dove_y = pp->dove_y + aux;
            }
            pp->min_y = nm;
            cambia_scala_flag = TRUE;
        }
        y = ((nM - pp->min_y) / pp->sc_y) - ARR;
        aux = pp->dove_y - y;
        if (aux < (IND_P_LIM + 1)) {	/* max troppo alto */
            aux2 = (IND_P_LIM + 1) - aux;
            if (pp->dove_y + aux2 >= pp->dim_y - 10) {
                pp->dove_y = 85;
                pp->max_y = nM;
                pp->sc_y = (pp->max_y - pp->min_y) / (pp->dove_y - (IND_P_LIM + 1));
            }
            else {
                pp->dove_y = pp->dove_y + aux2;
            }
            cambia_scala_flag = TRUE;
        }
        if (pp->max_y != nM)
            pp->max_y = nM;
        if (cambia_scala_flag)
            cambia_scala(pp);
        pp->diff_y = ((pp->max_y - pp->min_y) / pp->sc_y) - ARR;
        y = ((pp->res->value1 - pp->min_y) / pp->sc_y) - ARR;
    }
    pp->valore = pp->res->value1;
    *pt_y = pp->dove_y - y;
#ifdef DEBUG
    fprintf(stderr, "      End disegno_y\n");
#endif // DEBUG
}


void disegno_x(struct ind_prest_list *pp,
               int            *pt_x) {
    float           m, x;

#ifdef DEBUG
    fprintf(stderr, "      Start disegno_x\n");
#endif // DEBUG
    if (!pp->ind_p_flag) {
        pp->sc_x = sim_time - pp->tempo;
        x = 1.0;
        pp->cont_x = 1;
    }
    else {
        pp->cont_x = pp->cont_x + 1;
        m = sim_time / pp->cont_x;
        if ((2 * m < pp->sc_x) || (pp->sc_x * 2 <= m)) {
            pp->sc_x = m;
            cambia_scala(pp);
        }
        x = (sim_time / pp->sc_x) + ARR;
    }
    *pt_x = pp->dove_x + x;
    if (*pt_x >= pp->dim_x) {
        cambia_blocco(pp, pt_x);
    }
    pp->tempo = sim_time;
#ifdef DEBUG
    fprintf(stderr, "      End disegno_x\n");
#endif // DEBUG
}


void ind_prest_list_new() {
    struct ind_prest_list *p_h;

#ifdef DEBUG
    fprintf(stderr, "  Start ind_prest_list_new\n");
#endif // DEBUG
    for (p_h = head; p_h != NULL; p_h = p_h->next) {
        if ((sim_time != p_h->tempo) || (pulisci_flag)) {
            gdiClearRect(p_h->pw, 0, 0, p_h->dim_x, 7);
            ind_prestazione_list(p_h);
        }
    }
#ifdef DEBUG
    fprintf(stderr, "  End ind_prest_list_new\n");
#endif // DEBUG
}


void gradua_y(struct ind_prest_list *pp) {
    char            string[LINEMAX];
    char            ss[LINEMAX];
    int             i, prec = 0, d, aux = 0;
    int             grad_flag;
    float           ee, nf;

#ifdef DEBUG
    fprintf(stderr, "      Start gradua_y\n");
#endif // DEBUG
    if (pp->diff_y > 10) {
#ifdef DEBUG
        fprintf(stderr, "        ... entered if ( %d > 10 )\n", pp->diff_y);
#endif // DEBUG
        gdiClearRect(pp->pw, 0, 8, IND_P_X_LIM, pp->dim_y);
        sprintf(ss, "%.1e", pp->min_y);
        sprintf(string, "%c.%c", ss[0], ss[2]);
        gdiDrawText(pp->pw, 0, pp->dove_y, SET, sail_font, string);
        if (string[0] != '0') {
            sprintf(string, "%c%c%c", ss[4], ss[5], ss[6]);
            gdiDrawText(pp->pw, 0, aux + 7, SET, sail_font, string);
        }
        aux = pp->dove_y - pp->diff_y;
        sprintf(ss, "%.1e", pp->max_y);
        sprintf(string, "%c.%c", ss[0], ss[2]);
        gdiDrawText(pp->pw, 0, aux, SET, sail_font, string);
        if (string[0] != '0') {
            sprintf(string, "%c%c%c", ss[4], ss[5], ss[6]);
            gdiDrawText(pp->pw, 0, aux + 7, SET, sail_font, string);
        }
        DrawIntVector(IND_P_X_LIM - 4, pp->dove_y, IND_P_X_LIM, pp->dove_y, SET, pp->pw);
        DrawIntVector(IND_P_X_LIM - 4, aux, IND_P_X_LIM, aux, SET, pp->pw);
        grad_flag = FALSE;
        ee = pp->sc_y;
        while (!grad_flag) {
#ifdef DEBUG
            fprintf(stderr, "       ... outer while ee=%g\n", ee);
#endif // DEBUG
            sprintf(string, "%c", ss[0]);
            i = atoi(string);
            nf = ((i - pp->min_y) / ee) - ARR;
            aux = pp->dove_y - nf;
#ifdef DEBUG
            fprintf(stderr, "       ... i=%d, nf=%g, aux=%d, prec=%d\n", i, nf, aux, prec);
#endif // DEBUG
            if ((aux < pp->dove_y) && ((d = aux - prec) > 5)) {
                DrawIntVector(IND_P_X_LIM - 2, aux, IND_P_X_LIM, aux, SET, pp->pw);
                grad_flag = TRUE;
                prec = aux;
            }
            while (i > 0) {
                i = i - 1;
                nf = ((i - pp->min_y) / ee) - ARR;
                aux = pp->dove_y - nf;
                if ((aux < pp->dove_y) && ((d = aux - prec) > 5) && (d < 70)) {
                    DrawIntVector(IND_P_X_LIM - 2, aux, IND_P_X_LIM, aux, SET, pp->pw);
                    if (i > 2)
                        grad_flag = TRUE;
                    prec = aux;
                }
            }
#ifdef DEBUG
            fprintf(stderr, "        ... end inner while\n");
#endif // DEBUG
            if (nf >= 0.0)
                ee *= 10;
            else
                ee /= 10;
        }
#ifdef DEBUG
        fprintf(stderr, "        ... end outer while\n");
#endif // DEBUG
    }
#ifdef DEBUG
    fprintf(stderr, "      End gradua_y\n");
#endif // DEBUG
}


static void ind_prestazione_list(struct ind_prest_list *pp) {
    char            string[LINEMAX];
    int             pt_x;
    int             pt_y;


#ifdef DEBUG
    fprintf(stderr, "  Start ind_prestazione_list, show_result_flag=%d\n",
            show_result_flag);
#endif // DEBUG
    if (show_result_flag) {
        disegno_y(pp, &pt_y);
        disegno_x(pp, &pt_x);
        sprintf(string, "  %g", /* pp->res->tag, */ pp->res->value1);
        gdiDrawText(pp->pw, 6, 8, SET, sail_font, string);
        DrawIntVector(IND_P_X_LIM, IND_P_LIM, pp->dim_x, IND_P_LIM, SET, pp->pw);
        DrawIntVector(IND_P_X_LIM, IND_P_LIM, IND_P_X_LIM, pp->dim_y - 9, SET, pp->pw);
        DrawIntVector(IND_P_X_LIM, pp->dim_y - 9, pp->dim_x, pp->dim_y - 9, SET, pp->pw);
        gradua_y(pp);
        write_pt(pp, &pt_x, &pt_y);
        memorizza(pp);
        pp->ind_p_flag = TRUE;
    }
    else {
        gdiClearRect(pp->pw, 0, 0, pp->dim_x, pp->dim_y);
        sprintf(string, "  %s", pp->res->tag);
        gdiDrawText(pp->pw, 6, 8, SET, sail_font, string);
        DrawIntVector(IND_P_X_LIM, IND_P_LIM, pp->dim_x, IND_P_LIM, SET, pp->pw);
        DrawIntVector(IND_P_X_LIM, IND_P_LIM, IND_P_X_LIM, pp->dim_y - 9, SET, pp->pw);
        DrawIntVector(IND_P_X_LIM, pp->dim_y - 9, pp->dim_x, pp->dim_y - 9, SET, pp->pw);
        pp->ind_p_flag = FALSE;
        pp->tempo = 0.0;
        pp->cont_x = 0;
        pp->diff_y = 0;
        pp->dove_x = IND_P_X_LIM;
        pp->prec_x = IND_P_X_LIM;
        pp->dove_y = 50;
        remove_mem_list(pp);
    }
#ifdef DEBUG
    fprintf(stderr, "  End ind_prestazione_list\n");
#endif // DEBUG
}


extern DrawingSurfacePun new_popup(int, int, int, int, int, int, void(*)(), char *);

void ind_prestazione(struct res_object *res) {
    DrawingSurfacePun		poppw;
    char            string[LINEMAX];
    struct mem_ind_prest *m_i_p;
    int             x, y;
    struct ind_prest_list *new_i_p;

#ifdef DEBUG
    fprintf(stderr, "          Start ind_prestazione\n");
#endif // DEBUG
    new_i_p = (struct ind_prest_list *) emalloc(sizeof(struct ind_prest_list));
    new_i_p->next = head;
    head = new_i_p;
    new_i_p->res = res;
    new_i_p->dove_x = 0;
    new_i_p->dim_x = 140;
    new_i_p->dim_y = 95;
    new_i_p->dove_y = 50;
    new_i_p->tempo = 0.0;
    new_i_p->prec_x = IND_P_X_LIM;
    new_i_p->ind_p_flag = FALSE;
    new_i_p->cont_x = 0;
    x = res->center.x;
    y = res->center.y;
    sprintf(string, "%s diagram", res->tag);

    poppw = new_popup(x, y,
                      new_i_p->dim_x, new_i_p->dim_y,
                      new_i_p->dim_x + 10, new_i_p->dim_y + 10,
                      repaint_indprest, string);

    new_i_p->pw = poppw;
    m_i_p = (struct mem_ind_prest *) emalloc(sizeof(struct mem_ind_prest));
    m_i_p->val = 0.0;
    m_i_p->tem = 0.0;
    m_i_p->next = NULL;
    new_i_p->p_first = new_i_p->p_last = m_i_p;
    gdiClearRect(new_i_p->pw, 0, 0, new_i_p->dim_x, new_i_p->dim_y);
    ind_prestazione_list(new_i_p);
#ifdef DEBUG
    fprintf(stderr, "          Start ind_prestazione\n");
#endif // DEBUG
}


static void remove_mem_list(struct ind_prest_list *pp) {
    struct mem_ind_prest *p_f;

#ifdef DEBUG
    fprintf(stderr, "      Start remove_mem_list\n");
#endif // DEBUG
    while (pp->p_first->next != NULL) {
        p_f = pp->p_first;
        pp->p_first->next = p_f->next;
        pp->p_first = pp->p_first->next;
        free(p_f);
    }
#ifdef DEBUG
    fprintf(stderr, "      End remove_mem_list\n");
#endif // DEBUG
}


void remove_ind_prestazione(DrawingSurfacePun       pw_p) {
    struct ind_prest_list *l_p, *p_l_p;
#ifdef DEBUG
    fprintf(stderr, "  Start remove_ind_prestazione\n");
#endif // DEBUG

#define	PNUL	((struct ind_prest_list *) NULL)

    for (p_l_p = PNUL, l_p = head;
            l_p != PNUL && l_p->pw != pw_p;
            p_l_p = l_p, l_p = l_p->next);
    if (l_p != PNUL) {
        remove_mem_list(l_p);
        if (p_l_p != PNUL)
            p_l_p->next = l_p->next;
        else
            head = l_p->next;
        free(l_p);
    }
#ifdef DEBUG
    fprintf(stderr, "  End remove_ind_prestazione\n");
#endif // DEBUG
}

void popup_res(XButtonEvent          *ie) {
    struct net_object *near;
    int             object;

    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((near = near_obj(fix_x, fix_y, &object)) == NULL
            || (object != RESULT && object != PLACE))
        return;
    if (object == RESULT)
        ind_prestazione(near->results);
    else
        ShowTokenDistr(near->places, ie);
}
