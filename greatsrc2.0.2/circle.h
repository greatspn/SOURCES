#ifdef ___CIRCLE___

void Dot(int , int , int , int);
void Circle(float , float , float , int , DrawingSurfacePun);

#else
#	ifndef 	__CIRCLE__
#	define	__CIRCLE__

extern	void Dot(int , int , int , int);
extern	void Circle(float , float , float , int , DrawingSurfacePun);

#	endif
#endif
