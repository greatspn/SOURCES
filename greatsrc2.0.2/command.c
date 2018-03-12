/*
 * command.c
 */



#include "global.h"
#include "infodialog.h"
#include "trap.h"
#include "deadl.h"
#include "ecs.h"
#include "res.h"
#include "tinv.h"
#include "pinv.h"
#include "sc.h"
#include "unbound.h"
#include "me.h"
#include "inputdialog.h"
#include "menuinit.h"
#include "parse.h"
#include "optionsdialog.h"

#include <stdarg.h>

#if defined(SunOS5x)
#	include <wait.h>
#else
#	include <sys/wait.h>
#endif
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/param.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
//#include "funct.h"

#ifndef MAXHOSTNAMELEN
#	include <netdb.h>
#endif
#define DEBUG
static char **ShellErrors;
static int whichcalc;

static void solve_handler();
static void deadl_handler();
static void trap_handler();
static void pinv_handler();
static void tinv_handler();
static void ecsets_handler();
static void sbound_handler();
extern XtAppContext	appContext;
//extern reset_object();
extern int merging;
extern char *curfile;
static Widget bell_global_item;
static int fd[2];
static FILE *fpipe;
static char *pipeName;

static char     command[LINEMAX];
static int      exstat;

Widget      host_name_item;
char           *nets_dir_path;

static void solve_com(char *, ...);
static void solve_com2(char *, ...);

static int      complain;
static void(*___handler)();
static XtWorkProcId wpID;

static char *message1;
static char *message2;
int (* collect_func)();

extern Widget msg_popup_f;
extern Widget GetConsoleWidget();
extern void ShowConsoleDialog();
#define MAX_SHELL_ERRORS	255
#define KILLEDBYUSER	1000
static int killed = 0;
static XtSignalId SIGCHLD_Notify_Signal;

static char *OKMessages[] = {
    "P-invariants obtained",
    "T-invariants obtained",
    "Minimal Deadlocks obtained",
    "Minimal Traps obtained",
    "E-conflit sets obtained",
    "Structural Boundedness obtained",
    "TRG solution obtained",
    "EMC solution obtained",
    "GSPN Steady State solution obtained",
    "GSPN Transient solution obtained",
    "SWN Symbolic Simulation completed",
    "SWN Symbolic RG solution obtained",
    "SWN Ordinary Simulation completed",
    "SWN Ordinary RG solution obtained",
    "SWN Symbolic Transient solution obtained",
    "SWN Ordinary Transient solution obtained",
    "SWN Ordinary Unfolding Net obtained",
    "EGSPN Simulation completed",
    "EGSPN TRG obtained",
    "EGSPN Transient solution obtained",
    "SWN Extended Symbolic RG obtained",
    "SWN Dynamic Symbolic RG obtained",
};




static int CheckForDeadRemoveChild(int p) {
    int status, ret;
    struct rusage rus;

    if ((ret = wait3(&status, WNOHANG, &rus)) != 0) {	/* non blocking wait for child p */
        char message[1000];
        int errCode;

        ClearWaitCursor(mainwin);
        errCode = WEXITSTATUS(status);
        printf("errCode = %d, status = %d\n", errCode, status);

        if (errCode) {
            strcpy(message , "Error removing result files");
            ShowErrorDialog(message, mainwin);
        }
        res_visible = FALSE;
        redisplay_canvas();
        return TRUE;
    }
    return FALSE;
}

void DoRemoveRes(int k) {
    SetWaitCursor(mainwin);

    XtAppAddWorkProc(appContext, (XtWorkProc)CheckForDeadRemoveChild, 0);
    switch (fork()) {
    case 0: {
        static char *what[] = {"Res", "All"};
        char temp[3000];

        sprintf(temp, "%s/Remove%sCommand", getenv("GREATSPN_SCRIPTDIR"), what[k]);
        printf("Executing /bin/bash  %s %s\n", temp, GetCurrentFilename());
        execl("/bin/bash", "", temp, GetCurrentFilename(), (char *)0);
        /*execl(temp,(char *)GetCurrentFilename(),(char *)0);*/
        break;
    }
    case -1:
        ShowErrorDialog("Sorry : cannot fork", mainwin);
        break;
    }
}

void DoRemoveResCB(Widget w, XtPointer call_data, XtPointer client_data) {
    DoRemoveRes(0);
}

void DoRemoveAllCB(Widget w, XtPointer call_data, XtPointer client_data) {
    DoRemoveRes(1);
}

void RemoveResProc(int k) {
    char *message[] = {
        "Warning this operation wiill remove the RESULTS files !",
        "Warning this operation wiill remove ALL the net files !"
    };

    if ((char *)GetCurrentFilename() == NULL)
        return;

    ShowWarningDialog(message[k], k ? DoRemoveAllCB : DoRemoveResCB, NULL, "Ok", "Cancel", 1);
}




char *GetShellErrorMessage(int status) {
    char *retval;

    if (status < MAX_SHELL_ERRORS) {
        static char codemess[30];

        retval = ShellErrors[status];

        if (retval == NULL) {
            sprintf(codemess, "???? (code %d)", status);
            retval = codemess;
        }
    }
    else
        retval = "Shell Error Reporting System is not working Properly";
    puts(retval);
    return retval;
}












static char *moduleName[] = {"pinv", "tinv", "deadl", "traps", "struct", "sbound",
                             "checkRG", "newMT", "newSO", "randomTR"
                            };

#define MAX_PARALLEL_COMPUTATION 1
static struct {
    pid_t pID;
    char *moduleID;
} aComp[MAX_PARALLEL_COMPUTATION];
#define getCompID() 1
#define relComp(id)

#include "calc.h"

static void
Collect() {


    switch (whichcalc) {
    case 0:
        CollectPinvar(TRUE);
        break;
    case 1:
        CollectTinvar(TRUE);
        break;
    case 2:
        CollectDeadl(TRUE);
        break;
    case 3:
        CollectTrap(TRUE);
        break;
    case 4:
        CollectEcsSets(TRUE);
        break;
    case 5:
        CollectUnbound(TRUE);
        break;
    case 6:
    case 7:
    case 8:
    case 9:
        if (complain)
            CollectRes(complain);
        break;
    case 10:
    case 11:
    case 12:
    case 13:
    case  _SWN_SYM_TRANSIENT:
    case  _SWN_ORD_TRANSIENT:
    case  _SWN_ORD_UNFOLD:
    case  _EGSPN_SIMULATION :
    case  _EGSPN_COMPUTERG  :
    case _EGSPN_TRANSIENT   :
    /****17/7/08 ESRG by Marco***/
    case _SWN_ESRG:
    case _SWN_DSRG:
    case _EDIT_GD:
    /****17/7/08 ESRG by Marco***/
    /****11/6/10 MDD by Marco***/
    case _GSPN_MDD:
        /****11/6/10 MDD by Marco***/
        CollectRes(FALSE);
    default:
        break;
    }
    if (whichcalc <= 13)
        ShowInfoDialog(OKMessages[whichcalc], GetConsoleWidget());
    if (whichcalc == 20)
        ShowInfoDialog(OKMessages[20], GetConsoleWidget());
    if (whichcalc == 21)
        ShowInfoDialog(OKMessages[21], GetConsoleWidget());


}



void InitGSPNShellErrors(void) {
    char confFile[1000];
    Boolean parseErr;
    struct stat tmp;
    FILE *cfin;
    int i;

    ShellErrors = (char **)emalloc(sizeof(char *)*MAX_SHELL_ERRORS);

    if (ShellErrors == NULL) {
        printf("MALLOC\n"); exit(0);
    }

    for (i = 0; i < MAX_SHELL_ERRORS; i++)
        ShellErrors[i] = NULL;

    sprintf(confFile, "%s/GreatConfig", getenv("GREATSPN_SCRIPTDIR"));
    if (stat(confFile, &tmp)) {
        if (errno == ENOENT) {
            ShowInfoDialog("Warning: GreatConfig configuration file not found\nShell Errors will be unclear", mainwin);
        }
    }
    else {

        if ((cfin = fopen(confFile, "rt")) != NULL) {
            int fileLine;

            parseErr = FALSE;
            fileLine = 0;

            while (!feof(cfin) && !parseErr) {
                char *parsed;
                char commandName[1000];
                char buffLine[132];
                int code;

                fgets(buffLine, sizeof(buffLine), cfin);
                fileLine++;
                parsed = LTrim(buffLine);
                switch (*parsed) {
                case '@':
                    break;
                case '\0':
                    break;
                default:
                    if (sscanf(parsed, "%d %s", &code, commandName) != 2) {
                        char msg[100];

                        sprintf(msg, "Error Parsing Configuration file Line %d", fileLine);
                        ShowErrorDialog(msg, mainwin);
                        parseErr = TRUE;
                    }
                    else {
                        if (code < MAX_SHELL_ERRORS)
                            ShellErrors[code] = strdup(commandName);
                        else

                        {
                            char msg[400];

                            sprintf(msg, "Warning  Error code too high in Line %d", fileLine);

                            ShowErrorDialog(msg, mainwin);
                        }
                    }
                }
            }
            fclose(cfin);
        }
        else {
            ShowErrorDialog("Error: cannot open ~/.GreatConfig configuration file", mainwin);
        }
    }

}



static void
set_bell_global_proc(item, value, event)
Widget      item;
unsigned int    value;
XEvent          *event;
{
    if (inib_flag)
        return;
    bell_global_on = (int)(value & 0x1);
}

static void CancelGetTotTimeCB(Widget w, XtPointer closure, XtPointer call_data) {
    ResetConsoleDialog();
}

static void GetTotTimeCB(Widget w, XtPointer par2, XtPointer call_data) {

    float           num;
    int             increm = FALSE;
    char num_buf[20];
    char *totime;
    char **tobuffer = (char **)par2;

    totime = *tobuffer;
    puts(totime);
    while (*totime == ' ')
        totime++;
    if (*totime == '+') {
        increm = TRUE;
        while (*++totime == ' ');
    }
    if (*totime < '0' || *totime > '9') {
        put_msg(2, "Error : time must be a positive number.");
        XtFree(totime);
        return;
    }
    sscanf(totime, "%f", &num);
    if (increm)
        num = -num;
    sprintf(num_buf, "%f", num);
    complain = 2;
    XtFree(totime);
    switch (whichcalc) {
    case _SWN_SYM_TRANSIENT :
        solve_com("...SOLVING...", solve_handler, "swn_sym_tr", curfile, num_buf, (char *)0);
        break;
    case _SWN_ORD_TRANSIENT :
        solve_com("...SOLVING...", solve_handler, "swn_ord_tr", curfile, num_buf, (char *)0);
        break;
    case _SWN_ORD_UNFOLD :
        solve_com("...SOLVING...", solve_handler, "unfolding", curfile, num_buf, (char *)0);
        break;
    case _EGSPN_TRANSIENT :
        solve_com("...SOLVING...", solve_handler, "gspn_tr", curfile, num_buf, (char *)0);
        break;
    default : /* case 9 */
        solve_com("...SOLVING...", solve_handler, "randomTR", curfile, num_buf, (char *)0);
        break;
    }

}




void restore_proc()

{
    if (inib_flag)
        return;
    if (receiving_msg)
        return;
    if (action_on && (cur_command != ADD || cur_object == IMTRANS)
            && cur_command != SELECT && cur_command != MOVE) {
        next_msg(2, cant_interrupt);
        /*	if (bell_global_on)
        	window_bell(menu_sw); */
        return;
    }
    puts("Undo");
    restore_obj();
    /*	cur_command = NULL_CHOICE;
    cur_object = NULL_CHOICE;
    menu_action();*/
}



static void
clear_proc(item, event)
Widget      item;
XEvent      *event;
{
    if (inib_flag)
        return;
    if (receiving_msg)
        return;
    if (selection_on || action_on) {
        next_msg(2, cant_interrupt);
        /*	if (bell_global_on)
        	window_bell(menu_sw);*/
        return;
    }
    clear_proc_f();
    redisplay_canvas();
}

void
clear_proc_f() {
    ClearRes();
    ClearPinv();
    ClearDeadl();
    ClearTrap();
    ClearTinv();
    ClearEcs();
    ClearSc();
    ClearMe();
    ClearUnbound();
    free_bkup_netobj();
    bkup_netobj[0] = netobj;
    netobj = NULL;
    init_net();
    ResetGrill();
    ResetZoom();
    flag_tinv = 1;
    flag_pinv = 1;
    ResetModify();
    setup_initialstate();
    ReinitializeLayers();
    Reset_T_P_Popups();

}

void
clear_proc_m() {
    ClearRes();
    ClearPinv();
    ClearDeadl();
    ClearTrap();
    ClearTinv();
    ClearEcs();
    ClearSc();
    ClearMe();
    ClearUnbound();
    flag_tinv = 1;
    flag_pinv = 1;
    /*	ResetModify();*/
}

/* static XtInputId IID;*/

void
InputCB(client_data, source, id)
XtPointer client_data;
int       *source;
XtInputId *id;
{
    char line[1000];

    /*	if(!feof(fpipe) && killed==0){*/
    if (fgets(line, sizeof(line), fpipe) != NULL && killed == 0) {
        ShowLine(line);
        /* }*/
    }
    else {
        XtRemoveInput(*id);
        fclose(fpipe);
        unlink(pipeName);
    }

}

static void
SwnOrdUnfoldHandler(status)
int status;
{
    if (status) {
        char message[300];
        sprintf(message, "Unfolding failed, exit value %d", status);
        ShowErrorDialog(message, GetConsoleWidget());
    }
    else {
        ShowInfoDialog("Unfolding successfully completed", GetConsoleWidget());
    }
}


static void
SwnSymSimHandler(status)
int status;
{
    if (status) {
        char message[300];
        sprintf(message, "Simulation failed, exit value %d", status);
        ShowErrorDialog(message, GetConsoleWidget());
    }
    else {
        ShowInfoDialog("Simulation successfully completed", GetConsoleWidget());
    }
}

void
ScheduleSwn(int whichHandler, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8) {
    char s1[30];
    char s2[30];
    char s3[30];
    char s4[30];
    char s5[30];
    char s6[30];
    char s7[30];

    printf("ScheduleSwn = %d\n", whichHandler);
    if (whichHandler != _SWN_SYM_RG && whichHandler != _SWN_ORD_RG /*  & 0x1*/) {
        sprintf(s1, "%d", p1);
        sprintf(s2, "%d", p2);
        sprintf(s3, "%d", p3);
        sprintf(s4, "%d", p4);
        sprintf(s5, "%d", p5);
        sprintf(s6, "%d", p6);
        sprintf(s7, "%d", p7);
    }

    switch (whichHandler) {
    case _SWN_SYM_SIM:
        if (p8)
            solve_com("Starting SWN ", SwnSymSimHandler, "swn_sym_sim", GetCurrentFilename(),
                      "-f", s1, "-t", s2, "-m", s3, "-M", s4, "-c", s5, "-a", s6, "-s", s7, "-o", "0", (char *)0);
        else
            solve_com("Starting SWN ", SwnSymSimHandler, "swn_sym_sim", GetCurrentFilename(),
                      "-f", s1, "-t", s2, "-m", s3, "-M", s4, "-c", s5, "-a", s6, "-s", s7, (char *)0);
        break;
    case _SWN_SYM_RG:
        solve_com("Starting SWN ", SwnSymSimHandler, "swn_sym_rg", GetCurrentFilename(), p1 ? "-o" : (char *)0, (char *)0);
        break;
    /****17/7/08 ESRG by Marco***/
    case _SWN_ESRG:
        solve_com("Starting SWN ", SwnSymSimHandler, "swn_sym_esrg", GetCurrentFilename(), p1 ? "-o" : "-m", p2 ? "-s" : "-e", (char *)0, (char *)0, (char *)0, (char *)0);
        break;
    case _SWN_DSRG:
        solve_com("Starting SWN ", SwnSymSimHandler, "swn_sym_dsrg", GetCurrentFilename(), p1 ? "-o" : "-m", (char *)0, (char *)0, (char *)0, (char *)0, (char *)0);
        break;
    case _EDIT_GD:
        sprintf(s1, "%s.gd", GetCurrentFilename());
        solve_com("Starting SWN ", SwnSymSimHandler, "Edit", s1, (char *)0, (char *)0);
        break;
    /****17/7/08 ESRG by Marco***/
    case _GSPN_MDD:
        if (p3 == 1)
            solve_com("Starting GSPN ", SwnSymSimHandler, "swn_ord_rgMEDD", GetCurrentFilename(), "-N", "-B", s4, (char *)0, (char *)0, (char *)0, (char *)0);
        else
            solve_com("Starting GSPN ", SwnSymSimHandler, "swn_ord_rgMEDD", GetCurrentFilename(), "-B", s4, p2 ? "-E" : (char *)0, p3 ? "-N" : (char *)0, (char *)0, (char *)0);
        break;
    /****17/7/08 ESRG by Marco***/

    case _SWN_ORD_SIM:
        if (p8)
            solve_com("Starting SWN ", SwnSymSimHandler, "swn_ord_sim", GetCurrentFilename(),
                      "-f", s1, "-t", s2, "-m", s3, "-M", s4, "-c", s5, "-a", s6, "-s", s7, "-o", "0", (char *)0);
        else
            solve_com("Starting SWN ", SwnSymSimHandler, "swn_ord_sim", GetCurrentFilename(),
                      "-f", s1, "-t", s2, "-m", s3, "-M", s4, "-c", s5, "-a", s6, "-s", s7, (char *)0);
        break;
    case _SWN_ORD_RG:
        solve_com("Starting SWN ", SwnSymSimHandler, "swn_ord_rg", GetCurrentFilename(), p1 ? "-o" : (char *)0, (char *)0);
        break;
    case _EGSPN_SIMULATION:
        if (p8)
            solve_com("Starting SWN ", SwnSymSimHandler, "gspn_sim", GetCurrentFilename(),
                      "-f", s1, "-t", s2, "-m", s3, "-M", s4, "-c", s5, "-a", s6, "-s", s7, "-o", "0", (char *)0);
        else
            solve_com("Starting SWN ", SwnSymSimHandler, "gspn_sim", GetCurrentFilename(),
                      "-f", s1, "-t", s2, "-m", s3, "-M", s4, "-c", s5, "-a", s6, "-s", s7, (char *)0);
        break;
    case _EGSPN_COMPUTERG:
        solve_com("Starting SWN ", SwnSymSimHandler, "gspn_rg", GetCurrentFilename(), p1 ? "-o" : (char *)0, (char *)0);
        break;
    case _SWN_ORD_UNFOLD:
        solve_com("Starting SWN ", SwnOrdUnfoldHandler, "unfolding", GetCurrentFilename(), (char *)0);
        break;
    }
}

static void
lpbound_handler(status)
int status;
{
    puts("lpbound_handler");
    if (status) {
        ShowErrorDialog("ERROR: couldn't prepare LPP file!\n", GetConsoleWidget());
    }
}


void Solve(void) {
    switch (whichcalc) {
    case 0:
        solve_com("computing P-invariants", pinv_handler, "pinv", curfile, (char *)0);
        break;
    case 1:
        solve_com("computing T-invariants", tinv_handler, "tinv", curfile, (char *)0);
        break;
    case 2:
        solve_com("computing minimal Deadlocks", deadl_handler, "deadl", curfile, (char *)0);
        break;
    case 3:
        solve_com("computing minimal Traps", trap_handler, "traps", curfile, (char *)0);
        break;
    case 4:
        solve_com("computing E-conflict sets", ecsets_handler, "struct", curfile, (char *)0);
        break;
    case 5:
        solve_com("computing structural boundedness", sbound_handler, "sbound", curfile, (char *)0);
        break;
    case 6:
        complain = 1;
        solve_com("solving TRG", solve_handler, "checkRG", curfile, (char *)0);
        break;
    case 7:
        complain = 1;
        solve_com("solving EMC", solve_handler, "newMT", curfile, (char *)0);
        break;
    case 8:
        complain = 2;
        solve_com("Solving GSPN Steady State", solve_handler, "newSO", curfile, "-s10", (char *)0);
        break;
    case 9:
        complain = 1;
        ShowInputDialog("transient time (+ to increment previous value) ", CancelGetTotTimeCB, GetTotTimeCB, GetConsoleWidget());
        break;
    case 10:
        ShowSwnSimOptionsDialog(_SWN_SYM_SIM);
        break;
    case 11:
        ShowSwnRGOptionsDialog(_SWN_SYM_RG);
        break;
    case 12:
        ShowSwnSimOptionsDialog(_SWN_ORD_SIM);
        break;
    case 13:
        ShowSwnRGOptionsDialog(_SWN_ORD_RG);
        break;


    case 14: /* _SWN_SYM_TRANSIENT */
        complain = 1;
        ShowInputDialog("transient time (+ to increment previous value) ", CancelGetTotTimeCB, GetTotTimeCB, GetConsoleWidget());
        break;
    case 15: /* _SWN_ORD_TRANSIENT */
        complain = 1;
        ShowInputDialog("transient time (+ to increment previous value) ", CancelGetTotTimeCB, GetTotTimeCB, GetConsoleWidget());
        break;
    case 16: /* _EGSPN_SIMULATION */
        ShowSwnSimOptionsDialog(_EGSPN_SIMULATION);
        break;
    case 17: /* _EGSPN_COMPUTERG */
        ShowSwnRGOptionsDialog(_EGSPN_COMPUTERG);
        /*complain = 1;
        solve_com("solving TRG", solve_handler, "gspn_rg", curfile, (char*)0);*/
        break;
    case 18: /* _EGSPN_TRANSIENT */
        complain = 1;
        ShowInputDialog("transient time (+ to increment previous value) ", CancelGetTotTimeCB, GetTotTimeCB, GetConsoleWidget());
        break;
    case 19: /* _SWN_ORD_UNFOLD */
// 			solve_com("computing unfolded net", solve_handler, "unfolding.sh", curfile, (char*)0);
        ShowSwnUnfoldOptionsDialog(_SWN_ORD_UNFOLD);
        break;
    /****17/7/08 ESRG by Marco***/
    case 20:
        ShowSwnRGOptionsDialog(_SWN_ESRG);
        break;
    case 21:
        ShowSwnRGOptionsDialog(_SWN_DSRG);
        break;
    /****17/7/08 ESRG by Marco***/
    /****11/6/10 MDD by Marco***/
    case 23:
        ShowSwnRGOptionsDialog(_GSPN_MDD);
        break;
        /****11/6/10 MDD by Marco***/
    }
}

void SaveAndRetry(void) {
    SaveAndExec(ShowConsoleDialog);
}

void MainSolve(which)
int which;
{
    whichcalc = which;
    if (curfile == NULL && !figure_modified) {
        ShowErrorDialog("Invalid Net Name", frame_w);
        return;
    }
    else if (figure_modified) {
        ShowWarningDialog("The net must be saved to perform the requested calculations.\n\nDo you want to SAVE the net ?", (XtCallbackProc)SaveAndRetry, NULL, "Save", "Cancel", 0);
        return;
    }

    ShowConsoleDialog();

    /*SelectSolve();*/
}



/*
static Notify_value
lpbound_handler( me, pid, status, rusage )
Notify_client me;
int pid;
union wait * status;
struct rusage *rusage;
{
	if ( (status->w_status >> 8) & 0xff ) {
	put_msg(2, "ERROR: couldn't prepare LPP file!\n");
	  }
	xv_set(frame_w, FRAME_BUSY, FALSE, NULL);
	xv_set(msg_popup_f, FRAME_BUSY, FALSE, NULL);
}
*/









void PrepareLPMark(void) {
    whichcalc = 20;
    solve_com2("Preparing LPP file", lpbound_handler, "lpmark",  curfile, (char *)0);
}
void PrepareLPBounds(void) {
    whichcalc = 21;
    solve_com2("Preparing LPP file", lpbound_handler, "lpbound",  curfile, (char *)0);
}





void
prepare_lp_mark() {
    /*


    nets_dir_path = getenv("PWD");
    sprintf(edit_file, (char *)
    xv_get(	net_name_item,PANEL_VALUE));
    solve_com("Preparing LPP file", lpbound_handler, "lpmark",
    		  edit_file, nets_dir_path, (char*)0);
    */
}

void
prepare_lp_bounds() {
    /*

    nets_dir_path = getenv("PWD");
    sprintf(edit_file, (char *)
    xv_get(	net_name_item,PANEL_VALUE));
    solve_com("Preparing LPP file", lpbound_handler, "lpbound",
    		  edit_file, nets_dir_path, (char*)0);
    */
}





static Widget  invar_menu;

static void
invar_proc(item, event)
Widget      item;
XEvent          *event;
{
    if (inib_flag)
        return;
    if (receiving_msg)
        return;
    if (selection_on || action_on) {
        next_msg(2, cant_interrupt);
        /*	if (bell_global_on)
        	window_bell(menu_sw);*/
        return;
    }
    /*	menu_show(invar_menu, menu_sw, event, 0);*/
}

static Widget     solve_menu;

typedef void (*func_ptr)();


#if defined(SunOS5x) || defined(Linux)
static pid_t child;
#else
static 	int child;
#endif

void KillJob() {
    if (child)
#ifdef SunOS5x
        killed = !kill(-child, SIGTERM);
#else
        killed = !kill(-child, SIGTERM);
#endif
}


#define MAXARGS 20

//static
//XtSignalId SIGCHLD_Notify_Signal;

void
SIGCHLD_handler(sig_id)
int sig_id;
{
    signal(SIGCHLD, (void (*)(int))SIGCHLD_handler);
    XtNoticeSignal(SIGCHLD_Notify_Signal);
}

#include <errno.h>

void
SIGCHLD_Callback(XtPointer client_data, XtSignalId *id) {
    int            status;
    struct rusage  rus;
    char message[3000];

    child = 0;

    XtRemoveSignal(SIGCHLD_Notify_Signal);

    NotifyProcessTermination();
    wait3(&status, WNOHANG, &rus);



    if (!WIFEXITED(status)) { /* child does not exit normally */
        if (killed) {
            strcpy(message, "Process Terminated by user");
        }
        else {
            if (WIFSIGNALED(status))
                sprintf(message, "Solution failed in module %s\nProcess Terminated by signal %d",
                        aComp[1].moduleID, WTERMSIG(status));
        }
        ShowErrorDialog(message, GetConsoleWidget());
    }
    else {   /* child exit normally then test its exit status code */
        /* exit status of resolving process is passed by the sh process as an exit value */

        if (status) {

            if (killed) {
                strcpy(message, "Process Terminated by user");
            }
            else {
                sprintf(message, "Error in child termination: %s", strerror(WEXITSTATUS(status)));
            }

            ShowErrorDialog(message, GetConsoleWidget());

        }
        else   /* child process terminated normally */
            Collect();
    }
}

static void
solve_com(char *msg1, ...) {

    va_list args;

    char *argv[MAXARGS];
    int argc = 0;
    char *p;
    char userHostname[MAXHOSTNAMELEN];
    char buf[1000];
    char filename[300];
    int status;
    int retval;
    int maxfds ;
    unsigned i;
    static int my_client_object;
    func_ptr handler;
    struct stat     stb;
    int cID = getCompID();;

    for (i = 0; i < MAXARGS; i++)
        argv[i] = NULL;
    SIGCHLD_Notify_Signal = XtAppAddSignal(appContext,
                                           (XtSignalCallbackProc)SIGCHLD_Callback,
                                           (XtPointer)NULL);


    signal(SIGCHLD, (void (*)(int))SIGCHLD_handler);

    maxfds = (int)sysconf(_SC_OPEN_MAX);

    va_start(args, msg1);

    ___handler = va_arg(args, func_ptr);

    strcpy(userHostname, optGetHostname());

    Gspn = getenv("GREATSPN_SCRIPTDIR");

    printf("%s %s %s\n", userHostname, host_name, Gspn);

    if (strcmp(userHostname, host_name) == 0) {
        /*	argv[argc++] = "/bin/csh"; */ ;
    }
    else {
        argv[argc++] = "/usr/ucb/rsh";
        argv[argc++] = userHostname;
        /*	argv[argc++] = "csh";*/
    }

    sprintf(buf, "%s/%s", Gspn, (aComp[cID].moduleID = va_arg(args, char *)));
    argv[argc] = buf;
    do {
        p = NULL;
        p = va_arg(args, char *);
        //va_end(args);
        argv[++argc] = p;
        if (p != NULL)
            printf("%s\n", p);
    }
    while (p && (argc < MAXARGS));
    va_end(args);
    /* va_end( args ); */

#ifdef DEBUG
    for (i = 0; i <  argc;
            fprintf(stderr, "... solve_com: argv[%d] = %s\n", i, argv[i]), i++);
#endif

    killed = 0;

#ifdef DEBUG
    fprintf(stderr, " ... solve_com: making pipeName\n");
#endif



    pipeName = tmpnam(NULL);
    puts(pipeName);
    if (mkfifo(pipeName, 0777)) {
        ShowErrorDialog("Severe Error: cannot create the Pipe", GetConsoleWidget());
        return;
    }




#ifdef DEBUG
    fprintf(stderr, " ... solve_com (parent): opening pipe\n");
#endif


#ifdef DEBUG
    fprintf(stderr, " ... solve_com: forking\n");
#endif




#ifdef DEBUG
    fprintf(stderr, " ... solve_com (parent): setting child handler\n");
#endif


    /* SetDiedChildHandler(handler,child); */
    /*
    	printf("SetDiedChildHandler %d\n",getpid());
    	printf("Installing handler for %d\n",child);

    */
    aComp[cID].pID = child = fork();


    switch (child) {
    case -1:
        ShowErrorDialog("Sorry : cannot fork -- ", frame_w);
        fclose(fpipe);
        unlink(pipeName);
        break;
    case 0: {
        char temp[300];


#ifdef DEBUG
        fprintf(stderr, " ... solve_com (child): opening pipe\n");
#endif

        setpgid(0, 0);

        /*close( 1 );*/
        if ((fd[1] = open(pipeName, O_WRONLY)) == -1)
            exit(errno);
        if (dup2(fd[1], 1) == -1) exit(errno);
        /*close( 2 );
        dup(fd[1]);*/
        if (dup2(fd[1], 2) == -1) exit(errno);
        for (i = 3; i < maxfds; ++i)
            close(i);
        close(fd[0]);

        execv(argv[0], argv);

        sprintf(temp, "GreatSPN: Cannot exec %s", argv[0]);
        ShowErrorDialog(temp, frame_w);
    }
    break;

    }

    fd[0] = open(pipeName, O_RDONLY | O_NDELAY);
    fpipe = fdopen(fd[0], "r");
    XtAppAddInput(appContext,
                  fileno(fpipe),
                  (XtPointer)XtInputReadMask,
                  (XtInputCallbackProc)InputCB,
                  NULL);

}









static void
solve_com2(char *msg1, ...) {

    va_list args;

    char *argv[MAXARGS];
    int argc = 0;
    char *p;
    char userHostname[MAXHOSTNAMELEN];
    char buf[1000];
    char filename[300];
    int status;
    int retval;
    int maxfds ;
    unsigned i;
    static int my_client_object;
    func_ptr handler;
    struct stat     stb;
    int cID = getCompID();



    /*ShowInfoDialog(msg1,frame_w);*/

    maxfds = (int)sysconf(_SC_OPEN_MAX);

    va_start(args, msg1);
    ___handler = va_arg(args, func_ptr);

    strcpy(userHostname, optGetHostname());

    Gspn = getenv("GREATSPN_SCRIPTDIR");

    if (strcmp(userHostname, host_name) == 0) {
        /*		argv[argc++] = "/bin/csh";*/
    }
    else {
        argv[argc++] = "/usr/ucb/rsh";
        argv[argc++] = userHostname;
        /*		argv[argc++] = "csh";*/
    }
    sprintf(buf, "%s/%s", Gspn, va_arg(args, char *));
    argv[argc] = buf;

    do {
        p = va_arg(args, char *);
        va_end(args);
        argv[++argc] = p;
    }
    while (p);


#ifdef DEBUG
    for (i = 0; i < argc; printf("solve_com2: argv[%d] = %s\n", i, argv[i]), i++);
#endif

    /*SetDiedChildHandler2(handler,child);a*/
    killed = 0;

    aComp[cID].pID = child = fork();

    switch (child) {
    case -1:
        ShowErrorDialog("Sorry : cannot fork -- ", frame_w);
        break;
    case 0: {
        char temp[300];
        for (i = 3; i < maxfds; ++i) {
            close(i);
        }

        execv(argv[0], argv);

        sprintf(temp, "GreatSPN: Cannot exec %s", argv[0]);
        ShowErrorDialog(temp, frame_w);

    }
    break;
    }
}















/**********************************************************************************
 *
 *
 *
 *
 **********************************************************************************/

static void
pinv_handler(status)
int status;
{
    if (status) {
        char message[300];
        sprintf(message, "Solution failed, exit value %d", status);
        ShowErrorDialog(message, GetConsoleWidget());
    }
    else {
        ShowInfoDialog("P-invariants obtained", GetConsoleWidget());
        CollectPinvar(TRUE);
    }
}

static void
tinv_handler(status)
int status;
{
    if (status) {
        char message[300];
        sprintf(message, "Solution failed, exit value %d", status);
        ShowErrorDialog(message, GetConsoleWidget());
    }
    else {
        ShowInfoDialog("T-invariants obtained", GetConsoleWidget());
        CollectTinvar(TRUE);
    }
}

static void
deadl_handler(status)
int status;
{
    if (status) {
        char message[300];
        sprintf(message, "Solution failed, exit value %d", status);
        ShowErrorDialog(message, GetConsoleWidget());
    }
    else {
        ShowInfoDialog("Minimal Deadlocks obtained", GetConsoleWidget());
        CollectDeadl(TRUE);
    }
}

static void
solve_handler(status)
int status;
{
    if (status) {
        char message[300];
        sprintf(message, "Solution failed, exit value %d", status);
        ShowErrorDialog(message, GetConsoleWidget());
    }
    else {
        ShowInfoDialog("  ... SOLUTION OBTAINED ...", GetConsoleWidget());
        if (complain)
            CollectRes(complain);
    }
}

static void
trap_handler(status)
int status;
{
    if (status) {
        char message[300];
        sprintf(message, "Solution failed, exit value %d", status);
        ShowErrorDialog(message, GetConsoleWidget());
    }
    else {
        ShowInfoDialog("Minimal Traps obtained", GetConsoleWidget());
        CollectTrap(TRUE);
    }
}

static void
ecsets_handler(status)
int status;
{
    if (status) {
        char message[300];
        sprintf(message, "Solution failed, exit value %d", status);
        ShowErrorDialog(message, GetConsoleWidget());
    }
    else {
        ShowInfoDialog("E-conflit sets obtained", GetConsoleWidget());
        CollectEcsSets(TRUE);
    }
}

static void
sbound_handler(status)
int status;
{
    if (status) {
        char message[300];
        sprintf(message, "Solution failed, exit value %d", status);
        ShowErrorDialog(message, GetConsoleWidget());
    }
    else {
        ShowInfoDialog("...Solution Obtained...", GetConsoleWidget());
        CollectUnbound(TRUE);
    }
}


static
gspn_rg_proc(m, mi)
Widget            m;
Widget       mi;
{
    /*
    complain = 1;
    nets_dir_path = getenv("PWD");
    sprintf(edit_file, (char *)
    		xv_get(
    				net_name_item,PANEL_VALUE));
    solve_com("... SOLVING ...", solve_handler, "checkRG", edit_file,
    		  nets_dir_path, (char*)0);
    */
}


static
gspn_mt_proc(m, mi)
Widget            m;
Widget	   mi;
{
    /*
    complain = 1;
    nets_dir_path = getenv("PWD");
    sprintf(edit_file, (char *)
    xv_get(	net_name_item,PANEL_VALUE));
    solve_com("... SOLVING ...", solve_handler, "newMT", edit_file,
    		  nets_dir_path, (char*)0);
    */
}


static
gspn_gs_proc(m, mi)
Widget            m;
Widget       mi;
{
    /*
    complain = 2;
    nets_dir_path = getenv("PWD");
    sprintf(edit_file, (char *)
    xv_get(net_name_item,PANEL_VALUE));
    solve_com("... SOLVING ...", solve_handler, "newSO", edit_file,
    		  nets_dir_path, "-s10", (char*)0);
    */
}

static
gspn_tr_proc(m, mi)
Widget            m;
Widget       mi;
{
    /*	if (inib_flag)
    return;
    if (receiving_msg)
    return;
    if (selection_on || action_on) {
    next_msg(2,cant_interrupt);
    return;
    }
    receiver = get_totime;
    init_msgreceiving("transient time (+ to increment previous value): ");*/

}


static
solve_proc(item, event)
Widget      item;
XEvent          *event;
{
    /*
    if (inib_flag)
    return;
    if (receiving_msg)
    return;
    if (selection_on || action_on) {
    next_msg(2,cant_interrupt);
    if (bell_global_on)
    	window_bell(menu_sw);
    return;
    }
    menu_show(solve_menu, menu_sw, event, 0);
    */
}


static Widget paste_button;
static Widget mirror_button;


void
paste_proc() {
    if (inib_flag)
        return;
    if (receiving_msg)
        return;
    if (moving_images != NULL) {
        prev_x = select_left;
        prev_y = select_top;
        drop_selected((int) select_left, (int) select_top, netobj);
    }
    merging = 0;
    reset_object();
    menu_action();
}

void
mirror_proc() {
    if (inib_flag)
        return;
    if (receiving_msg)
        return;
    cur_command = MIRROR;
    menu_action();
}

static Widget modify_button;
static Widget     modify_menu;

static void
modify_proc(item, event)
Widget      item;
XEvent          *event;
{
    if (inib_flag)
        return;
    if (receiving_msg)
        return;
    /*	menu_show(modify_menu, menu_sw, event, 0); */
}


void
modify_menu_procCB(w, mitem, call_data)
Widget			w;
caddr_t call_data;
short mitem;

{

    if (inib_flag)
        return;
    modify_selected(mitem);

}


void InitCommands(void) {
    gethostname(host_name, LINEMAX);
    //init_simpan();
    /*init_fire();*/
}
