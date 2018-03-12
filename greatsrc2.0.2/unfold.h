#ifdef ___SWN___

void ShowSwnUnfoldOptionsDialog(int);
void InitSwnUnfoldOptionsDialog(void);
void HideSwnUnfoldOptionsDialog(void);
void RegisterSwnUnfoldOptionsDialog(void);

static void OkSwnUnfoldDialogCB(Widget w, XtPointer closure, XtPointer call_data);
static void CancelSwnUnfoldDialogCB(Widget w, XtPointer closure, XtPointer call_data);
#else

extern void InitSwnUnfoldOptionsDialog(void);
extern void RegisterSwnUnfoldOptionsDialog(void);
#endif

