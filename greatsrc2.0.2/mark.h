#ifdef ___MARK___

#	define TOKENS 0
#	define PARAM 1

void ShowMpar(struct mpar_object *, int);
void DeleteMpar(XButtonEvent *);
void RemoveMpar(struct mpar_object *);
void ShowPlaceMark(struct place_object *, int);
void NewMpar(XButtonEvent *);
void EditMarking(XButtonEvent *);
static void DropMPar(int , int);
static void MParTracking(int , int);


#else
#	ifndef 	__MARK__
#	define	__MARK__

extern 	void ShowMpar(struct mpar_object *, int);
extern void DeleteMpar(XButtonEvent *);
extern void RemoveMpar(struct mpar_object *);
extern void ShowPlaceMark(struct place_object *, int);
extern void NewMpar(XButtonEvent *);
extern void EditMarking(XButtonEvent *);
extern void MoveMPar(XButtonEvent *);


#	endif
#endif
