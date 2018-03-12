#ifdef ___SPLINE___

void DrawSpline(struct arc_object *, int , DrawingSurfacePun , int , int);
static void DrawCurv(float *, float *, float , float , float , float , struct arc_object *, int , DrawingSurfacePun , int , int);
static void CalcVer(float , float , float , float , float *, float *);
static void DrawCircle(float , float , float , float , float , float , float , int , DrawingSurfacePun) ;
static void CalcPoint(float , float , float , float , float , float , float *, float *, int , DrawingSurfacePun , int , int);

#	define RO                       30
#	define RADIUS                   30
#	define RED_FACTOR                0.4
#	define ARROW_L                   3.0

#else
#	ifndef 	__SPLINE__
#	define	__SPLINE__

extern		void DrawSpline(struct arc_object *, int , DrawingSurfacePun , int , int);

#	endif
#endif
