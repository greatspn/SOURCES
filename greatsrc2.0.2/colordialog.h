#ifdef ___COLORDIALOG___

void ShowColorChangeDialog(struct lisp_object *, int);
void InitColorChangeDialog(void);
void HideColorChangeDialog(void);
void RegisterColorChangeDialog(void);
static void CancelColorDialogCB(Widget , XtPointer , XtPointer);
static void ChangeColorTypeCB(Widget , int , XmToggleButtonCallbackStruct *);
static void SetColorTypeCB(Widget , XtPointer , XtPointer);
enum {
    COLORSET,
    COLORMARK,
    COLORFUN
};
#else
#	ifndef 	__COLORDIALOG__
#	define	__COLORDIALOG__

extern void ShowColorChangeDialog(struct lisp_object *, int);
extern void InitColorChangeDialog(void);
extern void HideColorChangeDialog(void);
extern void RegisterColorChangeDialog(void);

#	endif
#endif
