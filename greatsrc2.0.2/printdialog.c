#define ___PRINTDIALOG___
#include "global.h"
#include "postscript.h"
#include "draw.h"
#include "overview.h"
#include "showgdi.h"
#include "printarea.h"
#include "filer.h"
#include "printdialog.h"

#include <Mrm/MrmPublic.h>

#include <signal.h>
#if defined(SunOS5x)
#       include <wait.h>
#else
#       include <sys/wait.h>
#endif
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/param.h>
#include <unistd.h>
#include <fcntl.h>

extern XtAppContext	appContext;

static Widget   print_dlg, print_print, print_printer, print_label2,
       print_over, print_paperr, print_paper, print_frame3, print_frame4,
       print_pri, print_file;

static 	float a1, a2, a3, a4;

static DrawingSurface overDS, paperDS, paperrDS;
static 	char *tmpname;

static int rx, ry, rh = 20, rw = 20;
static int prx, pry, prh = 20, prw = 20;
float ax = 1., ay = 1.;
static Boolean dragging, sizing, rotate, fileprint, tex;

static void InitPrintOver(void) {
    InitDS(print_over, &overDS);

    XSetBackground(overDS.display, overDS.gc, gWhite);
    XSetForeground(overDS.display, overDS.gc, gBlack);
    /*	XSetPlaneMask(overDS.display,overDS.gc,1l);*/
    SetFunction(OR, &overDS);

    InitDS(print_paper, &paperDS);

    XSetBackground(paperDS.display, paperDS.gc, gWhite);
    XSetForeground(paperDS.display, paperDS.gc, gBlack);
    /*	XSetPlaneMask(paperDS.display,paperDS.gc,1l);*/
    SetFunction(XOR, &paperDS);

    InitDS(print_paperr, &paperrDS);

    XSetBackground(paperrDS.display, paperrDS.gc, gWhite);
    XSetForeground(paperrDS.display, paperrDS.gc, gBlack);
    /*	XSetPlaneMask(paperrDS.display,paperrDS.gc,1l);*/
    SetFunction(XOR, &paperrDS);
}

static void DisposePrintOver(void) {
    DisposeDS(&overDS);
    DisposeDS(&paperDS);
    DisposeDS(&paperrDS);
}

void HidePrintChangeDialog(void) {
    /*	XtUnmapWidget(XtParent(print_dlg));*/
    XtUnmanageChild(print_dlg);
}

static void CancelPrintDialogCB(Widget w, XtPointer closure, XtPointer call_data) {
    HidePrintChangeDialog();
}

static void UpdateLabel(void) {
    char string[200];
    XmString xms;
    Arg args[1];

    sprintf(string, "Top=%-2.1f Left=%-2.1f Width=%-2.1f Height=%-2.1f (cm)", ((float)pry) / 10., ((float)prx) / 10., ((float)prw) / 10., ((float)prh) / 10.);
    xms = XmStringCreate(string, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(args[0], XmNlabelString, xms);
    XtSetValues(print_label2, args, 1);
    XmStringFree(xms);

}


static void CheckBounds(void) {
    if (!rotate) {
        if (rw > 210) {
            prw = rw / 2;
            prh = rh / 2;
        }
        else {
            prw = rw;
            prh = rh;

        }
        if (rh > rw) {
            ax = 1.;
            ay = ((float)rh) / ((float)rw);
        }
        else {
            ax = ((float)rw) / ((float)rh);
            ay = 1.;
        }
    }
    else {
        if (rh > 210 || rw > 297) {
            prw = rw / 2;
            prh = rh / 2;
        }
        else {
            prw = rw;
            prh = rh;

        }
        if (rh > rw) {
            ax = 1.;
            ay = ((float)rh) / ((float)rw);
        }
        else {
            ax = ((float)rw) / ((float)rh);
            ay = 1.;
        }

    }
    UpdateLabel();
}


static void RotateCB(Widget w, XtPointer closure, XmToggleButtonCallbackStruct *call_data) {
    if (call_data->set) {
        rotate = TRUE;
        XtUnmapWidget(print_frame3);
        XtMapWidget(print_frame4);
    }
    else {
        rotate = FALSE;
        XtUnmapWidget(print_frame4);
        XtMapWidget(print_frame3);
    }
    prx = pry = 0;
    CheckBounds();
}

static void TexCB(Widget w, XtPointer closure, XmToggleButtonCallbackStruct *call_data) {
    if (call_data->set) {
        tex = TRUE;
        XmToggleButtonGadgetSetState(print_file, TRUE, TRUE);
        XtSetSensitive(print_pri, FALSE);
    }
    else {
        tex = FALSE;
        XtSetSensitive(print_pri, TRUE);
    }
}


static void FileCB(Widget w, XtPointer closure, XmToggleButtonCallbackStruct *call_data) {
    Arg args[1];
    XmString temp;

    fileprint = call_data->set;
    XtSetSensitive(print_printer, !fileprint);
    temp = XmStringCreate(fileprint ? "Save" : "Print", XmSTRING_DEFAULT_CHARSET);
    XtSetArg(args[0], XmNlabelString, temp);
    XtSetValues(print_print, args, 1);
    XmStringFree(temp);
}

void ShowPrintDialog(void) {
    int a1, a2, a3, a4;

    dragging = FALSE;
    GetPrintArea(&a1, &a2, &a3, &a4);
    rx = a1;
    ry = a2;
    rw = a3;
    rh = a4;
    CheckBounds();
    prx = pry = 0;
    /*	XtMapWidget(XtParent(print_dlg));*/
    XtManageChild(print_dlg);
    InitPrintOver();
}



static Boolean IsInDragRect(int x, int y) {
    return (x >= rx && x <= (rx + rw) && y >= ry && y <= (ry + rh));
}

static Boolean IsInDragHandle(int x, int y) {
    return (x >= (rx + rw - 5) && x <= (rx + rw) && y >= (ry + rh - 5) && y <= (ry + rh));
}

static Boolean IsInDragPaperRect(int x, int y) {
    return (x >= prx && x <= (prx + prw) && y >= pry && y <= (pry + prh));
}

static Boolean IsInDragPaperHandle(int x, int y) {
    return (x >= (prx + prw - 5) && x <= (prx + prw) && y >= (pry + prh - 5) && y <= (pry + prh));
}

static void DrawHrect(DrawingSurfacePun ds) {
    XDrawRectangle(ds->display, ds->drawable, ds->gc, rx, ry, rw, rh);
    XFillRectangle(ds->display, ds->drawable, ds->gc, rx + rw - 5, ry + rh - 5, 5, 5);
}

static void DrawHrectPaper(DrawingSurfacePun ds) {
    XDrawRectangle(ds->display, ds->drawable, ds->gc, prx, pry, prw, prh);
    XFillRectangle(ds->display, ds->drawable, ds->gc, prx + prw - 5, pry + prh - 5, 5, 5);
}

static void RedrawPrintOverviewCB(Widget w, DrawingSurfacePun ds, XmDrawingAreaCallbackStruct *str) {
    if (((XExposeEvent *)str->event)->count != 0)
        return;
    printf("RedrawPrintOverviewCB ds=%p\n", ds);
    RedisplayPrintOverview(ds);
}

static int CheckForDeadPrintChild(int p) {
    int status, ret;

#ifdef __HP_SOURCE
    int reserved;
    /*
    #elif defined(SunOS5x)
            siginfo_t info;
            id_t id;*/
#else
    struct rusage rus;
#endif

#ifdef __HP_SOURCE
    if ((ret = wait3(&status, WNOHANG, &reserved)) != 0)	/* non blocking wait for child p */
        /*#elif defined(SunOS5x)
                if((ret = waitid(P_ALL,id,&info,WNOHANG))!=0)*/   /* non blocking wait for child p */
#else
    if ((ret = wait3(&status, WNOHANG, &rus)) != 0)	/* non blocking wait for child p */
#endif
    {
        char message[1000];
        int errCode;

        ClearWaitCursor(print_dlg);
        errCode = WEXITSTATUS(status);
        printf("errCode = %d, status = %d\n", errCode, status);

        if (errCode & 2) {
            sprintf(message , "Sorry cannot remove temporary file :\n%s", tmpname);
            ShowErrorDialog(message, print_dlg);
        }
        if (errCode & 1) {
            char *printername;

            printername = XmTextFieldGetString(print_printer);
            sprintf(message , "Sorry cannot print on %s", printername);
            ShowErrorDialog(message, print_dlg);
            XtFree(printername);
        }
        if (!errCode) {
            SetPrintArea(a1, a2, a3, a4);
            HidePrintChangeDialog();
        }
        return TRUE;
    }
    return FALSE;
}

static void PrintPrintDialogCB(Widget w, XtPointer closure, XtPointer call_data) {

    /*	GetTruePrintArea(&a1,&a2,&a3,&a4); */
    SetWaitCursor(print_dlg);
    a1 = (float)(rx * 5);
    a2 = (float)(ry * 5);
    a3 = (float)(rw * 5);
    a4 = (float)(rh * 5);
    tmpname = tmpnam(NULL);
    StartPS(tmpname, a1, a2, a3, a4, prx, pry, prh, prw, rotate, tex);
    PSflag = TRUE;
    RedisplayNet();
    EndPS(tex);
    PSflag = FALSE;
    if (FALSE) {
        ShowErrorDialog("Sorry : cannot generate Temporary Postscipt File", print_dlg);
    }
    else {

        if (fileprint) {
            if (tex)
                SaveTeX(print_dlg, tmpname);
            else
                SavePostscript(print_dlg, tmpname);
        }
        else {
            XtAppAddWorkProc(appContext, (XtWorkProc)CheckForDeadPrintChild, 0);
            switch (fork()) {
            case 0: {
                char *printername;
                char temp[3000];

                sprintf(temp, "%s/PrintCommand", getenv("GREATSPN_SCRIPTDIR"));
                printername = XmTextFieldGetString(print_printer);
                printf("Executing /bin/csh -f %s %s %s\n", temp, printername, tmpname);
                execl("/bin/csh", "-f", temp, printername, tmpname, (char *)0);
                break;
            }
            case -1:
                ShowErrorDialog("Sorry : cannot fork", print_dlg);
                break;
            }

        }
    }
}

void UpdateAndQuit(void) {
    SetPrintArea(a1, a2, a3, a4);
    HidePrintChangeDialog();
}

static void RedrawPrintPaperCB(Widget w, DrawingSurfacePun ds, XmDrawingAreaCallbackStruct  *str) {
    if (((XExposeEvent *)str->event)->count != 0)
        return;
    XClearWindow(ds->display, ds->drawable);
    SetFunction(XOR, ds);
    DrawHrectPaper(ds);
    /*	SetFunction(XOR,ds);*/
}

static void RedisplayPrintOverview(DrawingSurfacePun ds) {
    XClearWindow(ds->display, ds->drawable);
    SetFunction(OR, ds);
    over_draw_net(netobj, ds->gc, ds->drawable);

    SetFunction(XOR, ds);
    DrawHrect(&overDS);
}

static void PrintDialogPopDownCB(Widget w, XtPointer closure, XtPointer call_data) {
    DisposePrintOver();
}


static void PrintOverEH(Widget w, XtPointer closure, XEvent *ev, Boolean *continue_to_dispatch) {
    int x, y;
    static int xdb, ydb;

    switch (ev->type) {
    case ButtonPress:
        x = ev->xbutton.x;
        y = ev->xbutton.y;
        if (IsInDragRect(x, y)) {
            sizing = IsInDragHandle(x, y);
            dragging = TRUE;
            xdb = x;
            ydb = y;
            SetFunction(XOR, &overDS);
        }
        break;
    case ButtonRelease: {
        x = ev->xbutton.x;
        y = ev->xbutton.y;
        dragging = FALSE;
        SetFunction(OR, &overDS);
        if (rotate)
            DrawHrectPaper(&paperrDS);
        else
            DrawHrectPaper(&paperDS);
        prx = pry = 0;
        CheckBounds();
        if (rotate)
            DrawHrectPaper(&paperrDS);
        else
            DrawHrectPaper(&paperDS);

        break;
    }
    case MotionNotify:
        x = ev->xmotion.x;
        y = ev->xmotion.y;
        if (dragging) {
            int	newx, newy;

            if (sizing) {
                newx = rw + (x - xdb);
                newy = rh + (y - ydb);

                DrawHrect(&overDS);

                if ((rx + newx) < 400 && newx >= 6)
                    rw += (x - xdb);
                if ((ry + newy) < 240 && newy >= 6)
                    rh += (y - ydb);

                DrawHrect(&overDS);
                xdb = x;
                ydb = y;
            }
            else {
                newx = rx + (x - xdb);
                newy = ry + (y - ydb);
                DrawHrect(&overDS);

                if ((newx + rw) < 400 && newx >= 0)
                    rx += (x - xdb);
                if ((newy + rh) < 240 && newy >= 0)
                    ry += (y - ydb);

                DrawHrect(&overDS);
                xdb = x;
                ydb = y;
            }
        }
        break;
    }

}


static void PrintPaperEH(Widget w, XtPointer closure, XEvent *ev, Boolean *continue_to_dispatch) {
    int x, y;
    static int xdb, ydb;

    switch (ev->type) {
    case ButtonPress:
        x = ev->xbutton.x;
        y = ev->xbutton.y;
        if (IsInDragPaperRect(x, y)) {
            sizing = IsInDragPaperHandle(x, y);
            dragging = TRUE;
            xdb = x;
            ydb = y;
            UpdateLabel();
        }
        break;
    case ButtonRelease: {
        x = ev->xbutton.x;
        y = ev->xbutton.y;
        dragging = FALSE;
        UpdateLabel();
        break;
    }
    case MotionNotify:
        x = ev->xmotion.x;
        y = ev->xmotion.y;
        if (dragging) {
            int	newx, newy;

            if (sizing) {
                if (ax == 1.) {
                    newx = x - prx;
                    newy = (int)(((float)newx) * ay);
                }
                else {
                    newy = y - pry;
                    newx = (int)(((float)newy) * ax);
                }

                DrawHrectPaper(&paperDS);

                /*					if((prx+newx) < 210 && newx >= 6 )
                	prw+=(x-xdb);
                if((pry+newy) < 297 && newy >= 6)
                	prh+=(y-ydb);  */

                if ((prx + newx) < 210 && newx >= 6  && (pry + newy) < 297 && newy >= 6) {
                    prw = newx;
                    prh = newy;
                }
                DrawHrectPaper(&paperDS);
                xdb = x;
                ydb = y;
            }
            else {
                newx = prx + (x - xdb);
                newy = pry + (y - ydb);
                DrawHrectPaper(&paperDS);

                if ((newx + prw) < 210 && newx >= 0)
                    prx += (x - xdb);
                if ((newy + prh) < 297 && newy >= 0)
                    pry += (y - ydb);

                DrawHrectPaper(&paperDS);
                xdb = x;
                ydb = y;
            }
            UpdateLabel();
        }
        break;
    }

}

static void PrintPaperREH(Widget w, XtPointer closure, XEvent *ev, Boolean *continue_to_dispatch) {
    int x, y;
    static int xdb, ydb;

    switch (ev->type) {
    case ButtonPress:
        x = ev->xbutton.x;
        y = ev->xbutton.y;
        if (IsInDragPaperRect(x, y)) {
            sizing = IsInDragPaperHandle(x, y);
            dragging = TRUE;
            xdb = x;
            ydb = y;
            UpdateLabel();
        }
        break;
    case ButtonRelease: {
        x = ev->xbutton.x;
        y = ev->xbutton.y;
        dragging = FALSE;
        UpdateLabel();
        break;
    }
    case MotionNotify:
        x = ev->xmotion.x;
        y = ev->xmotion.y;
        if (dragging) {
            int	newx, newy;

            if (sizing) {
                if (ax == 1.) {
                    newx = x - prx;
                    newy = (int)(((float)newx) * ay);
                }
                else {
                    newy = y - pry;
                    newx = (int)(((float)newy) * ax);
                }

                DrawHrectPaper(&paperrDS);

                if ((prx + newx) < 297 && newx >= 6  && (pry + newy) < 210 && newy >= 6) {
                    prw = newx;
                    prh = newy;
                }
                DrawHrectPaper(&paperrDS);
                xdb = x;
                ydb = y;
            }
            else {
                newx = prx + (x - xdb);
                newy = pry + (y - ydb);
                DrawHrectPaper(&paperrDS);

                if ((newx + prw) < 297 && newx >= 0)
                    prx += (x - xdb);
                if ((newy + prh) < 210 && newy >= 0)
                    pry += (y - ydb);

                DrawHrectPaper(&paperrDS);
                xdb = x;
                ydb = y;
            }
            UpdateLabel();
        }
        break;
    }

}

static void PrintDialogCenterCB(Widget w, int closure, XtPointer call_data) {
    int pagew, pageh;

    pagew = rotate ? 297 : 210;
    pageh = rotate ? 210 : 297;

    if (rotate)
        DrawHrectPaper(&paperrDS);
    else
        DrawHrectPaper(&paperDS);

    switch (closure) {
    case CENTER_V:
        pry = (pageh - prh) / 2;
        break;
    case CENTER_H:
        prx = (pagew - prw) / 2;
        break;
    case CENTER_HV:
        prx = (pagew - prw) / 2;
        pry = (pageh - prh) / 2;
        break;
    }
    if (rotate)
        DrawHrectPaper(&paperrDS);
    else
        DrawHrectPaper(&paperDS);

    UpdateLabel();
}

void RegisterPrintDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"CENTER_H", (XtPointer)CENTER_H},
        {"CENTER_V"		, (XtPointer)CENTER_V},
        {"CENTER_HV", (XtPointer)CENTER_HV},
        {"RotateCB"		, (XtPointer)RotateCB},
        {"TexCB"		, (XtPointer)TexCB},
        {"FileCB"		, (XtPointer)FileCB},
        {"RedrawPrintOverviewCB"		, (XtPointer)RedrawPrintOverviewCB},
        {"PrintPrintDialogCB"		, (XtPointer)PrintPrintDialogCB},
        {"RedrawPrintPaperCB"		, (XtPointer)RedrawPrintPaperCB},
        {"PrintDialogCenterCB"		, (XtPointer)PrintDialogCenterCB},
        {"CancelPrintDialogCB"		, (XtPointer)CancelPrintDialogCB},
        {"PrintDialogPopDownCB"		, (XtPointer)PrintDialogPopDownCB},
        {"OVERDS_ADDRESS"		, (XtPointer) &overDS},
        {"PAPERDS_ADDRESS"		, (XtPointer) &paperDS},
        {"PAPERRDS_ADDRESS"		, (XtPointer) &paperrDS},
    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);
    printf("RegisterPrintDialog ds=%p\n", &overDS);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterPrintDialog names\n");
}

void InitPrintChangeDialog(void) {
    char *printer;

    print_dlg = XtNameToWidget(frame_w, "*PrintDialog");
    print_print = XtNameToWidget(print_dlg, "*PrintDialog_Print");
    print_over = XtNameToWidget(print_dlg, "*PrintDialog_Over");
    print_frame3 = XtNameToWidget(print_dlg, "*PrintDialog_Frame3");
    print_paper = XtNameToWidget(print_frame3, "*PrintDialog_Paper");
    print_frame4 = XtNameToWidget(print_dlg, "*PrintDialog_Frame4");
    print_paperr = XtNameToWidget(print_frame4, "*PrintDialog_PaperR");
    print_label2 = XtNameToWidget(print_dlg, "*PrintDialog_Label2");
    print_printer = XtNameToWidget(print_dlg, "*PrintDialog_Printer");
    print_file = XtNameToWidget(print_dlg, "*PrintDialog_File");
    print_pri = XtNameToWidget(print_dlg, "*PrintDialog_Pri");

    /*	InitPrintOver();*/
    rotate = FALSE;
    printer = getenv("GSPN_DEFAULT_PRINTER");
    if (printer == NULL) {
        ShowInfoDialog("Warning GSPN_DEFAULT_PRINTER environment variable is not defined:\n\nDefault printer is unknownprinter", mainwin);
        printer = "unknownprinter";
    }
    XmTextFieldSetString(print_printer, printer);
    /*XtMapWidget(print_dlg);*/

    XtAddCallback(print_over, XmNexposeCallback, (XtCallbackProc) RedrawPrintOverviewCB, &overDS);
    XtAddCallback(print_paper, XmNexposeCallback, (XtCallbackProc) RedrawPrintPaperCB, &paperDS);
    XtAddCallback(print_paperr, XmNexposeCallback, (XtCallbackProc) RedrawPrintPaperCB, &paperrDS);

    XtAddEventHandler(print_over, PointerMotionMask | ButtonPressMask | ButtonReleaseMask, FALSE, PrintOverEH, NULL);
    XtAddEventHandler(print_paper, PointerMotionMask | ButtonPressMask | ButtonReleaseMask, FALSE, PrintPaperEH, NULL);
    XtAddEventHandler(print_paperr, PointerMotionMask | ButtonPressMask | ButtonReleaseMask, FALSE, PrintPaperREH, NULL);

}


