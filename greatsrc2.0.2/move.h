#ifdef ___MOVE___

void LineUp(XButtonEvent *);
void MovePlace(XButtonEvent *);
void MoveTrans(XButtonEvent *);

static void VerticalLineUp(int , int);
static void HorizontalLineUp(int , int);
static void ShowMovingPlace(void);
static void PlaceTracking(int , int);
static void PlaceDrop(int , int);
static void ShowMovingTrans(void);
static void TransTracking(int , int);
static void TransDrop(int , int);

#else
#	ifndef 	__MOVE__
#	define	__MOVE__

extern void LineUp(XButtonEvent *);
extern void MovePlace(XButtonEvent *);
extern void MoveTrans(XButtonEvent *);

#	endif
#endif
