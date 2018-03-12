#ifdef ___LINE___

void StartLineTracking(XButtonEvent *);
void StartMoveLine(XButtonEvent *);
void LineTrack(int , int);

static void AddPoint(int , int);
static void DelPoint(int , int);
static void ElasticLine(int , int);

#else
#	ifndef 	__GLINE__
#	define	__GLINE__

extern void StartLineTracking(XButtonEvent *);
extern void StartMoveLine(XButtonEvent *);
extern void LineTrack(int , int);

#	endif
#endif
