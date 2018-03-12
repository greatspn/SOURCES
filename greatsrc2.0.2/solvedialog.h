#ifdef ___SOLVEDIALOG___

void ShowSolveDialog(char *, XtCallbackProc);
void InitSolveDialog(void);

#else
#	ifndef 	__SOLVEDIALOG__
#	define	__SOLVEDIALOG__

extern void ShowSolveDialog(char *, XtCallbackProc);
extern void InitSolveDialog(void);

#	endif
#endif
