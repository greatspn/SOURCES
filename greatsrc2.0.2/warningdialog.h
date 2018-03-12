#ifdef ___WARNINGDIALOG___

void InitWarningDialog(void);
void   ShowWarningDialog(char *, XtCallbackProc , XtCallbackProc , char *, char *, int);
static void NullCB(Widget , XtPointer , XtPointer);

#else
#	ifndef 	__WARNINGDIALOG__
#	define	__WARNINGDIALOG__

extern void InitWarningDialog(void);
extern void ShowWarningDialog(char *, XtCallbackProc , XtCallbackProc , char *, char *, int);

#	endif
#endif
