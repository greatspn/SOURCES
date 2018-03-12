#ifdef ___MDGRAMMARDIALOG___

void ShowMDGrammarDialog(void);
void HideMDGrammarDialog(void);
void SetVisibleMDGrammarDialog(void);
void InitMDGrammarDialog(void);
void RegisterMDGrammarDialog(void);
static void HideMDGrammarDialogCB(Widget , XtPointer , XtPointer);

#else
#	ifndef 	__MDGRAMMARDIALOG__
#	define	__MDGRAMMARDIALOG__

extern void ShowMDGrammarDialog(void);
extern void HideMDGrammarDialog(void);
extern void SetVisibleMDGrammarDialog(void);
extern void InitMDGrammarDialog(void);
extern void RegisterMDGrammarDialog(void);

#	endif
#endif
