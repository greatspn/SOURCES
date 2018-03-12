#define ___RES___
#include "res.h"
#include "draw.h"
#include "../WN/INCLUDE/compact.h"
#include "funct.h"

static struct net_object *near;
static int      object;
static char movingRes[300];
static struct place_object *place;
static struct trans_object *trans;
static struct group_object *group;
static struct res_object *n_res;
static struct probability *prob;
static struct probability *temp_pr;
static int      noplace, notanmark, novanmark;
static float    error, attime;
static int      thru_visible = FALSE;


static void ResTracking(int x, int y) {
    gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, movingRes);
    fix_x = x;
    fix_y = y;
    gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, movingRes);
}

static void DropRes(int x, int y) {
    float  xloc, yloc;

    xloc = x / zoom_level;
    yloc = y / zoom_level;

    gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y), XOR, canvas_font, movingRes);
    near->results->center.x = xloc;
    near->results->center.y = yloc;
    gdiDrawText(&mainDS, (int)(xloc), (int)(yloc), XOR, canvas_font, movingRes);

    reset_canvas_reader();
    menu_action();
}

void MoveRes(XButtonEvent  *ie) {
    float           xloc, yloc;

    xloc = event_x(ie) / zoom_level;
    yloc = event_y(ie) / zoom_level;
    near = near_obj(xloc, yloc, &object);
    if ((near == NULL) || (object != RESULT)) {
        return;
    }

    fix_x = near->results->center.x * zoom_level;
    fix_y = near->results->center.y * zoom_level;

    if (near->results->value1 >= 0.0) {
        if (near->results->value2 > 0.0)
            sprintf(movingRes, "%s=%g+/-%g", near->results->tag, near->results->value1, near->results->value2);
        else
            sprintf(movingRes, "%s=%g", near->results->tag, near->results->value1);
    }
    else
        strcpy(movingRes, near->results->tag);
    move = ResTracking;
    left = DropRes;
    SetMouseHelp("DROP the Result Definition", NULL, NULL);

    set_canvas_tracker();
}


void SetThruVisible(Boolean p) {
    thru_visible = p;
}

void ClearRes(void) {
    char            linebuf[LINEMAX];
    float           xloc, yloc;

    ClearLive();
    if (!res_visible)
        return;
    for (place = netobj->places; place != NULL; place = place->next)
        for (prob = place->distr, place->distr = NULL; prob != NULL;
                prob = temp_pr) {
            temp_pr = prob->next;
            free((char *) prob);
        }
    for (n_res = netobj->results; n_res != NULL; n_res = n_res->next) {
        ShowRes(n_res, CLEAR);
        n_res->value1 = -1.0;
        ShowRes(n_res, OR);
    }
    res_visible = FALSE;
    if (!thru_visible)
        return;
    for (trans = netobj->trans; trans != NULL; trans = trans->next) {
        xloc = (trans->center.x + trans->ratepos.x) * zoom_level;
        yloc = (trans->center.y + trans->ratepos.y + 10) * zoom_level;
        if (trans->through2 > 0.0)
            sprintf(linebuf, "Thru=%g+/-%g", trans->through1, trans->through2);
        else
            sprintf(linebuf, "Thru=%g", trans->through1);
        gdiDrawText(&mainDS, (int)(xloc), (int)(yloc),
                    CLEAR, canvas_font, linebuf);
        trans->through1 = 0.0;
    }
    for (group = netobj->groups; group != NULL; group = group->next) {
        for (trans = group->trans; trans != NULL; trans = trans->next) {
            xloc = (trans->center.x + trans->ratepos.x) * zoom_level;
            yloc = (trans->center.y + trans->ratepos.y + 10) * zoom_level;
            if (trans->through2 > 0.0)
                sprintf(linebuf, "Thru=%g+/-%g", trans->through1, trans->through2);
            else
                sprintf(linebuf, "Thru=%g", trans->through1);
            gdiDrawText(&mainDS, (int)(xloc), (int)(yloc),
                        CLEAR, canvas_font, linebuf);
            trans->through1 = 0.0;
        }
    }
    thru_visible = FALSE;
    RedisplayNet();
}


void ClearThroughputs(void) {
    char            linebuf[LINEMAX];
    float           xloc, yloc;

    if (!thru_visible)
        return;
    for (trans = netobj->trans; trans != NULL; trans = trans->next) {
        xloc = (trans->center.x + trans->ratepos.x) * zoom_level;
        yloc = (trans->center.y + trans->ratepos.y + 10) * zoom_level;
        if (trans->through2 > 0.0)
            sprintf(linebuf, "Thru=%g+/-%g", trans->through1, trans->through2);
        else
            sprintf(linebuf, "Thru=%g", trans->through1);
        gdiDrawText(&mainDS, (int)(xloc), (int)(yloc),
                    CLEAR, canvas_font, linebuf);
    }
    for (group = netobj->groups; group != NULL; group = group->next) {
        for (trans = group->trans; trans != NULL; trans = trans->next) {
            xloc = (trans->center.x + trans->ratepos.x) * zoom_level;
            yloc = (trans->center.y + trans->ratepos.y + 10) * zoom_level;
            if (trans->through2 > 0.0)
                sprintf(linebuf, "Thru=%g+/-%g", trans->through1, trans->through2);
            else
                sprintf(linebuf, "Thru=%g", trans->through1);
            gdiDrawText(&mainDS, (int)(xloc), (int)(yloc),
                        CLEAR, canvas_font, linebuf);
        }
    }
    thru_visible = FALSE;
}


void ShowRes(struct res_object *res, int op) {
    char            string[2 * TAG_SIZE + 2];
    float           loc_x, loc_y;

    loc_x = res->center.x * zoom_level;
    loc_y = res->center.y * zoom_level;
    if (res->value1 >= 0.0) {
        if (res->value2 > 0.0)
            sprintf(string, "%s=%g+/-%g", res->tag, res->value1, res->value2);
        else
            sprintf(string, "%s=%g", res->tag, res->value1);
    }
    else
        strcpy(string, res->tag);
    gdiDrawText(&mainDS, (int)loc_x, (int)loc_y, op, canvas_font, string);
}

void PutRes(struct res_object *res, int op, DrawingSurfacePun ds, int x_off, int y_off) {
    char            string[2 * TAG_SIZE + 2];
    float           loc_x, loc_y;

    loc_x = (res->center.x - x_off) * zoom_level;
    loc_y = (res->center.y - y_off) * zoom_level;
    if (res->value1 >= 0.0) {
        if (res->value2 > 0.0)
            sprintf(string, "%s=%g+/-%g", res->tag, res->value1, res->value2);
        else
            sprintf(string, "%s=%g", res->tag, res->value1);
    }
    else
        strcpy(string, res->tag);
    gdiDrawText(ds, (int)loc_x, (int)loc_y, op, canvas_font, string);
}


void TypeResDef(struct res_object *res) {
    char            string[10000];

    if (res->text == NULL)
        return;
    sprintf(string, "%s : \n%s", res->tag, res->text);
    ShowShowDialog(string);

}


void ShowTokenDistr(struct place_object *place, XButtonEvent *ie) {
    if ((prob = place->distr) == NULL) {
        char mess[2000];

        sprintf(mess, "No up_to_date token distribution for place %s", place->tag);
        ShowInfoDialog(mess, mainwin);
        return;
    }
    histogram(place, ie);
}


void TypeRes(XButtonEvent *ie) {
    struct net_object *near;
    int             object;

    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((near = near_obj(fix_x, fix_y, &object)) == NULL
            || (object != RESULT && object != PLACE))
        return;
    if (object == RESULT)
        TypeResDef(near->results);
    else
        ShowTokenDistr(near->places, ie);
}


void NewRes(XButtonEvent *ie) {
    struct net_object *near;
    int             object;

    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((near = near_obj(fix_x, fix_y, &object)) != NULL)
        return;

    ShowResChangeDialog(NULL, 0);

}


void EditRes(XButtonEvent *ie) {
    struct net_object *near;
    int             object;

    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((near = near_obj(fix_x, fix_y, &object)) == NULL
            || object != RESULT)
        return;
    ShowResChangeDialog(near->results, 2);

}

void DeleteResult(XButtonEvent *ie) {
    float           x_loc, y_loc;
    int             object;
    struct net_object *near;

    x_loc = event_x(ie) / zoom_level;
    y_loc = event_y(ie) / zoom_level;
    if ((near = near_obj(x_loc, y_loc, &object)) == NULL
            || object != RESULT) {
        return;
    }
    free_bkup_netobj();
    SetModify();
    bkup_netobj[0] = create_net();
    bkup_netobj[0]->results = near->results;
    RemoveResult(near->results);
    near->results->next = NULL;
    ShowRes(near->results, CLEAR);
}

void RemoveResult(struct res_object *result) {
    struct res_object *last_res;

    if (netobj->results == result) {
        netobj->results = result->next;
        return;
    }
    for (last_res = netobj->results; last_res->next != NULL;
            last_res = last_res->next)
        if (last_res->next == result) {
            last_res->next = result->next;
            return;
        }
}

static void GetPascalReal(FILE *file, char *num) {
    register int    i;

    for (i = 0; i++ < 8; * (num++) = getc(file));
}

static void GetTokenDistr(FILE *file, int sim) {
    register int    i;
    int             min_t, max_t;
    double          rr;

    for (place = netobj->places; place != NULL; place = place->next) {
        GetPascalReal(file, (char *)&rr);
        min_t = (int)(rr + 0.5);
        GetPascalReal(file, (char *)&rr);
        max_t = (int)(rr + 0.5);
        place->distr = prob = (struct probability *) emalloc(PROB_SIZE);
        prob->prob = 0.0;
        prob->val = -1;
        temp_pr = prob->next = (struct probability *) emalloc(PROB_SIZE);
        temp_pr->val = -2;
        for (i = min_t; i <= max_t; i++) {
            temp_pr = temp_pr->next = (struct probability *) emalloc(PROB_SIZE);
            GetPascalReal(file, (char *)&rr);
            prob->prob += i * rr;
            temp_pr->prob = rr;
            temp_pr->val = i;
        }
        temp_pr->next = NULL;
    }
    if (sim)
        for (place = netobj->places; place != NULL; place = place->next) {
            GetPascalReal(file, (char *)&rr);
            place->distr->next->prob = rr;
        }
    else
        for (place = netobj->places; place != NULL; place = place->next) {
            rr = error * place->distr->prob;
            place->distr->next->prob = rr;
        }
}


static void GetThroughputs(void) {
    float           xloc, yloc;
    char            linebuf[LINEMAX];
    char           *c_p;

    if (!res_visible)
        return;
    if (thru_visible)
        return;

    if (rate_visible) {
        ShowRate(FALSE);
        rate_visible = FALSE;
        update_mode();
    }
    else
        EraseLive();

    thru_visible = TRUE;
    for (trans = netobj->trans; trans != NULL; trans = trans->next) {
        fgets(linebuf, LINEMAX, dfp);
        for (c_p = linebuf; * (c_p++) != '=';);
        if (sscanf(c_p, "%g +/- %g", &(trans->through1), &(trans->through2)) < 2)
            trans->through2 = -1.0;
        xloc = (trans->center.x + trans->ratepos.x) * zoom_level;
        yloc = (trans->center.y + trans->ratepos.y + 10) * zoom_level;
        if (trans->through2 > 0.0)
            sprintf(linebuf, "Thru=%g+/-%g", trans->through1, trans->through2);
        else
            sprintf(linebuf, "Thru=%g", trans->through1);
        gdiDrawText(&mainDS, (int)(xloc), (int)(yloc),
                    OR, canvas_font, linebuf);
    }
    for (group = netobj->groups; group != NULL; group = group->next) {
        for (trans = group->trans; trans != NULL; trans = trans->next) {
            fgets(linebuf, LINEMAX, dfp);
            for (c_p = linebuf; * (c_p++) != '=';);
            if (sscanf(c_p, "%g +/- %g", &(trans->through1), &(trans->through2)) < 2)
                trans->through2 = -1.0;
            xloc = (trans->center.x + trans->ratepos.x) * zoom_level;
            yloc = (trans->center.y + trans->ratepos.y + 10) * zoom_level;
            if (trans->through2 > 0.0)
                sprintf(linebuf, "Thru=%g+/-%g", trans->through1, trans->through2);
            else
                sprintf(linebuf, "Thru=%g", trans->through1);
            gdiDrawText(&mainDS, (int)(xloc), (int)(yloc),
                        OR, canvas_font, linebuf);
        }
    }
}

void ShowThroughputs(void) {
    char            filename[LINEMAX];
    char            linebuf[LINEMAX];

    if (!res_visible)
        return;
    if (thru_visible)
        return;
    sprintf(filename, "%s.sta", edit_file);
    {
        struct stat     stb, stb2;

        sprintf(linebuf, "%s.net", edit_file);
        if ((stat(filename, &stb) < 0) || (stat(linebuf, &stb2) < 0)
                || (stb2.st_mtime > stb.st_mtime)) {
            return;
        }
    }
    if ((dfp = fopen(filename, "r")) == NULL) {
        return;
    }
    for (n_res = netobj->results; n_res != NULL; n_res = n_res->next) {
        while (getc(dfp) != '\n');
    }
    GetThroughputs();
    (void) fclose(dfp);
}


void CollectRes(int complain) {
    FILE           *fff;
    int             ii, norec;

    char            filename[LINEMAX];
    char            linebuf[LINEMAX];
    char           *c_p;
    double          rr;
    char message[2000];

    ClearRes();
    if (complain > 0) {
        sprintf(filename, "%s.rgr_aux", edit_file);
        if ((fff = fopen(filename, "r")) == NULL) {

            sprintf(message, "Error : can't open file %s for read", filename);
            ShowErrorDialog(message, mainwin);
            return;
        }
        fscanf(fff, "toptan= %d\n", &notanmark);
        fscanf(fff, "topvan= %d\n", &novanmark);
        fscanf(fff, "maxmark= %d\n", &ii);
        (void) fclose(fff);
        novanmark = ii - novanmark;
        sprintf(message, "the RS contains %d Tangible and %d Vanishing",
                notanmark, novanmark);
        ShowInfoDialog(message, mainwin);
        if (complain == 1) {
            unsigned long   ul;
            int             nolive, nodead, notrans;
            sprintf(filename, "%s.livlck", edit_file);
            if ((fff = fopen(filename, "r")) == NULL) {
                sprintf(message, "Error : can't open file %s for read", filename);
                ShowErrorDialog(message, mainwin);
                return;
            }
            load_compact(&ul, fff);
            nolive = ul;
            load_compact(&ul, fff);
            nodead = ul;

            if (nolive && nodead) {
                for (norec = nolive + nodead; norec--;  load_compact(&ul, fff));
            }
            load_compact(&ul, fff);
            notrans = ul;
            (void) fclose(fff);
            if (nolive == 1 && !nodead && !notrans)
                ShowInfoDialog("the Initial Marking is a Home State", mainwin);

            else if (nolive == 1 && !nodead) {
                sprintf(message, "M0 is transient; Home Space has cardinality %d", notanmark - notrans);
                ShowInfoDialog(message, mainwin);
            }
            else {
                sprintf(message, "NO Home Space: %d Livelocks, %d Deadlocks, and %d transient states",
                        nolive, nodead, notrans);
                ShowInfoDialog(message, mainwin);
            }
            if (LoadLive() > 0)
                ShowInfoDialog("the net is LIVE", mainwin);
            else if (LoadLive() < 0) {
                ShowErrorDialog("Couldn't open .liveness file !", mainwin);
                return;
            }
        }
    }
    else if (LoadLive() == 0)
        ShowInfoDialog("the net is NOT LIVE", mainwin);
    sprintf(filename, "%s.sta", edit_file);
    {
        struct stat     stb, stb2;

        sprintf(linebuf, "%s.net", edit_file);
        if ((stat(filename, &stb) < 0) || (stat(linebuf, &stb2) < 0)
                || (stb2.st_mtime > stb.st_mtime)) {
            if (complain > 1 || complain < 0) {

                sprintf(message, "No up-to-date result for net %s", edit_file);
                ShowInfoDialog(message, mainwin);
            }
            return;
        }
    }
    if ((dfp = fopen(filename, "r")) == NULL) {
        if (complain > 1 || complain < 0) {
            sprintf(message, "Error : can't open file %s for read", filename);
            ShowErrorDialog(message, mainwin);
        }
        return;
    }
    for (n_res = netobj->results; n_res != NULL; n_res = n_res->next) {
        fgets(linebuf, LINEMAX, dfp);
        for (c_p = linebuf; * (c_p++) != '=';);
        ShowRes(n_res, CLEAR);
        if (sscanf(c_p, "%g +/- %g", &(n_res->value1), &(n_res->value2)) < 2)
            n_res->value2 = -1.0;
        ShowRes(n_res, SET);
    }
    res_visible = (complain >= 0) || (cur_object == RESULT);
    GetThroughputs();
    (void) fclose(dfp);
    for (place = netobj->places, noplace = 0; place != NULL;
            place = place->next, noplace++);
    sprintf(filename, "%s.mpd", edit_file);
    if ((dfp = fopen(filename, "r")) == NULL) {
        /* simulation results */
        if (complain > 0) {
            sprintf(message, "Error : can't open file %s for read", filename);
            ShowErrorDialog(message, mainwin);
            return;
        }
        sprintf(filename, "%s.tpd", edit_file);
        if ((dfp = fopen(filename, "r")) != NULL) {
            GetTokenDistr(dfp, TRUE);
            GetPascalReal(dfp, (char *)&rr);
            attime = -1.0;
            (void) fclose(dfp);
        }
    }
    else {
        /* analytic results */
        if (complain < 0) {
            sprintf(message, "Error : can't open file %s for read", filename);
            ShowErrorDialog(message, mainwin);
            return;
        }
        GetPascalReal(dfp, (char *)&rr);
        error = -rr;
        GetPascalReal(dfp, (char *)&rr);
        attime = rr;
        (void) fclose(dfp);
        if (complain) {
            if (attime > 0.0) {
                sprintf(message, "the transient solution at time %g has precision %.2g", attime, error);
            }
            else {
                sprintf(message, "the steady-state solution has precision %.2g", error);
            }
            ShowInfoDialog(message, mainwin);
        }
        sprintf(filename, "%s.tpd", edit_file);
        if ((dfp = fopen(filename, "r")) != NULL) {
            GetTokenDistr(dfp, FALSE);
            (void) fclose(dfp);
        }
    }
}
