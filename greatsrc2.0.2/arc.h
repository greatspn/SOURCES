#ifdef ___ARC___

void ChangeArcType(XButtonEvent *);
void NewLine(XButtonEvent *);
void NewHinib(XButtonEvent *);
void DeleteArc(XButtonEvent *);
void RemoveArc(struct arc_object *);
void SeePoints(int , int);
void ClearLine(float , float , float , float);
void AddSegment(int , int);
static void NewPoints(float , float *);
static void AdjustPoint(struct net_object *);
static void CalcTang(float , float *);
static void Readjust(float , float , float *, float *, float , float);
static void LineTracking(int , int);
static void ChangePoint(int , int);
static void ClearSeg(struct arc_object *);
static void CleanupArc(struct arc_object *);
static void NewArc(char);

#	define	S075	0.13053
#	define	S150	0.25882
#	define	S225	0.38268
#	define	S300	0.5
#	define	S375	0.60876
#	define	S450	0.70711
#	define  T150	0.26795
#	define	T300    0.57735

#	define 	MIN_D 	8


#else
#	ifndef 	__ARC__
#	define	__ARC__

extern 	void ChangeArcType(XButtonEvent *);
extern void NewLine(XButtonEvent *);
extern void NewHinib(XButtonEvent *);
extern void DeleteArc(XButtonEvent *);
extern void RemoveArc(struct arc_object *);
extern void SeePoints(int , int);
extern void ClearLine(float , float , float , float);
extern void AddSegment(int , int);

#	endif
#endif
