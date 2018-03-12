#ifdef ___MENUINIT___

void SetPopupActionTo(int);
void UpdateMenuStatusTo(int);
void UpdatePopupStatusTo(int);
void InitMenuBar(void);
void InitPopups(void);
void SetGridMenuItemValue(Boolean);
void SetZoomMenuItemValue(Boolean);
void RegisterMenuValues(void);
void Reset_T_P_Popups(void);

Widget MENUITEM(int);
Widget POPUPITEM(int);

static void InitZoomMenu(void);
static void InitRescaleMenu(void);
static void InitHelpMenu(void);
static void InitHelpMenu(void);
static void InitNetMenu(void);
static void InitEditMenu(void);
static void InitViewMenu(void);
static void InitGridMenu(void);
static void InitFileMenu(void);
static void InitPopupMenu(void);
static void InitPinvPopupMenu(void);
static void InitPinvPopupMenu(void);
static void InitEndShowPopupMenu(void);
static void InitSwnMenu(void);
extern Widget menubar, mainwin, statuswin, popup, PinvPopup, TinvPopup, drawwin, workwin, EndShowPopup;
enum {
    F,
    T
};
#else
#	ifndef 	__MENUINIT__
#	define	__MENUINIT__

extern	void SetPopupActionTo(int);
extern void UpdateMenuStatusTo(int);
extern void UpdatePopupStatusTo(int);
extern void InitMenuBar(void);
extern void InitPopups(void);
extern void SetGridMenuItemValue(Boolean);
extern void SetZoomMenuItemValue(Boolean);
extern Widget MENUITEM(int);
extern Widget POPUPITEM(int);
extern void RegisterMenuValues(void);
extern void Reset_T_P_Popups(void);


#	endif
#endif
