#define ___ECS___
#	include "global.h"
#	include "draw.h"
#	include "showgdi.h"
#include "ecs.h"
#include "Canvas.h"
#include "filer.h"
#include "showdialog.h"

static FILE           *p_f;
static struct coordinate *t_i_list = NULL;
static struct t_i_l_type *t_invars = NULL;
static int      num_ecs = 0;
static int jj;

void ClearEcs(void) {
    struct t_i_l_type *from = t_invars;
    struct t_i_object *tip, *pin;

    if (t_invars == NULL)
        return;
    t_invars = t_invars->next;
    from->next = NULL;
    for (; t_invars != NULL; t_invars = from) {
        from = t_invars->next;
        tip = t_invars->i_l;
        free((char *) t_invars);
        for (; tip != NULL; tip = pin) {
            pin = tip->next;
            free((char *) tip);
        }
    }
}

static void NextConfYESCB(Widget w, XtPointer client_data, XtPointer call_data) {
    DehighlightEcs();
    (void) fclose(p_f);
    redisplay_canvas();
}



static void EndConf(char *answ) {
    DehighlightEcs();
#ifdef DEBUG
    fprintf(stderr, "  ... end_conf: closing p_f\n");
#endif
    (void) fclose(p_f);
    redisplay_canvas();
}


static void NextConfNOCB(Widget w, XtPointer client_data, XtPointer call_data) {
    struct t_i_l_type *from;
    struct t_i_object *tip;
    struct group_object *glp;
    struct trans_object *tlp;
    struct place_object *plp;
    struct arc_object *alp;
    int             ii = num_ecs;

    DehighlightEcs();
    gdiClearDrawingArea();
#ifdef DEBUG
    fprintf(stderr, "  ... next_conf: drawing places\n");
#endif
    for (plp = netobj->places; plp != NULL; plp = plp->next)
        DrawPlace(plp, SET);
#ifdef DEBUG
    fprintf(stderr, "  ... next_conf: searching ecs\n");
#endif
    for (from = t_invars; --ii; from = from->next);
#ifdef DEBUG
    fprintf(stderr, "  ... next_conf: drawing trans\n");
#endif
    for (tip = from->i_l; tip != NULL; tip = tip->next) {
        DrawTrans(tip->t_p, SET);
#ifdef DEBUG
        fprintf(stderr, "     ... next_conf: drawing arcs\n");
#endif
        for (alp = netobj->arcs; alp != NULL; alp = alp->next)
            if (alp->trans == tip->t_p)
                DrawArc(alp, SET);
    }
    if ((ii = fscanf(p_f, "%d ", &num_ecs)) < 1) {
        fprintf(stderr, "ERROR reading num_ecs (%d)!\n", ii);
    }
    if ((ii = fscanf(p_f, "%d ", &jj)) < 1) {
        fprintf(stderr, "ERROR reading jj (%d)!\n", ii);
    }
#ifdef DEBUG
    fprintf(stderr, "  ... next_conf: searching trans %d (num_ecs=%d)\n", jj, num_ecs);
#endif
    for (jj--, tlp = netobj->trans;
            (tlp != NULL) && jj; jj--, tlp = tlp->next);
    for (glp = netobj->groups; tlp == NULL; glp = glp->next) {
        for (tlp = glp->trans; (tlp != NULL) && jj;
                jj--, tlp = tlp->next);
    }
#ifdef DEBUG
    fprintf(stderr, "     ... next_conf: found\n");
#endif
    {
        int             nw_x, nw_y;
        int             diam = trans_length + trans_length;

        nw_x = (int)(tlp->center.x * zoom_level - trans_length);
        nw_y = (int)(tlp->center.y * zoom_level - trans_length);
        gdiDrawingAreaInvertRect(nw_x, nw_y, diam, diam);
    }
#ifdef DEBUG
    fprintf(stderr, "   ... next_conf: highlighting\n");
#endif
    HighlightEcs(tlp);
    for (num_ecs -= 2; num_ecs > 0 ; --num_ecs) {
        if ((ii = fscanf(p_f, "%d ", &jj)) < 1) {
            fprintf(stderr, "ERROR reading jj (%d)!\n", ii);
        }
#ifdef DEBUG
        fprintf(stderr, "     ... next_conf: searching trans %d\n", jj);
#endif
        for (jj--, tlp = netobj->trans;
                (tlp != NULL) && jj > 0 ; jj--, tlp = tlp->next);
        for (glp = netobj->groups; tlp == NULL; glp = glp->next) {
            for (tlp = glp->trans; (tlp != NULL) && jj > 0 ;
                    jj--, tlp = tlp->next);
        }
#ifdef DEBUG
        fprintf(stderr, "     ... next_conf: found; drawing it\n");
#endif
        DrawTrans(tlp, SET);
#ifdef DEBUG
        fprintf(stderr, "     ... next_conf: drawing arcs\n");
#endif
        for (alp = netobj->arcs; alp != NULL; alp = alp->next)
            if (alp->trans == tlp)
                DrawArc(alp, SET);
#ifdef DEBUG
        fprintf(stderr, "     ... next_conf: highlighting\n");
#endif
        HighlightEcs(tlp);
    }
    if ((ii = fscanf(p_f, "%d\n", &jj)) < 1) {
        fprintf(stderr, "ERROR reading jj (%d)!\n", ii);
    }
#ifdef DEBUG
    fprintf(stderr, "   ... next_conf: searching trans %d\n", jj);
#endif
    for (jj--, tlp = netobj->trans;
            (tlp != NULL) && jj > 0 ; jj--, tlp = tlp->next);
    for (glp = netobj->groups; tlp == NULL; glp = glp->next) {
        for (tlp = glp->trans; (tlp != NULL) && jj > 0 ;
                jj--, tlp = tlp->next);
    }
#ifdef DEBUG
    fprintf(stderr, "   ... next_conf: found\n");
#endif
    {
        int             nw_x, nw_y;
        int             diam = trans_length + trans_length;

        nw_x = (int)(tlp->center.x * zoom_level - trans_length);
        nw_y = (int)(tlp->center.y * zoom_level - trans_length);
        gdiDrawingAreaInvertRect(nw_x, nw_y, diam, diam);
    }
#ifdef DEBUG
    fprintf(stderr, "   ... next_conf: inverted; highlighting\n");
#endif
    HighlightEcs(tlp);
#ifdef DEBUG
    fprintf(stderr, "   ... next_conf: highlighting list\n");
#endif
    highlight_list(t_i_list, (int)(trans_length), TRUE, NULL);
    if ((ii = fscanf(p_f, "%d ", &num_ecs)) < 1) {
        fprintf(stderr, "ERROR reading num_ecs (%d)!\n", ii);
    }
#ifdef DEBUG
    fprintf(stderr, "  end next_conf: num_ecs=%d\n", num_ecs);
#endif
    if (num_ecs) {
        ShowWarningDialog("Quit ?" ,
                          NextConfYESCB,
                          NextConfNOCB, "Yes", "No", 0);
    }
    else {
        DehighlightEcs();
        (void) fclose(p_f);
        redisplay_canvas();

        receiver = EndConf;
        ShowInfoDialog("Press OK to continue", mainwin);
    }
}

static void CheckConfusionYESCB(Widget w, XtPointer client_data, XtPointer call_data) {

#ifdef DEBUG
    fprintf(stderr, "  ... check_confusion: closing p_f\n");
#endif
    (void) fclose(p_f);

}
static void CheckConfusionNOCB(Widget w, XtPointer client_data, XtPointer call_data) {

#ifdef DEBUG
    fprintf(stderr, "Start check_confusion: num_ecs=%d\n", num_ecs);
#endif
    NextConfNOCB(NULL, NULL, NULL);

}

void CollectEcsSets(int complain) {
    char            buf[LINEMAX], buf2[LINEMAX];
    struct t_i_object *tip;
    struct t_i_l_type *til = NULL;
    int             i, nt;
    struct trans_object *tlp;
    struct group_object *glp;
    char string[300];

    sprintf(buf, "%s.ecs", GetCurrentFilename());
    sprintf(buf2, "%s.net", GetCurrentFilename());

    {
        struct stat     stb, stb2;

        if ((stat(buf, &stb) < 0) || (stat(buf2, &stb2) < 0)
                || (stb2.st_mtime > stb.st_mtime)) {
            if (complain) {
                sprintf(string, "No up-to-date E-conflit sets for net %s", GetCurrentFilename());
                ShowInfoDialog(string, frame_w);
            }
            return;
        }
    }
    if ((p_f = fopen(buf, "r")) == NULL) {
        if (complain) {
            sprintf(string, "Can't open file %s for read", buf);
            ShowErrorDialog(string, frame_w);
        }
        return;
    }
    ClearEcs();
    fscanf(p_f, "%d", &num_ecs);
    t_invars = NULL;
    if (num_ecs <= 0) {
        fclose(p_f);
        return;
    }
    while (num_ecs != 0) {
        if (t_invars == NULL)
            t_invars = til = (struct t_i_l_type *) emalloc(sizeof(struct t_i_l_type));
        else {
            til->next = (struct t_i_l_type *) emalloc(sizeof(struct t_i_l_type));
            til = til->next;
        }
        til->i_l = tip = NULL;
        for (i = num_ecs; i-- > 0;) {
            if (tip == NULL)
                til->i_l = tip = (struct t_i_object *) emalloc(sizeof(struct t_i_object));
            else {
                tip->next = (struct t_i_object *) emalloc(sizeof(struct t_i_object));
                tip = tip->next;
            }
            fscanf(p_f, "%d", &nt);
            for (nt--, tlp = netobj->trans;
                    (tlp != NULL) && (nt > 0); nt--, tlp = tlp->next);
            for (glp = netobj->groups; tlp == NULL; glp = glp->next) {
                for (tlp = glp->trans; (tlp != NULL) && (nt > 0); nt--, tlp = tlp->next);
            }
            tip->t_p = tlp;
        }
        tip->next = NULL;
        fscanf(p_f, "%d", &num_ecs);
    }
    while (getc(p_f) != '\n');

    til->next = t_invars;
#ifdef DEBUG
    fprintf(stderr, "  ... collect_ecsets: before if\n");
#endif
    if (complain && (fscanf(p_f, "%d ", &num_ecs) > 0)) {
#ifdef DEBUG
        fprintf(stderr, "  ... collect_ecsets: num_ecs=%d\n", num_ecs);
#endif
        if (num_ecs) {
            ShowWarningDialog("WARNING: some ECS may yield confusion! Do you want to check ?" ,
                              CheckConfusionNOCB,            CheckConfusionYESCB, "Yes", "No", 0);
            return;
        }
    }
#ifdef DEBUG
    fprintf(stderr, "  ... collect_ecsets: closing p_f\n");
#endif
    (void) fclose(p_f);
}

static void HighlightEcs(struct trans_object *trans) {
    struct coordinate *c_p;

    c_p = (struct coordinate *) emalloc(COORD_SIZE);
    c_p->next = t_i_list;
    t_i_list = c_p;
    c_p->x = trans->center.x;
    c_p->y = trans->center.y;
}


void DehighlightEcs(void) {
    struct coordinate *n_p;

    if (t_i_list == NULL)
        return;
    dehighlight_list(t_i_list, (int)(trans_length));
    for (; t_i_list != NULL; t_i_list = n_p) {
        n_p = t_i_list->next;
        free((char *) t_i_list);
    }
}


void ShowEcs(XButtonEvent *ie) {
    struct t_i_l_type *from;
    struct t_i_object *tip;
    int             first = TRUE;
    char            string[LINEMAX];
    struct group_object *group;

    StatusPrintf("");
    ShowShowDialog("");
    if (figure_modified)
        ClearEcs();
    if (t_invars == NULL) {
        ShowInfoDialog("Sorry, no up-to-date conflit sets available", mainwin);
        return;
    }
    DehighlightEcs();
    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((cur_trans = near_titrans_obj(fix_x, fix_y)) == NULL)
        if ((cur_trans = near_imtrans_obj(fix_x, fix_y, &group)) == NULL) {
            return;
        }
    for (from = t_invars = t_invars->next; first || (t_invars != from);
            first = FALSE, t_invars = t_invars->next) {
        for (tip = t_invars->i_l; tip != NULL; tip = tip->next)
            if (tip->t_p == cur_trans)
                goto found;
    }
    sprintf(string, "transition %s is not contained in any E-conflit sets", cur_trans->tag);
    ShowShowDialog(string);
    StatusPrintf(string);
    return;
found:
    DisplayEcs();
}

static void DisplayEcs(void) {
    struct t_i_object *tip;
    char            string[LINEMAX];
    int 			prevdim, dim;

    /*StatusPrintf("SEE conflit sets with LEFT button"); */
    string[0] = '\0';
    prevdim = 0;
    for (tip = t_invars->i_l; tip != NULL; tip = tip->next) {
        HighlightEcs(tip->t_p);
        if (tip != t_invars->i_l)
            strcat(string, " + ");
        strcat(string, tip->t_p->tag);
        dim = (strlen(string)) / 60;

        if (dim > prevdim) {
            strcat(string, "\n");
            prevdim = dim;
        }
    }
    highlight_list(t_i_list, (int)(trans_length), FALSE, NULL);
    ShowShowDialog(string);
    StatusPrintf(string);
}
