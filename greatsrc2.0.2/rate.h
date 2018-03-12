#ifdef ___RATE___

void ShowRpar(struct rpar_object *, int);
void ShowTransRate(struct trans_object *, int);
void ShowRate(int);
void TypeRate(XButtonEvent *);
void NewRpar(XButtonEvent *);
void EditRate(XButtonEvent *);
void DeleteRpar(XButtonEvent *);
void RemoveRpar(struct rpar_object *);
void MoveRate(XButtonEvent *);

static void TRateTracking(int , int);
static void DropTRate(int , int);
static void MoveTRate(void);
static void RPTracking(int, int);
static void DropRateP(int, int);
static void MoveRateP(void);

#else
#	ifndef 	__RATE__
#	define	__RATE__

extern 	void ShowRpar(struct rpar_object *, int);
extern void ShowTransRate(struct trans_object *, int);
extern void ShowRate(int);
extern void TypeRate(XButtonEvent *);
extern void NewRpar(XButtonEvent *);
extern void EditRate(XButtonEvent *);
extern void DeleteRpar(XButtonEvent *);
extern void RemoveRpar(struct rpar_object *);
extern void MoveRate(XButtonEvent *);

#	endif
#endif
