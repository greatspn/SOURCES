#ifdef ___TRANSDIALOG___


void ShowTransChangeDialog(struct trans_object *, int);
void InitTransChangeDialog(void);
void HideTransChangeDialog(void);
void RegisterTransChangeDialog(void);
static void SizeEnablings(void);
static void SizeNoEnablings(void);
static void UpdateTransChangeDialogLabel(int);
static void UpdateControls(int);
static void GetNewPri(char *);


static void ScaleCB(Widget , XtPointer , XtPointer);
static void SelectOneCB(Widget , XtPointer , XmListCallbackStruct *);
static void ProcessCRCB(Widget , XtPointer , XmAnyCallbackStruct *);
static void MarkDepCB(Widget , XtPointer , XmToggleButtonCallbackStruct *);
static void CancelTransDialogCB(Widget , XtPointer , XtPointer);
static void SetTransKindCB(Widget , int , XmToggleButtonCallbackStruct *);
static void SetTransTypeCB(Widget , int , XtPointer);
static void ShowMDHelpCB(Widget , XtPointer , XtPointer);

enum {
    INFINITE,
    NSERVE,
    LOADDEP,
    MARKDEP,

    LABEL_RATE,
    LABEL_MDEP,
    LABEL_LOAD,
    LABEL_EMPTY,
    LABEL_WEIGHT,

    FLAG_PROCESS,
    FLAG_ABORT,

    TRANS_EXP,
    TRANS_DET,
    TRANS_IMM
};
#	define HEIGHT_ENABLINGS		558
#	define HEIGHT_NO_ENABLINGS	370
#	define RATEPAR_MAX_SIZE		4000
#	define MDEP_MAX_SIZE		4000
#else
#	ifndef 	__TRANSDIALOG__
#	define	__TRANSDIALOG__

extern void ShowTransChangeDialog(struct trans_object *, int);
extern void InitTransChangeDialog(void);
extern void HideTransChangeDialog(void);
extern void RegisterTransChangeDialog(void);

#	endif
#endif
