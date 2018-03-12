#ifdef ___LIVENESS___

void EraseLive(void);
void ClearLive(void);
void ShowLive(void);
int LoadLive(void);

#else
#	ifndef 	__LIVENESS__
#	define	__LIVENESS__

extern void EraseLive(void);
extern void ClearLive(void);
extern void ShowLive(void);
extern int LoadLive(void);

#	endif
#endif
