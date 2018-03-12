/*
 * flpinv.c - set flag for structural property to show.
 */

#include "global.h"


//static Widget     flag_menu;

void pflag_popup_menu(XEvent *event) {
    /* TBD */
    /*   menu_show(flag_menu, canvas_sw, event, 0);*/
}

//
//static void pflag_1_proc(Widget            m,Widget       mi)
//{
//    if (inib_flag)
//	return;
//    flag_pinv = 1;
//    menu_action();
//}
//
//static void pflag_2_proc(m, mi)
//Widget            m;
//Widget       mi;
//{
//    if (inib_flag)
//	return;
//    flag_pinv = 2;
//    menu_action();
//}
//
//static void pflag_3_proc(m, mi)
//Widget            m;
//Widget       mi;
//{
//	if (inib_flag)
//	return;
//    flag_pinv = 3;
//    menu_action();
//}
//
//static void pflag_4_proc(m, mi)
//Widget            m;
//Widget       mi;
//{
//    if (inib_flag)
//	return;
//    flag_pinv = 4;
//    menu_action();
//}
//
//static void pflag_5_proc(m, mi)
//Widget            m;
//Widget       mi;
//{
//	if (inib_flag)
//	return;
//	flag_pinv = 5;
//	menu_action();
//}
//
//static pflag_6_proc(m, mi)
//Widget            m;
//Widget       mi;
//{
//	if (inib_flag)
//	return;
//	flag_pinv = 6;
//	menu_action();
//}
//
void init_pflag_menu() {
    flag_pinv = 1;
    /*
    flag_menu =

    xv_create(NULL, MENU, XV_FONT, gachar_font,
    		MENU_ITEM,
    		MENU_STRING_ITEM, "Show P-invarians", 1,
    		MENU_NOTIFY_PROC, pflag_1_proc,
    		0,
    		MENU_ITEM,
    		MENU_STRING_ITEM, "Show minimal Deadlocks", 2,
    		MENU_NOTIFY_PROC, pflag_2_proc,
    		0,
    		MENU_ITEM,
    		MENU_STRING_ITEM, "Show minimal Traps", 3,
    		MENU_NOTIFY_PROC, pflag_3_proc,
    		0,
    		MENU_ITEM,
    		MENU_STRING_ITEM, "Show Implicit places", 4,
    		MENU_NOTIFY_PROC, pflag_4_proc,
    		0,
    		MENU_ITEM,
    		MENU_STRING_ITEM, "Show Absolute Marking Bounds", 5,
    		MENU_NOTIFY_PROC, pflag_5_proc,
    		0,
    		MENU_ITEM,
    		MENU_STRING_ITEM, "Show Average Marking Bounds", 6,
    		MENU_NOTIFY_PROC, pflag_6_proc,
    		0,
    		0);*/
}
