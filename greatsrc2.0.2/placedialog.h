#ifdef ___PLACEDIALOG___

void ShowPlaceChangeDialog(struct place_object *, int);
void InitPlaceChangeDialog(void);
void HidePlaceChangeDialog(void);
void RegisterPlaceChangeDialog(void);
static void CancelPlaceDialogCB(Widget , XtPointer , XtPointer);
static void SetPlaceTypeCB(Widget , XtPointer , XtPointer);
#else
#	ifndef 	__PLACEDIALOG__
#	define	__PLACEDIALOG__

extern void ShowPlaceChangeDialog(struct place_object *, int);
extern void InitPlaceChangeDialog(void);
extern void HidePlaceChangeDialog(void);
extern void RegisterPlaceChangeDialog(void);

#	endif
#endif
