#ifdef ___TOOLKIT___

void SetWaitCursor(Widget);
void ClearWaitCursor(Widget);
void SetCursor(int);
void InitCursors(void);
void SetBackground(Widget, String);
void SetForeground(Widget, String);
void RegisterNames(MrmRegisterArg *, int);
void OpenHierarchy(void);
Widget FetchWidget(Widget, String);
void SetApplicationTitle(char *);

extern Widget appshell;


static void CreaCursore(Cursor *, char *, int, int, int, int);
enum {
    WAIT_CURSOR,
    POINTER_CURSOR,
    PLACE_CURSOR,
    CROSS_CURSOR,

    VIMTRANS_CURSOR,
    VDETRANS_CURSOR,
    VEXTRANS_CURSOR,
    HIMTRANS_CURSOR,
    HDETRANS_CURSOR,
    HEXTRANS_CURSOR,
    MIMTRANS_CURSOR,
    MDETRANS_CURSOR,
    MEXTRANS_CURSOR,
    FIMTRANS_CURSOR,
    FDETRANS_CURSOR,
    FEXTRANS_CURSOR
};
#else
#	ifndef 	__TOOLKIT__
#	define	__TOOLKIT__

extern 	void SetWaitCursor(Widget);
extern void ClearWaitCursor(Widget);
extern void SetCursor(int);
extern void InitCursors(void);
extern void SetBackground(Widget, String);
extern void SetForeground(Widget, String);
extern void RegisterNames(MrmRegisterArg *, int);
extern void OpenHierarchy(void);
extern Widget FetchWidget(Widget, String);
extern 	void SetApplicationTitle(char *);

enum {
    WAIT_CURSOR,
    POINTER_CURSOR,
    PLACE_CURSOR,
    CROSS_CURSOR,

    VIMTRANS_CURSOR,
    VDETRANS_CURSOR,
    VEXTRANS_CURSOR,
    HIMTRANS_CURSOR,
    HDETRANS_CURSOR,
    HEXTRANS_CURSOR,
    MIMTRANS_CURSOR,
    MDETRANS_CURSOR,
    MEXTRANS_CURSOR,
    FIMTRANS_CURSOR,
    FDETRANS_CURSOR,
    FEXTRANS_CURSOR
};
#	endif
#endif
