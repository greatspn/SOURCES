/*
 * dismsg.c 30/10 NULL PROCS
 */

#include "global.h"
#include "funct.h"

#define			 CR		13	/* carriage return */
#define			 CTRL_U		21	/* delete line */
#define			 SP		32	/* space char */
#define			 DEL		127	/* delete char */
#define			 CTRL_H		8	/* delete char */

/************************  local variables and routines ******************/


static char     msg_input[LINEMAX];
static int      no_msg = 0;

// static Widget help_item;
static int cur_level = 0;


//static void set_help_global_proc(Widget      item,
//				 unsigned int    value,
//				 XEvent          *event)
//{
//	/*
//	if (inib_flag)
//		return;
//	cur_level = !(int) (value & 0x1);
//	*/
//}

void init_msg() {
    /*
    	help_item =
    		xv_create(menu_sw, PANEL_TOGGLE,
    		XY_POS( X_Ihelpglobal.x, X_Ihelpglobal.y ),
    								 PANEL_LABEL_STRING, "",
    								 PANEL_CHOICE_STRINGS, "help", 0,
    								 PANEL_NOTIFY_PROC, set_help_global_proc,
    								 PANEL_FEEDBACK, PANEL_INVERTED,
    				 0);

    	xv_set(help_item, PANEL_TOGGLE_VALUE, 0, TRUE, 0);
    	*/
}


void put_msg(int level, ...) {

    va_list args;
    char *format;

    clear_message();
    va_start(args, level);
    if (level < cur_level) {
        return;
    }
    no_msg = 1;
    format = va_arg(args, char *);
    vsprintf(msg_input, format, args);
    va_end(args);
    /*	xv_set(msg_1, XV_SHOW, TRUE, PANEL_LABEL_STRING, msg_input, 0); */
    StatusDisplay(msg_input);
    /*open_msg_f(); */

}

void next_msg(int level, ...) {
    /*
    va_list args;
    char * format;
    Panel_item msg_x ;
    int level;

    va_start( args );
    level = va_arg( args, int );
    if ( level < cur_level ) {
    return;
      }
    format = va_arg( args, char * );
    vsprintf(msg_input, format, args);
    va_end( args );

    if (++no_msg == 1) {
    msg_x = msg_1 ;
    open_msg_f();
      }
    else if (no_msg == 2)
    msg_x = msg_2 ;
    else if (no_msg == 3)
    msg_x = msg_3 ;
    else
    msg_x = msg_4 ;

    xv_set(msg_x, XV_SHOW, TRUE, PANEL_LABEL_STRING, msg_input, 0);
    */
}

void clear_message() {
    /*
    register int    i;

    if (inib_flag)
    return;
    close_msg_f();
    no_msg = 0;
    for (i = 0; i < LINEMAX; msg_input[i++] = ' ');
    xv_set(msg_1, XV_SHOW, FALSE, PANEL_LABEL_STRING, "", 0);
    xv_set(msg_2, XV_SHOW, FALSE, PANEL_LABEL_STRING, "", 0);
    xv_set(msg_3, XV_SHOW, FALSE, PANEL_LABEL_STRING, "", 0);
    xv_set(msg_4, XV_SHOW, FALSE, PANEL_LABEL_STRING, "", 0);
    */
}
