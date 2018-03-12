#include "global.h"
#include <Xm/Xm.h>

static Widget mousel, mousem, mouser;
Widget mousewin;
static char val = FALSE;
static char *lastRight = NULL;
static char *lastLeft = NULL;
static char *lastMiddle = NULL;


void SetMouseHelp(left, middle, right)
char *left, *right, *middle;
{

    XmString 	Temp;
    Arg 		LabelStringArg;

    lastLeft = left == NULL ? "" : left;
    lastRight = right == NULL ? "" : right;
    lastMiddle = middle == NULL ? "" : middle;

    if (!val)
        return;

    Temp = XmStringCreateSimple(lastLeft);
    XtSetArg(LabelStringArg, XmNlabelString, Temp);
    XtSetValues(mousel, &LabelStringArg, 1);
    XmStringFree(Temp);

    Temp = XmStringCreateSimple(lastRight);
    XtSetArg(LabelStringArg, XmNlabelString, Temp);
    XtSetValues(mouser, &LabelStringArg, 1);
    XmStringFree(Temp);

    Temp = XmStringCreateSimple(lastMiddle);
    XtSetArg(LabelStringArg, XmNlabelString, Temp);
    XtSetValues(mousem, &LabelStringArg, 1);
    XmStringFree(Temp);

}

void HelpToggleCB(w, client_data, call_data)
Widget	w;
char	*client_data;
caddr_t	call_data;
{

    val = !val;
    if (val) {
        XtManageChild(mousewin);
        SetMouseHelp(lastLeft, lastMiddle, lastRight);
    }
    else
        XtUnmanageChild(mousewin);
}


void InitMouseHelpDialog(void) {

    mousewin = XtNameToWidget(frame_w, "*MouseHelpDialog");
    mousel = XtNameToWidget(mousewin, "*MouseHelpLabelLeft");
    mousem = XtNameToWidget(mousewin, "*MouseHelpLabelMiddle");
    mouser = XtNameToWidget(mousewin, "*MouseHelpLabelRight");

}

