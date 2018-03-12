#define ___CC___
#	include "global.h"
#include "cc.h"
#include "filer.h"
#include "consoledialog.h"

static struct coordinate *t_i_list = NULL;
static struct trans_object *ttt = NULL;
static struct t_i_object **trans_ptr = NULL;
static int num_cc = 0, nn = 0;

static void ClearCc(void) {
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

void CollectCcSets(int complain) {
    FILE           *p_f;
    char            buf[LINEMAX], buf2[LINEMAX];
    int             i, nt, x, y;
    struct trans_object *tlp;
    struct t_i_object *tip;
    struct group_object *glp;
    char *edit_file;

    edit_file = GetCurrentFilename();

    sprintf(buf, "%s.cc", edit_file);
    {
        struct stat     stb, stb2;

        sprintf(buf2, "%s.net", edit_file);
        if ((stat(buf, &stb) < 0) || (stat(buf2, &stb2) < 0)
                || (stb2.st_mtime > stb.st_mtime)) {

            if (complain) {
                char message[1000];

                sprintf(message, "No up-to-date causal connection for net %s", edit_file);
                ShowInfoDialog(message, mainwin);
            }
            return;
        }
    }
    if ((p_f = fopen(buf, "r")) == NULL) {
        if (complain) {
            char message[1000];

            sprintf(message, "Can't open file %s for read", buf);
            ShowErrorDialog(message, GetConsoleWidget());
        }
        return;
    }
    ClearCc();
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
    while (fscanf(p_f, "%d\n", &num_cc) != EOF) {
        if (num_cc != 0) {
            trans_ptr[i] = tip = (struct t_i_object *) emalloc(sizeof(struct t_i_object));
            y = i;
            for (tlp = netobj->trans;
                    (tlp != NULL) && (y > 0); y--, tlp = tlp->next);
            for (glp = netobj->groups; tlp == NULL; glp = glp->next) {
                for (tlp = glp->trans; (tlp != NULL) && (y > 0);
                        y--, tlp = tlp->next);
            }
            tip->t_p = tlp;
            for (x = num_cc; x-- > 0;) {
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

static void HighlightCc(struct trans_object *trans) {
    struct coordinate *c_p;

    c_p = (struct coordinate *) emalloc(COORD_SIZE);
    c_p->next = t_i_list;
    t_i_list = c_p;
    c_p->x = trans->center.x;
    c_p->y = trans->center.y;
}


void DehighlightCc(void) {
    struct coordinate *n_p;
    int             rad = trans_length;

    if (t_i_list == NULL)
        return;
    highlight_item(&mainDS, ttt->center.x, ttt->center.y, rad);
    dehighlight_list(t_i_list, (int)(trans_length));
    for (; t_i_list != NULL; t_i_list = n_p) {
        n_p = t_i_list->next;
        free((char *) t_i_list);
    }
}


void ShowCc(XButtonEvent *ie) {
    struct t_i_object *tip;
    int             i;
    struct group_object *group;

    if (figure_modified)
        ClearCc();
    if (trans_ptr == NULL) {
        ShowInfoDialog("Sorry, no up-to-date causal connection available", mainwin);
        return;
    }
    DehighlightCc();
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
    StatusPrintf("transition %s is not causally connected", cur_trans->tag);
    return;
found:
    ttt = cur_trans;
    DisplayCc(trans_ptr[i]);
}

static void DisplayCc(struct t_i_object *head) {
    struct t_i_object *tip;
    char            string[LINEMAX];
    int             rad = trans_length;

    /*   put_msg(0,"SEE causal connection with LEFT button"); */
    string[0] = '\0';
    for (tip = head; tip != NULL; tip = tip->next) {
        if (tip == head) {
            sprintf(string, "%s  ==>  ", tip->t_p->tag);
            highlight_item(&mainDS, ttt->center.x, ttt->center.y, rad);
        }
        else
            strcat(string, tip->t_p->tag);
        strcat(string, " ");
        HighlightCc(tip->t_p);
        if (strlen(string) > 60) {
            next_msg(1, string);
            string[0] = '\0';
        }
    }
    highlight_list(t_i_list, (int)(trans_length), FALSE, NULL);
    next_msg(1, string);
}
