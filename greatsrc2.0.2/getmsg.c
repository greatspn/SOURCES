/*
 * msg.c 30/10 NULL PROC
 */

#include "global.h"

/************************  local variables and routines ******************/

#include <string.h>

#include <Xm/BulletinB.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
extern Widget mainwin;
static char     msg_input[LINEMAX];


/***********************************************************************
**                                                                    **
**               F O R W A R D   D E F I N I T I O N S                **
**                                                                    **
***********************************************************************/

static	void	MboxCB();
static	void	MboxCancelCB();



/***********************************************************************
**                                                                    **
**                    L O C A L   V A R I A B L E S                   **
**                                                                    **
***********************************************************************/

static	Widget	    Mbox_db,		/* The dialog box             */
         Mbox_lbl,		/* Label: "Find:"             */
         Mbox_txt,		/* Find-string Entry Field    */
         Mbox_sep,		/* Sep between pres & conf    */
         Mbox_btn_1,		/* Action pushbutton          */
         Mbox_btn_2;		/* Go-away pushbutton         */
static	XmString    Ok_str,		/* XmString for "Find"        */
        Cancel_str;		/* XmString for "Cancel"      */
//		    next_str,		/* XmString for "Next"        */
//		    done_str;		/* XmString for "Done"	      */
// static	int	    finding;		/* Flag for FindCB()	      */





/***********************************************************************
**                                                                    **
**  InitMsgDB()                                                       **
**                                                                    **
**                                                                    **
***********************************************************************/

void InitMsgDB() {
    Arg TempArg;


    Mbox_db = XmCreateBulletinBoardDialog(mainwin, "Message", NULL, 0);

    Mbox_lbl = XmCreateLabel(Mbox_db, "Message_Lbl", NULL, 0);
    XtManageChild(Mbox_lbl);
    Mbox_txt = XmCreateText(Mbox_db, "Message_Txt", NULL,  0);
    XtManageChild(Mbox_txt);

    Mbox_sep = XmCreateSeparator(Mbox_db, "Message_DB_Sep", NULL, 0);
    XtManageChild(Mbox_sep);

    Mbox_btn_1 = XmCreatePushButton(Mbox_db, "Message_OK_Btn", NULL, 0);
    XtManageChild(Mbox_btn_1);
    Mbox_btn_2 = XmCreatePushButton(Mbox_db, "Message_Cancel_Btn", NULL, 0);
    XtManageChild(Mbox_btn_2);

    XtAddCallback(Mbox_btn_1, XmNactivateCallback, MboxCB,    NULL);
    XtAddCallback(Mbox_btn_2, XmNactivateCallback, MboxCancelCB, NULL);

    XtSetArg(TempArg, XmNdefaultButton, Mbox_btn_1);
    XtSetValues(Mbox_db, &TempArg, 1);

    Ok_str = XmStringCreate("OK",   XmSTRING_DEFAULT_CHARSET);
    Cancel_str  = XmStringCreate("Cancel", XmSTRING_DEFAULT_CHARSET);
    XtSetArg(TempArg, XmNlabelString, Ok_str);
    XtSetValues(Mbox_btn_1, &TempArg, 1);
    XtSetArg(TempArg, XmNlabelString, Cancel_str);
    XtSetValues(Mbox_btn_2, &TempArg, 1);

}


void ShowMessage(int message) {
    /*
      XmString temp;
      Arg Msg;

     temp = XmStringCreate( Errors[message],   XmSTRING_DEFAULT_CHARSET );
     XtSetArg(Msg,XmNmessageString,temp);
     XtSetValues(ErrMsgDM,&Msg,1);
     XtManageChild(ErrMsgDM);
     puts("\nCiao Ciao\n");
     XmStringFree(temp);
     */
}

/***********************************************************************
**                                                                    **
**  ManageFindDB()                                                    **
**                                                                    **
**  Called when the dialog box is first presented, this function      **
**  manages the DB and sets the labels of its buttons to "Find"       **
**  and "Cancel". It also sets the "finding" flag FALSE, for the      **
**  first call to FindCB.                                             **
**                                                                    **
***********************************************************************/

void init_msgreceiving(char *message) {
    Arg TempArg;
    XmString TempXmString;

    TempXmString  = XmStringCreate(message, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(TempArg, XmNlabelString, TempXmString);
    XtSetValues(Mbox_lbl, &TempArg, 1);
    XmTextSetString(Mbox_txt, "");
    XtManageChild(Mbox_db);
//    _XmGrabTheFocus( Mbox_txt );
    XmStringFree(TempXmString);
}


/***********************************************************************
**                                                                    **
**  MboxCB( w, client_data, call_data )                               **
**                                                                    **
**                                                                    **
***********************************************************************/

static void MboxCB(Widget w, char *client_data,    XmAnyCallbackStruct *call_data) {
    char *TempString;

    TempString = XmTextGetString(Mbox_txt);
    strcpy(msg_input, TempString);
    XtFree(TempString);
    (*receiver)(msg_input);
}


/***********************************************************************
**                                                                    **
**  FindCanCB( w, client_data, call_data )                            **
**                                                                    **
**  Called from the "Cancel" or "OK" buttons of the Find DB. This     **
**  simply unmanages the DB.                                          **
**                                                                    **
***********************************************************************/

static void MboxCancelCB(Widget	w,
                         char *client_data,
                         XmAnyCallbackStruct *call_data) {
    XtUnmanageChild(Mbox_db);
}

void get_answer_proc(Widget      item,
                     XEvent          *event) {
    /*
    if (!receiving_msg)
    return;
    sprintf(msg_input, (char *) P_GET(get_answer, PANEL_VALUE));
    P_SET(get_answer, P_SHOW_ITEM, FALSE,
    	  PANEL_LABEL_STRING, "",
    	  PANEL_VALUE, "",
    	  0);
    receiving_msg = FALSE;
    (*receiver) (msg_input);
    */

}
/*
init_msgreceiving(msg_out)
char *msg_out;
{

	clear_message();
	receiving_msg = TRUE;
	xv_set(get_answer, XV_SHOW, TRUE,
		  PANEL_LABEL_STRING, msg_out,
		  PANEL_VALUE, "",
		  0);

	if (bell_global_on)
	window_bell(menu_sw);
	open_msg_f();

}
*/

void init_getmsg() {
    /*
    receiving_msg = FALSE;
    */
}











