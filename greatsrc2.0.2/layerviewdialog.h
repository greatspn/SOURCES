#ifdef ___LAYERVIEWDIALOG___

void SetUpdate(void);
void ResetUpdate(void);
void HideViewLayerDialog(void);
void ShowViewLayerDialog(void);
void NewViewLayerButton(int);
void UpdateViewDialog(void);
void UpdateLayerDialogs(void);
void ReinitializeViewDialog(void);
void InitViewLayerDialog(void);
void RegisterViewLayerDialog(void);
static void ViewLayerUpdateViewCB(Widget , Layer , XmToggleButtonCallbackStruct *);
static void ViewLayerEditCB(Widget , XtPointer , XtPointer);
static void ViewLayerDoneCB(Widget , XtPointer , XtPointer);
static void ViewLayerSelectAllCB(Widget , XtPointer , XtPointer);
static void ViewLayerDeselectAllCB(Widget , XtPointer , XtPointer);

#else
#	ifndef 	__LAYERVIEWDIALOG__
#	define	__LAYERVIEWDIALOG__

extern void SetUpdate(void);
extern void ResetUpdate(void);
extern void HideViewLayerDialog(void);
extern void ShowViewLayerDialog(void);
extern void NewViewLayerButton(int);
extern void UpdateViewDialog(void);
extern void UpdateLayerDialogs(void);
extern void ReinitializeViewDialog(void);
extern void InitViewLayerDialog(void);
extern void RegisterViewLayerDialog(void);

#	endif
#endif
