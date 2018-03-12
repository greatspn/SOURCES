#ifdef ___SWN___

void ShowSwnRGOptionsDialog(int);
void InitSwnRGOptionsDialog(void);
void HideSwnRGOptionsDialog(void);
void RegisterSwnRGOptionsDialog(void);

void ShowSwnSimOptionsDialog(int);
void InitSwnSimOptionsDialog(void);
void HideSwnSimOptionsDialog(void);
void RegisterSwnSimOptionsDialog(void);

static void OkSwnRGDialogCB(Widget w, XtPointer closure, XtPointer call_data);
static void CancelSwnRGDialogCB(Widget w, XtPointer closure, XtPointer call_data);
static void OkSimOptionsDialogCB(Widget w, XtPointer closure, XtPointer call_data);
static void CancelSimOptionsDialogCB(Widget w, XtPointer closure, XtPointer call_data);
static void EditorSWNDialogCB(Widget w, XtPointer closure, XtPointer call_data);
static void SwnSimOptionsCB(Widget w, int closure, XtPointer call_data);

enum {
    SWN_CLEVEL_60,
    SWN_CLEVEL_70,
    SWN_CLEVEL_80,
    SWN_CLEVEL_90,
    SWN_CLEVEL_95,
    SWN_CLEVEL_99
};


#else
#	ifndef 	__SWN__
#	define	__SWN__

void ShowSwnRGOptionsDialog(int);
void InitSwnRGOptionsDialog(void);
void HideSwnRGOptionsDialog(void);
void RegisterSwnRGOptionsDialog(void);

void ShowSwnSimOptionsDialog(int);
void InitSwnSimOptionsDialog(void);
void HideSwnSimOptionsDialog(void);
void RegisterSwnSimOptionsDialog(void);

#	endif
#endif
