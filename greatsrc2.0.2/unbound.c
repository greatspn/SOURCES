#define ___UNBOUND___
#include "global.h"
#include "consoledialog.h"
#include "showgdi.h"
#include "filer.h"
#include "unbound.h"
#include "showdialog.h"

static struct coordinate *t_i_list = NULL;
static struct t_i_l_type *t_invars = NULL;
static int      num_unbound = 0;
static int      place_flag = FALSE;
static FILE           *p_f;

void ClearUnbound(void) {
    struct t_i_l_type *from = t_invars;
    struct t_i_object *tip, *tip2;
    struct p_i_object *pip, *pip2;

    if (t_invars == NULL)
        return;
    DehighlightUnbound();
    t_invars = t_invars->next;
    from->next = NULL;
    for (; t_invars != NULL; t_invars = from) {
        from = t_invars->next;
        tip = t_invars->i_l;
        pip = t_invars->p_l;
        free((char *) t_invars);
        for (; tip != NULL; tip = tip2) {
            tip2 = tip->next;
            free((char *) tip);
        }
        for (; pip != NULL; pip = pip2) {
            pip2 = pip->next;
            free((char *) pip);
        }
    }
}


void CollectUnbound(int complain) {
    char            buf[LINEMAX], buf2[LINEMAX];
    struct t_i_l_type *til;
    struct trans_object *tlp;
    struct group_object *glp;
    char string[300];
    int 			prevdim, dim;

    strcpy(buf, GetCurrentFilename());
    strcpy(buf2, GetCurrentFilename());
    strcat(buf, ".unb");
    strcat(buf2, ".net");

    {
        struct stat     stb, stb2;

        if ((stat(buf, &stb) < 0) || (stat(buf2, &stb2) < 0)
                || (stb2.st_mtime > stb.st_mtime)) {
            if (complain) {
                sprintf(string, "No up-to-date Boundedness check for net %s", GetCurrentFilename());
                ShowInfoDialog(string, GetConsoleWidget());
            }
            return;
        }
    }
    if ((p_f = fopen(buf, "r")) == NULL) {
        if (complain) {
            sprintf(string, "Can't open file %s for read", buf);
            ShowErrorDialog(string, GetConsoleWidget());
        }
        return;
    }
    ClearUnbound();
    fscanf(p_f, "%d", &num_unbound);
    t_invars = til = NULL;
    if (num_unbound <= 0) {
        if (complain)
            ShowInfoDialog("The net is STRUCTURALLY BOUNDED", GetConsoleWidget());
        return;
    }
    ShowInfoDialog("The following places are NOT STRUCTURALLY BOUNDED:", GetConsoleWidget());
    string[0] = '\0';
    prevdim = 0;
    while (num_unbound) {
        int             ii;
        struct t_i_l_type *til_p =
            (struct t_i_l_type *) emalloc(sizeof(struct t_i_l_type));

        if (t_invars == NULL)
            t_invars = til = til_p;
        else
            til->next = til_p;
        til = til_p;
        til->i_l = NULL;
        til->p_l = NULL;
        while (num_unbound--) {
            struct p_i_object *pi_p;
            struct place_object *plp;
            struct coordinate *c_p;

            fscanf(p_f, "%d", &ii);
            for (ii--, plp = netobj->places;
                    (plp != NULL) && ii; ii--, plp = plp->next);
            pi_p = (struct p_i_object *) emalloc(sizeof(struct p_i_object));
            pi_p->next = til->p_l;
            til->p_l = pi_p;
            pi_p->place = plp;
            for (til_p = t_invars; til_p != NULL && til_p != til;
                    til_p = til_p->next)
                for (pi_p = til_p->p_l; pi_p != NULL; pi_p = pi_p->next)
                    if (pi_p->place == plp)
                        goto skip;
            strcat(buf, " ");
            strcat(buf, plp->tag);

            dim = (strlen(buf)) / 60;

            if (dim > prevdim) {
                strcat(buf, "\n");
                prevdim = dim;
            }

            c_p = (struct coordinate *) emalloc(COORD_SIZE);
            c_p->next = t_i_list;
            t_i_list = c_p;
            c_p->x = plp->center.x;
            c_p->y = plp->center.y;
skip:   /* don't highlight more than once */ ;
        }
        fscanf(p_f, "%d", &num_unbound);
        while (num_unbound--) {
            struct t_i_object *ti_p =
                (struct t_i_object *) emalloc(sizeof(struct t_i_object));

            ti_p->next = til->i_l;
            til->i_l = ti_p;
            fscanf(p_f, "%d", &ii);
            ti_p->mult = ii;
            fscanf(p_f, "%d", &ii);
            for (ii--, tlp = netobj->trans;
                    (tlp != NULL) && ii; ii--, tlp = tlp->next);
            for (glp = netobj->groups; tlp == NULL; glp = glp->next) {
                for (tlp = glp->trans; (tlp != NULL) && ii;
                        ii--, tlp = tlp->next);
            }
            ti_p->trans = tlp;
        }
        fscanf(p_f, "%d", &num_unbound);
    }
    til->next = t_invars;
    ShowInfoDialog(buf, GetConsoleWidget());
    (void) fclose(p_f);
    highlight_list(t_i_list, (int)(trans_length), FALSE, NULL);
}

static void HighlightUnbound(struct position center) {
    struct coordinate *c_p;

    c_p = (struct coordinate *) emalloc(COORD_SIZE);
    c_p->next = t_i_list;
    t_i_list = c_p;
    c_p->x = center.x;
    c_p->y = center.y;
}


void DehighlightUnbound(void) {
    struct coordinate *n_p;
    struct p_i_object *pip;

    if (t_i_list == NULL)
        return;
    dehighlight_list(t_i_list, (int)(trans_length));
    if (place_flag) {
        place_flag = FALSE;
        for (pip = t_invars->p_l; pip != NULL; pip = pip->next) {
            int             nw_x, nw_y;
            int             diam = trans_length + trans_length;

            nw_x = (int)(pip->place->center.x * zoom_level - trans_length);
            nw_y = (int)(pip->place->center.y * zoom_level - trans_length);
            gdiDrawingAreaInvertRect(nw_x, nw_y, diam, diam);
        }
    }
    for (; t_i_list != NULL; t_i_list = n_p) {
        n_p = t_i_list->next;
        free((char *) t_i_list);
    }
}


void ShowUnbound(XButtonEvent *ie) {
    struct t_i_l_type *from;
    struct p_i_object *pip;
    int             first = TRUE;
    char            string[LINEMAX];

    if (figure_modified)
        ClearUnbound();
    if (t_invars == NULL) {
        ShowInfoDialog("Sorry, no unbounded transition sequence available", mainwin);
        return;
    }
    DehighlightUnbound();
    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((cur_place = near_place_obj(fix_x, fix_y)) == NULL)
        return;
    for (from = t_invars = t_invars->next; first || (t_invars != from);
            first = FALSE, t_invars = t_invars->next) {
        for (pip = t_invars->p_l; pip != NULL; pip = pip->next)
            if (pip->place == cur_place)
                goto found;
    }
    sprintf(string, "place %s is structurally bounded", cur_place->tag);
    ShowShowDialog(string);
    return;
found:
    DisplayUnbound();
}

static void DisplayUnbound(void) {
    struct t_i_object *tip;
    struct p_i_object *pip;
    char            string[LINEMAX], buf[LINEMAX];
    int 			prevdim, dim;

    /*	put_msg(0,"Pick unbounded places with LEFT button"); */
    string[0] = '\0';
    prevdim = 0;
    t_i_list = NULL;
    place_flag = TRUE;
    for (pip = t_invars->p_l; pip != NULL; pip = pip->next) {
        int             nw_x, nw_y;
        int             diam = trans_length + trans_length;

        HighlightUnbound(pip->place->center);
        nw_x = (int)(pip->place->center.x * zoom_level - trans_length);
        nw_y = (int)(pip->place->center.y * zoom_level - trans_length);
        gdiDrawingAreaInvertRect(nw_x, nw_y, diam, diam);
    }
    for (tip = t_invars->i_l; tip != NULL; tip = tip->next) {
        HighlightUnbound(tip->trans->center);
        if (tip != t_invars->i_l)
            strcat(string, " + ");
        dim = (strlen(string)) / 60;
        if (dim > prevdim) {
            strcat(string, "\n");
            prevdim = dim;
        }

        if (tip->mult > 1) {
            sprintf(buf, "%d*", tip->mult);
            strcat(string, buf);
        }
        strcat(string, tip->trans->tag);
        dim = (strlen(string)) / 60;
        if (dim > prevdim) {
            strcat(string, "\n");
            prevdim = dim;
        }

    }
    highlight_list(t_i_list, (int)(trans_length), TRUE, NULL);
    ShowShowDialog(string);
}
