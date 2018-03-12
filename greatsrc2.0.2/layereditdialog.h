#ifdef ___LAYEREDITDIALOG___

void HideEditLayerDialog(void);
void ShowEditLayerDialog(void);
void EditLayerListSelectCB(Widget , XtPointer , XmListCallbackStruct *);
void ReinitializeEditDialog(void);
void UpdateEditDialog(void);
void InitEditLayerDialog(void);
void ReallocEditLayerList(void);
void RegisterEditLayerDialog(void);
Boolean TestLayerInEditLayerList(int);
static void SetAddAsDefault(void);
static void SetOkAsDefault(void);
static void LayerEditDeleteCB(Widget , XtPointer , XtPointer);
static void LayerEditCancelCB(Widget , int , XtPointer);
static void LayerEditTrueAddCB(Widget , XtPointer , XtPointer);
static void LayerEditTrueRenameCB(Widget , XtPointer , XtPointer);
static void EditLayerActionCB(Widget , int  , XtPointer);
static void EditLayerDoneCB(Widget , XtPointer , XtPointer);
static void LayerEditViewCB(Widget , XtPointer , XtPointer);
enum {
    ACTION_ADD,
    ACTION_RENAME
};

#else
#	ifndef 	__LAYEREDITDIALOG__
#	define	__LAYEREDITDIALOG__

extern void HideEditLayerDialog(void);
extern void ShowEditLayerDialog(void);
extern void EditLayerListSelectCB(Widget , XtPointer , XmListCallbackStruct *);
extern void ReinitializeEditDialog(void);
extern void UpdateEditDialog(void);
extern void InitEditLayerDialog(void);
extern void ReallocEditLayerList(void);
extern 	Boolean TestLayerInEditLayerList(int);
extern 	void RegisterEditLayerDialog(void);

#	endif
#endif
