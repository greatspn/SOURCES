#ifdef ___RESULTDIALOG___

void HideResChangeDialog(void);
void ShowResChangeDialog(struct res_object *, int);
void InitResChangeDialog(void);
void RegisterResultDialog(void);
static void SetResCB(Widget , XtPointer , XtPointer);
static void CancelResDialogCB(Widget , XtPointer , XtPointer);

#else
#	ifndef 	__RESULTDIALOG__
#	define	__RESULTDIALOG__

extern void HideResChangeDialog(void);
extern void ShowResChangeDialog(struct res_object *, int);
extern void InitResChangeDialog(void);
extern void RegisterResultDialog(void);

#	endif
#endif
