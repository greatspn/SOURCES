#ifdef ___MENUCALLBACKS___

static void AboutCB(Widget , XtPointer , XtPointer);
static void ViewModeCB(Widget , int , XmToggleButtonCallbackStruct *);
static void ViewMenuCB(Widget , int , XtPointer);
static void NetMenuCB(Widget , int , XtPointer);
static void FileMenuCB(Widget , int , XtPointer);
static void EditMenuCB(Widget , int , XtPointer);
static void RedrawDrawingAreaCB(Widget , XtPointer , XmDrawingAreaCallbackStruct *);
static void ClearStatusCB(Widget , XtPointer , XtPointer);
static void FileHelpMenuCB(Widget , int , XtPointer);
static void EditHelpMenuCB(Widget , int , XtPointer);
static void NetHelpMenuCB(Widget , int , XtPointer);
static void GridHelpMenuCB(Widget , XtPointer , XtPointer);
static void ZoomHelpMenuCB(Widget , XtPointer , XtPointer);
static void RescaleHelpMenuCB(Widget , XtPointer , XtPointer);
static void ViewHelpMenuCB(Widget, int , XtPointer);
static void HelpHelpMenuCB(Widget , int , XtPointer);
void RegisterMenuCallbacks(void);


#else
#	ifndef 	__MENUCALLBACKS__
#	define	__MENUCALLBACKS__

extern void RegisterMenuCallbacks(void);


#	endif
#endif



