#ifdef ___ALLOC___

char *emalloc(unsigned);
char *erealloc(char *, unsigned);

#else
#	ifndef 	__ALLOC__
#	define	__ALLOC__

extern	char *emalloc(unsigned);
extern char *erealloc(char *, unsigned);

#	endif
#endif
