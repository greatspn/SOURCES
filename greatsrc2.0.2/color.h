#ifdef ___COLOR___

void ShowPlaceColor(struct place_object *, DrawingSurfacePun , float , float , int);
void ShowTransColor(struct trans_object *, DrawingSurfacePun , float , float , int);
void DeleteLisp(XButtonEvent *);
void RemoveLisp(struct lisp_object *);
void MoveColor(XButtonEvent *);
void EditColor(XButtonEvent *);

static void ColorTracking(int , int);
static void DropColor(int , int);

#else
#	ifndef 	__COLOR__
#	define	__COLOR__

extern void ShowPlaceColor(struct place_object *, DrawingSurfacePun , float , float , int);
extern void ShowTransColor(struct trans_object *, DrawingSurfacePun , float , float , int);
extern void DeleteLisp(XButtonEvent *);
extern void RemoveLisp(struct lisp_object *);
extern void MoveColor(XButtonEvent *);
extern void EditColor(XButtonEvent *);

#	endif
#endif
