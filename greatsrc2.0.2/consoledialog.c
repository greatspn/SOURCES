#define	___CONSOLEDIALOG___
#include "global.h"
#include <Mrm/MrmPublic.h>
#include <sys/param.h>
#include <sys/utsname.h>
#include "consoledialog.h"
#include "command.h"

#ifndef MAXHOSTNAMELEN
#	include <netdb.h>
#endif

static Widget consoleDialog, console_ok, console_interrupt, console_list, console_start;

Widget GetConsoleWidget(void) {
    return consoleDialog;
}

void ResetConsoleDialog(void) {
    XtSetSensitive(console_ok, TRUE);
    XtSetSensitive(console_interrupt, FALSE);
    XtSetSensitive(console_start, TRUE);
    XmProcessTraversal(console_ok, XmTRAVERSE_CURRENT);
}


void NotifyProcessTermination(void) {
    XtSetSensitive(console_ok, TRUE);
    XtSetSensitive(console_interrupt, FALSE);
    XtSetSensitive(console_start, TRUE);
}

void   ShowLine(char *text) {
    XmString string;

    text[strlen(text) - 1] = '\0';
    string = XmStringCreateLtoR(text, XmFONTLIST_DEFAULT_TAG);
    XmListAddItem(console_list, string, 0);
    XmListSetBottomPos(console_list, 0);
    XmStringFree(string);

}

void   ShowConsoleDialog(void) {
    XtManageChild(consoleDialog);
    XmProcessTraversal(console_start, XmTRAVERSE_CURRENT);
    XFlush(XtDisplay(consoleDialog));
    XtSetSensitive(console_ok, TRUE);
    XtSetSensitive(console_interrupt, FALSE);
    XtSetSensitive(console_start, TRUE);
}

static void  ConsoleInterruptCB(Widget w, XtPointer closure, XtPointer call_data) {
    char requiredBuffer[200];

    KillJob();
    NotifyProcessTermination();
    strcpy(requiredBuffer, "Process Killed by user\n");
    ShowLine(requiredBuffer);
}

static void  ClearProcessCB(Widget w, XtPointer closure, XtPointer call_data) {
    KillJob();
}

static void ConsoleOkCB(Widget w, XtPointer closure, XtPointer call_data) {
    XtUnmanageChild(consoleDialog);
}

static void ConsoleClearCB(Widget w, XtPointer closure, XtPointer call_data) {
    XmListDeleteAllItems(console_list);
}

static void ConsoleStartCB(Widget w, XtPointer closure, XtPointer call_data) {
    XtSetSensitive(console_interrupt, TRUE);
    XtSetSensitive(console_start, FALSE);
    XtSetSensitive(console_ok, FALSE);

    Solve();
}

void RegisterConsoleDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"ClearProcessCB"	, (XtPointer)ClearProcessCB},
        {"ConsoleClearCB"	, (XtPointer)ConsoleClearCB},
        {"ConsoleInterruptCB"		, (XtPointer)ConsoleInterruptCB},
        {"ConsoleOkCB"			, (XtPointer)ConsoleOkCB},
        {"ConsoleStartCB"			, (XtPointer)ConsoleStartCB}
    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterEditLayerDialog names\n");
}

void   InitConsoleDialog(void) {
    //	char hostname[MAXHOSTNAMELEN];


    consoleDialog = XtNameToWidget(mainwin, "*ConsoleDialog");
    console_list = XtNameToWidget(consoleDialog, "*ConsoleDialogList");
    console_ok = XtNameToWidget(consoleDialog, "*ConsoleDialogOK");
    console_interrupt = XtNameToWidget(consoleDialog, "*ConsoleDialogInterrupt");
    /*	console_hostname = XtNameToWidget(consoleDialog, "*ConsoleDialogHostname");*/
    console_start = XtNameToWidget(consoleDialog, "*ConsoleDialogStart");
    /*
    p = getenv("GSPN_DEFAULT_HOSTNAME");
    if(p == NULL)
    {
    	gethostname(hostname,sizeof(hostname));
    }
    else
    	strcpy(hostname,p);

    XmTextFieldSetString(console_hostname,hostname);
    */
}







