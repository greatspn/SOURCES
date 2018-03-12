#define ___LIVENESS___
#	include "global.h"
#	include "showgdi.h"
#	include "rate.h"
#	include "res.h"
#include "liveness.h"
#include "filer.h"

static struct trans_object *trans;
static struct group_object *group;

static int      live_visible = FALSE;
static int      live_computed = FALSE;
static int      different;

void EraseLive(void) {
    char            linebuf[LINEMAX];
    float           xloc, yloc;

#ifdef DEBUG
    fprintf(stderr, "erase_live  computed=%d, visible=%d\n",
            live_computed, live_visible);
#endif
    if (!live_visible)
        return;
    live_visible = FALSE;
    for (trans = netobj->trans; trans != NULL; trans = trans->next) {
        xloc = (trans->center.x + trans->ratepos.x) * zoom_level;
        yloc = (trans->center.y + trans->ratepos.y + 10) * zoom_level;
        if (different)
            sprintf(linebuf, "L=%d,R=%d,E=%d",
                    trans->Lbound, trans->Rbound, trans->Ebound);
        else
            sprintf(linebuf, "L=%d", trans->Lbound);
        gdiDrawText(&mainDS, (int)(xloc), (int)(yloc),
                    CLEAR, canvas_font, linebuf);
    }
    for (group = netobj->groups; group != NULL; group = group->next) {
        for (trans = group->trans; trans != NULL; trans = trans->next) {
            xloc = (trans->center.x + trans->ratepos.x) * zoom_level;
            yloc = (trans->center.y + trans->ratepos.y + 10) * zoom_level;
            if (different)
                sprintf(linebuf, "L=%d,R=%d,E=%d",
                        trans->Lbound, trans->Rbound, trans->Ebound);
            else
                sprintf(linebuf, "L=%d", trans->Lbound);
            gdiDrawText(&mainDS, (int)(xloc), (int)(yloc),
                        CLEAR, canvas_font, linebuf);
        }
    }
}


void ClearLive(void) {
#ifdef DEBUG
    fprintf(stderr, "clear_live  computed=%d, visible=%d\n",
            live_computed, live_visible);
#endif
    if (live_computed)
        --live_computed;
    if (live_computed)
        return;
    if (live_visible)
        EraseLive();
}


void ShowLive(void) {
    float           xloc, yloc;
    char            linebuf[LINEMAX];

#ifdef DEBUG
    fprintf(stderr, "show_live  computed=%d, visible=%d\n",
            live_computed, live_visible);
#endif
    if (!live_computed)
        return;
    if (live_visible)
        return;
    if (rate_visible) {
        ShowRate(FALSE);
        rate_visible = FALSE;
        update_mode();
    }
    else
        ClearThroughputs();
    live_visible = TRUE;
    for (trans = netobj->trans; trans != NULL; trans = trans->next) {
        xloc = (trans->center.x + trans->ratepos.x) * zoom_level;
        yloc = (trans->center.y + trans->ratepos.y + 10) * zoom_level;
        if (different)
            sprintf(linebuf, "L=%d,R=%d,E=%d",
                    trans->Lbound, trans->Rbound, trans->Ebound);
        else
            sprintf(linebuf, "L=%d", trans->Lbound);
        gdiDrawText(&mainDS, (int)(xloc), (int)(yloc),
                    OR, canvas_font, linebuf);
    }
    for (group = netobj->groups; group != NULL; group = group->next) {
        for (trans = group->trans; trans != NULL; trans = trans->next) {
            xloc = (trans->center.x + trans->ratepos.x) * zoom_level;
            yloc = (trans->center.y + trans->ratepos.y + 10) * zoom_level;
            if (different)
                sprintf(linebuf, "L=%d,R=%d,E=%d",
                        trans->Lbound, trans->Rbound, trans->Ebound);
            else
                sprintf(linebuf, "L=%d", trans->Lbound);
            gdiDrawText(&mainDS, (int)(xloc), (int)(yloc),
                        OR, canvas_font, linebuf);
        }
    }
}


int LoadLive(void) {
    FILE           *fff;
    int             ii, min_liv;
    char            filename[LINEMAX];

    if (live_computed) {
        for (min_liv = -1, trans = netobj->trans; trans != NULL;
                trans = trans->next) {
            if (min_liv == -1 || min_liv > trans->Lbound)
                min_liv = trans->Lbound;
        }
        for (group = netobj->groups; group != NULL; group = group->next) {
            for (trans = group->trans; trans != NULL; trans = trans->next) {
                if (min_liv == -1 || min_liv > trans->Lbound)
                    min_liv = trans->Lbound;
            }
        }
        return (min_liv);
    }
    sprintf(filename, "%s.liveness", GetCurrentFilename());
    if ((fff = fopen(filename, "r")) == NULL)
        return (-1);
    fscanf(fff, "%*d %d\n", &different);
    for (min_liv = -1, trans = netobj->trans; trans != NULL;
            trans = trans->next) {
        fscanf(fff, " %d", &ii);
        trans->Ebound = ii;
        if (different) {
            int             jj, kk, maxj;
            fscanf(fff, " %d", &ii);
            fscanf(fff, " %d", &jj);
            for (kk = different, maxj = ii; --kk;) {
                fscanf(fff, " %d", &jj);
                if (jj > maxj)
                    maxj = jj;
            }
            trans->Rbound = maxj;
        }
        trans->Lbound = ii;
        if (min_liv == -1 || min_liv > ii)
            min_liv = ii;
        while (getc(fff) != '\n');
    }
    for (group = netobj->groups; group != NULL; group = group->next) {
        for (trans = group->trans; trans != NULL; trans = trans->next) {
            fscanf(fff, " %d", &ii);
            trans->Ebound = ii;
            if (different) {
                int             jj, kk, maxj;
                fscanf(fff, " %d", &ii);
                fscanf(fff, " %d", &jj);
                for (kk = different, maxj = ii; --kk;) {
                    fscanf(fff, " %d", &jj);
                    if (jj > maxj)
                        maxj = jj;
                }
                trans->Rbound = maxj;
            }
            trans->Lbound = ii;
            if (min_liv == -1 || min_liv > ii)
                min_liv = ii;
            while (getc(fff) != '\n');
        }
    }
    if (min_liv < 0)
        min_liv = 0;
    (void) fclose(fff);
    live_computed = 2;
    ShowLive();
    return (min_liv);
}
