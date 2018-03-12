#ifdef ___PRINTAREA___

Boolean IsPrintAreaVisible(void);
void GetTruePrintArea(float *, float *, float *, float *);
void GetPrintArea(int *, int *, int *, int *);
void SetPrintArea(float , float , float , float);
void DrawPrintArea(void);
void SetPrintAreaVisible(Boolean);
void EndPrintArea(int , int);
void InitPrintArea(XButtonEvent *);


#else
#	ifndef 	__PRINTAREA__
#	define	__PRINTAREA__

extern 	Boolean IsPrintAreaVisible(void);
extern void GetTruePrintArea(float *, float *, float *, float *);
extern void GetPrintArea(int *, int *, int *, int *);
extern void SetPrintArea(float , float , float , float);
extern void DrawPrintArea(void);
extern void SetPrintAreaVisible(Boolean);
extern void EndPrintArea(int , int);
extern void InitPrintArea(XButtonEvent *);

#	endif
#endif
