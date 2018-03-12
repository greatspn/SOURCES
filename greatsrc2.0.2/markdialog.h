#ifdef ___MARKDIALOG___

void HidePlaceChangeDialog(void);
void HideMarkChangeDialog(void);
void ShowMarkChangeDialog(struct mpar_object *, int);
void RegisterMarkDialog(void);
void InitMarkChangeDialog(void);

static void SetMarkCB(Widget , XtPointer , XtPointer);
static void CancelMarkDialogCB(Widget , XtPointer , XtPointer);

#else
#	ifndef 	__MARKDIALOG__
#	define	__MARKDIALOG__

extern void HidePlaceChangeDialog(void);
extern void HideMarkChangeDialog(void);
extern void ShowMarkChangeDialog(struct mpar_object *, int);
extern void RegisterMarkDialog(void);
extern void InitMarkChangeDialog(void);

#	endif
#endif
