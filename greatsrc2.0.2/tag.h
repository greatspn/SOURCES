#ifdef ___TAG___

void ShowTag(int);
void ShowPlaceTag(struct place_object *, DrawingSurfacePun , float , float , int);
void ShowTransTag(struct trans_object *, DrawingSurfacePun , float , float , int);
void MoveTag(XButtonEvent *);
void EditTag(XButtonEvent *);

static void TagTracking(int , int);
static void DropTag(int , int);

#else
#	ifndef 	__TAG__
#	define	__TAG__

extern	void ShowTag(int);
extern	void ShowPlaceTag(struct place_object *, DrawingSurfacePun , float , float , int);
extern	void ShowTransTag(struct trans_object *, DrawingSurfacePun , float , float , int);
extern	void MoveTag(XButtonEvent *);
extern	void EditTag(XButtonEvent *);

#	endif
#endif
