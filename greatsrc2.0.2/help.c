/*
 * help.c - pops up an help message window
 */

/*
#define DEBUG
*/

#include "global.h"


/*****************************   local area   **************************/
#define HELP_TOP -OVERVIEW_HEIGHT
#define HELP_LEFT 0
#define HELP_WIDTH 600
#define HELP_HEIGHT 500
#define HELP_WIN_HEIGHT 300

void help_popup_menu(event)
XEvent          *event;
{
    /*
    switch (cur_object) {
    case COLOR:
    	menu_show(help_co_menu, canvas_sw, event, 0);
    	return;
    case RESULT:
    	menu_show(help_re_menu, canvas_sw, event, 0);
    	return;
    case RPAR:
    	menu_show(help_md_menu, canvas_sw, event, 0);
    	return;
    defaul:return;
    }
    */
}


// static char            hlp_name[LINEMAX];
// static int num;

//static void help_repaint()
//	{
//
//	/*
//	FILE           *hlp;
//	char            string[LINEMAX];
//	char           *first;
//	int             i;
//#define VBAR '|'
//
//	if ((hlp = fopen(hlp_name, "r")) == NULL) {
//	put_msg("Can't open help file");
//	return;
//	}
//	for (string[0] = ' '; string[0] != VBAR; fgets(string, LINEMAX, hlp));
//	for (;;) {
//	sscanf(&(string[1]), "%d", &i);
//	if (i >= num)
//		break;
//	for (string[0] = ' '; string[0] != VBAR; fgets(string, LINEMAX, hlp));
//	}
//	for (i = 20;; i += 15) {
//	first = &(string[0]);
//	fgets(string, LINEMAX, hlp);
//	if (*first == VBAR)
//		break;
//	for (; *first != '\n' && *first != '\0'; first++);
//	if (*first == '\n')
//		*first = '\0';
//		gdiDrawText(help_pw, 10, i, SET, gachar_font, string);
//	}
//
//	(void) fclose(hlp);
//	*/
//}
//
void help(int n) {


    /* get text from file "tool.help" */
    /*
    	Gspn = getenv("GREATSPN_SCRIPTDIR");
    	sprintf(hlp_name, "%s/tool.help", Gspn);
    	num = n;

    	help_pw = new_popup(HELP_LEFT, HELP_TOP,
    			HELP_WIDTH, HELP_HEIGHT,
    			HELP_WIDTH, HELP_WIN_HEIGHT,
    			help_repaint, "GreatSPN: Help");
    	help_repaint();
    */
}

void help_md_e_proc(Widget            m, Widget       mi) {
    if (inib_flag)
        return;
    help(1);
}

void help_md_g_proc(Widget            m,
                    Widget       mi) {
    if (inib_flag)
        return;
    help(2);
}

void help_re_e_proc(Widget m, Widget mi) {
    if (inib_flag)
        return;
    help(3);
}

void help_re_g_proc(Widget m, Widget mi) {
    if (inib_flag)
        return;
    help(4);
}


void help_gd_e_proc(Widget            m,
                    Widget       mi) {
    if (inib_flag)
        return;
    help(5);
}

void help_gd_g_proc(Widget            m,
                    Widget       mi) {
    if (inib_flag)
        return;
    help(6);
}


void init_help_menu() {
    /*
    	help_md_menu =
    			xv_create(NULL,MENU, XV_FONT, gachar_font,
    				   MENU_ITEM,
    			  MENU_STRING_ITEM, "Example of M-D rate definition", 1,
    				   MENU_NOTIFY_PROC, help_md_e_proc,
    				   0,
    				   MENU_ITEM,
    				MENU_STRING_ITEM, "BNF syntax for M-D rates", 2,
    				   MENU_NOTIFY_PROC, help_md_g_proc,
    				   0,
    				   MENU_ITEM,
    				MENU_STRING_ITEM, "Example of general distr.", 3,
    				   MENU_NOTIFY_PROC, help_gd_e_proc,
    				   0,
    				   MENU_ITEM,
    				MENU_STRING_ITEM, "BNF syntax for general distr.", 4,
    				   MENU_NOTIFY_PROC, help_gd_g_proc,
    				   0,
    				   0);
    	help_re_menu =
    			xv_create(NULL,MENU, XV_FONT, gachar_font,
    				   MENU_ITEM,
    			MENU_STRING_ITEM, "Example of result definition", 1,
    				   MENU_NOTIFY_PROC, help_re_e_proc,
    				   0,
    				   MENU_ITEM,
    		   MENU_STRING_ITEM, "BNF syntax for result definitions", 2,
    				   MENU_NOTIFY_PROC, help_re_g_proc,
    				   0,
    				   0);
    	init_help2_menu();
    	*/
}
