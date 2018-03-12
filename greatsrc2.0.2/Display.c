#define ___DISPLAY___
#include "global.h"
#include <stdarg.h>

void StatusDisplay(char *string) {
    XmString 	Temp;
    Arg 		LabelStringArg;

    Temp = XmStringCreateSimple(string);
    XtSetArg(LabelStringArg, XmNlabelString, Temp);
    XtSetValues(statuswin, &LabelStringArg, 1);
    XmStringFree(Temp);
}

void StatusPrintf(char *format, ...) {
    va_list args;
    static char Message[256];

    va_start(args, format);
    /* format = va_arg( args, char * ); */
    vsprintf(Message, format, args);
    va_end(args);

    StatusDisplay(Message);
}

void ActionDisplay(char *string) {
    XmString 	Temp;
    Arg 		LabelStringArg;

    Temp = XmStringCreateSimple(string);
    XtSetArg(LabelStringArg, XmNlabelString, Temp);
    XtSetValues(currentaction, &LabelStringArg, 1);
    XmStringFree(Temp);
}
