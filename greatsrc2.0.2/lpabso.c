/*
 * lpabso.c
 */

#include "global.h"
#include "optionsdialog.h"
#include "filer.h"
#include "showdialog.h"

static char string[1024];
static int low_flag;
static int pla_flag;


static void compute_show_lpp(char *tag) {
    FILE *fp;
    char buf[512];
    char str2[2048];
    char message[2048];
    int vv;
    char cc;

    Gspn = getenv("GREATSPN_SCRIPTDIR");
//    gethostname( buf, 512);
    sprintf(str2, "/bin/rm -f %s.lp_out\n", GetCurrentFilename());
    system(str2);
    sprintf(str2, "%s.lp_in", GetCurrentFilename());
    if ((fp = fopen(str2, "w")) == NULL) {
        sprintf(message, "Can't open file %s for write", str2);
        ShowErrorDialog(message, mainwin);
        return;
    }
    fprintf(fp, string);
    (void)fclose(fp);
    sprintf(str2, "cat %s.lp_mark >> %s.lp_in\n",
            GetCurrentFilename(), GetCurrentFilename());
    system(str2);
//	if ( strcmp(optGetHostname(),host_name) == 0 )

    /*sprintf(str2, "csh %s/glp_solve %s", Gspn,
    		GetCurrentFilename());*/
    sprintf(str2, "%s/glp_solve %s", Gspn,
            GetCurrentFilename());

//    else
    /*sprintf(str2,"rsh %s csh %s/glp_solve %s ", optGetHostname(), Gspn,GetCurrentFilename());*/
//        sprintf(str2,"rsh %s %s/glp_solve %s ", optGetHostname(), Gspn,GetCurrentFilename());

    if (system(str2)) {
        sprintf(message, "Run time error for command: %s", str2);
        ShowErrorDialog(message, mainwin);
        return;
    }
    if (pla_flag)
        sprintf(string, "M[%s] ", tag);
    else
        sprintf(string, "Enab(%s) ", tag);
    sprintf(str2, "%s.lp_out", GetCurrentFilename());
    if ((fp = fopen(str2, "r")) == NULL) {
        sprintf(message, "Can't open file %s for read", str2);
        ShowErrorDialog(message, mainwin);
        return;
    }
    while ((cc = getc(fp)) != 'V' && cc != EOF);
    if (cc == 'V') {
        fscanf(fp, "alue of objective function: %d\n", &vv);
        if (low_flag) {
            if (vv < 0)
                sprintf(buf, ">= %d", -vv);
            else
                sprintf(buf, ">= 0");
        }
        else
            sprintf(buf, "<= %d", vv);
    }
    else
        sprintf(buf, " UNBOUNDED!");
    (void)fclose(fp);
    strcat(string, buf);
    ShowShowDialog(string);
    StatusPrintf(string);
}


void show_lp_lamb(XButtonEvent          *ie) {

    StatusPrintf("");
    ShowShowDialog("");
    if (figure_modified) {
        ShowInfoDialog("Save the net description first!", mainwin);
        return;
    }
    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((cur_place = near_place_obj(fix_x, fix_y)) == NULL) {
        return;
    }
    sprintf(string, "-M_%s;\n", cur_place->tag);
    low_flag = TRUE;
    pla_flag = TRUE;
    compute_show_lpp(cur_place->tag);
}


void show_lp_uamb(XButtonEvent          *ie) {
    StatusPrintf("");
    ShowShowDialog("");
    if (figure_modified) {
        ShowInfoDialog("Save the net description first!", mainwin);
        return;
    }
    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((cur_place = near_place_obj(fix_x, fix_y)) == NULL) {
        return;
    }
    sprintf(string, "M_%s;\n", cur_place->tag);
    low_flag = FALSE;
    pla_flag = TRUE;
    compute_show_lpp(cur_place->tag);
}


void show_lp_ueb(XButtonEvent          *ie) {
    struct group_object *g;

    StatusPrintf("");
    ShowShowDialog("");

    if (figure_modified) {
        ShowInfoDialog("Save the net description first!", mainwin);
        return;
    }
    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((cur_trans = near_titrans_obj(fix_x, fix_y)) == NULL) {
        if ((cur_trans = near_imtrans_obj(fix_x, fix_y, &g)) == NULL) {
            return;
        }
    }
    sprintf(string, "e_%s;\n", cur_trans->tag);

    /* construct conditions on input/inhibitor arcs */

    low_flag = FALSE;
    pla_flag = FALSE;
    compute_show_lpp(cur_trans->tag);
}

