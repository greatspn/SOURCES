#ifdef ___ERRORDIALOG___

void  ShowErrorDialog(char *, Widget);
static void KillErrorDlgCB(Widget , XtPointer , XtPointer);

#else
#	ifndef 	__ERRORDIALOG__
#	define	__ERRORDIALOG__

extern void  ShowErrorDialog(char *, Widget);

#	endif
#endif
