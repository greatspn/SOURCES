#ifdef ___ARCDIALOG___

void ShowArcChangeDialog(struct arc_object *, int);
void InitArcChangeDialog(void);
void HideArcChangeDialog(void);
void RegisterArcChangeDialog(void);
static void CancelArcDialogCB(Widget , XtPointer , XtPointer);
static void SetColorCB(Widget , XtPointer , XmToggleButtonCallbackStruct *);
static void SetArcTypeCB(Widget , XtPointer , XtPointer);
static void SetArcKindCB(Widget , int , XmToggleButtonCallbackStruct *);
enum {
    ARCIN,
    ARCOUT,
    ARCINHIB
};
#else
#	ifndef 	__ARCDIALOG__
#	define	__ARCDIALOG__

extern void ShowArcChangeDialog(struct arc_object *, int);
extern void InitArcChangeDialog(void);
extern void HideArcChangeDialog(void);
extern void RegisterArcChangeDialog(void);

#	endif
#endif
