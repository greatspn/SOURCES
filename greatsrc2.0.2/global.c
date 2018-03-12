/*
 * file:  global.c
 */


/*
#define DEBUG
#define DEBUG_GC
*/


#include	"global.h"

char           *Gspn;

Pixel gBlack, gWhite;

/***************************  graphical objects  *************************/


Widget          overview_sw, canvas_sw;
Widget          overview_pw, canvas_pw;
Widget           fire_f;
Widget           frame_w;
Widget           fire_sw;
Widget           menu_sw;
Widget      afire_f_button;
Widget      auto_f_button;
Widget      back_B_item;
Widget      back_R_item;
Widget      backup_B_item;
Widget      backup_R_item;
Widget      break_f_button;
Widget      c_l_stop_item;
Widget      close_f_button;
Widget      comment_B_item;
Widget      comment_R_item;
Widget      confidence_level_item;
Widget      cont_item;
Widget      cycle_number_B_item;
Widget      cycle_number_R_item;
Widget      delete_item;
Widget      error_item;
Widget      fire_mode_item;
Widget      forw_back_item;
Widget      get_answer;
Widget      grill_item;
Widget      lenght_B_item;
Widget      lenght_R_item;
Widget      memory_item;
Widget      msg_1, msg_2, msg_3, msg_4;
Widget      msg_time;
Widget      net_name_item;
Widget      reset_f_button;
Widget      seed_item;
Widget      set_time_item;
Widget      simulation_button;
Widget      start_s_button;
Widget      step_f_button;
Widget      stop_f_button;
Widget      time_slider;
Widget      token_item;
Widget      type_B_item;
Widget      type_R_item;
Widget      type_item;
Widget      val_item;
Widget      zoom_item;



/*#include	"gspnpos.c"*/

Display        *ovbr_display;
Drawable        ovbr_drawable;
GC              ovbr_gc;
GC              ovbr_dp_gc;
/*Display * display;*/
/*GC				ggc;

Drawable	drawable;*/
DrawingSurface mainDS;

Widget	overview_pw, canvas_pw;
Font		gachab_font, gachar_font, cour_font, cmr_font, sail_font;
Pixmap		circle_pixmap = (Pixmap)NULL ;



/***************************  global variables   *************************/

FILE           *dfp;
FILE           *nfp;
FILE    	*sim_req;
char            edit_file[LINEMAX];
float           select_left, select_top, select_width, select_height;
float           z_coef = 0.2;
int             bell_global_on = TRUE;
int             canvas_height = 1000;
int             canvas_width = 2000;
int             figure_modified = FALSE;
int             inib_flag = FALSE;
int             light_on = FALSE;
int             place_num, trans_num, group_num, cset_num;
int             rate_visible = FALSE;
int             res_visible = FALSE;
int             spline_on = FALSE;
int             tag_visible = FALSE;
struct net_object *bkup_netobj[MAX_BKUP];
struct net_object *netobj = NULL;
struct net_object selected;
char            host_name[LINEMAX];
float           select_left, select_top, select_width, select_height;
char           *cant_interrupt = "Sorry, can't interrupt current action";

/**********************  canvas variables  ************************/

/* canvas reader  */
void (*reader_middle)(), (*reader_left)();
void (*move)(), (*left)(), (*middle)();	/* canvas tracker */
void (*receiver)();	/* message receiver */

float           fix_x, fix_y, cur_x, cur_y;
float           prev_x, prev_y;
float           start_x, start_y;
float           zoom_level = 1.0;
int             action_on = FALSE;
int             cur_command;
int             cur_object;
int             cur_orient = HORIZONTAL;
int             flag_pinv;
int             flag_tinv;
int             grill_level = 1;
int             place_dec;
int             place_dim;
int             pointmarker_shown = 0;
int             receiving_msg = FALSE;
int             selection_on = FALSE;
struct images  *moving_images = NULL;


Font canvas_font;
XImage place_image;
XImage shadow;

Display	*gDisplay;
Screen 	*gScreen;
unsigned gDepth;

/****************************  Variables for Active Drawing  *************/

struct place_object *cur_place = NULL;
struct trans_object *cur_trans = NULL;
struct arc_object *cur_arc = NULL;

struct place_object *last_place;
struct trans_object *last_trans;
struct arc_object *last_arc;


/**************************  Default Variables for Fixed sized objects  ******/

float           place_radius = 7;
float           trans_length = 15;
float           titrans_height = 7;
float           imtrans_height = 3;
float           token_diameter = 3;
int             arrowht = 6;
int             arrowwid = 4;
float           not_radius = 2;

double          ISQRT2;

/* ****************************** ?????????????? *********************** */


FILE           *sim_ans;

Widget            help_co_menu;
Widget            help_md_menu;
Widget            help_re_menu;
Widget      host_name_item;
Widget      msg_fire;
Widget      search_string;

Widget       help_pw;
Font         time_font14;


/* Scrollbar       v_sb, h_sb;*/
char            s_string[LINEMAX];
char           *nets_dir_path, host_name[];
double          sim_time;
int             B_flag;
int             R_flag;
int             autofire_flag;
int             direction;
int             highlighted = FALSE;
int             int_val_step;
int             number_of_shots;
int             nw_x, nw_y;
int             pulisci_flag;
int             reset_flag;
int             show_result_flag;
int             simpid;
int             stopfire_flag;
int             timed_flag;
int             type_flag;
struct arc_object *a_list;
struct coordinate *s_list;
struct trans_object *trans_to_fire;
extern Widget ModifyArea;

Font         time_font18;
XImage    pi_im,
          shadow;		/* TBD */

void SetModify() {
    Arg args[1];
    XmString xms;

    if (!figure_modified) {
        figure_modified = 1;
        xms = XmStringCreateSimple("*");
        XtSetArg(args[0], XmNlabelString, xms);
        XtSetValues(ModifyArea, args, 1);
        XmStringFree(xms);
    }
}

void ResetModify() {
    Arg args[1];
    XmString xms;

    if (figure_modified) {
        figure_modified = 0;
        xms = XmStringCreateSimple("");
        XtSetArg(args[0], XmNlabelString, xms);
        XtSetValues(ModifyArea, args, 1);
        XmStringFree(xms);
    }
}

void null_proc(void) {
}



void xv_scrivo(DrawingSurfacePun       ds,
               int op, int  dx, int  dy, int  dw, int  dh, int  sx, int  sy,
               Drawable    pr) {

#ifdef DEBUG
    /*  fprintf(stderr,"    start of xv_scrivo drawable= %d pr=%d dx=%d, dy=%d, dw=%d, dh=%d, op=%d\n",
                drawable,pr, dx, dy, dw, dh, op );*/
#endif

#ifdef DEBUG_GC
    fprintf(stderr, "   ...xv_scrivo:  creating gc\n");
#endif // DEBUG_GC

    SetFunction(op, ds);
    XCopyArea(ds->display, pr, ds->drawable, ds->gc, 0, 0, dw, dh, dx, dy);
    XFlush(ds->display);

#ifdef DEBUG
    fprintf(stderr, "    end of xv_scrivo\n");
#endif
}

