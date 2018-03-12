#ifdef ___ABOUTDIALOG___

static void ShowFaceEH(Widget , XtPointer , XEvent *, Boolean *);
void ShowAboutDialog(void);
void InitAboutDialog(void);

#else
#	ifndef 	__ABOUTDIALOG__
#	define	__ABOUTDIALOG__

extern void ShowAboutDialog(void);
extern void InitAboutDialog(void);

#	endif
#endif
