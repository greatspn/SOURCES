#ifdef ___CONSOLEDIALOG___

Widget GetConsoleWidget(void);
void NotifyProcessTermination(void);
void   ShowLine(char *);
void   ShowConsoleDialog(void);
void RegisterConsoleDialog(void);
void   InitConsoleDialog(void);
char *GetUserHostName(void);
void ResetConsoleDialog(void);
static void  ConsoleInterruptCB(Widget , XtPointer , XtPointer);
static void ConsoleOkCB(Widget , XtPointer , XtPointer);
static void ConsoleClearCB(Widget , XtPointer , XtPointer);
static void ConsoleStartCB(Widget , XtPointer , XtPointer);
static void ClearProcessCB(Widget , XtPointer , XtPointer);

#else
#	ifndef 	__CONSOLEDIALOG__
#	define	__CONSOLEDIALOG__

extern 	Widget GetConsoleWidget(void);
extern void NotifyProcessTermination(void);
extern void   ShowLine(char *);
extern void   ShowConsoleDialog(void);
extern void RegisterConsoleDialog(void);
extern void   InitConsoleDialog(void);
extern char *GetUserHostName(void);
extern void ResetConsoleDialog(void);


#	endif
#endif
