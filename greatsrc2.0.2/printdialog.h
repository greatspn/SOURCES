/*#define SunOS5x*/

#include <stdlib.h>
/* for system() */

#ifdef ___PRINTDIALOG___

void HidePrintChangeDialog(void);
void ShowPrintDialog(void);
void InitPrintChangeDialog(void);
void RegisterPrintDialog(void);
void UpdateAndQuit(void);

static void UpdateLabel(void);
static void CheckBounds(void);
static void InitPrintOver(void);
static Boolean IsInDragRect(int , int);
static Boolean IsInDragHandle(int , int);
static Boolean IsInDragPaperRect(int , int);
static Boolean IsInDragPaperHandle(int , int);
static void DrawHrect(DrawingSurfacePun);
static void DrawHrectPaper(DrawingSurfacePun);
static void RedisplayPrintOverview(DrawingSurfacePun);

static void RotateCB(Widget , XtPointer , XmToggleButtonCallbackStruct *);
static void TexCB(Widget , XtPointer , XmToggleButtonCallbackStruct *);
static void FileCB(Widget , XtPointer , XmToggleButtonCallbackStruct *);
static void RedrawPrintOverviewCB(Widget , DrawingSurfacePun , XmDrawingAreaCallbackStruct *);
static void PrintPrintDialogCB(Widget , XtPointer , XtPointer);
static void RedrawPrintPaperCB(Widget , DrawingSurfacePun , XmDrawingAreaCallbackStruct *);
static void PrintDialogCenterCB(Widget , int , XtPointer);
static void CancelPrintDialogCB(Widget , XtPointer , XtPointer);

static void PrintOverEH(Widget , XtPointer , XEvent *, Boolean *);
static void PrintPaperEH(Widget , XtPointer , XEvent *, Boolean *);
static void PrintPaperREH(Widget , XtPointer , XEvent *, Boolean *);

enum {
    CENTER_H,
    CENTER_V,
    CENTER_HV
};
#else
#	ifndef 	__PRINTDIALOG__
#	define	__PRINTDIALOG__

extern void HidePrintChangeDialog(void);
extern void ShowPrintDialog(void);
extern void InitPrintChangeDialog(void);
extern void RegisterPrintDialog(void);
extern void UpdateAndQuit(void);

#	endif
#endif
