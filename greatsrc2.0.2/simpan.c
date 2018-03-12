/*
 * simpan.c
 */

/*
#define DEBUG
*/


#include "global.h"


FILE           *sim_us;
FILE           *sim_usl;


struct remember_B {
    char            m_c_f[2];
    char            m_b_f[2];
    char            m_d_f[2];
    char            m_m_f[2];
    int             m_c_i;
    int             m_b_i;
    int             m_d_i;
    int             m_m_i;
    char            m_l_f[256];
    char            m_L_f[256];
    char            m_s_f[256];
    char            m_e_f[256];
    char            m_back[256];
};


struct remember_R {
    char            m_c_f[2];
    char            m_b_f[2];
    char            m_d_f[2];
    char            m_m_f[2];
    int             m_c_i;
    int             m_b_i;
    int             m_d_i;
    int             m_m_i;
    char            m_l_f[256];
    char            m_L_f[256];
    char            m_s_f[256];
    char            m_e_f[256];
    char            m_back[256];
};


struct remember_R rem_R = {"0", "F", "H", "E", 0, 0, 0, 0, "1", "1000", "31415", "0.15", "2"};

struct remember_B rem_B = {"0", "F", "H", "E", 0, 0, 0, 0, "1", "1000", "31415", "0.15", "2"};


int             type_flag = TRUE;
int             type_panel_B_flag = FALSE;
int             type_panel_R_flag = TRUE;
int             move_flag = TRUE;
int             type_c_flag = TRUE;
int             esist_us = FALSE;
int             backup_B_flag = TRUE;
int             backup_R_flag = TRUE;
int             R_flag = FALSE;
int             B_flag = FALSE;
char            r_f[] = "b";
char            c_f[] = "0";
char            b_f[] = "F";
char            d_f[] = "H";
char            m_f[] = "E";
char            seed[256];
char            cycle_number_B[256];
char            cycle_number_R[256];
char            lenght_B[256];
char            lenght_R[256];
char            costant[256];
char            error[256];
char            back_B[256];
char            back_R[256];
int             c_i;
int             b_i;
int             d_i;
int             m_i;
int             simres;
int             cont;
int             conf_lev_prec;

//static
//rem_B_load()
//{
///*
//	P_SET(confidence_level_item, PANEL_VALUE, c_i = rem_B.m_c_i, 0);
//	sprintf(c_f, rem_B.m_c_f);
//	P_SET(backup_B_item, PANEL_VALUE, b_i = rem_B.m_b_i, 0);
//	sprintf(b_f, rem_B.m_b_f);
//	P_SET(delete_item, PANEL_VALUE, d_i = rem_B.m_d_i, 0);
//	sprintf(d_f, rem_B.m_d_f);
//	P_SET(memory_item, PANEL_VALUE, m_i = rem_B.m_m_i, 0);
//	sprintf(m_f, rem_B.m_m_f);
//	sprintf(error, rem_B.m_e_f);
//	P_SET(error_item, PANEL_VALUE, error, 0);
//	sprintf(cycle_number_B, rem_B.m_l_f);
//	P_SET(cycle_number_B_item, PANEL_VALUE, cycle_number_B, 0);
//	sprintf(lenght_B, rem_B.m_L_f);
//	P_SET(lenght_B_item, PANEL_VALUE, lenght_B, 0);
//	sprintf(seed, rem_B.m_s_f);
//	P_SET(seed_item, PANEL_VALUE, seed, 0);
//	sprintf(back_B, rem_B.m_back);
//	P_SET(back_B_item, PANEL_VALUE, back_B, 0);
//*/
//}
//
//
//rem_B_save()
//{
//  /*  rem_B.m_c_i = c_i;
//	rem_B.m_b_i = b_i;
//	rem_B.m_d_i = d_i;
//	rem_B.m_m_i = m_i;
//	sprintf(rem_B.m_c_f, c_f);
//	sprintf(rem_B.m_b_f, b_f);
//	sprintf(rem_B.m_d_f, d_f);
//	sprintf(rem_B.m_m_f, m_f);
//	sprintf(cycle_number_B, (char *) P_GET(cycle_number_B_item, PANEL_VALUE, 0));
//	sprintf(rem_B.m_l_f, cycle_number_B);
//	sprintf(lenght_B, (char *) P_GET(lenght_B_item, PANEL_VALUE, 0));
//	sprintf(rem_B.m_L_f, lenght_B);
//	sprintf(seed, (char *) P_GET(seed_item, PANEL_VALUE, 0));
//	sprintf(rem_B.m_s_f, seed);
//	sprintf(error, (char *) P_GET(error_item, PANEL_VALUE, 0));
//	sprintf(rem_B.m_e_f, error);
//	if (!backup_B_flag) {
//	sprintf(back_B, (char *) P_GET(back_B_item, PANEL_VALUE, 0));
//	sprintf(rem_B.m_back, back_B);
//	}
//	*/
//}
//
//
//static
//rem_R_load()
//{
//	/*
//	P_SET(confidence_level_item, PANEL_VALUE, c_i = rem_R.m_c_i, 0);
//	sprintf(c_f, rem_R.m_c_f);
//	P_SET(backup_R_item, PANEL_VALUE, b_i = rem_R.m_b_i, 0);
//	sprintf(b_f, rem_R.m_b_f);
//	P_SET(delete_item, PANEL_VALUE, d_i = rem_R.m_d_i, 0);
//	sprintf(d_f, rem_R.m_d_f);
//	P_SET(memory_item, PANEL_VALUE, m_i = rem_R.m_m_i, 0);
//	sprintf(m_f, rem_R.m_m_f);
//	sprintf(error, rem_R.m_e_f);
//	P_SET(error_item, PANEL_VALUE, error, 0);
//	sprintf(cycle_number_R, rem_R.m_l_f);
//	P_SET(cycle_number_R_item, PANEL_VALUE, cycle_number_R, 0);
//	sprintf(lenght_R, rem_R.m_L_f);
//	P_SET(lenght_R_item, PANEL_VALUE, lenght_R, 0);
//	sprintf(seed, rem_R.m_s_f);
//	P_SET(seed_item, PANEL_VALUE, seed, 0);
//	sprintf(back_R, rem_R.m_back);
//	P_SET(back_R_item, PANEL_VALUE, back_R, 0);
//	*/
//}
//
//
//rem_R_save()
//{
//	/*
//	rem_R.m_c_i = c_i;
//	rem_R.m_b_i = b_i;
//	rem_R.m_d_i = d_i;
//	rem_R.m_m_i = m_i;
//	sprintf(rem_R.m_c_f, c_f);
//	sprintf(rem_R.m_b_f, b_f);
//	sprintf(rem_R.m_d_f, d_f);
//	sprintf(rem_R.m_m_f, m_f);
//	sprintf(cycle_number_R, (char *) P_GET(cycle_number_R_item, PANEL_VALUE, 0));
//	sprintf(rem_R.m_l_f, cycle_number_R);
//	sprintf(lenght_R, (char *) P_GET(lenght_R_item, PANEL_VALUE, 0));
//	sprintf(rem_R.m_L_f, lenght_R);
//	sprintf(seed, (char *) P_GET(seed_item, PANEL_VALUE, 0));
//	sprintf(rem_R.m_s_f, seed);
//	sprintf(error, (char *) P_GET(error_item, PANEL_VALUE, 0));
//	sprintf(rem_R.m_e_f, error);
//	if (!backup_R_flag) {
//	sprintf(back_R, (char *) P_GET(back_R_item, PANEL_VALUE, 0));
//	sprintf(rem_R.m_back, back_R);
//	}
//	*/
//}
//
//
//char            cont_string[256];
//
//
//open_us()
//{
///*
//	char            fname[100];
//	int             nn;
//	double          nf;
//
//	sprintf(fname, "nets/%s.US", edit_file);
//	if ((sim_us = fopen(fname, "r")) != NULL) {
//	esist_us = TRUE;
//		load_compact(&nn, sim_us);
//	simres = nn;
//	*/
//
//	/* fprintf(stderr, "simres = %d\n", nn); */
//	/*
//	load_compact(&nn, sim_us);
//	load_compact(&nn, sim_us);*/
//
//
//	/* fprintf(stderr, "nn = %d\n", nn); */
//	/*
//	switch (nn) {
//	case 1:
//		type_c_flag = FALSE;
//		break;
//	case 2:
//	case 3:
//		type_c_flag = TRUE;
//		break;
//	default:
//		esist_us = FALSE;
//		break;
//	}
//	load_double(&nf, sim_us);
//	load_compact(&nn, sim_us);
//	load_double(&nf, sim_us);
//	sim_time = nf;              */
//	/* fprintf(stderr, "temp_sim &&&&& di simpan.c = %f\n", temp_sim); */
///*	}
//	else
//	esist_us = FALSE;
//	(void) fclose(sim_us);
//	*/
//}
//
//
//open_usl()
//{
//	/*
//	char            fname[100];
//	int             nn;
//
//	sprintf(fname, "nets/%s.USL", edit_file);
//	P_SET(msg_fire, P_SHOW_ITEM, FALSE, 0);
//	if ((sim_usl = fopen(fname, "r")) != NULL) {
//	load_compact(&nn, sim_usl);
//	cont = nn;*/
//	/* fprintf(stderr, "cont in usl = %d\n", cont); */
///*	load_compact(&nn, sim_usl);*/
//	/* fprintf(stderr, "nn per conf_lev_prec = %d\n", nn); */
///*	switch (nn) {
//	case 1:
//		conf_lev_prec = 0;
//		break;
//	case 2:
//		conf_lev_prec = 5;
//		break;
//	case 3:
//		conf_lev_prec = 9;
//		break;
//	default:
//		break;
//	}
//	(void) fclose(sim_usl);
//	}
//	*/
//}
//
//
//				/* TBD */
//int             nnn;		/* da togliere */
//
//static
//contator()
//{
//	/*
//	if (!type_flag)
//	sprintf(cont_string, "cycles:%d", cont);
//	else {
//	nnn = nnn + 1;*/		/* da togliere */
//	/* fprintf(stderr," cont e' prima  = %d\n",cont);  */
//	/* cont = cont / atoi(lenght_B); */
//	/* fprintf(stderr," cont e' dopo = %d\n",cont);  */
//	/* sprintf(cont_string, "batches:%d", cont);  */
///*	sprintf(cont_string, "batches:%d", nnn);  */	/* da togliere */
///*	}
//	P_SET(cont_item, PANEL_LABEL_STRING, cont_string, 0);
//	P_SET(cont_item, P_SHOW_ITEM, TRUE, 0);*/
//}
//
//
//result_sim()
//{
//	/*
//	if ((!R_flag) && (!B_flag)) {
//	show_result_flag = FALSE;
//	pulisci_flag = TRUE;
//	ind_prest_list_new();
//	pulisci_flag = FALSE;
//	}
//	cont = 0;
//	nnn = -1;*/			/* da togliere */
///*	contator();
//	open_s_a();
//	show_result_flag = TRUE;
//	open_us();
//	open_usl();
//	contator();*/
//	/* fprintf(stderr," simres e' = %d\n",simres); */
///*	while (simres != 20) {
//	read_us();
//	delete_s_a();*/
//	/* fprintf(stderr," distrutto .SIMCC\n"); */
///*	open_s_a();
//	open_us();
//	open_usl();
//	contator();*/
//	/* fprintf(stderr," simres e' = %d\n",simres); */
///*	}*/
//	/* fprintf(stderr," distrutto ultimo .SIMCC\n"); */
//	/*
//	read_us();
//	delete_s_a();
//	fire_close_proc();
//	P_SET(cont_item, P_SHOW_ITEM, FALSE, 0);
//	open_simpan_proc();
//	*/
//}
//
//
//backup_modify()
//{
///*
//	if (!type_flag) {
//	P_SET(backup_B_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(backup_R_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(back_R_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(comment_R_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(back_B_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(comment_B_item, P_SHOW_ITEM, FALSE, 0);
//    }
//    else {
//	P_SET(backup_R_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(backup_B_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(back_B_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(comment_B_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(back_R_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(comment_R_item, P_SHOW_ITEM, FALSE, 0);
//	}*/
//}
//
//
//backup_not_modify()
//{      /*
//	if (!type_flag) {
//	P_SET(backup_B_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(backup_R_item, P_SHOW_ITEM, TRUE, 0);
//	}
//	else {
//	P_SET(backup_R_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(backup_B_item, P_SHOW_ITEM, TRUE, 0);
//	}
//	P_SET(back_B_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(comment_B_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(back_R_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(comment_R_item, P_SHOW_ITEM, FALSE, 0);*/
//}
//
//
//type_initial()
//{             /*
//	P_SET(type_B_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(type_R_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(type_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(c_l_stop_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(cont_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(confidence_level_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(error_item, P_SHOW_ITEM, TRUE, 0);
//	if (!type_flag) {
//	if (move_flag != type_flag) {
//		rem_B_save();
//		rem_R_load();
//	}
//	P_SET(cycle_number_B_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(cycle_number_R_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(lenght_B_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(lenght_R_item, P_SHOW_ITEM, TRUE, 0);
//	}
//	else {
//	if (move_flag != type_flag) {
//		rem_R_save();
//		rem_B_load();
//	}
//	P_SET(cycle_number_R_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(cycle_number_B_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(lenght_R_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(lenght_B_item, P_SHOW_ITEM, TRUE, 0);
//	}           */
//}
//
//
//type_modify()
//{                 /*
//	P_SET(type_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(type_B_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(type_R_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(cont_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(confidence_level_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(error_item, P_SHOW_ITEM, TRUE, 0);
//	if (!type_flag) {
//	if (move_flag != type_flag) {
//		rem_B_save();
//		rem_R_load();
//	}
//	else {
//		if (type_panel_R_flag) {
//		P_SET(type_R_item, PANEL_VALUE, 1, 0);
//		sprintf(r_f, "P");
//		}
//	}
//	P_SET(cycle_number_B_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(cycle_number_R_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(lenght_B_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(lenght_R_item, P_SHOW_ITEM, TRUE, 0);
//	if (R_flag) {
//		P_SET(confidence_level_item, P_SHOW_ITEM, FALSE, 0);
//		sprintf(costant, "Confidence Level: 9%d%%", conf_lev_prec);
//		P_SET(c_l_stop_item, PANEL_LABEL_STRING, costant, 0);
//		P_SET(c_l_stop_item, P_SHOW_ITEM, TRUE, 0);
//	}
//	else {
//		P_SET(c_l_stop_item, P_SHOW_ITEM, FALSE, 0);
//		P_SET(confidence_level_item, P_SHOW_ITEM, TRUE, 0);
//	}
//	}
//	else {
//	if (move_flag != type_flag) {
//		rem_R_save();
//		rem_B_load();
//	}
//	P_SET(c_l_stop_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(cycle_number_R_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(cycle_number_B_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(lenght_R_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(lenght_B_item, P_SHOW_ITEM, TRUE, 0);
//	}               */
//}
//
//
//type_not_modify()
//{                     /*
//	P_SET(type_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(type_R_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(type_B_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(cont_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(confidence_level_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(error_item, P_SHOW_ITEM, TRUE, 0);
//	if (!type_flag) {
//	if (move_flag != type_flag) {
//		rem_B_save();
//		rem_R_load();
//	}
//	P_SET(cycle_number_B_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(cycle_number_R_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(lenght_B_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(lenght_R_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(c_l_stop_item, P_SHOW_ITEM, FALSE, 0);
//	}
//	else {
//	if (move_flag != type_flag) {
//		rem_R_save();
//		rem_B_load();
//	}
//	else {
//		if (!type_panel_B_flag) {
//		P_SET(type_B_item, PANEL_VALUE, 0, 0);
//		sprintf(r_f, "b");
//		}
//	}
//	if (B_flag) {
//		P_SET(confidence_level_item, P_SHOW_ITEM, FALSE, 0);
//		sprintf(costant, "Confidence Level: 9%d%%", conf_lev_prec);
//		P_SET(c_l_stop_item, PANEL_LABEL_STRING, costant, 0);
//		P_SET(c_l_stop_item, P_SHOW_ITEM, TRUE, 0);
//	}
//	else {
//		P_SET(c_l_stop_item, P_SHOW_ITEM, FALSE, 0);
//		P_SET(confidence_level_item, P_SHOW_ITEM, TRUE, 0);
//	}
//	P_SET(cycle_number_R_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(cycle_number_B_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(lenght_R_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(lenght_B_item, P_SHOW_ITEM, TRUE, 0);
//	}                   */
//}
//
//
//new_panel_s()
//{                         /*
//	P_SET(stop_f_button, P_SHOW_ITEM, FALSE, 0);
//	P_SET(step_f_button, P_SHOW_ITEM, FALSE, 0);
//	P_SET(val_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(afire_f_button, P_SHOW_ITEM, FALSE, 0);
//	P_SET(reset_f_button, P_SHOW_ITEM, FALSE, 0);
//	P_SET(token_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(auto_f_button, P_SHOW_ITEM, FALSE, 0);
//	P_SET(break_f_button, P_SHOW_ITEM, FALSE, 0);
//	P_SET(time_slider, P_SHOW_ITEM, FALSE, 0);
//	P_SET(forw_back_item, P_SHOW_ITEM, FALSE, 0);
//	P_SET(msg_time, P_SHOW_ITEM, FALSE, 0);
//	P_SET(msg_fire, P_SHOW_ITEM, FALSE, 0);
//	P_SET(start_s_button, P_SHOW_ITEM, TRUE, 0);
//	P_SET(seed_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(cycle_number_B_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(cycle_number_R_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(lenght_B_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(lenght_R_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(delete_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(memory_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(back_B_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(back_R_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(backup_B_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(backup_R_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(comment_B_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(comment_R_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(error_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(confidence_level_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(c_l_stop_item, P_SHOW_ITEM, TRUE, 0);
//	P_SET(cont_item, P_SHOW_ITEM, FALSE, 0);*/
//}
//
//
//open_s_proc(item, event)
//Widget      item;
//XEvent          *event;
//{
//	/*
//	if (inib_flag)
//	return;
//	window_fit(fire_sw);
//    window_fit(fire_f);
//	xv_set(fire_f, XV_SHOW, TRUE, 0);
//	new_panel_s();
//    open_us();
//    if (!esist_us) {
//	type_flag = TRUE;
//	move_flag = TRUE;
//	rem_B_load();
//	P_SET(type_item, PANEL_VALUE, 0, 0);
//	sprintf(r_f, "b");
//	type_initial();
//    }
//    else {
//	if (!type_c_flag) {
//	    type_flag = FALSE;
//	    move_flag = FALSE;
//	    type_panel_R_flag = TRUE;
//		rem_R_load();
//		type_modify();
//	}
//	else {
//		type_flag = TRUE;
//		move_flag = TRUE;
//		type_panel_B_flag = FALSE;
//		rem_B_load();
//		type_not_modify();
//	}
//	}
//	if (!type_flag) {
//	if (!backup_R_flag)
//		backup_modify();
//	else
//		backup_not_modify();
//	}
//	else {
//	if (!backup_B_flag)
//		backup_modify();
//	else
//		backup_not_modify();
//	}
//	move_flag = type_flag;
//	*/
//}
//
//
//open_simpan_proc()
//{     /*
//	window_fit(fire_sw);
//	window_fit(fire_f);
//	xv_set(fire_f, XV_SHOW, TRUE, 0);
//	open_us();
//	if (!esist_us)
//	type_initial();
//	else {
//	if (!type_c_flag)
//		type_modify();
//	else
//		type_not_modify();
//	}
//	if (!type_flag) {
//	if (!backup_R_flag)
//		backup_modify();
//	else
//		backup_not_modify();
//	}
//	else {
//	if (!backup_B_flag)
//		backup_modify();
//	else
//		backup_not_modify();
//	}
//	move_flag = type_flag;*/
//}
//
//
//set_type_proc(item, value, event)
//Widget      item;
//int             value;
//XEvent          *event;
//{
//	/*
//	if (inib_flag)
//	return;
//	if (receiving_msg)
//	return;
//	switch (value) {
//	case 1:
//	type_flag = FALSE;
//	show_result_flag = FALSE;
//	sprintf(r_f, "P");
//	break;
//	case 2:
//	type_flag = FALSE;
//	show_result_flag = FALSE;
//	sprintf(r_f, "P");
//	break;
//	default:
//	type_flag = TRUE;
//	show_result_flag = FALSE;
//	sprintf(r_f, "b");
//	break;
//	}
//	fire_close_proc();
//	open_simpan_proc();*/
//}
//
//
//set_type_B_proc(item, value, event)
//Widget      item;
//int             value;
//XEvent          *event;
//{
//
//	/*
//	if (inib_flag)
//	return;
//	if (receiving_msg)
//	return;
//	switch (value) {
//	case 1:
//	type_flag = FALSE;
//	show_result_flag = FALSE;
//	type_panel_B_flag = FALSE;
//	B_flag = FALSE;
//	R_flag = FALSE;
//	sprintf(r_f, "P");
//	break;
//	case 2:
//	type_flag = FALSE;
//	show_result_flag = FALSE;
//	type_panel_B_flag = FALSE;
//	B_flag = FALSE;
//	R_flag = FALSE;
//	sprintf(r_f, "P");
//	break;
//	case 3:
//	type_flag = TRUE;
//	show_result_flag = TRUE;
//	type_panel_B_flag = TRUE;
//	B_flag = TRUE;
//	R_flag = FALSE;
//	sprintf(r_f, "B");
//	break;
//	default:
//	type_flag = TRUE;
//	show_result_flag = FALSE;
//	type_panel_B_flag = TRUE;
//	B_flag = FALSE;
//	R_flag = FALSE;
//	sprintf(r_f, "b");
//	break;
//	}
//	fire_close_proc();
//	open_simpan_proc();
//	*/
//}
//
//
//set_type_R_proc(item, value, event)
//Widget      item;
//int             value;
//XEvent          *event;
//{
//	/*
//	if (inib_flag)
//	return;
//	if (receiving_msg)
//	return;
//	switch (value) {
//	case 1:
//	type_flag = FALSE;
//	show_result_flag = FALSE;
//	type_panel_R_flag = FALSE;
//	sprintf(r_f, "P");
//	R_flag = FALSE;
//	B_flag = FALSE;
//	break;
//	case 2:
//	type_flag = FALSE;
//	show_result_flag = FALSE;
//	type_panel_R_flag = FALSE;
//	sprintf(r_f, "P");
//	R_flag = FALSE;
//	B_flag = FALSE;
//	break;
//	case 3:
//	type_flag = FALSE;
//	show_result_flag = TRUE;
//	type_panel_R_flag = FALSE;
//	sprintf(r_f, "R");
//	R_flag = TRUE;
//	B_flag = FALSE;
//	break;
//	default:
//	type_flag = TRUE;
//	show_result_flag = FALSE;
//	type_panel_R_flag = TRUE;
//	sprintf(r_f, "b");
//	R_flag = FALSE;
//	B_flag = FALSE;
//	break;
//	}
//	fire_close_proc();
//	open_simpan_proc();
//	*/
//}
//
//
//set_confidence_level_proc(item, value, event)
//Widget     item;
//int             value;
//XEvent          *event;
//{
//	/*
//	if (inib_flag)
//	return;
//	if (receiving_msg)
//	return;
//	switch (value) {
//	case 1:
//	sprintf(c_f, "5");
//	c_i = 1;
//	break;
//	case 2:
//	sprintf(c_f, "9");
//	c_i = 2;
//	break;
//	default:
//	sprintf(c_f, "0");
//	c_i = 0;
//	break;
//	}
//	*/
//}
//
//
//set_backup_B_proc(item, value, event)
//Widget      item;
//int             value;
//XEvent          *event;
//{
//	/*
//	if (inib_flag)
//	return;
//	if (receiving_msg)
//	return;
//	switch (value) {
//	case 1:
//	fire_close_proc();
//	backup_B_flag = FALSE;
//	sprintf(b_f, "F");
//	open_simpan_proc();
//	b_i = 1;
//	break;
//	case 2:
//	backup_B_flag = TRUE;
//	backup_not_modify();
//	sprintf(b_f, "A");
//	b_i = 2;
//	break;
//	default:
//	backup_B_flag = TRUE;
//	backup_not_modify();
//	sprintf(b_f, "F");
//	b_i = 0;
//	break;
//	}
//	*/
//}
//
//
//set_backup_R_proc(item, value, event)
//Widget      item;
//int             value;
//XEvent          *event;
//{
//
//	/*
//	if (inib_flag)
//	return;
//	if (receiving_msg)
//	return;
//	switch (value) {
//	case 1:
//	fire_close_proc();
//	backup_R_flag = FALSE;
//	sprintf(b_f, "F");
//	b_i = 1;
//	open_simpan_proc();
//	break;
//	case 2:
//	backup_R_flag = TRUE;
//	backup_not_modify();
//	sprintf(b_f, "A");
//	b_i = 2;
//	break;
//	default:
//	backup_R_flag = TRUE;
//	backup_not_modify();
//	sprintf(b_f, "F");
//	b_i = 0;
//	break;
//	}
//	*/
//}
//
//
//set_delete_proc(item, value, event)
//Widget      item;
//int             value;
//XEvent          *event;
//{
///*
//	if (inib_flag)
//	return;
//	if (receiving_msg)
//	return;
//	switch (value) {
//	case 1:
//	sprintf(d_f, "T");
//	d_i = 1;
//	break;
//	case 2:
//	sprintf(d_f, "C");
//	d_i = 2;
//	break;
//	default:
//	sprintf(d_f, "H");
//	d_i = 0;
//	break;
//	}
//	*/
//}
//
//
//set_memory_proc(item, value, event)
//Widget      item;
//int             value;
//XEvent          *event;
//{
//
//	/*
//	if (inib_flag)
//	return;
//	if (receiving_msg)
//	return;
//	switch (value) {
//	case 1:
//	sprintf(m_f, "A");
//	m_i = 1;
//	break;
//	default:
//	sprintf(m_f, "E");
//	m_i = 0;
//	break;
//	}
//	*/
//}
//
//
//static
//cambia()
//{
///*
//	int             l;
//
//	l = atoi(lenght_B) * atoi(cycle_number_B);
//	sprintf(cycle_number_B, "%d", l);
//	l = atoi(lenght_B) * atoi(back_B);
//	sprintf(back_B, "%d", l);
//	*/
//}
//
//init_simulation()
//{
///*
//	char            command[200];
//
//	sprintf(error, (char *) P_GET(error_item, PANEL_VALUE, 0));
//	sprintf(seed, (char *) P_GET(seed_item, PANEL_VALUE, 0));
//	Gspn = getenv("GREATSPN_SCRIPTDIR");
//	if (!type_flag) {
//	sprintf(cycle_number_R,
//			(char *) P_GET(cycle_number_R_item, PANEL_VALUE, 0));
//	sprintf(lenght_R, (char *) P_GET(lenght_R_item, PANEL_VALUE, 0));
//	if (!backup_R_flag)
//		sprintf(back_R, (char *) P_GET(back_R_item, PANEL_VALUE, 0));
//	else
//		sprintf(back_R, "1");
//	fprintf(stderr,
//		"csh -f %s/simula %s -g -r%s -s%s -L%s -l%s -d%s -m%s -b%s%s -c%s -e%s\n",
//	Gspn, edit_file, r_f, seed, lenght_R, cycle_number_R, d_f, m_f, b_f,
//		back_R, c_f, error);
//	sprintf(command, "csh -f %s/simula %s -g -r%s -s%s -L%s -l%s -d%s -m%s -b%s%s -c%s -e%s\n",
//		Gspn, edit_file, r_f, seed, lenght_R, cycle_number_R, d_f, m_f, b_f, back_R, c_f, error);
//	}
//	else {
//	sprintf(cycle_number_B, (char *) P_GET(cycle_number_B_item, PANEL_VALUE, 0));
//	sprintf(lenght_B, (char *) P_GET(lenght_B_item, PANEL_VALUE, 0));
//	if (!backup_B_flag)
//		sprintf(back_B, (char *) P_GET(back_B_item, PANEL_VALUE, 0));
//	else
//		sprintf(back_B, "1");
//
//	*/
//	/* in questo punto e' da togliere "cambia" */
//	/*
//	cambia();
//
//	fprintf(stderr, "csh -f %s/simula %s -g -r%s -s%s -L%s -l%s -d%s -m%s -b%s%s -c%s -e%s\n",
//		Gspn, edit_file, r_f, seed, lenght_B, cycle_number_B, d_f, m_f, b_f, back_B, c_f, error);
//	sprintf(command, "csh -f %s/simula %s -g -r%s -s%s -L%s -l%s -d%s -m%s -b%s%s -c%s -e%s\n",
//		Gspn, edit_file, r_f, seed, lenght_B, cycle_number_B, d_f, m_f, b_f, back_B, c_f, error);
//	}
//	system(command);
//	*/
//}
//
//
//simpan_start_proc(item, event)
//Widget      item;
//XEvent          *event;
//{
//	/*
//	if (inib_flag)
//	return;
//	cur_command = NULL_CHOICE;
//	menu_action();
//	init_simulation();
//	result_sim();
//	*/
//}
//
//
//init_simpan()
//{
//
//	/*
//	fire_f = xv_create(frame_w, FRAME,
//						   FRAME_LABEL, "GreatSPN: Simulation Control",
//			   WIN_ERROR_MSG, "Can't create popup window",
//			   FRAME_DONE_PROC, fire_close_proc_alt,
//			   WIN_SHOW, FALSE,
//			   XV_FONT, gachar_font,
//			   XV_WIDTH, 200,
//			   XV_HEIGHT, 200,
//			   XV_X, 20,
//			   XV_Y, 70,
//			   0);
//	fire_sw = xv_create(fire_f, PANEL,
//				XV_FONT, gachar_font,
//				WIN_SHOW, TRUE,
//				PANEL_LAYOUT, PANEL_HORIZONTAL,
//				0);
//	type_item = xv_create(fire_sw, PANEL_CHOICE,
//				  PANEL_DISPLAY_LEVEL,    PANEL_CURRENT,
//				  PANEL_LABEL_STRING, "Type:",
//				  PANEL_CHOICE_STRINGS, "Batch Run", "Pilote + Reg. Run", "Reg. (No Pilote) Run", 0,
//				  XV_X, 0,
//				  XV_Y, xv_row(fire_sw,0),
//				  P_SHOW_ITEM, FALSE,
//				  PANEL_NOTIFY_PROC, set_type_proc,
//				  0);
//	type_B_item = xv_create(fire_sw, PANEL_CHOICE,
//								  PANEL_DISPLAY_LEVEL,    PANEL_CURRENT,
//					PANEL_LABEL_STRING, "Type:",
//					PANEL_CHOICE_STRINGS, "Batch Run", "Pilote + Reg. Run", "Reg. (No Pilote) Run", "Batch Continuation", 0,
//				  XV_X, 0,
//				  XV_Y, xv_row(fire_sw,0),
//					P_SHOW_ITEM, FALSE,
//					PANEL_NOTIFY_PROC, set_type_B_proc,
//					0);
//	type_R_item = xv_create(fire_sw, PANEL_CHOICE,
//								  PANEL_DISPLAY_LEVEL,    PANEL_CURRENT,
//					PANEL_LABEL_STRING, "Type:",
//					PANEL_CHOICE_STRINGS, "Batch Run", "Pilote + Reg. Run", "Reg. (No Pilote) Run", "Reg. Continuation", 0,
//					PANEL_VALUE, 1,
//				  XV_X, 0,
//				  XV_Y, xv_row(fire_sw,0),
//					P_SHOW_ITEM, FALSE,
//					PANEL_NOTIFY_PROC, set_type_R_proc,
//					0);
//	sprintf(cycle_number_B, "1");
//
//	cycle_number_B_item = xv_create(fire_sw, PANEL_TEXT,
//				   PANEL_LABEL_STRING, "Max Number Of Batches:",
//						PANEL_VALUE_DISPLAY_LENGTH, 4,
//						PANEL_VALUE, cycle_number_B,
//						PANEL_VALUE_FONT, gachab_font,
//						XV_xy(fire_sw,31,0),
//						P_SHOW_ITEM, FALSE,
//						0);
//	sprintf(lenght_B, "1000");
//
//	lenght_B_item = xv_create(fire_sw, PANEL_TEXT,
//					  PANEL_LABEL_STRING, "Batch Lenght:",
//					  PANEL_VALUE_DISPLAY_LENGTH, 8,
//					  PANEL_VALUE, lenght_B,
//					  PANEL_VALUE_FONT, gachab_font,
//					  XV_xy(fire_sw,31,1),
//					  P_SHOW_ITEM, FALSE,
//					  0);
//
//	sprintf(cycle_number_R, "1");
//
//	cycle_number_R_item = xv_create(fire_sw, PANEL_TEXT,
//				PANEL_LABEL_STRING, "Max Number Of Cycles:",
//						PANEL_VALUE_DISPLAY_LENGTH, 4,
//						PANEL_VALUE, cycle_number_R,
//						PANEL_VALUE_FONT, gachab_font,
//						XV_xy(fire_sw,31,0),
//						P_SHOW_ITEM, FALSE,
//						0);
//
//	sprintf(lenght_R, "1000");
//
//	lenght_R_item = xv_create(fire_sw, PANEL_TEXT,
//			   PANEL_LABEL_STRING, "Target Aver. Cycle Lenght:",
//					  PANEL_VALUE_DISPLAY_LENGTH, 8,
//					  PANEL_VALUE, lenght_R,
//					  PANEL_VALUE_FONT, gachab_font,
//					  XV_xy(fire_sw,31,1),
//					  P_SHOW_ITEM, FALSE,
//					  0);
//
//	cont_item = xv_create(fire_sw, PANEL_MESSAGE,
//				  PANEL_LABEL_STRING, "",
//				  XV_xy(fire_sw,55,8),
//				  0);
//
//	c_l_stop_item = xv_create(fire_sw, PANEL_MESSAGE,
//					  PANEL_LABEL_STRING, "",
//					  XV_xy(fire_sw,0,2),
//					  0);
//	confidence_level_item = xv_create(fire_sw, PANEL_CHOICE,
//								  PANEL_DISPLAY_LEVEL,    PANEL_CURRENT,
//					PANEL_LABEL_STRING, "Confidence Level:",
//				   PANEL_CHOICE_STRINGS, "90%", "95%", "99%", 0,
//					XV_X,0,XV_Y,xv_row(fire_sw,2),
//						  P_SHOW_ITEM, FALSE,
//				   PANEL_NOTIFY_PROC, set_confidence_level_proc,
//						  0);
//	sprintf(error, "0.15");
//
//	error_item = xv_create(fire_sw, PANEL_TEXT,
//				   PANEL_LABEL_STRING, "Error:",
//				   PANEL_VALUE_DISPLAY_LENGTH, 9,
//				   PANEL_VALUE, error,
//				   PANEL_VALUE_FONT, gachab_font,
//				   XV_xy(fire_sw,31,2),
//				   P_SHOW_ITEM, FALSE,
//				   MENU_STRING, "confidence level", 0,
//				   0);
//
//	sprintf(seed, "31415");
//
//	seed_item = xv_create(fire_sw, PANEL_TEXT,
//				  PANEL_LABEL_STRING, "Seed:",
//				  PANEL_VALUE_DISPLAY_LENGTH, 9,
//				  PANEL_VALUE, seed,
//				  PANEL_VALUE_FONT, gachab_font,
//				XV_X,0,XV_Y,xv_row(fire_sw,4),
//				  P_SHOW_ITEM, FALSE,
//				  0);
//
//	sprintf(back_B, "2");
//
//
//	back_B_item = xv_create(fire_sw, PANEL_TEXT,
//					PANEL_VALUE_DISPLAY_LENGTH, 4,
//					PANEL_VALUE, back_B,
//					PANEL_VALUE_FONT, gachab_font,
//					XV_xy(fire_sw,54,4),
//					P_SHOW_ITEM, FALSE,
//					0);
//
//	sprintf(back_R, "2");
//
//	back_R_item = xv_create(fire_sw, PANEL_TEXT,
//					PANEL_VALUE_DISPLAY_LENGTH, 4,
//					PANEL_VALUE, back_R,
//					PANEL_VALUE_FONT, gachab_font,
//					XV_xy(fire_sw,54,4),
//					P_SHOW_ITEM, FALSE,
//					0);
//	comment_B_item = xv_create(fire_sw, PANEL_MESSAGE,
//					   PANEL_LABEL_STRING, "Batches",
//					   XV_xy(fire_sw,60,4),
//					   P_SHOW_ITEM, FALSE,
//					   0);
//	comment_R_item = xv_create(fire_sw, PANEL_MESSAGE,
//					   PANEL_LABEL_STRING, "Cycles",
//					   XV_xy(fire_sw,60,4),
//					   P_SHOW_ITEM, FALSE,
//					   0);
//	backup_B_item = xv_create(fire_sw, PANEL_CHOICE,
//								  PANEL_DISPLAY_LEVEL,    PANEL_CURRENT,
//					  PANEL_LABEL_STRING, "Backup:",
//	 PANEL_CHOICE_STRINGS, "Each Batch", "   Every : ", "At The End", 0,
//					  XV_xy(fire_sw,31,4),
//					  P_SHOW_ITEM, FALSE,
//					  PANEL_NOTIFY_PROC, set_backup_B_proc,
//					  0);
//	backup_R_item = xv_create(fire_sw, PANEL_CHOICE,
//								  PANEL_DISPLAY_LEVEL,    PANEL_CURRENT,
//					  PANEL_LABEL_STRING, "Backup:",
//	 PANEL_CHOICE_STRINGS, "Each Cycle", "   Every : ", "At The End", 0,
//					  XV_xy(fire_sw,31,4),
//					  P_SHOW_ITEM, FALSE,
//					  PANEL_NOTIFY_PROC, set_backup_R_proc,
//					  0);
//	delete_item = xv_create(fire_sw, PANEL_CHOICE,
//								  PANEL_DISPLAY_LEVEL,    PANEL_CURRENT,
//					PANEL_LABEL_STRING, "Disabling Policy:",
//			  PANEL_CHOICE_STRINGS, "Head", "Tail", "Random", 0,
//					XV_X,0,XV_Y,xv_row(fire_sw,6),
//					P_SHOW_ITEM, FALSE,
//					PANEL_NOTIFY_PROC, set_delete_proc,
//					0);
//	memory_item = xv_create(fire_sw, PANEL_CHOICE,
//								  PANEL_DISPLAY_LEVEL,    PANEL_CURRENT,
//					PANEL_LABEL_STRING, "Transition Policy:",
//		   PANEL_CHOICE_STRINGS, "Enabling Memory", "Age Memory", 0,
//					XV_xy(fire_sw,31,6),
//					P_SHOW_ITEM, FALSE,
//					PANEL_NOTIFY_PROC, set_memory_proc,
//					0);
//	start_s_button = xv_create(fire_sw, PANEL_BUTTON,
//					   PANEL_LABEL_STRING, fire_sw, "start", 0,
//					   XV_xy(fire_sw,45,8),
//					   PANEL_NOTIFY_PROC, simpan_start_proc,
//					   P_SHOW_ITEM, FALSE,
//					   MENU_STRING, "start simulation", 0,
//					   0);
//					   */
//}
