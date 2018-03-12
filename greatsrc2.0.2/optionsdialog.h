#ifdef ___OPTIONSDIALOG___

char *optGetHostname(void);
Boolean optGetVerboseShow(void);

void HideOptionsDialog(void);
void ShowOptionsDialog(void);
void RegisterOptionsDialog(void);
void InitOptionsDialog(void);

void InitOptions(void);

static void CancelOptionsDialogCB(Widget , XtPointer , XtPointer);
static void OkOptionsDialogCB(Widget , XtPointer , XtPointer);
static void OptionsDialogDefalutHostnameCB(Widget , XtPointer , XtPointer);

static void SetDefaults(void);
static void LoadOptions(void);
static void SaveOptions(void);

#ifndef MAXHOSTNAMELEN
#       include <netdb.h>
#endif
typedef struct Options_TAG {
    char hostName[MAXHOSTNAMELEN];
    Boolean verboseShow;
} Options;

typedef Options *OptionsPun;


#else
#	ifndef 	__OPTIONSDIALOG__
#	define	__OPTIONSDIALOG__

extern char *optGetHostname(void);
extern Boolean optGetVerboseShow(void);
extern 	void HideOptionsDialog(void);
extern  void ShowOptionsDialog(void);
extern  void RegisterOptionsDialog(void);
extern  void InitOptionsDialog(void);
extern 	void InitOptions(void);

#	endif
#endif
