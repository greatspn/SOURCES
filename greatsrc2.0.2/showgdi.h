#ifdef ___SHOWGDI___

void InitDS(Widget , DrawingSurfacePun);
void DisposeDS(DrawingSurfacePun);
void gdiInitEngine(Widget);
void gdiInitDrawingAreaGC(void);
void SetFunction(int , DrawingSurfacePun);
void gdiDrawText(DrawingSurfacePun , int , int, int, int, char *);
void gdiDrawPoint(DrawingSurfacePun , int , int , int);
void MWSet();						/* ??????????? */
void gdiResizeDrawingArea(int, int);
void gdiClearDrawingArea(void);
void gdiClearRect(DrawingSurfacePun , int , int , int , int);
void gdiSetRect(DrawingSurfacePun , int , int , int , int);
void gdiDrawingAreaClearRect(int , int , int , int);
void gdiDrawingAreaInvertRect(int , int , int , int);
void gdiPreparePlaceModel(void);
void gdiRotateVertex(int , float , float , float *, float *);
void gdiRotatePolygon(short , XPoint *, float , float , float , float);
void gdiPrepareTransModels(void);
void gdiCopyTransModel(struct trans_object *, int , int  , int , DrawingSurfacePun);
void gdiCopyPlaceModel(int , int  , int , DrawingSurfacePun);
void gdiXorPlaceModel(int , int , DrawingSurfacePun);
void gdiReleaseGCs(void);

static void gdiDrawingAreaFillRect(int , int , int , int);

#else
#	ifndef 	__SHOWGDI__
#	define	__SHOWGDI__

extern void InitDS(Widget , DrawingSurfacePun);
extern 	void DisposeDS(DrawingSurfacePun);
extern 	void gdiInitEngine(Widget);
extern 	void gdiInitDrawingAreaGC(void);
extern 	void SetFunction(int , DrawingSurfacePun);
extern 	void gdiDrawText(DrawingSurfacePun , int , int, int, int, char *);
extern 	void gdiDrawPoint(DrawingSurfacePun , int , int , int);
extern 	void MWSet();
extern 	void gdiResizeDrawingArea(int, int);
extern 	void gdiClearDrawingArea(void);
extern 	void gdiClearRect(DrawingSurfacePun , int , int , int , int);
extern 	void gdiSetRect(DrawingSurfacePun , int , int , int , int);
extern 	void gdiDrawingAreaClearRect(int , int , int , int);
extern 	void gdiDrawingAreaInvertRect(int , int , int , int);
extern 	void gdiPreparePlaceModel(void);
extern 	void gdiRotateVertex(int , float , float , float *, float *);
extern 	void gdiRotatePolygon(short , XPoint *, float , float , float , float);
extern 	void gdiPrepareTransModels(void);
extern 	void gdiCopyTransModel(struct trans_object *, int , int  , int , DrawingSurfacePun);
extern 	void gdiCopyPlaceModel(int , int  , int , DrawingSurfacePun);
extern 	void gdiXorPlaceModel(int , int , DrawingSurfacePun);
extern 	void gdiReleaseGCs(void);

extern int pixdim;

#	endif
#endif
