#ifdef ___DRAW___
extern Font sym_font;  /* ???????????????????? */

void RedisplayNet(void);
void DrawArrow(float , float , float , float , int , DrawingSurfacePun);
void DrawPlace(struct place_object *, int);
void PutTrans(struct trans_object *, int , DrawingSurfacePun , float , float);
void PutPlace(struct place_object *, int , DrawingSurfacePun , float , float);
void DrawTokens(int , double , double , int , double);
void DrawMarking(struct place_object *, int);
void DrawTrans(struct trans_object *, int);
void ShowTransPri(float xx, float yy, int npri, DrawingSurfacePun ds, int);
void DrawArc(struct arc_object *, int);
void PutArc(struct arc_object *, int , DrawingSurfacePun , int , int);
void PutNet(struct net_object *, DrawingSurfacePun , int , int);
void DrawIntVector(int , int , int , int , int , DrawingSurfacePun);
void DrawFloatVector(float , float , float , float , int , DrawingSurfacePun);
void DrawSeg(struct arc_object *);
void InvertLine(float , float , float , float);

static void DrawTokenRow(int , double , int , int , int , double);
static void DrawInhibitor(float , float , float , float , int , DrawingSurfacePun , int , int);
void DrawNet(struct net_object *, int op);

#else
#	ifndef 	__DRAW__
#	define	__DRAW__

extern void DrawNet(struct net_object *, int op);
extern void RedisplayNet(void);
extern void DrawArrow(float , float , float , float , int , DrawingSurfacePun);
extern void DrawPlace(struct place_object *, int);
extern void PutTrans(struct trans_object *, int , DrawingSurfacePun , float , float);
extern void PutPlace(struct place_object *, int , DrawingSurfacePun , float , float);
extern void DrawTokens(int , double , double , int , double);
extern void DrawMarking(struct place_object *, int);
extern void DrawTrans(struct trans_object *, int);
extern void ShowTransPri(float xx, float yy, int npri, DrawingSurfacePun ds, int);
extern void DrawArc(struct arc_object *, int);
extern void PutArc(struct arc_object *, int , DrawingSurfacePun , int , int);
extern void PutNet(struct net_object *, DrawingSurfacePun , int , int);
extern void DrawIntVector(int , int , int , int , int , DrawingSurfacePun);
extern void DrawFloatVector(float , float , float , float , int , DrawingSurfacePun);
extern void DrawSeg(struct arc_object *);
extern void InvertLine(float , float , float , float);

#	endif
#endif
