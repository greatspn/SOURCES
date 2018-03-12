#ifdef ___POSTSCRIPT___

#	define A4WIDTH		595
#	define A4HEIGHT		842

static char *String2PSString(char *);
Boolean StartPS(char *, float , float , float , float ,	int , int , int , int , Boolean , Boolean);
void PSCircle(float , float , float);
void PSPGreek(int , int);
void PSText(float , float , char *);
void PSTransition(float , float , float , float , float , int);
void PSToken(float , float , float);
void PSSpline(float , float , float , float , float , float);
void PSline(float , float , float , float);
Boolean EndPS(Boolean);

#else
#	ifndef 	___POSTSCRIPT___
#	define	___POSTSCRIPT___

extern 	Boolean StartPS(char *, float , float , float , float ,	int , int , int , int , Boolean , Boolean);
extern void PSCircle(float , float , float);
extern void PSPGreek(int , int);
extern void PSText(float , float , char *);
extern void PSTransition(float , float , float , float , float , int);
extern void PSToken(float , float , float);
extern void PSSpline(float , float , float , float , float , float);
extern void PSline(float , float , float , float);
extern 	Boolean EndPS(Boolean);

extern 	Boolean PSflag;

#	endif
#endif
