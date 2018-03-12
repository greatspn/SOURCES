/*
* menu.c
*/

/*
#define DEBUG
 */


#include "global.h"

#include "draw.h"
#include	"menu.h"
#include	"menuinit.h"
#include	"menuS.h"
#include	"menuvalues.h"
#include	"buttons.h"
#include	"place.h"
#include	"trans.h"
#include	"move.h"
#include	"arc.h"
#include	"color.h"
#include	"box.h"
#include	"line.h"
#include	"tag.h"
#include	"mark.h"
#include	"trap.h"
#include	"cc.h"
#include	"deadl.h"
#include	"ecs.h"
#include	"implp.h"
#include 	"lisp.h"
#include 	"liveness.h"
#include 	"rate.h"
#include 	"res.h"
#include 	"grid.h"
#include 	"toolkit.h"
#include 	"tinv.h"
#include 	"pinv.h"
#include 	"sc.h"
#include "unbound.h"
#include "me.h"
#include "printarea.h"
#include "layer.h"
#include "Canvas.h"
#include "zoom.h"


int showison = 0;

int tmp_object;
int selection_in_progress;


#define TOKENS 0
/* #define PARAM 1 */



static void  reset_command();

void dehighlight_all() {
    DehighlightUnbound();
    DehighlightPinv();
    DehighlightDeadl();
    DehighlightTrap();
    DehighlightImplp();
    DehighlightTinv();
    DehighlightEcs();
    DehighlightSc();
    DehighlightCc();
    DehighlightMe();
    dehighlight_trans();
    dehighlight_search();
}




#define	READERS(l,m) { reader_left = l; reader_middle = m;}
#define	XFIVE(a,b,c,d)	switch (cur_orient) {				\
				case FORTYFIVE:      SetCursor( a ); break;	\
				case VERTICAL:       SetCursor( b ); break;	\
				case MINUSFORTYFIVE: SetCursor( c ); break;	\
				default:             SetCursor( d ); break;	\
									}

extern void popup_res();

void menu_action() {
    reset_canvas_reader();

    showison = 0;
    StatusPrintf("");
    switch (cur_command) {
    case PRINTAREA:
        reset_canvas_reader();
        READERS(InitPrintArea, null_proc);
        SetMouseHelp("DEFINE Top Left corner", NULL, "ACTIONS");
        SetCursor(CROSS_CURSOR);
        break;
    case ADD:
        setup_mark(PARAM);
        dehighlight_all();
        switch (cur_object) {
        case PLACE:
            if (rate_visible)
                ShowRate(FALSE);
            rate_visible = FALSE;
            READERS(NewPlace, null_proc);
            SetCursor(PLACE_CURSOR);
            SetMouseHelp("CREATE New PLACES", NULL, "ACTIONS");
            break;
        case IMTRANS:
            READERS(NewImtrans, RotateTrans);
            XFIVE(FIMTRANS_CURSOR, VIMTRANS_CURSOR, MIMTRANS_CURSOR, HIMTRANS_CURSOR);
            SetMouseHelp("CREATE IMMEDIATE Transitions", "ROTATE transitions", "ACTIONS");
            break;
        case DETRANS:
            READERS(NewTitrans, RotateTrans);
            SetMouseHelp("CREATE DETERMINISTIC Transitions", "ROTATE transitions", "ACTIONS");
            XFIVE(FDETRANS_CURSOR, VDETRANS_CURSOR, MDETRANS_CURSOR, HDETRANS_CURSOR);
            break;
        case EXTRANS:
            READERS(NewTitrans, RotateTrans);
            SetMouseHelp("CREATE EXPONENTIAL Transitions", "ROTATE transitions", "ACTIONS");
            XFIVE(FEXTRANS_CURSOR, VEXTRANS_CURSOR, MEXTRANS_CURSOR, HEXTRANS_CURSOR);
            break;
        case IO_ARC:
        case INH_ARC:
            if (rate_visible)
                ShowRate(FALSE);
            rate_visible = FALSE;
            READERS(NewLine, NewHinib);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("CREATE New I/O ARCS", "CREATE New INHIBITORS", "ACTIONS");
            break;
        case MARK:
            if (rate_visible)
                ShowRate(FALSE);
            rate_visible = FALSE;
            READERS(NewMpar, null_proc);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("CREATE New MARKINGS", NULL, "ACTIONS");
            break;
        case RATE:
            if (!rate_visible)
                ShowRate(TRUE);
            rate_visible = TRUE;
            READERS(NewRpar, null_proc);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("CREATE New RATE Parameters", NULL, "ACTIONS");
            break;
        case RESULT:
            if (rate_visible)
                ShowRate(FALSE);
            rate_visible = FALSE;
            READERS(NewRes, help_popup_menu);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("CREATE RESULT Definitions", NULL, "ACTIONS");
            break;
        case COLOR:
            if (rate_visible)
                ShowRate(FALSE);
            rate_visible = FALSE;
            READERS(NewLisp, help_popup_menu);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("CREATE New COLOR/FUNCT Defs", NULL, "ACTIONS");
            break;
        default:
            SetCursor(POINTER_CURSOR);
            READERS(null_proc, null_proc);
            if (moving_images == NULL) {
                /*
                put_msg(0, "ERROR: no object selected !");
                break;*/
                SetCursor(WAIT_CURSOR);
                SetMouseHelp(NULL, NULL, "ACTIONS");
                break;
            }
            if (IsLayerVisible(WHOLE_NET_LAYER)) {
                left = clonate;
                middle = null_proc;
                move = drag;
                set_canvas_tracker();
                SetMouseHelp("CLONATE Selected Objs", NULL, "ACTIONS");

            }
            else {
                AddSelectedLayer();
                put_msg(0, "Selected object ADDED to current layers");
                redisplay_canvas();
            }
            break;
        }
        break;
    case CHANGE:
        setup_mark(PARAM);
        dehighlight_all();
        switch (cur_object) {
        case IMTRANS:
        case DETRANS:
        case EXTRANS:
            READERS(ChangeTransKind, null_proc);
            SetMouseHelp("CHANGE Transitions KIND", NULL, "ACTIONS");
            SetCursor(POINTER_CURSOR);
            break;
        case IO_ARC:
        case INH_ARC:
            READERS(ChangeArcType,/* change_line*/ null_proc);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("CHANGE Arcs PROPERTIES", NULL , "ACTIONS");
            SetCursor(POINTER_CURSOR);
            break;
        case PLACE:
            READERS(EditPlace, null_proc);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("CHANGE Place PROPERTIES", NULL, "ACTIONS");
            break;
        case MARK:
            READERS(EditMarking, null_proc);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("CHANGE Place or Params MARKING", NULL, "ACTIONS");
            break;
        case RATE:
            if (!rate_visible)
                ShowRate(TRUE);
            rate_visible = TRUE;
            READERS(EditRate, help_popup_menu);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("CHANGE RATES", NULL, "ACTIONS");
            break;
        case RESULT:
            READERS(EditRes, help_popup_menu);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("CHANGE result DEFINITIONS", NULL, "ACTIONS");
            break;
        case NAME:
            if (!tag_visible)
                ShowTag(OR);
            tag_visible = TRUE;
            READERS(EditTag, null_proc);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("CHANGE object NAMES", NULL, "ACTIONS");
            break;
        case COLOR:
            if (!tag_visible)
                ShowTag(OR);
            tag_visible = TRUE;
            READERS(EditColor, help_popup_menu);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("CHANGE COLOR/FUNCT Definitions", NULL, "ACTIONS");
            break;
        default:
            READERS(null_proc, null_proc);
            clear_message();
            SetCursor(WAIT_CURSOR);
            SetMouseHelp(NULL, NULL, "ACTIONS");
            break;
        }
        break;
    case SELECT:
        setup_mark(PARAM);
        dehighlight_all();
        if (rate_visible)
            ShowRate(FALSE);
        rate_visible = FALSE;
        READERS(select_objects, InitBoxTracking);
        SetMouseHelp("PICK Objects", "DEFINE Corners Of Selection Box", "ACTIONS");

        /*
        * put_msg(0,"PICK objects with LEFT button"); next_msg(0,"DEFINE
        * corners of selection box with MIDDLE button");
        */
        SetCursor(POINTER_CURSOR);

#ifdef DEBUG
        fprintf(stderr,
                "menu_action: cur_object=NULL_CHOICE from cur_command=%d, cur_object=%d\n",
                cur_command, cur_object);
#endif
        tmp_object = cur_object;
        cur_object = NULL_CHOICE;
        break;
    case MOVE:
        setup_mark(PARAM);
        dehighlight_all();
        switch (cur_object) {
        case COLOR:
            if (!tag_visible)
                ShowTag(OR);
            tag_visible = TRUE;
            READERS(MoveColor, null_proc);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("DRAG Color Labels", NULL, "ACTIONS");
            break;
        case NAME:
            if (!tag_visible)
                ShowTag(OR);
            tag_visible = TRUE;
            READERS(MoveTag, null_proc);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("DRAG Object Names", NULL, "ACTIONS");
            break;
        case RATE:
            if (!rate_visible)
                ShowRate(TRUE);
            rate_visible = TRUE;
            READERS(MoveRate, null_proc);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("DRAG Transition Rates", NULL, "ACTIONS");
            break;
        case IO_ARC:
        case INH_ARC:
            if (rate_visible)
                ShowRate(FALSE);
            rate_visible = FALSE;
            READERS(StartLineTracking, StartMoveLine);
            SetMouseHelp("CHANGE arcs Intermediate POINTS ", "Change PLACE/TRANSITION arcs", "ACTIONS");
            SetCursor(POINTER_CURSOR);
            break;
        case PLACE:
            if (rate_visible)
                ShowRate(FALSE);
            rate_visible = FALSE;
            READERS(MovePlace, LineUp);
            SetMouseHelp("DRAG places", "PICK places for hor/vert LINE-UP",  "ACTIONS");
            SetCursor(CROSS_CURSOR);
            break;
        case IMTRANS:
        case DETRANS:
        case EXTRANS:
            if (rate_visible)
                ShowRate(FALSE);
            rate_visible = FALSE;
            READERS(MoveTrans, LineUp);
            SetMouseHelp("DRAG transitions", "PICK trans. for hor/vert LINE-UP", "ACTIONS");
            SetCursor(CROSS_CURSOR);
            break;
        case MARK:
            READERS(MoveMPar, LineUp);
            SetMouseHelp("DRAG markings", "PICK objs for hor/vert line-up", "ACTIONS");
            SetCursor(POINTER_CURSOR);
            break;
        case RESULT:
            if (rate_visible)
                ShowRate(FALSE);
            rate_visible = FALSE;
            READERS(MoveRes, LineUp);
            SetMouseHelp("DRAG result definitions", "PICK objs for hor/vert line-up", "ACTIONS");
            SetCursor(POINTER_CURSOR);
            break;
        default:
            if (rate_visible)
                ShowRate(FALSE);
            rate_visible = FALSE;
            READERS(null_proc, null_proc);
            if (moving_images == NULL) {
                /*
                put_msg(0, "ERROR: no object selected !");
                break;*/
                SetCursor(WAIT_CURSOR);
                SetMouseHelp(NULL, NULL, "ACTIONS");

                break;

            }
            left = drop;
            middle = null_proc;
            move = drag;
            set_canvas_tracker();
            SetMouseHelp("DROP selected objects", NULL, "ACTIONS");
            SetCursor(CROSS_CURSOR);
            break;
        }
        RedisplayNet();
        break;
    case MIRROR:
        puts("MIrror");
        setup_mark(PARAM);
        dehighlight_all();
        if (rate_visible)
            ShowRate(FALSE);
        rate_visible = FALSE;
        READERS(null_proc, null_proc);
        if (moving_images == NULL) {
            ShowErrorDialog("ERROR: no object selected !", frame_w);
            break;
        }
        reader_left = mirror_vertical;
        reader_middle = mirror_horizontal;
        SetMouseHelp("DROP VERT mirror image of selection", "DROP HORIZ mirror image of selection", "ACTIONS");
        SetCursor(CROSS_CURSOR);
        break;
    case DELETE:
        setup_mark(PARAM);
        dehighlight_all();
        switch (cur_object) {
        case PLACE:
            if (rate_visible)
                ShowRate(FALSE);
            rate_visible = FALSE;
            READERS(DeletePlace, null_proc);
            SetCursor(POINTER_CURSOR);
            if (IsLayerVisible(WHOLE_NET_LAYER))
                SetMouseHelp("DELETE places", NULL, "ACTIONS");
            else
                SetMouseHelp("REMOVE places from current layers", NULL, "ACTIONS");
            break;
        case IMTRANS:
        case DETRANS:
        case EXTRANS:
            if (rate_visible)
                ShowRate(FALSE);
            rate_visible = FALSE;
            READERS(DeleteTrans, null_proc);
            SetCursor(POINTER_CURSOR);
            if (IsLayerVisible(WHOLE_NET_LAYER))
                SetMouseHelp("DELETE transitions", NULL, "ACTIONS");
            else
                SetMouseHelp("REMOVE transitions from current layers", NULL, "ACTIONS");
            break;
        case IO_ARC:
        case INH_ARC:
            if (rate_visible)
                ShowRate(FALSE);
            rate_visible = FALSE;
            READERS(DeleteArc, null_proc);
            if (IsLayerVisible(WHOLE_NET_LAYER))
                SetMouseHelp("DELETE arcs", NULL, "ACTIONS");
            /* put_msg(0,"Delete arcs with LEFT button"); */
            else
                SetMouseHelp("REMOVE arcs from current layers", NULL, "ACTIONS");
            SetCursor(POINTER_CURSOR);
            break;
        case MARK:
            READERS(DeleteMpar, null_proc);
            SetCursor(POINTER_CURSOR);
            if (IsLayerVisible(WHOLE_NET_LAYER))
                SetMouseHelp("DELETE marking parameters", NULL, "ACTIONS");
            else
                SetMouseHelp("REMOVE m. params from current layers", NULL, "ACTIONS");
            break;
        case RATE:
            READERS(DeleteRpar, null_proc);
            SetCursor(POINTER_CURSOR);
            if (IsLayerVisible(WHOLE_NET_LAYER))
                SetMouseHelp("DELETE rate parameters", NULL, "ACTIONS");

            else
                SetMouseHelp("REMOVE r. params from current layers", NULL, "ACTIONS");

            break;
        case RESULT:
            READERS(DeleteResult, null_proc);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("DELETE performance results", NULL, "ACTIONS");

            break;
        case COLOR:
            READERS(DeleteLisp, null_proc);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("DELETE color/function definitions", NULL, "ACTIONS");
            break;
        default:
            clear_message();
            if (moving_images != NULL)
                delete_selected();
            else {
                SetCursor(WAIT_CURSOR);
                SetMouseHelp(NULL, NULL, "ACTIONS");

                break;
            }

            SetCursor(POINTER_CURSOR);
            READERS(null_proc, null_proc);
            redisplay_canvas();
            break;
        }
        break;
    case FIRE_TRANS:
        dehighlight_all();
        if (!tag_visible)
            ShowTag(OR);
        tag_visible = TRUE;
        setup_mark(TOKENS);
        if (rate_visible)
            ShowRate(FALSE);
        rate_visible = FALSE;
        set_enabled();
        READERS(fire_trans, decr_token);
        SetCursor(POINTER_CURSOR);
        if (cur_object != IMTRANS && cur_object != DETRANS &&
                cur_object != EXTRANS && cur_object != RESULT) {

#ifdef DEBUG
            fprintf(stderr,
                    "menu_action: cur_object=NULL_CHOICE from cur_command=%d, cur_object=%d\n",
                    cur_command, cur_object);
#endif

            cur_object = NULL_CHOICE;
        }
        break;
    case SHOW:
        showison = 1;
        setup_mark(PARAM);
        dehighlight_all();
        printf("SHOW cur_object = %d  flag_pinv = %d   flag_tinv = %d \n", cur_object, flag_pinv, flag_tinv);
        switch (cur_object) {
        case PLACE:
            if (rate_visible)
                ShowRate(FALSE);
            rate_visible = FALSE;
            if (!tag_visible)
                ShowTag(OR);
            tag_visible = TRUE;
            SetCursor(POINTER_CURSOR);
            switch (flag_pinv) {
            case PINV_MITEM:
                reader_left = ShowPinv;
                reader_middle = null_proc;
                SetMouseHelp("SEE P-invariants", NULL, "ACTIONS");
                break;
            case MINDEAD_MITEM:
                reader_left = ShowDeadl;
                reader_middle = null_proc;
                SetMouseHelp("SEE Deadlocks", NULL, "ACTIONS");
                break;
            case MINTRAPS_MITEM:
                reader_left = ShowTrap;
                reader_middle = null_proc;
                SetMouseHelp("SEE Traps", NULL, "ACTIONS");
                break;
            case IMPPLACES_MITEM:
                reader_left = ShowImplp;
                reader_middle = null_proc;
                SetMouseHelp("CHECK implicit places", NULL, "ACTIONS");
                break;
            case ABSMB_MITEM:
                PrepareLPMark();
                reader_left = show_lp_uamb;
                reader_middle = show_lp_lamb;
                SetMouseHelp("SHOW Absolute Max marking", "SHOW Absolute Min marking", "ACTIONS");
                break;
            case AVEMB_MITEM:
                PrepareLPBounds();
                reader_left = show_lp_ummb;
                reader_middle = show_lp_lmmb;
                SetMouseHelp("SHOW Max Average markings", "SHOW Min Average markings", "ACTIONS");
                break;
            }
            next_msg(0, "SELECT other structural properties with RIGHT button");
            break;
        case IMTRANS:
        case DETRANS:
        case EXTRANS:
            if (!tag_visible)
                ShowTag(OR);
            tag_visible = TRUE;
            SetCursor(POINTER_CURSOR);
            switch (flag_tinv) {
            case TINV_MITEM:
                reader_left = ShowTinv;
                reader_middle = null_proc;
                SetMouseHelp("SEE T-invariants", NULL, "ACTIONS");
                break;
            case ECS_MITEM:
                if (cur_object == IMTRANS) {
                    reader_left = ShowEcs;
                    reader_middle = null_proc;
                    SetMouseHelp("SEE Ext. Conflict Sets", NULL, "ACTIONS");
                }
                else {
                    ShowInfoDialog("Sorry, NO Ext. Conflict Sets is computed for timed transitions", frame_w);
                    reader_left = null_proc;
                    reader_middle = null_proc;
                }
                break;
            case ME_MITEM:
                reader_left = ShowMe;
                reader_middle = null_proc;
                SetMouseHelp("SEE Mutual Exclusion", NULL, "ACTIONS");
                break;
            case SC_MITEM:
                reader_left = ShowSc;
                reader_middle = null_proc;
                SetMouseHelp("SEE Structural Conflicts", NULL, "ACTIONS");
                break;
            case CC_MITEM:
                reader_left = ShowCc;
                reader_middle = null_proc;
                SetMouseHelp("SEE Causal Connections", NULL, "ACTIONS");
                break;
            case UNBSEQ_MITEM:
                reader_left = ShowUnbound;
                reader_middle = null_proc;
                SetMouseHelp("Pick unbounded places", NULL, "ACTIONS");
                break;
            case ACTLIVBOUND_MITEM:
                EraseLive();
                ShowLive();
                break;
            case LIVBOUND_MITEM:
                prepare_lp_mark();
                reader_left = show_lp_ueb;
                reader_middle = null_proc;
                SetMouseHelp("Show LP enabling bounds", NULL, "ACTIONS");
                break;
            case THRBOUND_MITEM:
                prepare_lp_bounds();
                reader_left = show_lp_uxb;
                reader_middle = show_lp_lxb;
                SetMouseHelp("Show LP Upper throughput bounds", "Show LP Lower throughput bounds", "ACTIONS");
                break;
            }
            if (flag_tinv != ACTLIVBOUND_MITEM)
                next_msg(0, "SELECT other structural properties with RIGHT button");
            break;
        case RATE:
            if (!rate_visible)
                ShowRate(TRUE);
            rate_visible = TRUE;
            READERS(TypeRate, help_popup_menu);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("SEE trans. m-dep rates", NULL, "ACTIONS");
            break;
        case RESULT:
            if (rate_visible)
                ShowRate(FALSE);
            rate_visible = FALSE;
            ShowThroughputs();
            READERS(TypeRes, help_popup_menu);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("SEE definition of results", "SEE token prob. distr.", "ACTIONS");
            break;
        case COLOR:
            READERS(TypeLisp, help_popup_menu);
            SetCursor(POINTER_CURSOR);
            SetMouseHelp("SEE color/function def.", NULL, "ACTIONS");
            break;
        default:
            clear_message();
            READERS(null_proc, null_proc);
            SetCursor(WAIT_CURSOR);
            SetMouseHelp(NULL, NULL, "ACTIONS");

            break;
        }
        break;
    case SIMULATE:
        dehighlight_all();
        if (!tag_visible)
            ShowTag(OR);
        tag_visible = TRUE;
        setup_mark(TOKENS);
        if (rate_visible)
            ShowRate(FALSE);
        rate_visible = FALSE;
        READERS(null_proc, null_proc);
        SetCursor(POINTER_CURSOR);
        if (cur_object != DETRANS && cur_object != EXTRANS
                && cur_object != RESULT) {

#ifdef DEBUG
            fprintf(stderr,
                    "menu_action: cur_object=IMTRANS from cur_command=%d, cur_object=%d\n",
                    cur_command, cur_object);
#endif

            cur_object = IMTRANS;
        }
        if (cur_object == RESULT) {
            reader_left = popup_res;
        }
        break;
    case ROTATE:
        puts("ROTATE\n");
        switch (cur_object) {
        case IMTRANS:
            READERS(RotateTrans, null_proc);
            XFIVE(FIMTRANS_CURSOR, VIMTRANS_CURSOR, MIMTRANS_CURSOR, HIMTRANS_CURSOR);
            SetMouseHelp("ROTATE transitions", NULL, "ACTIONS");
            break;
        case DETRANS:
        case EXTRANS:
            READERS(RotateTrans, null_proc);
            if (cur_object == EXTRANS) {
                SetMouseHelp("ROTATE transitions", NULL, "ACTIONS");
                XFIVE(FEXTRANS_CURSOR, VEXTRANS_CURSOR, MEXTRANS_CURSOR, HEXTRANS_CURSOR);
            }
            else {
                SetMouseHelp("ROTATE transitions", NULL, "ACTIONS");
                XFIVE(FDETRANS_CURSOR, VDETRANS_CURSOR, MDETRANS_CURSOR, HDETRANS_CURSOR);
            }
            break;
        default:
            clear_message();
            READERS(null_proc, null_proc);
            SetCursor(WAIT_CURSOR);
            SetMouseHelp(NULL, NULL, "ACTIONS");


        }
        break;
    default:
        setup_mark(PARAM);
        dehighlight_all();
        clear_message();
        READERS(null_proc, null_proc);
        SetCursor(WAIT_CURSOR);
        SetMouseHelp(NULL, NULL, "ACTIONS");

        break;
    }
    reset_command();

    update_mode();
}

// static Widget   com_item;



void SetActionsCB(Widget          w,
                  int             client_data,
                  XtPointer call_data) {
    static int oldmode = NULL_CHOICE;
    static char *message[] = {
        "Add", "Change", "Select", "Move", "Delete", "Show", "Rotate", "Define Printarea", "Redraw", "", "Fire Trans"
    };


    switch (client_data) {
    case ADD_MITEM:
        cur_command = ADD;
        break;
    case CHANGE_MITEM:
        cur_command = CHANGE;
        break;
    case SELECT_MITEM:
        oldmode = cur_command;
        cur_command = SELECT;
        selection_in_progress = TRUE;
        break;
    case MOVE_MITEM:
        cur_command = MOVE;
        break;
    case DELETE_MITEM:
        cur_command = DELETE;
        break;
    case SHOW_MITEM:
        oldmode = cur_command;
        cur_command = SHOW;
        break;
    case ROTATE_MITEM:
        cur_command = ROTATE;
        break;
    case PRINTAREA_MITEM:
        cur_command = PRINTAREA;
        break;
    case ENDSHOW_MITEM:
        cur_command = oldmode;
    case REDRAW_MITEM:
        clear_canvas();
        redisplay_canvas();
        break;
    default:
        if (cur_command != SIMULATE && cur_command != FIRE_TRANS)
            cur_command = NULL_CHOICE;
        break;
    }
    if (cur_command == NULL_CHOICE)
        ActionDisplay("");
    else if (cur_command - ADD < 12)
        ActionDisplay(message[cur_command - ADD]);
    else
        ActionDisplay("");

    printf("MenuAction %d\n", cur_command);
    menu_action();
}

void ActionsCB(Widget          w,
               int             client_data,
               XmToggleButtonCallbackStruct *call_data) {

    if (!call_data->set)
        return;
    SetActionsCB(w, client_data, NULL);
}

void PinvActionsCB(Widget w, int closure, XmToggleButtonCallbackStruct *call_data) {
    if (!call_data->set)
        return;

    flag_pinv = closure;
    menu_action();
}

void TinvActionsCB(Widget w, int closure, XmToggleButtonCallbackStruct *call_data) {
    if (!call_data->set)
        return;

    flag_tinv = closure;
    menu_action();
}

void force_selection() {
    cur_command = ADD;
    left = clonate;
    middle = null_proc;
    move = drag;
    set_canvas_tracker();
    SetMouseHelp("CLONATE Selected Objs", NULL, "ACTIONS");
}

static void reset_command() {
    /*
     * switch (cur_command) { case ADD: P_SET(com_item, XV_SHOW, TRUE,
     * PANEL_VALUE, 0, 0); break; case CHANGE: P_SET(com_item, XV_SHOW, TRUE,
     * PANEL_VALUE, 1, 0); break; case SELECT: P_SET(com_item, XV_SHOW, TRUE,
     * PANEL_VALUE, 2, 0); break; case MOVE: P_SET(com_item, XV_SHOW, TRUE,
     * PANEL_VALUE, 3, 0); break; case DELETE: P_SET(com_item, XV_SHOW, TRUE,
     * PANEL_VALUE, 4, 0); break; case SHOW: P_SET(com_item, XV_SHOW, TRUE,
     * PANEL_VALUE, 5, 0); break; case FIRE_TRANS: case SIMULATE:
     * P_SET(com_item, XV_SHOW, TRUE, PANEL_VALUE, 6, 0); break; default:
     * P_SET(com_item, XV_SHOW, TRUE, PANEL_VALUE, 6, 0); cur_command =
     * NULL_CHOICE; break; }
     */
}

// static Widget   obj_item;

/*
extern  InitPrintArea();

PrintAreaCB(w, client_data, call_data)
Widget          w;
int             client_data;
XmToggleButtonCallbackStruct *call_data;
{
	puts("PrintAreaCB\n");
	reset_canvas_reader();
	READERS(InitPrintArea, null_proc);
	SetMouseHelp("DEFINE Top Left corner", NULL, "ACTIONS");
	SetCursor(CROSS_CURSOR);
    update_mode();
}
*/

void ButtonsCB(Widget          w,
               int             client_data,
               XmToggleButtonCallbackStruct *call_data) {
    if (inib_flag || receiving_msg)
        return;
    if (selection_on || action_on || !call_data->set) {
        reset_object();
        next_msg(2, cant_interrupt);
        /*
         * if (bell_global_on) window_bell(menu_sw);
         */
        return;
    }

#ifdef DEBUG
    fprintf(stderr,
            "set_object: from cur_command=%d, cur_object=%d\n", cur_command, cur_object);
#endif
    if (!call_data->set)
        return;
    switch (client_data) {
    case PLACE_BUTTON:
        cur_object = PLACE;
        break;
    case IMTRANS_BUTTON:
        cur_object = IMTRANS;
        break;
    case EXTRANS_BUTTON:
        cur_object = EXTRANS;
        break;
    case DETRANS_BUTTON:
        cur_object = DETRANS;
        break;
    case ARC_BUTTON:
        cur_object = IO_ARC;
        break;
    case TOKEN_BUTTON:
        cur_object = MARK;
        break;
    case RATES_BUTTON:
        cur_object = RATE;
        break;
    case RESULTS_BUTTON:
        cur_object = RESULT;
        break;
    case TAG_BUTTON:
        cur_object = NAME;
        break;
    case COLOR_BUTTON:
        cur_object = COLOR;
        break;
    default:
        cur_object = NULL_CHOICE;
        break;
    }
    tmp_object = cur_object;
    UpdatePopupStatusTo(client_data);

#ifdef DEBUG
    fprintf(stderr,
            "set_object: to cur_command=%d, cur_object=%d\n", cur_command, cur_object);
#endif

    menu_action();
    if (cur_command == SIMULATE)
        redisplay_canvas();
}


void reset_object() {
    cur_object = tmp_object;
    /*
     * switch (cur_object) { case PLACE: P_SET(obj_item, XV_SHOW, TRUE,
     * PANEL_VALUE, 0, 0); break; case IMTRANS: P_SET(obj_item, XV_SHOW, TRUE,
     * PANEL_VALUE, 1, 0); break; case EXTRANS: P_SET(obj_item, XV_SHOW, TRUE,
     * PANEL_VALUE, 2, 0); break; case DETRANS: P_SET(obj_item, XV_SHOW, TRUE,
     * PANEL_VALUE, 3, 0); break; case IO_ARC: P_SET(obj_item, XV_SHOW, TRUE,
     * PANEL_VALUE, 4, 0); break; case MARK: P_SET(obj_item, XV_SHOW, TRUE,
     * PANEL_VALUE, 5, 0); break; case RATE: P_SET(obj_item, XV_SHOW, TRUE,
     * PANEL_VALUE, 6, 0); break; case RESULT: P_SET(obj_item, XV_SHOW, TRUE,
     * PANEL_VALUE, 7, 0); break; case NAME: P_SET(obj_item, XV_SHOW, TRUE,
     * PANEL_VALUE, 8, 0); break; case COLOR: P_SET(obj_item, XV_SHOW, TRUE,
     * PANEL_VALUE, 9, 0); break; default: P_SET(obj_item, XV_SHOW, TRUE,
     * PANEL_VALUE, 10, 0); cur_object = NULL_CHOICE; break; }
     */
}

void init_command_menu() {
    InitCursors();
    cur_object = PLACE;
    cur_command = ADD;

    menu_action();
    UpdatePopupStatusTo(PLACE_BUTTON);
}

void setup_initialstate() {

#ifdef DEBUG
    fprintf(stderr,
            "setup_initialstate: cur_object=PLACE from cur_command=%d, cur_object=%d\n",
            cur_command, cur_object);
#endif

    cur_object = PLACE;
    cur_command = ADD;
    flag_tinv = TINV_MITEM;
    flag_pinv = PINV_MITEM;
    /*	XmToggleButtonGadgetSetState(ToolButtons[0],TRUE,TRUE); */
#ifdef DEBUG
    fprintf(stderr, "   setup_initialstate: calling menu_action\n");
#endif
    menu_action();
    spline_on = FALSE;
    tag_visible = FALSE;
    rate_visible = FALSE;
    SetPrintAreaVisible(FALSE);
#ifdef DEBUG
    fprintf(stderr, "   setup_initialstate: calling update_model\n");
#endif
    update_mode();
#ifdef DEBUG
    fprintf(stderr, "   setup_initialstate: calling ResetGrill\n");
#endif
    ResetGrill();
#ifdef DEBUG
    fprintf(stderr, "   setup_initialstate: calling ResetZoom\n");
#endif
    ResetZoom();
#ifdef DEBUG
    fprintf(stderr, "   setup_initialstate: calling reset_canvas_scroll\n");
#endif
    reset_canvas_scroll();
#ifdef DEBUG
    fprintf(stderr, "   setup_initialstate: calling UpdatePopupStatusTo\n");
#endif
    UpdatePopupStatusTo(PLACE_BUTTON);
#ifdef DEBUG
    fprintf(stderr, "   setup_initialstate: calling UpdateMenuStatusTo\n");
#endif
    UpdateMenuStatusTo(NORMAL_MSTATUS);
#ifdef DEBUG
    fprintf(stderr, "   setup_initialstate: calling SetPopupActionTo\n");
#endif
    SetPopupActionTo(0);
}


/* ?????????????????????????????????? */

void init_setup_initialstate() {

#ifdef DEBUG
    fprintf(stderr,
            "init_setup_initialstate: cur_object=PLACE from cur_command=%d, cur_object=%d\n",
            cur_command, cur_object);
#endif

    cur_object = PLACE;
    cur_command = ADD;
    flag_tinv = TINV_MITEM;
    flag_pinv = PINV_MITEM;
    /*	XmToggleButtonGadgetSetState(ToolButtons[0],TRUE,TRUE); */
#ifdef DEBUG
    fprintf(stderr, "   init_setup_initialstate: calling menu_action\n");
#endif
    menu_action();
    spline_on = FALSE;
    tag_visible = FALSE;
    rate_visible = FALSE;
    SetPrintAreaVisible(FALSE);
#ifdef DEBUG
    fprintf(stderr, "   init_setup_initialstate: calling update_model\n");
#endif
    update_mode();
#ifdef DEBUG
    fprintf(stderr, "   init_setup_initialstate: calling ResetGrill\n");
#endif
    ResetGrill();
#ifdef DEBUG
    fprintf(stderr, "   init_setup_initialstate: calling reset_canvas_scroll\n");
#endif
    reset_canvas_scroll();
#ifdef DEBUG
    fprintf(stderr, "   init_setup_initialstate: calling UpdatePopupStatusTo\n");
#endif
    UpdatePopupStatusTo(PLACE_BUTTON);
#ifdef DEBUG
    fprintf(stderr, "   init_setup_initialstate: calling UpdateMenuStatusTo\n");
#endif
    UpdateMenuStatusTo(NORMAL_MSTATUS);
#ifdef DEBUG
    fprintf(stderr, "   init_setup_initialstate: calling SetPopupActionTo\n");
#endif
    SetPopupActionTo(0);
}
