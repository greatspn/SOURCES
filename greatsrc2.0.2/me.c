#define ___ME___
#include "global.h"
#include "me.h"
#include "filer.h"
#include "layer.h"
#include "Canvas.h"
#include "showdialog.h"

static struct coordinate *t_i_list = NULL;
struct t_i_object **trans_ptr = NULL;
static int      num_me = 0, nn = 0;

void ClearMe(void) {
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


void CollectMesets(int complain) {
    int             i, nt;
    FILE           *p_f;
    char            buf[LINEMAX], buf2[LINEMAX];
    struct t_i_object *tip;
    struct trans_object *tlp, *tlpe;
    struct group_object *glp;
    char string[300];

    sprintf(buf, "%s.me", GetCurrentFilename());
    sprintf(buf2, "%s.net", GetCurrentFilename());
    {
        struct stat     stb, stb2;

        if ((stat(buf, &stb) < 0) || (stat(buf2, &stb2) < 0)
                || (stb2.st_mtime > stb.st_mtime)) {
            if (complain) {
                sprintf(string, "No up-to-date mutual exclusion for net %s", GetCurrentFilename());
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
    ClearMe();
    fscanf(p_f, "%d\n", &num_me);
    if (num_me <= 0) {
        fclose(p_f);
        return;
    }
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
    for (i = 0; i != nn; i++)
        trans_ptr[i] = NULL;
    while (num_me != 0) {
        i = num_me - 1;
        for (tlp = netobj->trans;
                (tlp != NULL) && (i > 0); i--, tlp = tlp->next);
        for (glp = netobj->groups; tlp == NULL; glp = glp->next) {
            for (tlp = glp->trans; (tlp != NULL) && (i > 0);
                    i--, tlp = tlp->next);
        }
        tlpe = tlp;
        i = num_me - 1;
        tip = trans_ptr[i];
        if (tip == NULL) {
            trans_ptr[i] = tip = (struct t_i_object *) emalloc(sizeof(struct t_i_object));
            tip->t_p = tlpe;
            tip->next = NULL;
        }
        fscanf(p_f, "%d", &nt);
        i = nt;
        while (tip->next != NULL)
            tip = tip->next;
        tip->next = (struct t_i_object *) emalloc(sizeof(struct t_i_object));
        tip = tip->next;
        for (i--, tlp = netobj->trans;
                (tlp != NULL) && (i > 0); i--, tlp = tlp->next);
        for (glp = netobj->groups; tlp == NULL; glp = glp->next) {
            for (tlp = glp->trans; (tlp != NULL) && (i > 0);
                    i--, tlp = tlp->next);
        }
        tip->t_p = tlp;
        tip->next = NULL;
        tip = trans_ptr[nt - 1];
        if (tip == NULL) {
            trans_ptr[nt - 1] = tip = (struct t_i_object *) emalloc(sizeof(struct t_i_object));
            tip->t_p = tlp;
            tip->next = NULL;
        }
        while (tip->next != NULL)
            tip = tip->next;
        tip->next = (struct t_i_object *) emalloc(sizeof(struct t_i_object));
        tip = tip->next;
        tip->t_p = tlpe;
        tip->next = NULL;
        fscanf(p_f, "%d\n", &num_me);
    }
    (void) fclose(p_f);
}


static void HighlightMe(struct trans_object *trans) {
    struct coordinate *c_p;

    c_p = (struct coordinate *) emalloc(COORD_SIZE);
    c_p->next = t_i_list;
    t_i_list = c_p;
    c_p->x = trans->center.x;
    c_p->y = trans->center.y;
}


void DehighlightMe(void) {
    struct coordinate *n_p;

    if (t_i_list == NULL)
        return;
    dehighlight_list(t_i_list, (int)(trans_length));
    for (; t_i_list != NULL; t_i_list = n_p) {
        n_p = t_i_list->next;
        free((char *) t_i_list);
    }
}


void ShowMe(XButtonEvent *ie) {
    int             i;
    char            string[LINEMAX];
    struct group_object *group;

    StatusPrintf("");
    ShowShowDialog("");
    if (figure_modified)
        ClearMe();
    if (trans_ptr == NULL) {
        ShowInfoDialog("Sorry, no up-to-date mutual exclusion available", mainwin);
        return;
    }
    DehighlightMe();
    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((cur_trans = near_titrans_obj(fix_x, fix_y)) == NULL)
        if ((cur_trans = near_imtrans_obj(fix_x, fix_y, &group)) == NULL) {
            return;
        }
    for (i = 0; i < nn; i++) {
        if (trans_ptr[i] != NULL && trans_ptr[i]->t_p == cur_trans)
            goto found;
    }
    sprintf(string, "transition %s is not contained in any mutual exclusion", cur_trans->tag);
    ShowShowDialog(string);
    StatusPrintf(string);
    return;
found:
    DisplayMe(trans_ptr[i]);
}

static void DisplayMe(struct t_i_object *head) {
    struct t_i_object *tip;
    char            string[LINEMAX];

    /*put_msg(0,"SEE mutual exclusion with LEFT button");*/
    string[0] = '\0';
    for (tip = head; tip != NULL; tip = tip->next) {
        HighlightMe(tip->t_p);
        strcat(string, " ");
        strcat(string, tip->t_p->tag);
        if (strlen(string) > 60) {
            next_msg(1, string);
            string[0] = '\0';
        }
    }
    highlight_list(t_i_list, (int)(trans_length), FALSE, NULL);
    ShowShowDialog(string);
    StatusPrintf(string);
}
