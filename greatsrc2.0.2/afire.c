/*
 * afire.c
 */

#define DEBUG


#include "global.h"
#include "afire.h"
#include "Canvas.h"

#define FORW 1
#define BACK 2
#define TIMED 1
#define UNTIMED 2

int             timed_flag = FALSE;
int             reset_flag = FALSE;
int             simpan_flag = FALSE;
int             simul_panel_flag = FALSE;
char            tok_str[9];
char            numtk[10];
char            val_step[LINEMAX];
char            val_set_time[LINEMAX];
int             autofire_flag = FALSE;
int             stopfire_flag = FALSE;
int             direction = FORW;

int             int_val_step = 1;
static Widget sim_slider, sim_fire_button, sim_settime_button, sim_auto_button, sim_step_button, sim_forward, sim_backward, sim_untimed, sim_interactive, sim_dlg, sim_done, sim_break, sim_step	, sim_stop, sim_Time, sim_tokenmoves, sim_rc3, sim_time_label;


int GetSimSliderTime(void) {
    int t;

    XmScaleGetValue(sim_slider, &t);
    return t;
}



void afire_trans(XButtonEvent *ie) {
    struct group_object *group;

#ifdef DEBUG
    fprintf(stderr, "\n*** Start of afire_trans\n");
#endif
    fix_x = event_x(ie) / zoom_level;
    fix_y = event_y(ie) / zoom_level;
    if ((cur_trans = near_titrans_obj(fix_x, fix_y)) == NULL) {
        if ((cur_trans = near_imtrans_obj(fix_x, fix_y, &group)) == NULL) {
            return;
        }
    }
    trans_to_fire = cur_trans;
#ifdef DEBUG
    fprintf(stderr, "\n  *** trans_to_fire %d\n", trans_to_n(trans_to_fire));
#endif
    set_t_fire_name(cur_trans->tag);
}



static void fire_stop_proc_alt() {
    if (inib_flag)
        return;
    if (receiving_msg)
        return;
    autofire_flag = FALSE;
    stopfire_flag = TRUE;
    /*
    xv_set(stop_f_button, XV_SHOW, FALSE, 0);
    xv_set(time_slider, XV_SHOW, FALSE, 0);
    xv_set(step_f_button, XV_SHOW, TRUE, 0);
    xv_set(val_item, XV_SHOW, TRUE, 0);
    xv_set(set_time_item, XV_SHOW, TRUE, 0);
    xv_set(auto_f_button, XV_SHOW, TRUE, 0);
    xv_set(token_item, XV_SHOW, TRUE, 0);
    xv_set(break_f_button, XV_SHOW, TRUE, 0);
    xv_set(reset_f_button, XV_SHOW, TRUE, 0);
    */
    XtSetSensitive(sim_stop, FALSE);
    XtSetSensitive(sim_step_button, TRUE);
    XtSetSensitive(sim_step, TRUE);
    XtSetSensitive(sim_Time, TRUE);
    XtSetSensitive(sim_auto_button, TRUE);
    XtSetSensitive(sim_tokenmoves, TRUE);
    XtSetSensitive(sim_break, TRUE);
    XtSetSensitive(sim_settime_button, TRUE);
    reader_left = null_proc;
    if (direction == FORW) {
        /*
        put_msg(0,"Forward Interactive simulation");
        next_msg(0,"Select transition to fire with LEFT button");
        xv_set(afire_f_button, XV_SHOW, TRUE, 0);
        xv_set(msg_fire, XV_SHOW, TRUE, 0);
        */
        XtSetSensitive(sim_fire_button, TRUE);
        reader_left = afire_trans;
    }
    else {
        /*
        put_msg(0,"Backward Interactive simulation");
        xv_set(afire_f_button, XV_SHOW, FALSE, 0);
        xv_set(msg_fire, XV_SHOW, FALSE, 0);
        */
        XtSetSensitive(sim_fire_button, FALSE);
    }
    XtSetSensitive(sim_time_label, TRUE);
    /*	xv_set(msg_time, XV_SHOW, TRUE, 0);*/
    trans_to_fire = NULL;

}


static void fire_stop_proc() {
    if (inib_flag)
        return;
    if (receiving_msg)
        return;
    autofire_flag = FALSE;
    stopfire_flag = FALSE;
    /*
    xv_set(stop_f_button, XV_SHOW, FALSE, 0);
    xv_set(time_slider, XV_SHOW, FALSE, 0);
    xv_set(step_f_button, XV_SHOW, TRUE, 0);
    xv_set(val_item, XV_SHOW, TRUE, 0);
    xv_set(set_time_item, XV_SHOW, TRUE, 0);
    xv_set(auto_f_button, XV_SHOW, TRUE, 0);
    xv_set(token_item, XV_SHOW, TRUE, 0);
    xv_set(break_f_button, XV_SHOW, TRUE, 0);
    xv_set(reset_f_button, XV_SHOW, TRUE, 0);
    */
    XtSetSensitive(sim_stop, FALSE);
    XtSetSensitive(sim_step_button, TRUE);
    XtSetSensitive(sim_step, TRUE);
    XtSetSensitive(sim_Time, TRUE);
    XtSetSensitive(sim_auto_button, TRUE);
    XtSetSensitive(sim_tokenmoves, TRUE);
    XtSetSensitive(sim_break, TRUE);
    XtSetSensitive(sim_settime_button, TRUE);
    reader_left = null_proc;
    if (direction == FORW) {
        puts("Forward Interactive simulation");
        /*
        put_msg(0,"Forward Interactive simulation");
        next_msg(0,"Select transition to fire with LEFT button");
        xv_set(afire_f_button, XV_SHOW, TRUE, 0);
        xv_set(msg_fire, XV_SHOW, TRUE, 0);
        */
        XtSetSensitive(sim_fire_button, TRUE);
        reader_left = afire_trans;
    }
    else {
        puts("Backward Interactive simulation");
        /*
        put_msg(0,"Backward Interactive simulation");
        xv_set(afire_f_button, XV_SHOW, FALSE, 0);
        xv_set(msg_fire, XV_SHOW, FALSE, 0);
        */
        XtSetSensitive(sim_fire_button, FALSE);
    }

    /* xv_set(msg_time, XV_SHOW, TRUE, 0); */
    trans_to_fire = NULL;
}


static void fire_start_proc() {
    char *ss;

#ifdef DEBUG
    fprintf(stderr, "GreatSPN: start of fire_start_proc\n");
#endif
    if (inib_flag)
        return;
    if (receiving_msg)
        return;
    autofire_flag = TRUE;
    stopfire_flag = FALSE;
    reader_left = null_proc;
    trans_to_fire = NULL;

    /*
    put_msg(0,"Automatic interactive simulation");
    */
    XtSetSensitive(sim_stop, FALSE);
    XtSetSensitive(sim_step_button, FALSE);
    XtSetSensitive(sim_settime_button, FALSE);
    XtSetSensitive(sim_auto_button, FALSE);
    XtSetSensitive(sim_fire_button, FALSE);
    XtSetSensitive(sim_break, FALSE);
    XtSetSensitive(sim_stop, TRUE);
    XtSetSensitive(sim_time_label, TRUE);
    XtSetSensitive(sim_step, TRUE);
    XtSetSensitive(sim_tokenmoves, TRUE);

    ss = XmTextFieldGetString(sim_step);
    sprintf(val_step, ss);
    XtFree(ss);
    if ((int_val_step = atoi(val_step)) < 1) {
        int_val_step = 1;
        sprintf(val_step, "1");
        XmTextFieldSetString(sim_step, val_step);

    }
#ifdef DEBUG
    fprintf(stderr, " ... GreatSPN: val_step='%d'\n", int_val_step);
#endif
    start_simula();
#ifdef DEBUG
    fprintf(stderr, "GreatSPN: End of fire_start_proc\n");
#endif
}


void set_t_fire_name(char           *ss) {
    /*	xv_set(msg_fire, PANEL_LABEL_STRING, ss, 0);*/
}


static void fire_afire_proc() {
    int             nt;
    char *ss;

#ifdef DEBUG
    fprintf(stderr, "GreatSPN: start of fire_afire_proc\n");
#endif
    if (inib_flag)
        return;
    if (receiving_msg)
        return;
    if ((nt = trans_to_n(trans_to_fire)) <= 0)
        return;
#ifdef DEBUG
    fprintf(stderr, " ... GreatSPN: Forcing transition %d\n", nt);
#endif
    modify_evlist(nt);
    stopfire_flag = FALSE;
    reset_flag = FALSE;

    ss = XmTextFieldGetString(sim_step);
    sprintf(val_step, ss);
    XtFree(ss);
    if ((int_val_step = atoi(val_step)) < 1) {
        int_val_step = 1;
        sprintf(val_step, "1");
        XmTextFieldSetString(sim_step, val_step);

    }
#ifdef DEBUG
    fprintf(stderr, " ... GreatSPN: val_step='%d'\n", int_val_step);
#endif
    start_simula();
#ifdef DEBUG
    fprintf(stderr, "GreatSPN: End of fire_afire_proc\n");
#endif
}


static void fire_reset_proc() {
    double          ddd;
    char *ss;

    if (inib_flag)
        return;
    if (receiving_msg)
        return;
#ifdef DEBUG
    fprintf(stderr, "Forcing simulation time\n");
#endif

    /*
    sprintf(val_set_time, (char *)
    	xv_get(set_time_item, PANEL_VALUE, 0));
    */
    ss = XmTextFieldGetString(sim_Time);
    sprintf(val_set_time, ss);
    XtFree(ss);
    sscanf(val_set_time, "%lg", &ddd);
    if (ddd < 0.0) {
        ddd = 0.0;
        /* ??????????????????
        sprintf(val_set_time, "0.0");
        xv_set(set_time_item, PANEL_VALUE, val_set_time, 0);
        */
    }
    set_simulation_clock(ddd);
}

/* int numberstep_flag=TRUE; */



/*
static
fire_step_proc(item, event)
Widget      item;
XButtonEvent          *event;
{

	if (inib_flag)
	return;
	if (receiving_msg)
	return;
	stopfire_flag = FALSE;
	if (selection_on || action_on) {
	next_msg(2,cant_interrupt);
	if (bell_global_on)
		window_bell(menu_sw);
	return;
	}
	trans_to_fire = NULL;
	start_simula();

}
*/

void timed_fire_modify() {
    /*
    xv_set(fire_f, FRAME_BUSY, TRUE, NULL);
    xv_set(frame_w, FRAME_BUSY, TRUE, NULL);
    cur_command = SIMULATE;
    menu_action();
    xv_set(msg_time, PANEL_LABEL_STRING, "... Starting Simulation", 0);
    xv_set(msg_time, XV_SHOW, TRUE, 0);
    fire_stop_proc();
    xv_set(msg_fire, PANEL_LABEL_STRING, "", 0);
    init_int_sim();
    xv_set(fire_f, FRAME_BUSY, FALSE, NULL);
    xv_set(frame_w, FRAME_BUSY, FALSE, NULL);
    */
    cur_command = SIMULATE;
    menu_action();
    puts("Starting Simulation ....");
    fire_stop_proc();
    init_int_sim();
}


void untimed_fire_modify() {
    /*
    xv_set(break_f_button, XV_SHOW, FALSE, 0);
    xv_set(stop_f_button, XV_SHOW, FALSE, 0);
    xv_set(step_f_button, XV_SHOW, FALSE, 0);
    xv_set(val_item, XV_SHOW, FALSE, 0);
    xv_set(set_time_item, XV_SHOW, FALSE, 0);
    xv_set(afire_f_button, XV_SHOW, FALSE, 0);
    xv_set(reset_f_button, XV_SHOW, FALSE, 0);
    xv_set(auto_f_button, XV_SHOW, FALSE, 0);
    xv_set(time_slider, XV_SHOW, FALSE, 0);
    xv_set(msg_time, XV_SHOW, FALSE, 0);
    xv_set(msg_fire, XV_SHOW, FALSE, 0);
    cur_command = FIRE_TRANS;
    menu_action();
    XtSetSensitive(sim_time_label,FALSE);
    */
    cur_command = FIRE_TRANS;
    menu_action();
}


void new_panel_f() {

    XtSetSensitive(sim_tokenmoves, TRUE);
    XtSetSensitive(sim_forward, TRUE);
    XtSetSensitive(sim_backward, TRUE);
    XtSetSensitive(sim_untimed , TRUE);
    XtSetSensitive(sim_interactive , TRUE);

    XtSetSensitive(sim_break, FALSE);
    XtSetSensitive(sim_step , FALSE);
    XtSetSensitive(sim_stop , FALSE);
    XtSetSensitive(sim_Time , TRUE);
    XtSetSensitive(sim_step_button, FALSE);
    XtSetSensitive(sim_auto_button, FALSE);
    XtSetSensitive(sim_settime_button , FALSE);
    XtSetSensitive(sim_fire_button , FALSE);
    XtSetSensitive(sim_slider , FALSE);
    XmToggleButtonGadgetSetState(sim_untimed, 1, 1);
    XmToggleButtonGadgetSetState(sim_forward, 1, 1);
    res_visible = FALSE;
    redisplay_canvas();

    XtManageChild(sim_dlg);
}

static void open_fire_proc(void);

static void SetFireProcCB(Widget w, int closure, XmToggleButtonCallbackStruct *call_data) {
    if (!call_data->set)
        return;
    switch (closure) {
    case TIMED:
        show_result_flag = TRUE;
        if (simpan_flag) {
            fire_close_proc();
            timed_flag = TRUE;
//				if (!type_flag)
//					rem_R_save();
//				else
//					rem_B_save();
            simul_panel_flag = FALSE;
            open_f_proc();
        }
        else {
            fire_close_proc();
            timed_flag = TRUE;
            open_fire_proc();

        }
        simpan_flag = FALSE;

        XtSetSensitive(sim_break, TRUE);
        XtSetSensitive(sim_step , TRUE);
        XtSetSensitive(sim_stop , TRUE);
        XtSetSensitive(sim_step_button, TRUE);
        XtSetSensitive(sim_auto_button, TRUE);
        XtSetSensitive(sim_settime_button , TRUE);
        XtSetSensitive(sim_fire_button , TRUE);
        XtSetSensitive(sim_slider , FALSE);
        XtSetSensitive(sim_time_label, TRUE);

        break;
    case UNTIMED:
        timed_flag = FALSE;
        show_result_flag = FALSE;
        if (simpan_flag) {
            fire_close_proc();
//				if (!type_flag)
//					rem_R_save();
//				else
//					rem_B_save();
            simul_panel_flag = FALSE;
            open_f_proc();
        }
        else {
            redisplay_canvas();
            fire_close_proc();
            open_fire_proc();
        }
        simpan_flag = FALSE;
        XtSetSensitive(sim_break, FALSE);
        XtSetSensitive(sim_step , FALSE);
        XtSetSensitive(sim_stop , FALSE);
        XtSetSensitive(sim_step_button, FALSE);
        XtSetSensitive(sim_auto_button, FALSE);
        XtSetSensitive(sim_settime_button , FALSE);
        XtSetSensitive(sim_fire_button , FALSE);
        XtSetSensitive(sim_slider , FALSE);
        XtSetSensitive(sim_time_label, False);
        break;
    }
}

static void FireCloseProcAltCB(Widget w, XtPointer closure, XtPointer call_data) {
    fire_close_proc_alt();
}


void fire_close_proc_alt() {
    XtUnmanageChild(sim_dlg);

    /* xv_set(fire_f, XV_SHOW, FALSE, 0); */

    if (timed_flag)
        close_int_sim();
    if (simpan_flag) {
        // Functions bodies are commented out anyway
//		if (!type_flag)
//			rem_R_save();
//		else
//			rem_B_save();
    }
    XmToggleButtonGadgetSetState(sim_untimed, TRUE, TRUE);

    /*xv_set(fire_mode_item, PANEL_VALUE, 0, 0);*/
    cur_command = NULL_CHOICE;
    redisplay_canvas();
    menu_action();
    timed_flag = FALSE;
    simpan_flag = FALSE;
    simul_panel_flag = FALSE;
    show_result_flag = TRUE;
    B_flag = FALSE;
    R_flag = FALSE;
    res_visible = TRUE;
    clear_canvas();
    redisplay_canvas();
}

void open_f_proc() {
    if (simul_panel_flag)
        return;

    simul_panel_flag = TRUE;
    new_panel_f();

    if (timed_flag)
        timed_fire_modify();
    else
        untimed_fire_modify();
}

static void OpenFProcCB(Widget w, int closure, XtPointer call_data) {

    open_f_proc();

}


static void open_fire_proc(void) {

    if (inib_flag)
        return;
    /*
    xv_set(fire_sw,
    XV_WIDTH, 360,
    XV_HEIGHT, 190,
    0);
    window_fit(fire_f);
    xv_set(fire_f,
    WIN_SHOW, TRUE,
    0);
    */
    if (timed_flag)
        timed_fire_modify();
    else
        untimed_fire_modify();

}


void fire_close_proc() {
    /*
    xv_set(fire_f, WIN_SHOW, FALSE, 0);
    if (timed_flag)
    close_int_sim();
    cur_command = NULL_CHOICE;
    menu_action();
    */
    if (timed_flag)
        close_int_sim();
    cur_command = NULL_CHOICE;
    menu_action();
}


static void SetModeCB(Widget w, int closure, XmToggleButtonCallbackStruct *call_data) {

    if (!call_data->set || closure == direction)
        return;

    direction = closure;
    if (timed_flag) {
        if (autofire_flag)
            fire_start_proc();
        else
            fire_stop_proc();
    }
    else
        set_enabled();

}

void reset_mode_proc() {
    if (timed_flag) {
        direction = FORW;
        XmToggleButtonGadgetSetState(sim_forward, TRUE, TRUE);
        /*		xv_set(forw_back_item, PANEL_VALUE, 0, 0);*/
        autofire_flag = FALSE;
        fire_stop_proc();
    }

}

static void ValProcCB(Widget w, int closure, XtPointer *call_data) {
    char *ss;

    ss = XmTextFieldGetString(sim_step);
    sprintf(val_step, ss);
    XtFree(ss);

    if ((int_val_step = atoi(val_step)) < 1) {
        int_val_step = 1;
        sprintf(val_step, "1");
        XmTextFieldSetString(sim_step, val_step);

    }
#ifdef DEBUG
    fprintf(stderr, " ... GreatSPN: val_step='%d'\n", int_val_step);
#endif
    start_simula();
}

void set_no_shots() {
    char *pp;

    pp = XmTextFieldGetString(sim_tokenmoves);
    sprintf(numtk, pp);
    XtFree(pp);
    if ((number_of_shots = atoi(numtk)) < 0) {
        number_of_shots = 0;
        sprintf(numtk, "0");

        XmTextFieldSetString(sim_tokenmoves, numtk);

    }
}


void display_curtime(double tt) {
    Arg args[1];
    XmString xms;
    char            ss[50];

    sprintf(ss, "time=%1.6lg", tt);

#ifdef DEBUG
    fprintf(stderr, "GreatSPN display_curtime %s\n", ss);
#endif
    xms = XmStringCreateSimple(ss);
    XtSetArg(args[0], XmNlabelString, xms);
    XtSetValues(sim_time_label, args, 1);
    XmStringFree(xms);
    XSync(mainDS.display, FALSE);
}

static void FireCB(Widget w, XtPointer closure, XtPointer call_data) {
    fire_afire_proc();
}

static void AutoCB(Widget w, XtPointer closure, XtPointer call_data) {
    fire_start_proc();
}

static void StopCB(Widget w, XtPointer closure, XtPointer call_data) {
    fire_stop_proc_alt();
}

static void SetTimeCB(Widget w, XtPointer closure, XtPointer call_data) {
    fire_reset_proc();
}

static void BreakPointCB(Widget w, XtPointer closure, XtPointer call_data) {
}

static void ScaleValueChangedCB(Widget w, XtPointer closure, XmScaleCallbackStruct *call_data) {
}


static void ScaleDragCB(Widget w, XtPointer closure, XmScaleCallbackStruct *call_data) {
}



void RegisterSimulationDialog(void) {
    static MrmRegisterArg	regvec[] = {
        {"FORWARD"	, (XtPointer)FORW},
        {"BACKWARD"	, (XtPointer)BACK},
        {"TIMED"	, (XtPointer)TIMED},
        {"UNTIMED"	, (XtPointer)UNTIMED},
        {"ValProcCB"	, (XtPointer)ValProcCB},
        {"SetModeCB"	, (XtPointer)SetModeCB},
        {"FireCloseProcAltCB"	, (XtPointer)FireCloseProcAltCB},
        {"SetFireProcCB"	, (XtPointer)SetFireProcCB},
        {"OpenFProcCB"	, (XtPointer)OpenFProcCB},
        {"StepCB", (XtPointer)ValProcCB},
        {"StopCB", (XtPointer)StopCB},
        {"FireCB", (XtPointer)FireCB},
        {"AutoCB", (XtPointer)AutoCB},
        {"SetTimeCB", (XtPointer)SetTimeCB},
        {"BreakPointCB", (XtPointer)BreakPointCB},
        {"ScaleChangedCB", (XtPointer)ScaleValueChangedCB},
        {"ScaleDraggingCB", (XtPointer)ScaleDragCB}

    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterSimulationDialog names\n");
}

void InitSimulationDialog(void) {
    sim_dlg = XtNameToWidget(frame_w, "*SimulationDialog");
    sim_done = XtNameToWidget(sim_dlg, "*SimDialogDone");
    sim_break = XtNameToWidget(sim_dlg, "*SimDialogBreakpoint");
    sim_step = XtNameToWidget(sim_dlg, "*SimDialogStep");
    sim_stop = XtNameToWidget(sim_dlg, "*SimDialogStop");
    sim_Time = XtNameToWidget(sim_dlg, "*SimDialogTime");
    sim_time_label = XtNameToWidget(sim_dlg, "*SimDialogLabel2");
    sim_tokenmoves = XtNameToWidget(sim_dlg, "*SimDialogTokenMoves");
    sim_rc3 = XtNameToWidget(sim_dlg, "*SimDialogRC3");
    sim_forward = XtNameToWidget(sim_dlg, "*simBTN2");
    sim_backward = XtNameToWidget(sim_dlg, "*simBTN3");
    sim_untimed = XtNameToWidget(sim_dlg, "*simBTN0");
    sim_interactive = XtNameToWidget(sim_dlg, "*simBTN1");
    sim_step_button = XtNameToWidget(sim_dlg, "*simBTN4");
    sim_auto_button = XtNameToWidget(sim_dlg, "*simBTN6");
    sim_settime_button = XtNameToWidget(sim_dlg, "*simBTN7");
    sim_fire_button = XtNameToWidget(sim_dlg, "*simBTN5");
    sim_slider = XtNameToWidget(sim_dlg, "*SimScale");



}
void init_break() {
}

void open_break_proc() {
}

