#ifdef ___OVERVIEW___

void DrawOverPlace(struct place_object *, int);
void DrawOverTrans(struct trans_object *, int);
void over_draw_net(struct net_object *, GC , Window);
int over_draw_place(struct place_object *, GC , Window);
int over_draw_trans(struct trans_object *, GC , Window);
void ShowOverview(void);
void InitOverview(void);
void ClearOverview(void);
void RedisplayOverview(DrawingSurfacePun);
void ShowOverviewDialog(void);
void HideOverviewDialog(void);
void InitOverviewDialog(void);

static Boolean IsInDragRect(int , int);

static void CalcRect(void);
static void InitDragCB(Widget , int , XmScrollBarCallbackStruct *);
static void EndDragCB(Widget , XtPointer , XtPointer);
static void OverSynchCB(Widget , int , XmScrollBarCallbackStruct *);
static void RedrawOverviewCB(Widget , DrawingSurfacePun , XmDrawingAreaCallbackStruct *);

static void OverResizeEH(Widget , XtPointer , XConfigureEvent *, Boolean *);
static void OverHandler(Widget , XtPointer , XEvent *, Boolean *);

enum {
    X_SCROLL,
    Y_SCROLL
};
#else
#	ifndef 	__OVERVIEW__
#	define	__OVERVIEW__

extern void DrawOverPlace(struct place_object *, int);
extern void DrawOverTrans(struct trans_object *, int);
extern int over_draw_place(struct place_object *, GC , Window);
extern int over_draw_trans(struct trans_object *, GC , Window);
extern void over_draw_net(struct net_object *, GC , Window);
extern void ShowOverview(void);
extern void ShowOverview(void);
extern void InitOverview(void);
extern void ClearOverview(void);
extern void RedisplayOverview(DrawingSurfacePun);
extern void ShowOverviewDialog(void);
extern void HideOverviewDialog(void);
extern void InitOverviewDialog(void);

#	endif
#endif
