/*
 * fltinv.c - set flag for structural property to show.
 */

#include "global.h"


// static Widget     flag_menu;

void tflag_popup_menu(XEvent          *event) {
    /* TBD */
    /*  menu_show(flag_menu, canvas_sw, event, 0);*/
}

//
//static
//tflag_1_proc(m, mi)
//Widget            m;
//Widget       mi;
//{
//	if (inib_flag)
//	return;
//	flag_tinv = 1;
//	menu_action();
//}
//
//static
//tflag_2_proc(m, mi)
//Widget            m;
//Widget       mi;
//{
//	if (inib_flag)
//	return;
//	flag_tinv = 2;
//	menu_action();
//}
//
//static
//tflag_3_proc(m, mi)
//Widget            m;
//Widget       mi;
//{
//	if (inib_flag)
//	return;
//    flag_tinv = 3;
//    menu_action();
//}
//
//static
//tflag_4_proc(m, mi)
//Widget            m;
//Widget       mi;
//{
//    if (inib_flag)
//	return;
//	flag_tinv = 4;
//    menu_action();
//}
//
//static
//tflag_5_proc(m, mi)
//Widget            m;
//Widget       mi;
//{
//    if (inib_flag)
//	return;
//    flag_tinv = 5;
//    menu_action();
//}
//
//static
//tflag_6_proc(m, mi)
//Widget            m;
//Widget       mi;
//{
//    if (inib_flag)
//	return;
//    flag_tinv = 6;
//    menu_action();
//}
//
//static
//tflag_7_proc(m, mi)
//Widget            m;
//Widget       mi;
//{
//    if (inib_flag)
//	return;
//    flag_tinv = 7;
//    menu_action();
//}
//
//static
//tflag_8_proc(m, mi)
//Widget            m;
//Widget       mi;
//{
//    if (inib_flag)
//	return;
//	flag_tinv = 8;
//	menu_action();
//}
//
//static
//tflag_9_proc(m, mi)
//Widget            m;
//Widget       mi;
//{
//	if (inib_flag)
//	return;
//	flag_tinv = 9;
//	menu_action();
//}
//

void init_tflag_menu() {
    flag_tinv = 1;
    /*
    flag_menu =
    xv_create(NULL, MENU, XV_FONT, gachar_font,
    		MENU_ITEM,
    		MENU_STRING_ITEM, "Show T-invarians", 1,
    		MENU_NOTIFY_PROC, tflag_1_proc,
    		0,
    	    MENU_ITEM,
    		MENU_STRING_ITEM, "Show ECS", 2,
    		MENU_NOTIFY_PROC, tflag_2_proc,
    		0,
    	    MENU_ITEM,
    		MENU_STRING_ITEM, "Show ME", 3,
    		MENU_NOTIFY_PROC, tflag_3_proc,
    		0,
    	    MENU_ITEM,
    		MENU_STRING_ITEM, "Show SC", 4,
    		MENU_NOTIFY_PROC, tflag_4_proc,
    		0,
    	    MENU_ITEM,
    		MENU_STRING_ITEM, "Show CC", 5,
    		MENU_NOTIFY_PROC, tflag_5_proc,
    		0,
    	    MENU_ITEM,
    		MENU_STRING_ITEM, "Show unbounded sequences", 6,
    		MENU_NOTIFY_PROC, tflag_6_proc,
    		0,
    	    MENU_ITEM,
    		MENU_STRING_ITEM, "Show Actual liveness Bounds", 7,
    		MENU_NOTIFY_PROC, tflag_7_proc,
    		0,
    	    MENU_ITEM,
    		MENU_STRING_ITEM, "Show LP liveness Bounds", 8,
    		MENU_NOTIFY_PROC, tflag_8_proc,
    		0,
    	    MENU_ITEM,
    		MENU_STRING_ITEM, "Show LP Throughput Bounds", 9,
    		MENU_NOTIFY_PROC, tflag_9_proc,
    		0,
    		0);
    		*/
}
