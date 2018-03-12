#ifdef ___RESCALE___

#	define rescale_x(oldx) (center_x+((oldx)-center_x)*how_much)
#	define rescale_y(oldy) (center_y+((oldy)-center_y)*how_much)

void RegisterRescale(void);
static void RescaleNet(struct net_object *);
static void RescaleCB(Widget , int , XtPointer);

#else
#	ifndef 	__RESCALE__
#	define	__RESCALE__

extern 	void RegisterRescale(void);
extern int rescale_modified;

#	endif
#endif
