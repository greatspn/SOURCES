#define ___SC___
#include "global.h"
#include "showgdi.h"
#include "sc.h"
#include "showdialog.h"

static struct coordinate *t_i_list = NULL;
static struct t_i_object **trans_ptr = NULL;
static int      num_sc = 0, nn = 0;
static struct trans_object *ttt = NULL;

void ClearSc(void) {
    struct t_i_object *tip, *pin;
    int             i;

    if (trans_ptr == NULL)
        return;
    for (i = 0; i < nn; i++)
        if (trans_ptr[i] != NULL) {
            tip = trans_ptr[i];
            for (; tip != NULL; tip = pin) {
                pin = tip->next;
                free((char *) tip);
            }
        }
    free(trans_ptr);
    trans_ptr = NULL;
}


void CollectScsSets(int complain) {
    FILE           *p_f;
    char            buf[LINEMAX], buf2[LINEMAX];
    int             i, nt, x, y;
    struct trans_object *tlp;
    struct t_i_object *tip;
    struct group_object *glp;

    sprintf(buf, "%s.sc", edit_file);
    {
        struct stat     stb, stb2;
        int s1, s2;

        sprintf(buf2, "%s.net", edit_file);

        s1 = stat(buf, &stb);
        s2 = stat(buf2, &stb2);
        printf(".net=%zu .cc=%zu\n", stb2.st_mtime, stb.st_mtime);
        if ((s1 < 0) || (s2 < 0) || (stb2.st_mtime > stb.st_mtime)) {
            if (complain) {
                sprintf(buf, "No up-to-date structural conflit for net %s", edit_file);
                ShowInfoDialog(buf, mainwin);
            }
            return;
        }
    }
    if ((p_f = fopen(buf, "r")) == NULL) {
        if (complain) {
            sprintf(buf, "Can't open file %s for read", buf);
            ShowErrorDialog(buf, mainwin);
        }
        return;
    }
    ClearSc();
    i = 0;
    nn = 0;
    tlp = netobj->trans;
    while (tlp != NULL) {
        nn++;
        tlp = tlp->next;
    }
    glp = netobj->groups;
    while (glp != NULL) {
        for (tlp = glp->trans; tlp != NULL; tlp = tlp->next)
            nn++;
        glp = glp->next;
    }
    trans_ptr = (struct t_i_object **) emalloc(nn * sizeof(struct t_i_object));
    for (x = 0; x != nn; x++)
        trans_ptr[x] = NULL;
    while (fscanf(p_f, "%d\n", &num_sc) != EOF) {
        if (num_sc != 0) {
            trans_ptr[i] = tip = (struct t_i_object *) emalloc(sizeof(struct t_i_object));
            y = i;
            for (tlp = netobj->trans;
                    (tlp != NULL) && (y > 0); y--, tlp = tlp->next);
            for (glp = netobj->groups; tlp == NULL; glp = glp->next) {
                for (tlp = glp->trans; (tlp != NULL) && (y > 0);
                        y--, tlp = tlp->next);
            }
            tip->t_p = tlp;
            for (x = num_sc; x-- > 0;) {
                tip->next = (struct t_i_object *) emalloc(sizeof(struct t_i_object));
                tip = tip->next;
                fscanf(p_f, "%d", &nt);
                for (nt--, tlp = netobj->trans;
                        (tlp != NULL) && (nt > 0); nt--, tlp = tlp->next);
                for (glp = netobj->groups; tlp == NULL; glp = glp->next) {
                    for (tlp = glp->trans; (tlp != NULL) && (nt > 0);
                            nt--, tlp = tlp->next);
                }
                tip->t_p = tlp;
            }
            tip->next = NULL;
        }
        i++;
    }
    (void) fclose(p_f);

}
static void HighlightSc(struct trans_object *trans) {
    struct coordinate *c_p;

    c_p = (struct coordinate *) emalloc(COORD_SIZE);
    c_p->next = t_i_list;
    t_i_list = c_p;
    c_p->x = trans->center.x;
    c_p->y = trans->center.y;
}



void DehighlightSc(void) {
    struct coordinate *n_p;
    int             nw_x, nw_y;
    int             diam = trans_length + trans_length;

    if (t_i_list == NULL)
        return;
    nw_x = (int)(ttt->center.x * zoom_level - trans_length);
    nw_y = (int)(ttt->center.y * zoom_level - trans_length);
    gdiDrawingAreaInvertRect(nw_x, nw_y, diam, diam);
    dehighlight_list(t_i_list, (int)(trans_length));
    for (; t_i_list != NULL; t_i_list = n_p) {
        n_p = t_i_list->next;
        free((char *) t_i_list);
    }
}


void ShowSc(XButtonEvent *ie) {
    struct t_i_object *tip;
    int             i;
    char            string[LINEMAX];
    struct group_object *group;

    StatusPrintf("");
    ShowShowDialog("");
    if (figure_modified)
        ClearSc();
    if (trans_ptr == NULL) {
        ShowInfoDialog("Sorry, no up-to-date structural conflict available", mainwin);
        return;
    }
    DehighlightSc();
    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((cur_trans = near_titrans_obj(fix_x, fix_y)) == NULL)
        if ((cur_trans = near_imtrans_obj(fix_x, fix_y, &group)) == NULL) {
            return;
        }
    for (i = 0; i < nn; i++) {
        tip = trans_ptr[i];
        if (tip != NULL && tip->t_p == cur_trans)
            goto found;
    }
    sprintf(string, "transition %s is not contained in any structural conflit ", cur_trans->tag);
    ShowShowDialog(string);
    StatusPrintf(string);
    return;
found:
    ttt = cur_trans;
    DisplaySc(trans_ptr[i]);
}

static void DisplaySc(struct t_i_object *head) {
    struct t_i_object *tip;
    char            string[3000];
    int 			prevdim, dim;

    /*	put_msg(0,"SEE structural conflit with LEFT button");*/
    *string = '\0';
    prevdim = 0;
    for (tip = head; tip != NULL; tip = tip->next) {
        if (tip == head) {
            int             nw_x, nw_y;
            int             diam = trans_length + trans_length;

            sprintf(string, "%s is in structural conflit with:",
                    tip->t_p->tag);
            nw_x = (int)(ttt->center.x * zoom_level - trans_length);
            nw_y = (int)(ttt->center.y * zoom_level - trans_length);
            gdiDrawingAreaInvertRect(nw_x, nw_y, diam, diam);
        }
        else
            strcat(string, tip->t_p->tag);
        strcat(string, " ");
        HighlightSc(tip->t_p);
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
