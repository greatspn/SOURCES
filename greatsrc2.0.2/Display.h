#ifdef ___DISPLAY___

void StatusDisplay(char *);
void StatusPrintf(char *, ...);
void ActionDisplay(char *);
extern Widget currentaction;
#else
#	ifndef 	__DISPLAY__
#	define	__DISPLAY__

extern void StatusDisplay(char *);
extern void StatusPrintf(char *, ...);
extern void ActionDisplay(char *);

#	endif
#endif
