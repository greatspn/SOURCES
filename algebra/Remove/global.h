#include    <stdio.h>
#include    <stdarg.h>
#include    <stdlib.h>
#include    <errno.h>
#include    <math.h>

#ifndef NULL
#define NULL    0
#endif
#define TRUE    1
#define FALSE   0

#define  LOC_MOVE 9
#define  MS_LEFT  0
#define  MS_MIDDLE 1
#define  MS_RIGHT 2

#define SET 0
#define CLEAR   1
#define XOR 2
#define EQUIV   3
#define INVERT  4
#define OR      5
#define AND     6
#define ANDREVERSE 7
#define ANDINVERTED 8
#define NOOP   9
#define NOR 10
#define ORREVERSE 11
#define COPYINVERTED 12
#define ORINVERTED 13
#define NAND 14
#define COPY 15

#undef  mpr_static
#define mpr_static(a,b,c,d,e)
#define event_x(e) (e->x)
#define event_y(e) (e->y)
#define window_fit(e) e=e

#include    <sys/types.h>
#include    <sys/stat.h>
//#include    <Xm/XmAll.h>
//#include    <Mrm/MrmPublic.h>
#include    "convers.h"
#include    "defines.h"

#include    "const.h"
#include    "object.h"
//#include    "choices.h"

#include    "funct.h"
#include    "var1.h"

#include "alloc.h"
//#include "Display.h"
//#include "warningdialog.h"
//#include "errordialog.h"
//#include "infodialog.h"
//#include "toolkit.h"

extern void SetModify(void);
extern void ResetModify(void);

//float trans_length;
//float titrans_height;
