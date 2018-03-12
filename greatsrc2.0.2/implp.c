#define ___IMPLP___
#	include "global.h"
#	include "draw.h"
#	include "showgdi.h"
#	include "optionsdialog.h"
#include "filer.h"
#include "implp.h"
#include "showdialog.h"

static struct coordinate *p_i_list = NULL;
static struct p_i_l_type *p_invars = NULL;
static int      num_pinv = 0;
static int      sumtok = 0, cur_tok;

static void ClearImplp(void) {
    struct p_i_l_type *from = p_invars;
    struct p_i_object *pip, *pin;

    if (p_invars == NULL)
        return;
    p_invars = p_invars->next;
    from->next = NULL;
    for (; p_invars != NULL; p_invars = from) {
        from = p_invars->next;
        pip = p_invars->i_l;
        free((char *) p_invars);
        for (; pip != NULL; pip = pin) {
            pin = pip->next;
            free((char *) pip);
        }
    }
}


static void CollectImplp(int complain) {
    FILE           *p_f;
    char            buf[LINEMAX], buf2[LINEMAX];
    struct p_i_object *pip;
    struct p_i_l_type *pil = NULL;
    int             i, j, m, np;
    struct place_object *plp;
    char message[1000];


    sprintf(buf, "%s.impl", GetCurrentFilename());
    {
        struct stat     stb, stb2;

        sprintf(buf2, "%s.net", edit_file);
        if ((stat(buf, &stb) < 0) || (stat(buf2, &stb2) < 0)
                || (stb2.st_mtime > stb.st_mtime)) {
            if (complain) {
                sprintf(message, "No up-to-date implicants for net %s", GetCurrentFilename());
                ShowInfoDialog(message, mainwin);
            }
            return;
        }
    }
    if ((p_f = fopen(buf, "r")) == NULL) {
        if (complain) {
            sprintf(message, "Can't open file %s for read", buf);
            ShowErrorDialog(message, mainwin);

        }
        return;
    }
    ClearImplp();
    fscanf(p_f, "%d\n", &num_pinv);
    p_invars = NULL;
    if (num_pinv <= 0)
        return;
    for (i = num_pinv; i-- > 0;) {
        if (p_invars == NULL)
            p_invars = pil = (struct p_i_l_type *) emalloc(sizeof(struct p_i_l_type));
        else {
            pil->next = (struct p_i_l_type *) emalloc(sizeof(struct p_i_l_type));
            pil = pil->next;
        }
        fscanf(p_f, "%d", &j);
        pil->i_l = pip = NULL;
        for (; j-- > 0;) {
            if (pip == NULL)
                pil->i_l = pip = (struct p_i_object *) emalloc(sizeof(struct p_i_object));
            else {
                pip->next = (struct p_i_object *) emalloc(sizeof(struct p_i_object));
                pip = pip->next;
            }
            fscanf(p_f, "%d %d", &m, &np);
            for (plp = netobj->places; --np > 0; plp = plp->next);
            pip->mult = m;
            pip->p_p = plp;
        }
        pip->next = NULL;
    }
    pil->next = p_invars;
    (void) fclose(p_f);
}

static void HighlightImplp(struct place_object *place) {
    struct coordinate *c_p;

    c_p = (struct coordinate *) emalloc(COORD_SIZE);
    c_p->next = p_i_list;
    p_i_list = c_p;
    c_p->x = place->center.x;
    c_p->y = place->center.y;
}


void DehighlightImplp(void) {
    struct coordinate *n_p;
    struct p_i_object *pip;

    if (p_i_list == NULL)
        return;
    dehighlight_list(p_i_list, (int)(place_radius + place_radius));
    for (; p_i_list != NULL; p_i_list = n_p) {
        n_p = p_i_list->next;
        free((char *) p_i_list);
    }
    if (p_invars == NULL)
        return;
    for (pip = p_invars->i_l; pip != NULL && pip->next != NULL;
            pip = pip->next);
    {
        int             nw_x, nw_y;

        nw_x = (int)(pip->p_p->center.x * zoom_level - trans_length);
        nw_y = (int)(pip->p_p->center.y * zoom_level - trans_length);
        /*	gdiDrawingAreaInvertRect(nw_x, nw_y, diam, diam); */
    }
    ClearImplp();
}


void ShowImplp(XButtonEvent *ie) {

    struct p_i_l_type *from;
    struct p_i_object *pip;
    int             first = TRUE;
    char            string[1024];
    struct place_object *ppp;
    int             np;
    char message[1000];



    StatusPrintf("Check implicit places with LEFT button");
    if (figure_modified) {
        ShowInfoDialog("Save the net description first!", mainwin);
        return;
    }
    DehighlightImplp();
    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    StatusPrintf("");
    ShowShowDialog("");
    if ((cur_place = near_place_obj(fix_x, fix_y)) == NULL) {
        return;
    }
    for (np = 1, ppp = netobj->places; ppp != cur_place;
            ppp = ppp->next, ++np);
#ifdef DEBUG
    fprintf(stderr, "GreatSPN: checkin place %d\n", np);
#endif
    Gspn = getenv("GREATSPN_SCRIPTDIR");
    if (strcmp(optGetHostname(), host_name) == 0)
        sprintf(string, "csh %s/implp %s %d", Gspn, GetCurrentFilename(), np);
    else
        sprintf(string, "rsh %s csh %s/implp %s %d", optGetHostname(), Gspn,
                GetCurrentFilename(), np);


#ifdef DEBUG
    fprintf(stderr, "GreatSPN: %s\n", string);
#endif

    if (system(string)) {

        sprintf(message, "Run time error for command: %s", string);
        ShowErrorDialog(message, mainwin);
        return;
    }
    CollectImplp(TRUE);
    if (p_invars == NULL) {
        sprintf(string, "Place %s IS NOT Structurally IMPLICIT",
                cur_place->tag);
        StatusPrintf(string);
        ShowShowDialog(string);

        return;
    }

    for (from = p_invars = p_invars->next;
            first || (p_invars != from);
            first = FALSE, p_invars = p_invars->next) {
        sumtok = 0;
        for (pip = p_invars->i_l; pip != NULL && pip->next != NULL;
                pip = pip->next) {
            if (pip->p_p->mpar == NULL)
                sumtok += (pip->p_p->m0 * pip->mult);
            else
                sumtok += (pip->p_p->mpar->value * pip->mult);
        }
        if (cur_place->mpar == NULL)
            cur_tok = cur_place->m0 * pip->mult;
        else
            cur_tok = cur_place->mpar->value * pip->mult;
        if (sumtok <= cur_tok)
            goto found;
    }
found:
    DisplayImplp();
}

void DisplayImplp(void) {
    struct p_i_object *pip;
    char            string[3000], buf[3000];
    int 			prevdim, dim;

    string[0] = '\0';
    prevdim = 0;
    for (pip = p_invars->i_l; pip != NULL && pip->next != NULL;
            pip = pip->next) {
        HighlightImplp(pip->p_p);
        if (pip != p_invars->i_l)
            strcat(string, " + ");
        if (pip->mult != 1) {
            sprintf(buf, "%d ", pip->mult);
            strcat(string, buf);
        }
        strcat(string, pip->p_p->tag);
        dim = (strlen(string)) / 60;

        if (dim > prevdim) {
            strcat(string, "\n");
            prevdim = dim;
        }
    }
    strcat(string, " ==> ");
    if (pip->mult != 1) {
        sprintf(buf, "%d ", pip->mult);
        strcat(string, buf);
    }
    strcat(string, pip->p_p->tag);
    HighlightImplp(pip->p_p);
    /*
    {
    int             nw_x, nw_y;
    int             diam = trans_length + trans_length;

    nw_x = (int) (pip->p_p->center.x * zoom_level - trans_length);
    nw_y = (int) (pip->p_p->center.y * zoom_level - trans_length);
    gdiDrawingAreaInvertRect(nw_x, nw_y, diam, diam);
    }
    */

    highlight_list(p_i_list, (int)(place_radius + place_radius),
                   FALSE, NULL);

    if (sumtok <= cur_tok) {
        sprintf(buf, " %d <= %d, i.e. %s IS IMPLICIT",
                sumtok, cur_tok, cur_place->tag);
        strcat(string, buf);
    }
    else {
        sprintf(buf, " %d > %d, i.e. %s is NOT IMPLICIT",
                sumtok, cur_tok, cur_place->tag);
        strcat(string, buf);
    }
    StatusPrintf(string);
    ShowShowDialog(string);
}
