#ifdef ___COMMENTDIALOG___

void ShowCommentDialog(char *);
void RegisterCommentDialog(void);
void InitCommentDialog(void);
static void EditCommentOKCB(Widget , XtPointer , XtPointer);

#else
#	ifndef 	__COMMENTDIALOG__
#	define	__COMMENTDIALOG__

extern void ShowCommentDialog(char *);
extern void RegisterCommentDialog(void);
extern void InitCommentDialog(void);

#	endif
#endif
