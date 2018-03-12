#define ___TINV___
#include "global.h"
#include "filer.h"
#include "tinv.h"
#include "showdialog.h"

static struct coordinate *t_i_list = NULL;
static struct t_i_l_type *t_invars = NULL;
static int      num_tinv = 0;

void ClearTinv(void) {
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


void CollectTinvar(int complain) {
    FILE           *p_f;
    char            buf[LINEMAX], buf2[LINEMAX];
    struct t_i_object *tip;
    struct t_i_l_type *til = NULL;
    int             i, j, m, nt;
    struct trans_object *tlp;
    struct group_object *glp;
    char string[300];

    strcpy(buf, GetCurrentFilename());
    strcpy(buf2, GetCurrentFilename());
    strcat(buf, ".tin");
    strcat(buf2, ".net");

    {
        struct stat     stb, stb2;

        if ((stat(buf, &stb) < 0) || (stat(buf2, &stb2) < 0)
                || (stb2.st_mtime > stb.st_mtime)) {
            if (complain) {
                sprintf(string, "No up-to-date T-invariants for net %s", GetCurrentFilename());
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
    ClearTinv();
    fscanf(p_f, "%d\n", &num_tinv);
    t_invars = NULL;
    if (num_tinv <= 0)
        return;
    for (i = num_tinv; i-- > 0;) {
        if (t_invars == NULL)
            t_invars = til = (struct t_i_l_type *) emalloc(sizeof(struct t_i_l_type));
        else {
            til->next = (struct t_i_l_type *) emalloc(sizeof(struct t_i_l_type));
            til = til->next;
        }
        fscanf(p_f, "%d", &j);
        til->i_l = tip = NULL;
        for (; j-- > 0;) {
            if (tip == NULL)
                til->i_l = tip = (struct t_i_object *) emalloc(sizeof(struct t_i_object));
            else {
                tip->next = (struct t_i_object *) emalloc(sizeof(struct t_i_object));
                tip = tip->next;
            }
            fscanf(p_f, "%d %d", &m, &nt);
            for (nt--, tlp = netobj->trans;
                    (tlp != NULL) && (nt > 0); nt--, tlp = tlp->next);
            for (glp = netobj->groups; tlp == NULL; glp = glp->next) {
                for (tlp = glp->trans; (tlp != NULL) && (nt > 0);
                        nt--, tlp = tlp->next);
            }
            tip->mult = m;
            tip->t_p = tlp;
        }
        tip->next = NULL;
    }
    til->next = t_invars;
    (void) fclose(p_f);
}

static void HighlightTinv(struct trans_object *trans) {
    struct coordinate *c_p;

    c_p = (struct coordinate *) emalloc(COORD_SIZE);
    c_p->next = t_i_list;
    t_i_list = c_p;
    c_p->x = trans->center.x;
    c_p->y = trans->center.y;
}


void DehighlightTinv(void) {
    struct coordinate *n_p;

    if (t_i_list == NULL)
        return;
    dehighlight_list(t_i_list, (int)(trans_length));
    for (; t_i_list != NULL; t_i_list = n_p) {
        n_p = t_i_list->next;
        free((char *) t_i_list);
    }
}


void ShowTinv(XButtonEvent *ie) {
    struct t_i_l_type *from;
    struct t_i_object *tip;
    int             first = TRUE;
    char            string[LINEMAX];
    struct group_object *group;

    ShowShowDialog("");
    StatusPrintf("");
    if (figure_modified)
        ClearTinv();
    if (t_invars == NULL) {
        ShowInfoDialog("Sorry, no up-to-date transition invariants available", mainwin);
        return;
    }
    DehighlightTinv();
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
    sprintf(string, "transition %s is not contained in any T-invariant", cur_trans->tag);
    ShowShowDialog(string);
    StatusPrintf(string);
    return;
found:
    DisplayTinv();
}

static void DisplayTinv(void) {
    struct t_i_object *tip;
    char            string[LINEMAX], buf[LINEMAX];
    int 			prevdim, dim;

    /*    put_msg(0,"SEE transition invariants with LEFT button");*/
    string[0] = '\0';
    prevdim = 0;
    for (tip = t_invars->i_l; tip != NULL; tip = tip->next) {
        HighlightTinv(tip->t_p);
        if (tip != t_invars->i_l)
            strcat(string, " + ");
        if (tip->mult != 1) {
            sprintf(buf, "%d ", tip->mult);
            strcat(string, buf);
        }
        strcat(string, tip->t_p->tag);
        dim = (strlen(string)) / 60;

        if (dim > prevdim) {
            strcat(string, "\n");
            prevdim = dim;
        }
    }
    highlight_list(t_i_list, (int)(trans_length), FALSE, NULL);
    StatusPrintf(string);
    ShowShowDialog(string);
}
