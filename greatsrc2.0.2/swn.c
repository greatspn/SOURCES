#define ___SWN___
#	include "global.h"
#	include "res.h"
#	include <Mrm/MrmPublic.h>
#include "consoledialog.h"
#include "swn.h"

static int whichClevel;
static int whichType;
static Widget sim_dialog, sim_InitWasteText, sim_MidWasteText, sim_MinBatchText, sim_MaxBatchText, sim_ApproxText, sim_SeedText, sim_VerboseShow;
static Widget rg_dialog, rg1_dialog, rg2_dialog, rg3_dialog, rg_Verbose, rg1_Verbose, rg2_Verbose, rg3_Verbose, rg_Editor, rg_EXST, rg3_NEXT, rg3_EXST, rg3_TEXT;


#include "calc.h"

void ShowSwnRGOptionsDialog(int t) {
    Arg args[1];
    XmString xms = NULL;

    whichType = t;
    if (t == _SWN_ORD_RG)
        xms = XmStringCreateSimple("SWN Ordinary RG Options");
    else if (t == _SWN_SYM_RG)
        xms = XmStringCreateSimple("SWN Symbolic RG Options");
    else if (t == _EGSPN_COMPUTERG)
        xms = XmStringCreateSimple("E-GSPN RG Options");
    /****17/7/08 ESRG by Marco***/
    else if (t == _SWN_ESRG)
        xms = XmStringCreateSimple("ESRG Options");
    else if (t == _SWN_DSRG)
        xms = XmStringCreateSimple("DSRG Options");
    /****17/7/08 ESRG by Marco***/
    /****10/6/10 MDD by Marco***/
    else if (t == _GSPN_MDD)
        xms = XmStringCreateSimple("MDD Options");
    /****10/6/10 MDD by Marco***/
    XtSetArg(args[0], XmNdialogTitle, xms);

    /****17/7/08 ESRG by Marco***/
    if (t == _SWN_ESRG)
        /****17/7/08 ESRG by Marco***/

        XtSetValues(rg1_dialog, args, 1);

    /****17/7/08 ESRG by Marco***/
    else if (t == _SWN_DSRG)
        XtSetValues(rg2_dialog, args, 1);
    /****10/6/10 MDD by Marco***/
    else if (t == _GSPN_MDD)
        XtSetValues(rg3_dialog, args, 1);
    /****10/6/10 MDD by Marco***/
    else
        XtSetValues(rg_dialog, args, 1);
    /****17/7/08 ESRG by Marco***/


    XmStringFree(xms);

    /****17/7/08 ESRG by Marco***/
    if (t == _SWN_ESRG)
        /****17/7/08 ESRG by Marco***/

        XtManageChild(rg1_dialog);

    /****17/7/08 ESRG by Marco***/
    else if (t == _SWN_DSRG) {
        XtManageChild(rg2_dialog);
    }
    /****10/6/10 MDD by Marco***/
    else if (t == _GSPN_MDD) {
        XtManageChild(rg3_dialog);
    }
    /****10/6/10 MDD by Marco***/
    else
        XtManageChild(rg_dialog);
    /****17/7/08 ESRG by Marco***/

}

void HideSwnRGOptionsDialog(void) {
    /****17/7/08 ESRG by Marco***/
    if (whichType == _SWN_ESRG)
        XtUnmanageChild(rg1_dialog);
    else if (whichType == _SWN_DSRG) {
        XtUnmanageChild(rg2_dialog);
    }
    /****10/6/10 MDD by Marco***/
    else if (whichType == _GSPN_MDD) {
        XtUnmanageChild(rg3_dialog);
    }
    /****10/6/10 MDD by Marco***/
    else {
        XtUnmanageChild(rg_dialog);
    }
    /****17/7/08 ESRG by Marco***/
}

/****17/7/08 ESRG by Marco***/
static void Editor1SWNDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
//XtSetSensitive(rg_Editor,FALSE);
    //char *name;
    //sprintf(name,"gedit %s.gd &",GetCurrentFilename());
    //system(name);
    ScheduleSwn(_EDIT_GD/*p*/, 0, 0, 0, 0, 0, 0, 0, 0);
    //ResetConsoleDialog();
    //HideSwnRGOptionsDialog();
}
/****17/7/08 ESRG by Marco***/

static void OkSwnRGDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    Boolean verboseOn, ExStOn, MTorEV, NEXT;
    /*int p;*/
    /****17/11/10 MDD by Marco***/
    char *tmp;
    int bound;
    /****17/11/10 MDD by Marco***/
    if (whichType == _SWN_ESRG) {
        verboseOn = XmToggleButtonGadgetGetState(rg1_Verbose);
        ExStOn = XmToggleButtonGadgetGetState(rg_EXST);
        ScheduleSwn(whichType/*p*/, verboseOn, ExStOn, 0, 0, 0, 0, 0, 0);
    }
    else {
        if (whichType == _SWN_DSRG) {
            verboseOn = XmToggleButtonGadgetGetState(rg2_Verbose);
            ScheduleSwn(whichType/*p*/, verboseOn, 0, 0, 0, 0, 0, 0, 0);
        }
        /****10/6/10 MDD by Marco***/
        if (whichType == _GSPN_MDD) {
            MTorEV = XmToggleButtonGadgetGetState(rg3_EXST);
            NEXT = XmToggleButtonGadgetGetState(rg3_NEXT);
            XtFree(tmp);
            tmp = XmTextFieldGetString(rg3_TEXT);
            if ((bound = atoi(tmp)) >= 0) {
                //verboseOn = XmToggleButtonGadgetGetState(rg3_Verbose);
                ScheduleSwn(whichType/*p*/, verboseOn, MTorEV, NEXT, bound, 0, 0, 0, 0);
            }
            else {
                ScheduleSwn(whichType/*p*/, verboseOn, MTorEV, NEXT, 255, 0, 0, 0, 0);
            }
        }
        /****10/6/10 MDD by Marco***/
        else {
            verboseOn = XmToggleButtonGadgetGetState(rg_Verbose);
            ScheduleSwn(whichType/*p*/, verboseOn, 0, 0, 0, 0, 0, 0, 0);
        }
    }
    HideSwnRGOptionsDialog();
}

static void CancelSwnRGDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    ResetConsoleDialog();
    HideSwnRGOptionsDialog();
}



void RegisterSwnRGOptionsDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"OkSwnRGDialogCB"	, (XtPointer)OkSwnRGDialogCB},
        {"CancelSwnRGDialogCB"	, (XtPointer)CancelSwnRGDialogCB},
//			{"SwnDSRGDialog_Gard",   (XtPointer)EditorSwnRG},
        {"SwnDSRGDialog_GardCB", (XtPointer)Editor1SWNDialogCB}
    };

    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterSwnRGOptionsDialog names\n");
}


void InitSwnRGOptionsDialog(void) {
    rg_dialog = XtNameToWidget(frame_w, "*SwnRGDialog");
    rg_Verbose = XtNameToWidget(rg_dialog, "*SwnRGDialog_VerboseShow");
    /****17/7/08 ESRG by Marco***/
    rg1_dialog = XtNameToWidget(frame_w, "*SwnESRGDialog");
    rg1_Verbose = XtNameToWidget(rg1_dialog, "*SwnRGDialog_VerboseShow");
    rg_EXST = XtNameToWidget(rg1_dialog, "*SwnESRGDialog_ExacStrong");

    rg2_dialog = XtNameToWidget(frame_w, "*SwnDSRGDialog");
    rg2_Verbose = XtNameToWidget(rg2_dialog, "*SwnRGDialog_VerboseShow");
    rg_Editor = XtNameToWidget(rg2_dialog, "*SwnDSRGDialog_Gard");

    /****17/7/08 ESRG by Marco***/

    /****10/6/10 MDD by Marco***/
    rg3_dialog = XtNameToWidget(frame_w, "*GSPNMDDDialog");
    rg3_EXST = XtNameToWidget(rg3_dialog, "*GSPNMDDDialog_ExacStrong");
    rg3_NEXT = XtNameToWidget(rg3_dialog, "*GSPNMDDDialog_Next");
    //rg3_Verbose = XtNameToWidget(rg3_dialog, "*SwnRGDialog_VerboseShow");
    /****10/6/10 MDD by Marco***/
}



void ShowSwnSimOptionsDialog(int t) {
    Arg args[1];
    XmString xms = NULL;

    whichType = t;
    if (t == _SWN_ORD_SIM)
        xms = XmStringCreateSimple("SWN Ordinary Simulation Options");
    else if (t == _SWN_SYM_SIM)
        xms = XmStringCreateSimple("SWN Symbolic Simulation Options");
    else if (t == _EGSPN_SIMULATION)
        xms = XmStringCreateSimple("E-GSPN Simulation Options");

    XtSetArg(args[0], XmNdialogTitle, xms);
    XtSetValues(sim_dialog, args, 1);
    XmStringFree(xms);
    XtManageChild(sim_dialog);
}

void HideSwnSimOptionsDialog(void) {
    XtUnmanageChild(sim_dialog);
}



static void OkSimOptionsDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    char *tmp;
    char *errmess = NULL;
    Widget focusTo;
    int initWaste, midWaste, minBatch, maxBatch, approx, seed, confLevel, verbose;
    static int cvtClevel[] = {60, 70, 80, 90, 95, 99};


    tmp = XmTextFieldGetString(sim_InitWasteText);
    if ((initWaste = atoi(tmp)) >= 0) {
        XtFree(tmp);
        tmp = XmTextFieldGetString(sim_MidWasteText);
        if ((midWaste = atoi(tmp)) > 0) {
            XtFree(tmp);
            tmp = XmTextFieldGetString(sim_MinBatchText);
            if ((minBatch = atoi(tmp)) > 0) {
                XtFree(tmp);
                tmp = XmTextFieldGetString(sim_MaxBatchText);
                if ((maxBatch = atoi(tmp)) > 0) {
                    XtFree(tmp);
                    if (minBatch <= maxBatch) {
                        tmp = XmTextFieldGetString(sim_ApproxText);
                        if ((approx = atoi(tmp)) > 0) {
                            XtFree(tmp);
                            tmp = XmTextFieldGetString(sim_SeedText);
                            if ((seed = atoi(tmp)) <= 0) {
                                errmess = "Incorrect Seed Value [ > 0]";
                                focusTo = sim_SeedText;
                            }
                            else {
                                // char paramStr[500];

                                confLevel = cvtClevel[whichClevel];
                                verbose = XmToggleButtonGadgetGetState(sim_VerboseShow);

                                /*sprintf(paramStr,"swn_sym_sim -f %d -t %d -m %d -M %d -c %d -a %d -s %d %s",
                                		initWaste,midWaste,minBatch,maxBatch,confLevel,approx,seed,verbose);
                                puts(paramStr);
                                */

                                ScheduleSwn(whichType/* << 1*/, initWaste, midWaste, minBatch, maxBatch, confLevel, approx, seed, verbose);
                            }
                        }
                        else {
                            errmess = "Incorrect Accuracy Value [ > 0]";
                            focusTo = sim_ApproxText;
                        }
                    }
                    else {
                        errmess = "Wrong Minimum or Maximum Batch Length [ Min <= Max]";
                        focusTo = sim_MaxBatchText;
                    }

                }
                else {
                    errmess = "Incorrect Maximum Batch Length [ > 0]";
                    focusTo = sim_MaxBatchText;
                }
            }
            else {
                errmess = "Incorrect Minimum Batch Length [ > 0]";
                focusTo = sim_MinBatchText;
            }
        }
        else {
            errmess = "Incorrect Batch Spacing Value [ > 0]";
            focusTo = sim_MidWasteText;
        }
    }
    else {
        errmess = "Incorrect Initial Transitory Value [ >= 0]";
        focusTo = sim_InitWasteText;
    }
    XtFree(tmp);
    if (errmess != NULL) {
        int insertlen;

        ShowErrorDialog(errmess, sim_dialog);
        tmp = XmTextFieldGetString(focusTo);
        insertlen = strlen(tmp);
        XmProcessTraversal(focusTo, XmTRAVERSE_CURRENT);
        XmTextFieldSetSelection(focusTo, 0, insertlen, XtLastTimestampProcessed(XtDisplay(focusTo)));
        XmTextFieldSetHighlight(focusTo, 0, insertlen, XmHIGHLIGHT_SELECTED);
        XtFree(tmp);
    }
    else
        HideSwnSimOptionsDialog();
}

static void CancelSimOptionsDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    ResetConsoleDialog();
    HideSwnSimOptionsDialog();
}

static void SwnSimOptionsCB(Widget w, int closure, XtPointer call_data) {
    whichClevel = closure;
}

void RegisterSwnSimOptionsDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"SWN_CLEVEL_60"	, (XtPointer)SWN_CLEVEL_60},
        {"SWN_CLEVEL_70"	, (XtPointer)SWN_CLEVEL_70},
        {"SWN_CLEVEL_80"	, (XtPointer)SWN_CLEVEL_80},
        {"SWN_CLEVEL_90"	, (XtPointer)SWN_CLEVEL_90},
        {"SWN_CLEVEL_95"	, (XtPointer)SWN_CLEVEL_95},
        {"SWN_CLEVEL_99"	, (XtPointer)SWN_CLEVEL_99},
        {"SwnSimOptionsCB"	, (XtPointer)SwnSimOptionsCB},
        {"CancelSimOptionsDialogCB"	, (XtPointer)CancelSimOptionsDialogCB},
        {"OkSimOptionsDialogCB"	, (XtPointer)OkSimOptionsDialogCB}
    };

    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterSwnSimOptionsDialog names\n");
}


void InitSwnSimOptionsDialog(void) {
    sim_dialog = XtNameToWidget(frame_w, "*SimOptionsDialog");
    sim_InitWasteText = XtNameToWidget(sim_dialog, "*SimOptionsDialog_InitWasteText");
    sim_MidWasteText = XtNameToWidget(sim_dialog, "*SimOptionsDialog_MidWasteText");
    sim_MinBatchText = XtNameToWidget(sim_dialog, "*SimOptionsDialog_MinBatchText");
    sim_MaxBatchText = XtNameToWidget(sim_dialog, "*SimOptionsDialog_MaxBatchText");
    sim_ApproxText = XtNameToWidget(sim_dialog, "*SimOptionsDialog_ApproxText");
    sim_SeedText = XtNameToWidget(sim_dialog, "*SimOptionsDialog_SeedText");
    sim_VerboseShow = XtNameToWidget(sim_dialog, "*SimOptionsDialog_VerboseShow");
    rg3_TEXT = XtNameToWidget(rg3_dialog, "*GSPNMDDOptionsDialog_PX");
    XmTextFieldSetString(rg3_TEXT, "255");
    XmTextFieldSetString(sim_InitWasteText, "1000");
    XmTextFieldSetString(sim_MidWasteText, "1000");
    XmTextFieldSetString(sim_MinBatchText, "1000");
    XmTextFieldSetString(sim_MaxBatchText, "2000");
    XmTextFieldSetString(sim_ApproxText, "15");
    XmTextFieldSetString(sim_SeedText, "31415");
    whichClevel = SWN_CLEVEL_60;
}

