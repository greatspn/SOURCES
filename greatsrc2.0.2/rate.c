#define ___RATE___
#	include "global.h"
#	include "liveness.h"
#	include "showgdi.h"
#	include "showdialog.h"
#	include "ratedialog.h"
#	include "transdialog.h"
#	include "res.h"
#include "rate.h"

static struct net_object *near;
static int      object;
static char     t_enabl_string[LINEMAX];
static char     t_rate_string[LINEMAX];


void ShowRpar(struct rpar_object *rpar, int op) {
    char            string[2 * TAG_SIZE + 2];
    float           xloc, yloc;

    if (!IsLayerListVisible(rpar->layer))
        return;
    xloc = rpar->center.x * zoom_level;
    yloc = rpar->center.y * zoom_level;
    sprintf(string, "%s=%f", rpar->tag, rpar->value);
    gdiDrawText(&mainDS, (int) xloc, (int) yloc, op, canvas_font, string);
}

void ShowTransRate(struct trans_object *trans, int op) {
    char            rat[TAG_SIZE];
    float           xloc, yloc;


    if (!IsLayerListVisible(trans->layer))
        return;

    xloc = (trans->center.x + trans->ratepos.x) * zoom_level;
    yloc = (trans->center.y + trans->ratepos.y) * zoom_level;

#ifdef DEBUG
    fprintf(stderr, "\nshow_trans_rate %s enabl=%d\n", trans->tag, trans->enabl);
#endif

    if (trans->kind == DETERMINISTIC && trans->enabl == 1) {
        sprintf(rat, "determ.");
        gdiDrawText(&mainDS, (int)(xloc), (int)(yloc),
                    /* INVERT */ op, canvas_font, rat);
    }
    else if (trans->enabl != 1) {
        if (trans->enabl == 0)
            sprintf(rat, "inf-server");
        else if (trans->enabl > 0)
            sprintf(rat, "%3d-server", trans->enabl);
        else
            sprintf(rat, "%3d-en-dep", -(trans->enabl));
        gdiDrawText(&mainDS, (int)(xloc), (int)(yloc),
                    /* INVERT */ op, canvas_font, rat);
    }
    if (trans->mark_dep != NULL)
        sprintf(rat, "..m-d");
    else if (trans->rpar != NULL)
        sprintf(rat, "%s", (trans->rpar)->tag);
    else if (trans->enabl >= 0)
        sprintf(rat, "%f", trans->fire_rate.ff);
    else
        sprintf(rat, "%f", trans->fire_rate.fp[0]);
    gdiDrawText(&mainDS, (int)(xloc), (int)(yloc + 10),
                /* INVERT */ op, canvas_font, rat);
}

void ShowRate(int show) {
    struct group_object *group;
    int             op;

    op = CLEAR;
    if (show) {
        ClearThroughputs();
        EraseLive();
        op = SET;
    }
    for (last_trans = netobj->trans; last_trans != NULL;
            last_trans = last_trans->next)
        ShowTransRate(last_trans, op);
    for (group = netobj->groups; group != NULL; group = group->next)
        for (last_trans = group->trans; last_trans != NULL;
                last_trans = last_trans->next)
            ShowTransRate(last_trans, op);
}


void TypeRate(XButtonEvent   *ie) {
    float           xloc, yloc;
    int             ii;
    char 			buffer[10000];

    xloc = event_x(ie) / zoom_level;
    yloc = event_y(ie) / zoom_level;
    if ((near = near_obj(xloc, yloc, &object)) == NULL ||
            (object != EXTRANS && object != DETRANS && object != IMTRANS))
        return;

    if (near->trans->mark_dep != NULL)
        type_md_def(near->trans);
    else if (near->trans->rpar != NULL) {
        sprintf(buffer, "transition %s rate : parameter %s", near->trans->tag, near->trans->rpar->tag);
        ShowShowDialog(buffer);
    }
    else if ((ii = near->trans->enabl) >= 0) {
        sprintf(buffer, "transition %s rate : %f", near->trans->tag, near->trans->fire_rate.ff);
        ShowShowDialog(buffer);
    }
    else {
        char             rr[20];
        sprintf(buffer, "transition %s rate : %f en_dep: 1.0", near->trans->tag, near->trans->fire_rate.fp[0]);
        ii = -ii;
        while (--ii) {
            sprintf(rr, ", \n%f",	near->trans->fire_rate.fp[-(ii + (near->trans->enabl))]);
            strcat(buffer, rr);
        }
        ShowShowDialog(buffer);
    }
}

void NewRpar(XButtonEvent   *ie) {

    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((near = near_obj(fix_x, fix_y, &object)) != NULL)
        return;
    ShowRateChangeDialog(NULL, 0);
}


void EditRate(XButtonEvent   *ie) {
    float           xloc, yloc;

    xloc = event_x(ie) / zoom_level;
    yloc = event_y(ie) / zoom_level;
    if ((near = near_obj(xloc, yloc, &object)) == NULL
            || (object != RPAR && object != EXTRANS && object != DETRANS && object != IMTRANS))
        return;

    if (object == RPAR)
        ShowRateChangeDialog(near->rpars, 2);
    else
        ShowTransChangeDialog(near->trans, 2);
}


void DeleteRpar(XButtonEvent   *ie) {
    float           x_loc, y_loc;
    int             object;
    struct net_object *near;
    struct group_object *group;
    struct trans_object *trans;

    x_loc = event_x(ie) / zoom_level;
    y_loc = event_y(ie) / zoom_level;
    if ((near = near_obj(x_loc, y_loc, &object)) == NULL
            || object != RPAR) {
        return;
    }
    ShowRpar(near->rpars, CLEAR);
    if (!IsLayerVisible(WHOLE_NET_LAYER)) {
        ResetVisibleLayers(near->rpars->layer);
        return;
    }
    free_bkup_netobj();
    SetModify();
    bkup_netobj[0] = create_net();
    for (trans = netobj->trans; trans != NULL; trans = trans->next)
        if (trans->rpar == near->rpars) {
            ShowTransRate(trans, CLEAR);
            trans->rpar = NULL;
            ShowTransRate(trans, CLEAR);
            ClearRes();
        }
    for (group = netobj->groups; group != NULL; group = group->next)
        for (trans = group->trans; trans != NULL;
                trans = trans->next)
            if (trans->rpar == near->rpars) {
                ShowTransRate(trans, CLEAR);
                trans->rpar = NULL;
                ShowTransRate(trans, CLEAR);
                ClearRes();
            }
    bkup_netobj[0]->rpars = near->rpars;
    RemoveRpar(near->rpars);
    near->rpars->next = NULL;
}

void RemoveRpar(struct rpar_object *rpar) {
    struct rpar_object *last_rpar;

    if (netobj->rpars == rpar) {
        netobj->rpars = rpar->next;
        return;
    }
    for (last_rpar = netobj->rpars; last_rpar->next != NULL;
            last_rpar = last_rpar->next)
        if (last_rpar->next == rpar) {
            last_rpar->next = rpar->next;
            return;
        }
}

static void RPTracking(int x, int y) {

    gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y),
                XOR, canvas_font, t_rate_string);
    fix_x = x;
    fix_y = y;
    gdiDrawText(&mainDS, x, y ,
                XOR, canvas_font, t_rate_string);
}

static void DropRateP(int x, int y) {
    float           xloc, yloc;

    xloc = x / zoom_level;
    yloc = y / zoom_level;
    gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y),
                XOR, canvas_font, t_rate_string);
    near->rpars->center.x = xloc;
    near->rpars->center.y = yloc;
    ShowRpar(near->rpars, OR);
    menu_action();
}

static void MoveRateP(void) {

    fix_x = near->rpars->center.x * zoom_level;
    fix_y = near->rpars->center.y * zoom_level;

    sprintf(t_rate_string, "%s=%f", near->rpars->tag, near->rpars->value);

    move = RPTracking;
    left = DropRateP;
    middle = null_proc;
    put_msg(0, "DROP the RateParameter with LEFT button");
    set_canvas_tracker();
}

static void TRateTracking(int x, int y) {

    gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y),
                XOR, canvas_font, t_enabl_string);
    gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y + 10),
                XOR, canvas_font, t_rate_string);
    fix_x = x;
    fix_y = y;
    gdiDrawText(&mainDS, x, y,
                XOR, canvas_font, t_enabl_string);
    gdiDrawText(&mainDS, x, y + 10,
                XOR, canvas_font, t_rate_string);
}

static void DropTRate(int x, int y) {
    float           xloc, yloc;

    xloc = x / zoom_level;
    yloc = y / zoom_level;
    gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y),
                XOR, canvas_font, t_enabl_string);
    gdiDrawText(&mainDS, (int)(fix_x), (int)(fix_y + 10),
                XOR, canvas_font, t_rate_string);
    near->trans->ratepos.x = xloc - near->trans->center.x;
    near->trans->ratepos.y = yloc - near->trans->center.y;
    ShowTransRate(near->trans, SET);
    menu_action();
}

static void MoveTRate(void) {

    fix_x = (near->trans->center.x + near->trans->ratepos.x) * zoom_level;
    fix_y = (near->trans->center.y + near->trans->ratepos.y) * zoom_level;
    if (near->trans->kind == DETERMINISTIC && near->trans->enabl == 1) {
        sprintf(t_enabl_string, "determ.");
    }
    else if (near->trans->enabl != 1) {
        if (near->trans->enabl == 0)
            sprintf(t_enabl_string, "inf-server");
        else if (near->trans->enabl < 0)
            sprintf(t_enabl_string, "%3d-en-dep", -(near->trans->enabl));
        else
            sprintf(t_enabl_string, "%3d-server", near->trans->enabl);
    }
    else
        t_enabl_string[0] = '\0';
    if (near->trans->mark_dep != NULL)
        sprintf(t_rate_string, "..m-d");
    else if (near->trans->rpar != NULL)
        sprintf(t_rate_string, "%s", near->trans->rpar->tag);
    else if (near->trans->enabl < 0)
        sprintf(t_rate_string, "%f", near->trans->fire_rate.fp[0]);
    else
        sprintf(t_rate_string, "%f", near->trans->fire_rate.ff);
    move = TRateTracking;
    left = DropTRate;
    middle = null_proc;
    set_canvas_tracker();
}

void MoveRate(XButtonEvent   *ie) {
    float           xloc, yloc;

    xloc = event_x(ie) / zoom_level;
    yloc = event_y(ie) / zoom_level;
    if ((near = near_obj(xloc, yloc, &object)) == NULL
            || (object != DETRANS && object != IMTRANS && object != EXTRANS && object !=  RPAR))
        return;

    switch (object) {
    case DETRANS:
    case IMTRANS:
    case EXTRANS:
        SetMouseHelp("DROP the Transition Rate", NULL, NULL);
        MoveTRate();
        break;
    case RPAR:
        SetMouseHelp("DROP the Rate Parameter", NULL, NULL);
        MoveRateP();
        break;
    }
}
