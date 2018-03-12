#ifdef ___GRID___

#	define GRILL_1  1
#	define GRILL_5  2
#	define GRILL_10 3
#	define GRILL_20 4
#	define GRILL_30 5
#	define GRILL_40 6
#	define GRILL_50 7

void ResetGrill(void);
void DisplayGrid(void);
void DisplayGridRect(int , int , int , int);
int GetGridLevel(void);
void RegisterGrid(void);

static void SetGrill(int);
static void GridCB(Widget, int , XmToggleButtonCallbackStruct *);

#else
#	ifndef 	__GRID__
#	define	__GRID__

extern void ResetGrill(void);
extern void DisplayGrid(void);
extern int GetGridLevel(void);
extern void RegisterGrid(void);
extern void DisplayGridRect(int , int , int , int);

#	endif
#endif



