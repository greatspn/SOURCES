#define ___OPTIONSDIALOG___
#	include "global.h"
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#	include <Mrm/MrmPublic.h>
#include "optionsdialog.h"

static Widget   options_dlg, options_hostname, options_defhostname, options_verboseshow;
static Options opt;


char *optGetHostname(void) {
    return opt.hostName;
}

Boolean optGetVerboseShow(void) {
    return opt.verboseShow;
}


static void SetDefaults(void) {
    ShowInfoDialog("user OPTIONS have been set to application default", mainwin);
    if (gethostname(opt.hostName, sizeof(opt.hostName))) {
        perror("Error in gethostname call");
        ShowErrorDialog("Sorry: Cannot retrieve the current Hostname", mainwin);
    }
    opt.verboseShow = FALSE;
    SaveOptions();
}

static void LoadOptions(void) {
    FILE *infile;
    char filename[2000];

    sprintf(filename, "%s/.GreatDefaults", getenv("HOME"));
    if ((infile = fopen(filename, "rb")) != NULL) {
        if (fread(&opt, sizeof(Options), 1, infile) != 1) {
            ShowErrorDialog("Sorry: cannot read ~/.GreatDefaults file", mainwin);
            SetDefaults();
        }
        fclose(infile);
    }
    else {
        ShowErrorDialog("Sorry: cannot open ~/.GreatDefaults for read", mainwin);
        SetDefaults();
    }
}

static void SaveOptions(void) {
    FILE *outfile;
    char filename[2000];

    sprintf(filename, "%s/.GreatDefaults", getenv("HOME"));
    if ((outfile = fopen(filename, "wb")) != NULL) {
        if (fwrite(&opt, sizeof(Options), 1, outfile) != 1) {
            ShowErrorDialog("Sorry: cannot write ~/.GreatDefaults file", mainwin);
        }
        fclose(outfile);
    }
    else {
        printf("Error %d\n", errno);
        ShowErrorDialog("Sorry: cannot open ~/.GreatDefaults for write", mainwin);
    }
}

void InitOptions(void) {
    struct stat sf;
    char filename[2000];

    sprintf(filename, "%s/.GreatDefaults", getenv("HOME"));

    if (stat(filename, &sf)) {
        if (errno != ENOENT)
            ShowErrorDialog("Sorry: cannot access ~/.GreatDefaults file", mainwin);
        SetDefaults();
    }
    else
        LoadOptions();
}

void HideOptionsDialog(void) {
    XtUnmanageChild(options_dlg);
}


static void CancelOptionsDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    HideOptionsDialog();
}

static void OkOptionsDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    char *tmp;

    tmp = XmTextFieldGetString(options_hostname);
    strcpy(opt.hostName, tmp);
    XtFree(tmp);
    opt.verboseShow = XmToggleButtonGadgetGetState(options_verboseshow);
    SaveOptions();
    HideOptionsDialog();
}

static void OptionsDialogDefalutHostnameCB(Widget w, XtPointer closure, XtPointer call_data) {
    char tmp[MAXHOSTNAMELEN];

    gethostname(tmp, sizeof(tmp));
    XmTextFieldSetString(options_hostname, tmp);
}

void ShowOptionsDialog(void) {

    XmTextFieldSetString(options_hostname, opt.hostName);
    XmToggleButtonGadgetSetState(options_verboseshow, opt.verboseShow, FALSE);
    XtManageChild(options_dlg);
}

void RegisterOptionsDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"OkOptionsDialogCB"		, (XtPointer)OkOptionsDialogCB},
        {"CancelOptionsDialogCB"		, (XtPointer)CancelOptionsDialogCB},
        {"OptionsDialogDefalutHostnameCB"		, (XtPointer)OptionsDialogDefalutHostnameCB}
    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterOptionsDialog names\n");
}

void InitOptionsDialog(void) {
    Arg args[1];
    XmString str;
    char tmp[MAXHOSTNAMELEN + 2];

    options_dlg = XtNameToWidget(frame_w, "*OptionsDialog");
    options_hostname = XtNameToWidget(options_dlg, "*OptionsDialog_HNameText");
    options_defhostname = XtNameToWidget(options_dlg, "*OptionsDialog_DefHost");
    options_verboseshow = XtNameToWidget(options_dlg, "*OptionsDialog_VerboseShow");
    strcpy(tmp, " ");
    gethostname(tmp + 1, sizeof(tmp));
    strcat(tmp, " ");
    str = XmStringCreateSimple(tmp);
    XtSetArg(args[0], XmNlabelString, str);
    XtSetValues(options_defhostname, args, 1);
    XmStringFree(str);
}






