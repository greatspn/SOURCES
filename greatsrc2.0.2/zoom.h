#ifdef ___ZOOM___

#	define ZOOM_0_5  1
#	define ZOOM_0_75 2
#	define ZOOM_1_0  3
#	define ZOOM_1_5  4
#	define ZOOM_2_0  5

void ResetZoom(void);
void RegisterZooomMenuCallbacks(void);
void GetPixmap(Pixmap *pxpp, DrawingSurfacePun ds, int xc, int yc, int w, int h);
static void SetZoom(int);
static void ZoomCB(Widget, int , XmToggleButtonCallbackStruct *);

#else
#	ifndef 	__ZOOM__
#	define	__ZOOM__

extern void ResetZoom(void);
extern void RegisterZooomMenuCallbacks(void);
extern void GetPixmap(Pixmap *pxpp, DrawingSurfacePun ds, int xc, int yc, int w, int h);
#	endif
#endif

