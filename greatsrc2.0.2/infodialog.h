#ifdef ___INFODIALOG___

static void KillInfoDlgCB(Widget , XtPointer , XtPointer);
void   ShowInfoDialog(String , Widget);

#	define MAX_WARNING_INSTANCES	10

#else
#	ifndef 	__INFODIALOG__
#	define	__INFODIALOG__

extern	void   ShowInfoDialog(String , Widget);

#	endif
#endif
