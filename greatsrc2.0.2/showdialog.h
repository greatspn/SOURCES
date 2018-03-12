#ifdef ___SHOWDIALOG___

void InitShowDialog(void);
void RegisterShowDialog(void);
void ShowShowDialog(char *);
void HideShowDialog(void);
static void HideShowDialogCB(Widget , XtPointer , XtPointer);

#else
#	ifndef 	__SHOWDIALOG__
#	define	__SHOWDIALOG__

extern void InitShowDialog(void);
extern void RegisterShowDialog(void);
extern void ShowShowDialog(char *);
extern void HideShowDialog(void);

#	endif
#endif
