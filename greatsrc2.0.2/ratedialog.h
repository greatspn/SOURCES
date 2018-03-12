#ifdef ___RATEDIALOG___

void HideRateChangeDialog(void);
void ShowRateChangeDialog(struct rpar_object *, int);
void RegisterRateDialog(void);
void InitRateChangeDialog(void);
static void SetRateCB(Widget , XtPointer , XtPointer);
static void CancelRateDialogCB(Widget , XtPointer , XtPointer);

#else
#	ifndef 	__RATEDIALOG__
#	define	__RATEDIALOG__

extern void HideRateChangeDialog(void);
extern void ShowRateChangeDialog(struct rpar_object *, int);
extern void RegisterRateDialog(void);
extern void InitRateChangeDialog(void);

#	endif
#endif
