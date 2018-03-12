/*
 * file:  global.c
 */

/*
#define DEBUG
#define DEBUG_GC
*/

#include    "global.h"
char           *Gspn;

/***************************  global variables   *************************/

FILE           *dfp;
FILE           *nfp;
FILE        *sim_req;
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

/****************************  Variables for Active Drawing  *************/

struct place_object *last_place;
struct trans_object *last_trans;

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
