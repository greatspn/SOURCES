#ifdef ___INPUTDIALOG___

void  ShowInputDialog(char *, XtCallbackProc , XtCallbackProc , Widget);
static void KillInputDlgCB(Widget , XtPointer , XtPointer);
#define INPUT_CANCEL ((XtPointer) 0)
#define INPUT_OK ((XtPointer) 1)

#else
#	ifndef 	__INPUTDIALOG__
#	define	__INPUTDIALOG__

extern void  ShowInputDialog(char *, XtCallbackProc , XtCallbackProc , Widget);

#	endif
#endif
