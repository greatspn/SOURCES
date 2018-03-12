#ifdef ___PLACE___

void NewPlace(XButtonEvent *);
void EditPlace(XButtonEvent *);
void DeletePlace(XButtonEvent *);
void RemovePlace(struct place_object *);

#else
#	ifndef 	__PLACE__
#	define	__PLACE__

extern void NewPlace(XButtonEvent *);
extern void EditPlace(XButtonEvent *);
extern void DeletePlace(XButtonEvent *);
extern void RemovePlace(struct place_object *);

#	endif
#endif
