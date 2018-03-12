#ifdef ___FILER___

void InitFilesPath(void);
char *GetCurrentFilename(void);
void InitFileDialog(Widget);
void Quit(void);
void NewNet(void);
void NewCB(Widget , XtPointer , XtPointer);
void OpenNet(void);
void SavePostscript(Widget , char *);
void SaveAsRequest(void);
void ExitRequest(void);
void SaveRequest(void);
void NewRequest(void);
void OpenRequest(void);
void SaveAndExec(void (*)());
void SaveTeX(Widget , char *);

static	void UnmanageStdFile(void);
static void CancelCB(Widget , XtPointer , XtPointer);
static	void ManageStdFile(Widget , char *, char *, char *, XtCallbackProc , char *, XtCallbackProc , XtCallbackProc);
static	void FileOkCB(Widget , XtPointer , XmFileSelectionBoxCallbackStruct *);

static	void ReadProc(void);
static	void WriteProc(void);
static void QuitCB(Widget , XtPointer , XtPointer);
static void OpenCB(Widget , XtPointer , XtPointer);
static void SaveNet(Widget , char *);
static	void PostscriptCB(Widget , XtPointer , XmFileSelectionBoxCallbackStruct *);
static void TeXCB(Widget , XtPointer , XmFileSelectionBoxCallbackStruct *);
static void CancelPSCB(Widget , XtPointer , XtPointer);

#else
#	ifndef 	__FILER__
#	define	__FILER__

extern 	void InitFilesPath(void);
extern char *GetCurrentFilename(void);
extern void InitFileDialog(Widget);
extern void Quit(void);
extern void NewNet(void);
extern void NewCB(Widget , XtPointer , XtPointer);
extern void OpenNet(void);
extern void SavePostscript(Widget , char *);
extern void SaveAsRequest(void);
extern void ExitRequest(void);
extern void SaveRequest(void);
extern void NewRequest(void);
extern void OpenRequest(void);
extern void MergeRequest(void);
extern void SaveAndExec(void (*)());
extern void SaveTeX(Widget , char *);

#	endif
#endif
