extern	char           *Gspn;

extern Widget  statuswin, popup, workwin;
extern Widget  drawwin;

extern	Widget           frame_w;
extern	Widget           mainwin;
extern	Widget           sb1;
extern	Widget           sb2;
extern DrawingSurface  mainDS;
extern Font		gachab_font, gachar_font, cour_font, cmr_font, sail_font;


/***************************  global variables   *************************/

extern	FILE           *dfp;
extern	FILE           *nfp;
extern	FILE           *sim_req;
extern	char            edit_file[LINEMAX];
extern	float           select_left, select_top, select_width, select_height;
extern	float           z_coef;
extern	int             bell_global_on;
extern	int             canvas_height;
extern	int             canvas_width;
extern	int             figure_modified;
extern	int             inib_flag;
extern	int             light_on;
extern	int             place_num, trans_num, group_num, cset_num, layer_num;
extern	int             rate_visible;
extern	int             res_visible;
extern	int             spline_on;
extern	int             tag_visible;
extern	struct net_object *bkup_netobj[MAX_BKUP];
extern	struct net_object *netobj;
extern	struct net_object selected;

extern	float           select_left, select_top, select_width, select_height;
extern	char           *cant_interrupt;

extern int		color_depth;

/**********************  canvas variables  ************************/

/* canvas reader  */
extern	void (*reader_middle)(), (*reader_left)();
extern	void (*move)(), (*left)(), (*middle)();	/* canvas tracker */
extern	void (*receiver)();	/* message receiver */

extern	float           fix_x, fix_y, cur_x, cur_y;
extern	float           prev_x, prev_y;
extern	float           start_x, start_y;
extern	float           zoom_level;
extern	int             action_on;
extern	int             cur_command;
extern	int             cur_object;
extern	int             cur_orient;
extern	int             flag_pinv;
extern	int             flag_tinv;
extern	int             grill_level;
extern	int             place_dec;
extern	int             place_dim;
extern	int             pointmarker_shown;
extern	int             receiving_msg;
extern	int             selection_on;
extern	struct images  *moving_images;


extern	Font canvas_font;

/****************************  Variables for Active Drawing  *************/

extern	struct place_object *cur_place;
extern	struct trans_object *cur_trans;
extern	struct arc_object *cur_arc;

extern	struct place_object *last_place;
extern	struct trans_object *last_trans;
extern	struct arc_object *last_arc;


/**************************  Default Variables for Fixed sized objects  ******/

extern	float           place_radius;
extern	float           trans_length;
extern	float           titrans_height;
extern	float           imtrans_height;
extern	float           token_diameter;
extern	int             arrowht;
extern	int             arrowwid;
extern	float           not_radius;

extern	double          ISQRT2;

/* ****************************** ?????????????? *********************** */


extern	FILE           *sim_ans;

extern	Widget            help_co_menu;
extern	Widget           help_md_menu;
extern	Widget           help_re_menu;
extern	Widget      host_name_item;
extern	Widget      msg_fire;
extern	Widget      search_string;


extern	Widget       help_pw;
extern	Font         time_font14;
extern	Font         time_font18;
extern	XImage    pi_im;

extern	char            s_string[LINEMAX];
extern	char           *nets_dir_path, host_name[];
extern	double          sim_time;
extern	int             B_flag;
extern	int             R_flag;
extern	int             autofire_flag;
extern	int             direction;
extern	int             highlighted;
extern	int             int_val_step;
extern	int             number_of_shots;
extern	int             nw_x, nw_y;
extern	int             pulisci_flag;
extern	int             reset_flag;
extern	int             show_result_flag;
extern	int             simpid;
extern	int             stopfire_flag;
extern	int             timed_flag;
extern	int             type_flag;
extern	struct arc_object *a_list;
extern	struct coordinate *s_list;
extern	struct trans_object *trans_to_fire;

extern Pixel gWhite, gBlack;
extern Display	*gDisplay;
extern Screen 	*gScreen;
extern unsigned gDepth;
