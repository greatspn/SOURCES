/*
 * panel.c
 */

#include "global.h"
#include "menuH.h"
#include "menuinit.h"
#include "printarea.h"
#include "layer.h"



#define SPLINE_CHOICE (unsigned int)0x00000001
#define TAG_CHOICE (unsigned int)0x00000002
#define RATE_CHOICE (unsigned int)0x00000004

//static Widget mode_item;
Widget    msg_popup_f;
//static Widget    msg_popup_sw;
//static Widget bell_item;
//static int      bell_on = TRUE;
//static int      blink_on = TRUE;

//static void set_bell_proc(Widget      item,
//			  unsigned int    value,
//			  XEvent          *event)
//{
//	if (inib_flag)
//	return;
//	bell_on = (int) (value & 0x1);
//	/* blink_on = (int) (value & 0x2); */
//	blink_on = TRUE;
//}
//

void update_mode() {
    Arg Alist;

    XtSetArg(Alist, XmNset, spline_on ? TRUE : FALSE);
    XtSetValues(MENUITEM(VIEW_SPLINE_HANDLE), &Alist, 1);
    XtSetArg(Alist, XmNset, tag_visible ? TRUE : FALSE);
    XtSetValues(MENUITEM(VIEW_TAG_HANDLE), &Alist, 1);
    XtSetArg(Alist, XmNset, rate_visible ? TRUE : FALSE);
    XtSetValues(MENUITEM(VIEW_RATE_HANDLE), &Alist, 1);
    XtSetArg(Alist, XmNset, IsPrintAreaVisible() ? TRUE : FALSE);
    XtSetValues(MENUITEM(VIEW_PRINTAREA_HANDLE), &Alist, 1);
}

#define Notify_value int
//static Notify_value my_blinker(int *cli, int which)
//{
//	/*
//	Notify_client   me = (Notify_client) msg_popup_f;
//
//	(void) notify_set_itimer_func(me, my_blinker,
//	   ITIMER_REAL, ((struct itimerval *) 0), ((struct itimerval *) 0));
//
//	window_fit(msg_popup_sw);
//	window_fit(msg_popup_f);
//	{ Rect brect, mrect;
//	  frame_get_rect(frame_w,&brect);
//	  frame_get_rect(msg_popup_f,&mrect);
//	  mrect.r_top = brect.r_top+30;
//	  mrect.r_left = brect.r_left+OVERVIEW_WIDTH+20;
//	  frame_set_rect(msg_popup_f,&mrect);
//	}
//	xv_set(msg_popup_f, XV_SHOW, TRUE, 0);
//	if (bell_on)
//	window_bell(msg_popup_sw);
//
//	return (NOTIFY_DONE);
//*/
//}
//
//
//void open_msg_f()
//{
//	/*
//	int             i;
//
//	if (blink_on) {
//	Notify_client   me = (Notify_client) msg_popup_f;
//	struct itimerval blink_timer;
//
//	blink_timer.it_interval.tv_usec = 300000;
//	blink_timer.it_interval.tv_sec = 0;
//	blink_timer.it_value.tv_usec = 300000;
//	blink_timer.it_value.tv_sec = 0;
//	(void) notify_set_itimer_func(me, my_blinker,
//			   ITIMER_REAL, &blink_timer, ((struct itimerval *) 0));
//	return;
//	}
//	(void) my_blinker(&i, i);
//	*/
//}
//
//close_msg_f()
//{
//	/*
//	Notify_client   me = (Notify_client) msg_popup_f;
//
//	(void) notify_set_itimer_func(me, my_blinker,
//	   ITIMER_REAL, ((struct itimerval *) 0), ((struct itimerval *) 0));
//	xv_set(msg_popup_f, XV_SHOW, FALSE, 0);
//*/
//}
//

void init_panel() {

    InitLayer();
    init_getmsg();
    init_msg();
    InitCommands();

    init_command_menu();
    init_highlight();
//	init_search();

}
