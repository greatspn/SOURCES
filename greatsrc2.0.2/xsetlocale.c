#ifdef RHMOTIFBUG
#include <locale.h>

char *_Xsetlocale(iii, name)
int iii;
char *name;
{
    setlocale(iii, name);
}

#endif

