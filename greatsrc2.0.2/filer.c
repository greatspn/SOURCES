#define ___FILER___

#include "global.h"
#include "postscript.h"
#include "rescale.h"
#include "funct.h"
#include "showgdi.h"
#include "printdialog.h"
#include "Canvas.h"
#include "res.h"
#include "pinv.h"
#include "tinv.h"
#include "deadl.h"
#include "trap.h"
#include "ecs.h"
#include "sc.h"
#include "me.h"
#include "unbound.h"


#include <Xm/Xm.h>
#include <stdlib.h>
#include <unistd.h>


static char *epsdir;
static	char *netdir;
static	char *psdir;

char *curfile, *psfilename;
static void (*toExec)();
static Widget	stdfile_db;
static void	(*fileproc)();
static void MergeProc(char *name, XtPointer clientData, XtPointer callData);

void InitFilesPath(void) {
    if ((netdir = getenv("GSPN_NET_DIRECTORY")) == NULL) {
        ShowInfoDialog("Warning the GSPN_NET_DIRECTORY variable is not defined:\n\nusing your HOME directory to store .net files", mainwin);
        netdir = getenv("HOME");
    }
    if ((psdir = getenv("GSPN_PS_DIRECTORY")) == NULL) {
        ShowInfoDialog("Warning the GSPN_PS_DIRECTORY variable is not defined:\n\nusing your HOME directory to store .ps files", mainwin);
        psdir = getenv("HOME");
    }
    if ((epsdir = getenv("GSPN_EPS_DIRECTORY")) == NULL) {
        ShowInfoDialog("Warning the GSPN_EPS_DIRECTORY variable is not defined:\n\nusing your HOME directory to store .eps files", mainwin);
        epsdir = getenv("HOME");
    }
}



char *GetCurrentFilename(void) {
    return curfile;
}


static	void UnmanageStdFile(void) {
    XtDestroyWidget(stdfile_db);
}


static void CancelCB(Widget w, XtPointer closure, XtPointer call_data) {
    UnmanageStdFile();
}


void InitFileDialog(Widget w) {
    stdfile_db = FetchWidget(w, "FileDialog");
}



static	void ManageStdFile(Widget w, char *title, char *basedir, char *dirmask,
                           XtCallbackProc proc, char *defspec , XtCallbackProc processname, XtCallbackProc cancel) {
    XmString	    temp1, temp2, temp3, temp4;
    Arg args[4];
    int slen;
    char buffer[300];

    InitFileDialog(w);
    XtAddCallback(stdfile_db, XmNokCallback, processname, NULL);

    if (cancel != NULL)
        XtAddCallback(stdfile_db, XmNcancelCallback, cancel, NULL);
    else
        XtAddCallback(stdfile_db, XmNcancelCallback, (XtCallbackProc) CancelCB, NULL);

    temp1 = XmStringCreate(title,   XmSTRING_DEFAULT_CHARSET);

    if (dirmask == NULL)
        temp3 = XmStringCreate("*", XmSTRING_DEFAULT_CHARSET);
    else
        temp3 = XmStringCreate(dirmask, XmSTRING_DEFAULT_CHARSET);

    if (basedir == NULL)
        temp4 = XmStringCreate("", XmSTRING_DEFAULT_CHARSET);
    else
        temp4 = XmStringCreate(basedir, XmSTRING_DEFAULT_CHARSET);

    puts(basedir);
    XtSetArg(args[0], XmNdialogTitle, temp1);
    XtSetArg(args[1], XmNpattern,     temp3);
    XtSetArg(args[2], XmNdirectory,   temp4);
    XtSetValues(stdfile_db, args, 3);

    XmFileSelectionDoSearch(stdfile_db, NULL);


    if (defspec != NULL) {
        slen = strlen(defspec);
        for (; slen > 0; slen--)
            if (defspec[slen] == '/')
                break;
        strcpy(buffer, basedir);
        strcat(buffer, defspec + slen);
        temp2 = XmStringCreate(buffer, XmSTRING_DEFAULT_CHARSET);

        XtSetArg(args[0], XmNdirSpec,     temp2);
        XtSetValues(stdfile_db, args, 1);
        XmStringFree(temp2);
    }
    XmStringFree(temp1);
    XmStringFree(temp3);
    XmStringFree(temp4);

    XtManageChild(stdfile_db);
    fileproc = proc;
}

static	void FileOkCB(Widget w, XtPointer closure, XtPointer par3) {
    XmFileSelectionBoxCallbackStruct *call_data = (XmFileSelectionBoxCallbackStruct *) par3;
    if (curfile != NULL)
        XtFree(curfile);
    XmStringGetLtoR(call_data->value, XmSTRING_DEFAULT_CHARSET, &curfile);
    {
        Widget w;
        static char *str = NULL;
        char *c;
        if (str != NULL) XtFree(str);
        w = XmFileSelectionBoxGetChild(stdfile_db, XmDIALOG_TEXT);
        str = (char *)XmTextGetString(w);
        for (c = str + strlen(str); *c != '/' ; c--) ;
        *(c + 1) = '\0';
        netdir = str;

    }

    strcpy(edit_file, curfile);
    SetWaitCursor(stdfile_db);
    (*fileproc)();
    ClearWaitCursor(stdfile_db);
    UnmanageStdFile();
}

static	void FileMergeOkCB(Widget w, XtPointer closure, XtPointer par3) {
    char *tmpfile;

    XmFileSelectionBoxCallbackStruct *call_data = (XmFileSelectionBoxCallbackStruct *) par3;
    XmStringGetLtoR(call_data->value, XmSTRING_DEFAULT_CHARSET, &tmpfile);

    SetWaitCursor(stdfile_db);
    (*fileproc)(tmpfile);
    ClearWaitCursor(stdfile_db);
    UnmanageStdFile();
}


static	void ReadProc(Widget w, XtPointer closure, XtPointer call_data) {
    int  FnameLen;

    FnameLen = strlen(curfile);
    printf("\n%s    %d", curfile, FnameLen);
    if (FnameLen) {
        for (; FnameLen > 0; FnameLen--)
            if (curfile[FnameLen] == '.') {
                clear_proc_f();
                curfile[FnameLen] = '\0';
                read_file(curfile);
                break;
            }
    }
}

static void MergeProc(char *name, XtPointer clientData, XtPointer callData) {
    int  FnameLen;

    FnameLen = strlen(name);
    printf("\nMerge with %s  %d", name, FnameLen);
    if (FnameLen) {
        for (; FnameLen > 0; FnameLen--)
            if (name[FnameLen] == '.') {
                clear_proc_f();
                name[FnameLen] = '\0';
                merge_file(name);
                break;
            }
    }
}


static	void WriteProc(Widget w, XtPointer closure, XtPointer call_data) {
    int  FnameLen;

    FnameLen = strlen(curfile);
    printf("\n%s    %d", curfile, FnameLen);
    if (FnameLen > 4) {
        if (!strcmp(curfile + FnameLen - 4, ".net"))
            curfile[FnameLen - 4] = '\0';
    }
    if (FnameLen) {
        int retcode;

        retcode = write_file(curfile);
        if (retcode) {
            char message[300];

            sprintf(message, "Can't write %s ", curfile);
            ShowErrorDialog(message, frame_w);
        }
        else if (toExec != NULL) {
            (*toExec)();
            toExec = NULL;
        }
    }

}




void Quit(void) {
    gdiReleaseGCs();
    exit(0);
}

static void QuitCB(Widget w, XtPointer closure, XtPointer call_data) {
    Quit();
}

void NewNet(void) {

    if (curfile != NULL)
        XtFree(curfile);
    curfile = NULL;
    *edit_file = '\0';
    clear_proc_f();
    setup_initialstate();
    gdiClearDrawingArea();
}

void NewCB(Widget w, XtPointer closure, XtPointer call_data) {
    NewNet();
}

void OpenNet(void) {

    SetWaitCursor(mainwin);
    ManageStdFile(mainwin, "Open...", netdir, "*.net", ReadProc, NULL , FileOkCB, NULL);
    ClearWaitCursor(mainwin);
}

void MergeNet(void) {

    SetWaitCursor(mainwin);
    ManageStdFile(mainwin, "Merge...", netdir, "*.net", (XtCallbackProc)MergeProc, NULL , FileMergeOkCB, NULL);
    ClearWaitCursor(mainwin);
}

static void OpenCB(Widget w, XtPointer closure, XtPointer call_data) {
    OpenNet();
}



static void SaveNet(Widget w, char *tmpfname) {

    SetWaitCursor(w);

    ManageStdFile(w, "Save Net File As...", netdir, "*.net", WriteProc, curfile , FileOkCB, NULL);
    ClearWaitCursor(w);
}


void SaveAndExec(void (*what)()) {
    toExec = what;

    SetWaitCursor(mainwin);
    if (curfile == NULL) {
        SaveNet(mainwin, curfile);
    }
    else {
        Widget w = NULL;
        XtPointer closure = NULL;
        XtPointer call_data = NULL;
        WriteProc(w, closure, call_data);
    }
    ClearWaitCursor(mainwin);

}

static	void PostscriptCB(Widget w, XtPointer closure, XtPointer par3) {
    XmFileSelectionBoxCallbackStruct *call_data = (XmFileSelectionBoxCallbackStruct *)par3;
    char *newfilename;
    int  FnameLen;
    char buffer[300];
    char command[600];

    SetWaitCursor(stdfile_db);
    XmStringGetLtoR(call_data->value, XmSTRING_DEFAULT_CHARSET, &newfilename);
    strcpy(buffer, newfilename);
    XtFree(newfilename);

    FnameLen = strlen(buffer);
    if (FnameLen > 3) {
        if (strcmp(buffer + FnameLen - 3, ".ps") && strcmp(buffer + FnameLen - 3, ".PS"))
            strcat(buffer, ".ps");
    }
    printf("%s  ->   %s\n", psfilename, buffer);
    if (FnameLen) {
        sprintf(command, "mv %s %s", psfilename, buffer);
        system(command);
    }
    ClearWaitCursor(stdfile_db);
    UnmanageStdFile();
    UpdateAndQuit();

}


static void TeXCB(Widget w, XtPointer closure, XtPointer par3) {
    XmFileSelectionBoxCallbackStruct *call_data = (XmFileSelectionBoxCallbackStruct *)par3;
    char *newfilename;
    int  FnameLen;
    char buffer[300];
    char command[600];

    SetWaitCursor(stdfile_db);
    XmStringGetLtoR(call_data->value, XmSTRING_DEFAULT_CHARSET, &newfilename);
    strcpy(buffer, newfilename);
    XtFree(newfilename);

    FnameLen = strlen(buffer);
    if (FnameLen > 4) {
        if (strcmp(buffer + FnameLen - 4, ".eps") && strcmp(buffer + FnameLen - 4, ".EPS"))
            strcat(buffer, ".eps");
    }
    if (FnameLen) {
        sprintf(command, "mv %s %s", psfilename, buffer);
        system(command);
    }
    ClearWaitCursor(stdfile_db);
    UnmanageStdFile();
    UpdateAndQuit();

}

static void CancelPSCB(Widget w, XtPointer closure, XtPointer call_data) {
    UnmanageStdFile();
    if (unlink(psfilename)) {
        ShowErrorDialog("Cannot remove Temporary Poscript File", frame_w);
    }
}

void SavePostscript(Widget w, char *tmpfname) {

    psfilename = tmpfname;
    SetWaitCursor(w);
    ManageStdFile(mainwin, "Save Poscript File As ....", psdir, "*.ps", NULL, curfile, PostscriptCB, CancelPSCB);
    ClearWaitCursor(w);
}

void SaveTeX(Widget w, char *tmpfname) {

    psfilename = tmpfname;
    SetWaitCursor(w);
    ManageStdFile(mainwin, "Save TeX File As ....", epsdir, "*.eps", NULL, curfile, TeXCB, CancelPSCB);
    ClearWaitCursor(w);
}

void SaveAsRequest(void) {
    SaveNet(mainwin, curfile);
}

void ExitRequest(void) {

    if (figure_modified || rescale_modified)
        ShowWarningDialog("Changes have been made: QUIT ANYWAY ?", (XtCallbackProc)QuitCB, NULL, "Quit", "Cancel", 1);
    else
        Quit();
}

void SaveRequest(void) {
    char *temp;
    char command[3000];

    if (curfile == NULL)
        SaveNet(mainwin, curfile);
    else {
        SetWaitCursor(mainwin);
        ClearRes();
        ClearPinv();
        ClearDeadl();
        ClearTrap();
        ClearTinv();
        ClearEcs();
        ClearSc();
        ClearMe();
        ClearUnbound();

        /*	setup_initialstate();*/
        gdiClearDrawingArea();
        redisplay_canvas();
        temp = getenv("GREATSPN_SCRIPTDIR");
        sprintf(command, "%s/RMNET %s", temp, curfile);
        system(command);
        {
            Widget w = NULL;
            XtPointer closure = NULL;
            XtPointer call_data = NULL;
            WriteProc(w, closure, call_data);
        }
        ClearWaitCursor(mainwin);
    }
}

void NewRequest(void) {
    if (figure_modified || rescale_modified)
        ShowWarningDialog("Changes have been made: DISCARD CHANGES ?", (XtCallbackProc)NewCB, NULL, "Discard", "Cancel", 1);
    else
        NewNet();
}

void OpenRequest(void) {
    if (figure_modified || rescale_modified)
        ShowWarningDialog("Changes have been made: DISCARD CHANGES ?", (XtCallbackProc)OpenCB, NULL, "Discard", "Cancel", 1);
    else
        OpenNet();
}

void MergeRequest(void) {
    MergeNet();
}
