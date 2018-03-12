#ifdef ___BOX___

void InitBoxTracking(XButtonEvent *);
void DrawRectBox(float , float , float , float , int , DrawingSurfacePun);
void ElasticBox(int , int);

#else
#	ifndef 	__BOX__
#	define	__BOX__

extern void InitBoxTracking(XButtonEvent *);
extern void DrawRectBox(float , float , float , float , int , DrawingSurfacePun);
extern void ElasticBox(int , int);

#	endif
#endif
