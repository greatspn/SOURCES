/***********************************************************************
**  file: main.c                                                      **
**                                                                    **
***********************************************************************/

#include "global.h"
#include "Prova.h"
#include "buttons.h"
#include "menuvalues.h"
#include "arcdialog.h"
#include "aboutdialog.h"
#include "colordialog.h"
#include "placedialog.h"
#include "transdialog.h"
#include "showdialog.h"
#include "optionsdialog.h"
#include "warningdialog.h"
#include "solvedialog.h"
#include "resultdialog.h"
#include "ratedialog.h"
#include "printdialog.h"
#include "mdgrammardialog.h"
#include "layerviewdialog.h"
#include "layereditdialog.h"
#include "markdialog.h"
#include "consoledialog.h"
#include "commentdialog.h"
#include "overview.h"
#include "showgdi.h"
#include "rescale.h"
#include "grid.h"
#include "filer.h"
#include "toolkit.h"
#include "MenuCallBacks.h"
#include "simdialog.h"
#include "swn.h"
#include "zoom.h"
#include "menuinit.h"
#include "afire.h"
#include <errno.h>
#include "unfold.h"


XtAppContext	appContext;


/***********************************************************************
**                                                                    **
**               F O R W A R D   D E F I N I T I O N S                **
**                                                                    **
***********************************************************************/

void    InitMainWindow();
extern void    InitMenuBar();
void    InitWorkWindow();
void    InitCommandWindow();
void	InitOther();
extern void InitPopup();
extern void PopPopupEH();
extern void PopPopupMainEH();
extern void InitDialogs();
extern void RedrawOverviewCB();
extern void HelpToggleCB();
extern void ActionsCB();
extern void SetActionsCB();
extern void PinvActionsCB();
extern void TinvActionsCB();
extern void PrintAreaCB();
extern void modify_menu_procCB();
extern void InitGSPNShellErrors();


/***********************************************************************
**                                                                    **
**                   G L O B A L   V A R I A B L E S                  **
**                                                                    **
***********************************************************************/
static String fallback[] = {
    "GreatSPN_Motif*FileDialog*background:				Gray65",
    "GreatSPN_Motif*FileDialog*foreground:				Black",
    "GreatSPN_Motif*InputDialog.*.background:				Gray65",
    "GreatSPN_Motif*InputDialog.*.foreground:				Black",
    "GreatSPN_Motif*TearOffControl.background:			 Gray50",
    "GreatSPN_Motif*ScrollArea.ScrolledWindowClipWindow.background:	Gray50",
    "GreatSPN_Motif*LayerViewDialog_Scroller.ScrolledWindowClipWindow.background:	Gray65",
    "GreatSPN_Motif*TearOffControl.background:			 Gray50",
    "GreatSPN_Motif*.foreground:			 Black",
    /*   "GreatSPN_Motif*MenuBar*background:	Gray50",*/
    /*"GreatSPN_Motif.title:		GreatSPN 2.0 : Graphical Editor and Analyzer for Timed and Stochastic Petri Nets",*/
    NULL
};

static MrmRegisterArg	regvec[] = {{"ButtonsCB", (XtPointer) ButtonsCB},
    {"HelpToggleCB", (XtPointer)	HelpToggleCB},
    {"ActionsCB", (XtPointer)	ActionsCB},
    {"SetActionsCB", (XtPointer)	SetActionsCB},
    {"PinvActionsCB", (XtPointer)	PinvActionsCB},
    {"TinvActionsCB", (XtPointer)	TinvActionsCB},
    {"modify_menu_procCB", (XtPointer)	modify_menu_procCB},

    /*Command Area Buttons Values*/
    {"PLACE_BUTTON", (XtPointer)	PLACE_BUTTON},
    {"IMTRANS_BUTTON", (XtPointer)IMTRANS_BUTTON	},
    {"EXTRANS_BUTTON", (XtPointer)	EXTRANS_BUTTON},
    {"DETRANS_BUTTON", (XtPointer)DETRANS_BUTTON},
    {"ARC_BUTTON", (XtPointer)ARC_BUTTON	},
    {"TOKEN_BUTTON", (XtPointer)TOKEN_BUTTON	},
    {"RATES_BUTTON", (XtPointer)RATES_BUTTON	},
    {"RESULTS_BUTTON", (XtPointer)RESULTS_BUTTON	},
    {"TAG_BUTTON", (XtPointer)TAG_BUTTON	},
    {"COLOR_BUTTON", (XtPointer)COLOR_BUTTON	}
};
static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);


Widget	appshell,			/* Application Shell          */
        mainwin,			/* XmMainWindow		      */
        menubar,			/* MainWindow Menu Bar	      */
        workwin,			/* MainWindow Work Area       */
        horzscroll,			/* MainWindow Horizontal Scrl */
        vertscroll,			/* MainWindow Vertical Scroll */
        logo;
Arg	arglist[16];			/* For programmatic rsrc stuf */
Widget sb1, sb2;

int color_depth = 1;
Widget  clientwin, statuswin, ModifyArea, currentaction, commandwin, scrollwin, insetframe, popup, PinvPopup, TinvPopup, EndShowPopup;
Widget  ToolButtons[10], statusframe, drawwin;

extern Widget mousewin;

/***********************************************************************
**                                                                    **
**  InitDialogs()                                                     **
**                                                                    **
***********************************************************************/


void   InitDialogs() {
    InitOverviewDialog();
    InitArcChangeDialog();
    InitTransChangeDialog();
    InitEditLayerDialog();
    InitViewLayerDialog();
    InitMDGrammarDialog();
    InitCommentDialog();
    InitWarningDialog();
    InitAboutDialog();
    InitPlaceChangeDialog();
    InitMarkChangeDialog();
    InitRateChangeDialog();
    InitColorChangeDialog();
    InitShowDialog();
    InitResChangeDialog();
    InitPrintChangeDialog();
    InitSolveDialog();
    InitConsoleDialog();
    InitOptionsDialog();
    InitSimulationDialog();
    InitSwnRGOptionsDialog();
    InitSwnSimOptionsDialog();
    InitSwnUnfoldOptionsDialog();
}

void InitMainWindow(void) {
    frame_w = mainwin = XtNameToWidget(appshell, "*MainWin");
    fprintf(stderr, "mainwin %p\n", mainwin);
}

Font sym_font;
void	InitFonts() {
    Display *display = XtDisplay(mainwin);
    XFontStruct *font_info;

    Font *fns[] = {
        &gachab_font,
        &gachar_font,
        &cour_font,
        &time_font18,
        &time_font14,
        &cmr_font,
        &sail_font,
        &sym_font
    };

    char *fnn[] = {
        "-adobe-courier-bold-o-normal--12-120-75-75-m-70-iso8859-1",
        "-adobe-courier-medium-r-normal--12-120-75-75-m-70-iso8859-1",
        "-adobe-courier-medium-r-normal--10-100-75-75-m-60-iso8859-1",
        "-adobe-times-medium-i-normal--18-180-75-75-p-94-iso8859-1",
        "-adobe-times-medium-i-normal--14-100-100-100-p-73-iso8859-1",
        "-adobe-helvetica-bold-o-normal--8-80-75-75-p-50-iso8859-1",
        "-adobe-helvetica-medium-o-normal--8-80-75-75-p-47-iso8859-1",
        "-adobe-Symbol-medium-R-normal--10-100-75-75-p-61-ADOBE-FONTSPECIFIC"
    };

    int i;

    for (i = 0; i < (sizeof(fnn) / sizeof(char *)); i++) {
        /*      puts( fnn[i] );*/
        /*
         * puts is not buffered, you don't need to call fflush
         */
        if (NULL == (font_info =  XLoadQueryFont(display, fnn[i]))) {
            (void) fprintf(
                stderr,
                "Sorry,can't find \"%s\".\nUsing \"fixed\" instead.\n",
                fnn[i]
            );
            fflush(stderr);
            /*
             * If it can't be found, be polite and set it to something
             * else so the program doesn't just puke
             */
            font_info = XLoadQueryFont(display, "fixed");
        }

        *fns[i] = font_info->fid;
        XFlush(XtDisplay(frame_w));
    }

}


void	InitWorkWindow() {
    Arg args[2];

    workwin = XtNameToWidget(mainwin, "ScrollArea");
    XtSetArg(args[0], XmNhorizontalScrollBar, &sb1);
    XtSetArg(args[1], XmNverticalScrollBar, &sb2);
    XtGetValues(workwin, args, 2);

    XtRemoveAllCallbacks(sb1, XmNdragCallback);
    XtRemoveAllCallbacks(sb2, XmNdragCallback);

    /*    InitMouseHelp(); */

    drawwin = XtNameToWidget(workwin, "*DrawWin");
    XtAddEventHandler(drawwin, ButtonPressMask, FALSE, PopPopupEH, NULL);
    XtAddEventHandler(drawwin, PointerMotionMask, FALSE, PopPopupEH, NULL);

    statuswin = XtNameToWidget(mainwin, "*StatusArea");
    ModifyArea = XtNameToWidget(mainwin, "*ModifyArea");
    currentaction = XtNameToWidget(mainwin, "*CurrentAction");

}


void	InitOther() {
    edit_file[0] = '\0';

    /*    puts("    ...  start init_net"); fflush(stdout);*/
    if (init_net() == 0)
        exit(1);

    {
        register int    i;
        for (i = 0; i < MAX_BKUP; bkup_netobj[i++] = NULL);
    }

    ISQRT2 = (double) 1 / sqrt((double) 2.0);
    /*    puts("    ...  start reset_net_object"); fflush(stdout);*/
    reset_net_object(&selected);
    /*    puts("    ...  start InitMouseHelpDialog"); fflush(stdout);*/
    InitMouseHelpDialog();
    /*    puts("    ...  start init_panel"); fflush(stdout);*/
    init_panel();
    /*    puts("    ...  start InitDialogs"); fflush(stdout);*/
    InitDialogs();
    /*
      puts("    ...  start setup_initialstate"); fflush(stdout);
     setup_initialstate();
    */
    /*    puts("    ...  start InitOptions"); fflush(stdout);*/
    InitOptions();

    /*    puts("    ...  start init_animation"); fflush(stdout);*/
    init_animation();

    /*    puts("    ...  start clear_proc_f"); fflush(stdout);*/
    clear_proc_f();

    /* ?????????????? */
    /*    puts("    ...  start init_setup_initialstate"); fflush(stdout);*/
    setup_initialstate();
    /* ?????????????? */
}





/***********************************************************************
**                                                                    **
**  main( argc, argv )                                                **
**                                                                    **
**  Program entry point. Creates shell, calls initialization funcs,   **
**  and turns control over to event loop.                             **
**                                                                    **
***********************************************************************/

int	main(int     argc,
         char    *argv[]) {

    MrmInitialize();

    appshell = XtAppInitialize(&appContext, "GreatSPN_Motif", NULL, 0, &argc, argv, fallback, NULL, 0);
#ifdef Linux
    /* Enables the application to talk with editres.
       For debug purposes only */
    /*   XmdRegisterEditres(appshell);*/
#endif
    OpenHierarchy();

    if (MrmRegisterNames(regvec, regnum)
            != MrmSUCCESS)
        XtError("can't register names\n");
    RegisterArcChangeDialog();
    RegisterColorChangeDialog();
    RegisterPlaceChangeDialog();
    RegisterTransChangeDialog();
    RegisterShowDialog();
    RegisterResultDialog();
    RegisterRateDialog();
    RegisterPrintDialog();
    RegisterMDGrammarDialog();
    RegisterMarkDialog();
    RegisterViewLayerDialog();
    RegisterEditLayerDialog();
    RegisterConsoleDialog();
    RegisterCommentDialog();
    RegisterZooomMenuCallbacks();
    RegisterMenuCallbacks();
    RegisterRescale();
    RegisterGrid();
    RegisterMenuValues();
    RegisterOptionsDialog();
    RegisterSimulationDialog();
    RegisterSwnSimOptionsDialog();
    RegisterSwnRGOptionsDialog();
    RegisterSwnUnfoldOptionsDialog();

    /*    puts(" ...  start FetchWidget"); fflush(stdout);*/
    mainwin = FetchWidget(appshell, "MainWin");

    /*    puts(" ...  start InitMainWindow"); fflush(stdout);*/
    InitMainWindow();
    /*    puts(" ...  start InitWorkWindow"); fflush(stdout);*/
    InitWorkWindow();
    /*    puts(" ...  start InitFonts"); fflush(stdout);*/
    InitFonts();
    /*    puts(" ...  start InitMenuBar"); fflush(stdout);*/
    InitMenuBar();
    /*    puts(" ...  start InitPopups"); fflush(stdout);*/
    InitPopups();

    /*    puts(" ...  start XtManageChild"); fflush(stdout);*/
    XtManageChild(mainwin);
    /*    puts(" ...  start XtRealizeWidget"); fflush(stdout);*/
    XtRealizeWidget(appshell);
    /*    puts(" ...  start InitFilesPath"); fflush(stdout);*/
    InitFilesPath();
    /*    puts(" ...  start InitGSPNShellErrors"); fflush(stdout);*/
    InitGSPNShellErrors();
    /*    puts(" ...  start gdiInitEngine"); fflush(stdout);*/
    gdiInitEngine(appshell);
    /*    puts(" ...  start gdiInitDrawingAreaGC"); fflush(stdout);*/
    gdiInitDrawingAreaGC();
    /*    puts(" ...  start InitOther"); fflush(stdout);*/
    InitOther();
    /*
        InitMsgDB();
    */
    /*    puts(" ...  start XmUpdateDisplay"); fflush(stdout);*/
    XmUpdateDisplay(mainwin);
    /*    puts(" ...  start XtAppMainLoop"); fflush(stdout);*/
    XtAppMainLoop(appContext);

    return 0;
}
