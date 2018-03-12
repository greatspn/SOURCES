#ifdef ___RES___
#	include "global.h"
#	include "liveness.h"
#	include "rate.h"
#	include "showgdi.h"
#	include "showdialog.h"
#	include "resultdialog.h"

void ClearRes(void);
void ClearThroughputs(void);
void ShowRes(struct res_object *, int);
void TypeResDef(struct res_object *);
void ShowTokenDistr(struct place_object *, XButtonEvent *);
void TypeRes(XButtonEvent *);
void NewRes(XButtonEvent *);
void EditRes(XButtonEvent *);
void DeleteResult(XButtonEvent *);
void RemoveResult(struct res_object *);
void ShowThroughputs(void);
void CollectRes(int);
void MoveRes(XButtonEvent *);
void PutRes(struct res_object *res, int op, DrawingSurfacePun ds, int x_off, int y_off);

static void GetPascalReal(FILE *, char *);
static void GetTokenDistr(FILE *, int);
static void GetThroughputs(void);
static void ResTracking(int , int);
static void DropRes(int , int);


#else
#	ifndef 	__RES__
#	define	__RES__

extern 	void ClearRes(void);
extern void ClearThroughputs(void);
extern void ShowRes(struct res_object *, int);
extern void TypeResDef(struct res_object *);
extern void ShowTokenDistr(struct place_object *, XButtonEvent *);
extern void TypeRes(XButtonEvent *);
extern void NewRes(XButtonEvent *);
extern void EditRes(XButtonEvent *);
extern void DeleteResult(XButtonEvent *);
extern void RemoveResult(struct res_object *);
extern void ShowThroughputs(void);
extern void CollectRes(int);
extern void MoveRes(XButtonEvent *);
extern void PutRes(struct res_object *res, int op, DrawingSurfacePun ds, int x_off, int y_off);
#	endif
#endif
