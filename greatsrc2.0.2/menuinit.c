#define ___MENUINIT___
#include "global.h"
#include "menuH.h"
#include "menuS.h"
#include "menuvalues.h"
#include "menuinit.h"
#include "toolkit.h"

/*#define DEBUG*/
static Widget  GridMenuItem;
static Widget  ZoomMenuItem;
static Widget  PinvMenuItem;
static Widget  TinvMenuItem;
static Widget MHandles[HANDLESNUM];
static Widget PHandles[POPUPHANDLESNUM];
static char MenuStatus[][HANDLESNUM] = {
    /*
    *
    *      +- File New				   +- Edit Undo
    *      | +- File Open			   | +- Edit Add
    *      | | +- File Save        	| | +- Edit Delete
    *      | | | +-File Save As		| | | +- Edit Cancel
    *      | | | | +-File Comment	   | | | | +- Edit Move
    *      | | | | |   +- File Print	| | | | | +- Edit Modify
    *      | | | | |   | + FileExit	| | | | | | +- Edit Layers
    *      | | | | |   | |			   | | | | | |	|								 */
    {
        T, T, T, F, F, F, F , F, F, F, T,			   T, F, F, F, F, F, F,	             F, F, F, T, F, F, F,
        F, F, F, F, F, F, F,		 F, F, F, F, F, 	F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
        F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, T, T, T
    },
    {
        T, T, T, T, T,  T, T, T, T, T, T,			   T, F, F, F, F, F, T,				    T, T, T, T, T, T, T,
        T, T, T, T, T, T, T,		 T, T, T, T, T,  T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,
        T, T, T, T, T, T, T, T, T, T, T, T, T, T,  T, T, T, T
    },

    {
        F, F, F, F, F, F, F, F, F, F, T,			   F, T, T, T, T, T, F,				    F, F, F, T, F, F, F,
        F, F, F, F, F, F, F,	    F, F, F, F, F,	 F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
        F, F, F, F, F, F, F, F, F, F, F, F, F, F,  F, F, T, T
    },
    {
        F, F, F, F, F, F, F, F, F, F, F,			   F, F, F, F, F, F, F,				    F, F, F, F, F, F, F,
        F, F, F, F, F, F, F,	    F, F, F, F, F,	 F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
        F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F
    },
    {
        F, F, F, F, F, F, F, F, F, F, T,			   F, F, F, F, F, F, F,				    T, T, T, T, T, T, F,
        F, F, F, F, F, F, F,       F, F, F, F, F,	 F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
        F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F
    }
};



static char PopupStatus[][POPUPHANDLESNUM] = {
    /*
    *
    *
    *    +- Add
    *    | +- Change
    *    | | +- Select
    *    | | | +- Move
    *    | | | | +- Delete
    *    | | | | | +- Show
    *    | | | | | | + Cancel
    *    | | | | | | | + Rotate
    *    | | | | | | | | + Define Print Area
    *    | | | | | | | | | + Redraw
    *    | | | | | | | | | |			*/
    {T, T, T, T, T, T, F, F, T, T},
    {T, T, T, T, T, T, F, T, T, T},
    {T, T, T, T, T, T, F, T, T, T},
    {T, T, T, T, T, T, F, T, T, T},
    {T, T, T, T, T, F, F, F, T, T},
    {T, T, T, T, T, F, F, F, T, T},
    {T, T, T, T, T, T, F, F, T, T},
    {T, T, T, T, T, T, F, F, T, T},
    {F, T, T, T, F, F, F, F, T, T},
    {T, T, T, T, T, T, F, F, T, T},
    {F, F, F, F, F, F, F, F, T, T},
    {T, F, F, T, T, F, T, F, F, T}
};

Widget MENUITEM(int p) {
    return MHandles[p];
}

Widget POPUPITEM(int p) {
    return PHandles[p];
}

void SetPopupActionTo(int action) {
    XmToggleButtonGadgetSetState(POPUPITEM(action), TRUE, TRUE);
}

void UpdateMenuStatusTo(int status) {
    register int i;

    for (i = 0; i < HANDLESNUM; i++) {

#ifdef DEBUG
        printf("\n UpdateMenuStatusTo   handle %p    %c", MENUITEM(i), MenuStatus[status][i] + '0');
#endif

        XtSetSensitive(MENUITEM(i), MenuStatus[status][i]);
    }
}

void UpdatePopupStatusTo(int status) {
    register int i;

    for (i = 0; i < POPUPHANDLESNUM; i++) {

#ifdef DEBUG
        printf("\n XtSetSensitive   handle %p    %c", POPUPITEM(i), PopupStatus[status][i] + '0');
#endif

        XtSetSensitive(POPUPITEM(i), PopupStatus[status][i]);
    }
}

void InitMenuBar(void) {

    menubar = XtNameToWidget(mainwin, "MenuBar");

    InitFileMenu();
    InitNetMenu();
    InitEditMenu();
    InitViewMenu();
    InitGridMenu();
    InitZoomMenu();
    InitRescaleMenu();
    InitSwnMenu();
    InitHelpMenu();
    UpdateMenuStatusTo(INITIAL_MSTATUS);

}

static void InitFileMenu(void) {

    MHandles[FILE_NEW_HANDLE]		 = XtNameToWidget(menubar, "*File_New");
    MHandles[FILE_OPEN_HANDLE] 		 = XtNameToWidget(menubar, "*File_Open");
    MHandles[FILE_MERGE_HANDLE] 		 = XtNameToWidget(menubar, "*File_Merge");

    MHandles[FILE_SAVE_HANDLE] 		 = XtNameToWidget(menubar, "*File_Save");
    MHandles[FILE_SAVEAS_HANDLE] 	 = XtNameToWidget(menubar, "*File_SaveAs");
    MHandles[FILE_REMOVE_RESULTS_HANDLE] 		 = XtNameToWidget(menubar, "*File_RemoveResults");
    MHandles[FILE_REMOVE_ALL_HANDLE] 		 = XtNameToWidget(menubar, "*File_RemoveAll");

    MHandles[FILE_COMMENT_HANDLE] 	 = XtNameToWidget(menubar, "*File_Comment");
    MHandles[FILE_OPTIONS_HANDLE] 	 = XtNameToWidget(menubar, "*File_Options");
    MHandles[FILE_PRINT_HANDLE] 		 = XtNameToWidget(menubar, "*File_Print");
    MHandles[FILE_EXIT_HANDLE]		 = XtNameToWidget(menubar, "*File_Exit");
}

void Reset_T_P_Popups(void) {
    /*
      Arg args[1];

    XtSetArg(args[0],XmNset,TRUE);
    XtSetValues(PinvMenuItem,args,1);
    XtSetValues(TinvMenuItem,args,1);
    */
}

void InitPinvPopupMenu(void) {
    PinvPopup = FetchWidget(drawwin, "PinvPopup");
    PinvMenuItem = XtNameToWidget(PinvPopup, "Pinv_Pop");
}

void InitTinvPopupMenu(void) {
    TinvPopup = FetchWidget(drawwin, "TinvPopup");
    TinvMenuItem = XtNameToWidget(TinvPopup, "Tinv_Pop");
}
void InitEndShowPopupMenu(void) {
    EndShowPopup = FetchWidget(drawwin, "EndShowPopup");
}

void InitPopupMenu(void) {
    popup = XtNameToWidget(drawwin, "*.Popup");
    PHandles[POPUP_ADD_HANDLE] 		= XtNameToWidget(drawwin, "*Add_Pop");
    PHandles[POPUP_CHANGE_HANDLE] 	= XtNameToWidget(drawwin, "*Change_Pop");
    PHandles[POPUP_SELECT_HANDLE] 	= XtNameToWidget(drawwin, "*Select_Pop");
    PHandles[POPUP_MOVE_HANDLE] 	= XtNameToWidget(drawwin, "*Move_Pop");
    PHandles[POPUP_DELETE_HANDLE] 	= XtNameToWidget(drawwin, "*Delete_Pop");
    PHandles[POPUP_SHOW_HANDLE] 	= XtNameToWidget(drawwin, "*Show_Pop");
    PHandles[POPUP_ROTATE_HANDLE] 	= XtNameToWidget(drawwin, "*Rotate_Pop");
    PHandles[POPUP_PRINTAREA_HANDLE] 	= XtNameToWidget(drawwin, "*Printarea_Pop");
    PHandles[POPUP_PASTE_HANDLE] 	= XtNameToWidget(drawwin, "*Paste_Pop");
    PHandles[POPUP_REDRAW_HANDLE] 	= XtNameToWidget(drawwin, "*Redraw_Pop");
}

void InitPopups(void) {
    InitEndShowPopupMenu();
    InitPinvPopupMenu();
    InitTinvPopupMenu();
    InitPopupMenu();
}

static void InitNetMenu(void) {

    MHandles[NET_STRUCT_PINV_HANDLE] 	= 	XtNameToWidget(menubar, "*Net_Struct_P_Invariants");
    MHandles[NET_STRUCT_TINV_HANDLE] 	= 	XtNameToWidget(menubar, "*Net_Struct_T_Invariants");
    MHandles[NET_STRUCT_MINDEAD_HANDLE] 	= 	XtNameToWidget(menubar, "*Net_Struct_Minimal_Deadlocks");
    MHandles[NET_STRUCT_MINTRAP_HANDLE] 	= 	XtNameToWidget(menubar, "*Net_Struct_Minimal_Traps");
    MHandles[NET_STRUCT_ECS_HANDLE] 		= 	XtNameToWidget(menubar, "*Net_Struct_ECS");
    MHandles[NET_STRUCT_STRUBOU_HANDLE] 	= 	XtNameToWidget(menubar, "*Net_Struct_Struct_Bound");

    MHandles[NET_SOLVE_TRG_HANDLE] 		= 	XtNameToWidget(menubar, "*Net_Solve_Compute_TRG");
    MHandles[NET_SOLVE_EMC_HANDLE] 		= 	XtNameToWidget(menubar, "*Net_Solve_Compute_EMC");
    MHandles[NET_SOLVE_GSPN_HANDLE] 		= 	XtNameToWidget(menubar, "*Net_Solve_GSPN_Solution");

    MHandles[NET_SOLVE_GSPN_STST_HANDLE] = 	XtNameToWidget(menubar, "*Net_Solve_Solution_Steady_State");
    MHandles[NET_SOLVE_GSPN_TRAN_HANDLE] = 	XtNameToWidget(menubar, "*Net_Solve_Solution_Transient");

    MHandles[NET_STRUCT_HANDLE] 			= 	XtNameToWidget(menubar, "*Net_Struct");
    MHandles[NET_SOLVE_HANDLE] 			= 	XtNameToWidget(menubar, "*Net_Solve");
    MHandles[NET_SIMULATION_HANDLE] 		= 	XtNameToWidget(menubar, "*Net_Simulation");
}



static void InitEditMenu(void) {

    MHandles[EDIT_UNDO_HANDLE] 	= 	XtNameToWidget(menubar, "*Edit_Undo");
    MHandles[EDIT_ADD_HANDLE] 	= 	XtNameToWidget(menubar, "*Edit_Add");
    MHandles[EDIT_CUT_HANDLE] 	= 	XtNameToWidget(menubar, "*Edit_Cut");
    MHandles[EDIT_PASTE_HANDLE] 	= 	XtNameToWidget(menubar, "*Edit_Paste");
    MHandles[EDIT_MOVE_HANDLE] 	= 	XtNameToWidget(menubar, "*Edit_Move");
    MHandles[EDIT_MODIFY_HANDLE]	=	 XtNameToWidget(menubar, "*Edit_Modify");
    MHandles[EDIT_LAYERS_HANDLE] = 	 XtNameToWidget(menubar, "*Edit_Layers");
}

static void InitViewMenu(void) {

    MHandles[VIEW_SPLINE_HANDLE] 	=  XtNameToWidget(menubar, "*View_Spline");
    MHandles[VIEW_TAG_HANDLE] 		=  XtNameToWidget(menubar, "*View_Tag");
    MHandles[VIEW_RATE_HANDLE] 		=  XtNameToWidget(menubar, "*View_Rate");
    MHandles[VIEW_OVERVIEW_HANDLE] 	=  XtNameToWidget(menubar, "*View_Overview");
    MHandles[VIEW_SEARCH_HANDLE] 	=  XtNameToWidget(menubar, "*View_Search");
    MHandles[VIEW_PRINTAREA_HANDLE] 	=  XtNameToWidget(menubar, "*View_PrintArea");
    MHandles[VIEW_LAYERS_HANDLE] 	=  XtNameToWidget(menubar, "*View_Layers");
}

void SetGridMenuItemValue(Boolean state) {
    XmToggleButtonGadgetSetState(GridMenuItem, state, TRUE);
}

static void InitGridMenu(void) {
    MHandles[GRID_NONE_HANDLE] = XtNameToWidget(menubar, "*Grid_None");
    MHandles[GRID_5_HANDLE] = XtNameToWidget(menubar, "*Grid_5");
    MHandles[GRID_10_HANDLE] = XtNameToWidget(menubar, "*Grid_10");
    MHandles[GRID_20_HANDLE] = XtNameToWidget(menubar, "*Grid_20");
    MHandles[GRID_30_HANDLE] = XtNameToWidget(menubar, "*Grid_30");
    MHandles[GRID_40_HANDLE] = XtNameToWidget(menubar, "*Grid_40");
    MHandles[GRID_50_HANDLE] = XtNameToWidget(menubar, "*Grid_50");

    GridMenuItem =	XtNameToWidget(menubar, "*Grid_None");
}

void SetZoomMenuItemValue(Boolean state) {
    XmToggleButtonGadgetSetState(ZoomMenuItem, state, TRUE);
}

static void InitZoomMenu(void) {
    MHandles[ZOOM_1_HANDLE] 	=  XtNameToWidget(menubar, "*Zoom_1");
    MHandles[ZOOM_3_2_HANDLE] 	=  XtNameToWidget(menubar, "*Zoom_3_2");
    MHandles[ZOOM_2_HANDLE] 	=  XtNameToWidget(menubar, "*Zoom_2");
    MHandles[ZOOM_1_2_HANDLE] =  XtNameToWidget(menubar, "*Zoom_1_2");
    MHandles[ZOOM_3_4_HANDLE] =  XtNameToWidget(menubar, "*Zoom_3_4");
    ZoomMenuItem		 	=  XtNameToWidget(menubar, "*Zoom_1");
}

static void InitRescaleMenu(void) {
    MHandles[RESCALE_05_HANDLE] =	XtNameToWidget(menubar, "*Rescale_05");
    MHandles[RESCALE_06_HANDLE] =	XtNameToWidget(menubar, "*Rescale_06");
    MHandles[RESCALE_07_HANDLE] =	XtNameToWidget(menubar, "*Rescale_07");
    MHandles[RESCALE_08_HANDLE] =	XtNameToWidget(menubar, "*Rescale_08");
    MHandles[RESCALE_09_HANDLE] =	XtNameToWidget(menubar, "*Rescale_09");
    MHandles[RESCALE_10_HANDLE] =	XtNameToWidget(menubar, "*Rescale_10");
    MHandles[RESCALE_11_HANDLE] =	XtNameToWidget(menubar, "*Rescale_11");
    MHandles[RESCALE_12_HANDLE] =	XtNameToWidget(menubar, "*Rescale_12");
    MHandles[RESCALE_13_HANDLE] =	XtNameToWidget(menubar, "*Rescale_13");
    MHandles[RESCALE_14_HANDLE] =	XtNameToWidget(menubar, "*Rescale_14");
    MHandles[RESCALE_15_HANDLE] =	XtNameToWidget(menubar, "*Rescale_15");
    MHandles[RESCALE_16_HANDLE] =	XtNameToWidget(menubar, "*Rescale_16");
    MHandles[RESCALE_17_HANDLE] =	XtNameToWidget(menubar, "*Rescale_17");
    MHandles[RESCALE_18_HANDLE] =	XtNameToWidget(menubar, "*Rescale_18");
    MHandles[RESCALE_19_HANDLE] =	XtNameToWidget(menubar, "*Rescale_19");
    MHandles[RESCALE_20_HANDLE] =	XtNameToWidget(menubar, "*Rescale_20");

}

static void InitSwnMenu(void) {
    MHandles[SWN_SYMBOLIC_HANDLE] 	=  XtNameToWidget(menubar, "*Swn_Symbolic");
    MHandles[SWN_ORDINARY_HANDLE] 			=  XtNameToWidget(menubar, "*Swn_Ordinary");
}

static void InitHelpMenu(void) {
    MHandles[HELP_MOUSE_HELP_HANDLE] 	=  XtNameToWidget(menubar, "*Help_Toggle");
    MHandles[HELP_ABOUT_HANDLE] 			=  XtNameToWidget(menubar, "*Help_About");
}

void RegisterMenuValues(void) {
    static MrmRegisterArg	regvec[] = {
        /*Menu File Values*/
        {"FILE_NEW_MITEM", (XtPointer)	FILE_NEW_MITEM},
        {"FILE_OPEN_MITEM", (XtPointer)	FILE_OPEN_MITEM},
        {"FILE_MERGE_MITEM", (XtPointer)	FILE_MERGE_MITEM},

        {"FILE_SAVE_MITEM", (XtPointer)	FILE_SAVE_MITEM},
        {"FILE_SAVEAS_MITEM", (XtPointer)	FILE_SAVEAS_MITEM},
        {"FILE_REMOVE_RESULTS_MITEM", (XtPointer)	FILE_REMOVE_RESULTS_MITEM},
        {"FILE_REMOVE_ALL_MITEM", (XtPointer)	FILE_REMOVE_ALL_MITEM},

        {"FILE_COMMENT_MITEM", (XtPointer)	FILE_COMMENT_MITEM},
        {"FILE_OPTIONS_MITEM", (XtPointer)	FILE_OPTIONS_MITEM},
        {"FILE_PRINT_MITEM", (XtPointer)	FILE_PRINT_MITEM},
        {"FILE_EXIT_MITEM", (XtPointer)	FILE_EXIT_MITEM},
        /*Menu Edit Values*/
        {"EDIT_UNDO_MITEM", (XtPointer)	EDIT_UNDO_MITEM},
        {"EDIT_ADD_MITEM", (XtPointer)	EDIT_ADD_MITEM},
        {"EDIT_CUT_MITEM", (XtPointer)	EDIT_CUT_MITEM},
        {"EDIT_PASTE_MITEM", (XtPointer)	EDIT_PASTE_MITEM},
        {"EDIT_MOVE_MITEM", (XtPointer)	EDIT_MOVE_MITEM},
        {"EDIT_MODIFY_MITEM", (XtPointer)	EDIT_MODIFY_MITEM},
        {"EDIT_LAYERS_MITEM", (XtPointer)	EDIT_LAYERS_MITEM},
        {"EDIT_MODIFY_CLOCKWISE_MITEM", (XtPointer)	EDIT_MODIFY_CLOCKWISE_MITEM},
        {"EDIT_MODIFY_FLIPX_MITEM", (XtPointer)	EDIT_MODIFY_FLIPX_MITEM},
        {"EDIT_MODIFY_FLIPY_MITEM", (XtPointer)	EDIT_MODIFY_FLIPY_MITEM},
        {"EDIT_MIRROR_MITEM", (XtPointer)	EDIT_MIRROR_MITEM},
        {"FLIP_XAXIS", (XtPointer)	FLIP_XAXIS},
        {"FLIP_YAXIS", (XtPointer)	FLIP_YAXIS},
        {"ROTATE45", (XtPointer)	ROTATE45},
        {"ROTATE90", (XtPointer)	ROTATE90},
        {"ROTATE135", (XtPointer)	ROTATE135},
        {"ROTATE180", (XtPointer)	ROTATE180},
        {"ROTATEM135", (XtPointer)	ROTATEM135},
        {"ROTATEM90", (XtPointer)	ROTATEM90},
        {"ROTATEM45", (XtPointer)	ROTATEM45},
        /*Menu Popup Values*/
        {"ADD_MITEM", (XtPointer)	ADD_MITEM},
        {"CHANGE_MITEM", (XtPointer)	CHANGE_MITEM},
        {"SELECT_MITEM", (XtPointer)	SELECT_MITEM},
        {"MOVE_MITEM", (XtPointer)	MOVE_MITEM},
        {"DELETE_MITEM", (XtPointer)	DELETE_MITEM},
        {"SHOW_MITEM", (XtPointer)	SHOW_MITEM},
        {"PASTE_MITEM", (XtPointer)	PASTE_MITEM},
        {"ROTATE_MITEM", (XtPointer)	ROTATE_MITEM},
        {"ENDSHOW_MITEM", (XtPointer)	ENDSHOW_MITEM},
        {"PRINTAREA_MITEM", (XtPointer)	PRINTAREA_MITEM},
        {"REDRAW_MITEM", (XtPointer) REDRAW_MITEM},
        /*Menu View Values*/
        {"VIEW_SPLINE_MITEM", (XtPointer)	VIEW_SPLINE_MITEM},
        {"VIEW_TAG_MITEM", (XtPointer)	VIEW_TAG_MITEM},
        {"VIEW_RATE_MITEM", (XtPointer)	VIEW_RATE_MITEM},
        {"VIEW_OVERVIEW_MITEM", (XtPointer)	VIEW_OVERVIEW_MITEM},
        {"VIEW_SEARCH_MITEM", (XtPointer)	VIEW_SEARCH_MITEM},
        {"VIEW_PRINTAREA_MITEM", (XtPointer)	VIEW_PRINTAREA_MITEM},
        {"VIEW_LAYERS_MITEM", (XtPointer)	VIEW_LAYERS_MITEM},
        /*Menu Grid Values*/
        {"GRID_NONE_MITEM", (XtPointer)	GRID_NONE_MITEM},
        {"GRID_5_MITEM", (XtPointer)	GRID_5_MITEM},
        {"GRID_10_MITEM", (XtPointer)	GRID_10_MITEM},
        {"GRID_20_MITEM", (XtPointer)	GRID_20_MITEM},
        {"GRID_30_MITEM", (XtPointer)	GRID_30_MITEM},
        {"GRID_40_MITEM", (XtPointer)	GRID_40_MITEM},
        {"GRID_50_MITEM", (XtPointer)	GRID_50_MITEM},
        /*Menu Zoom Values*/
        {"ZOOM_1_MITEM", (XtPointer)	ZOOM_1_MITEM},
        {"ZOOM_3_2_MITEM", (XtPointer)	ZOOM_3_2_MITEM},
        {"ZOOM_2_MITEM", (XtPointer)	ZOOM_2_MITEM},
        {"ZOOM_1_2_MITEM", (XtPointer)	ZOOM_1_2_MITEM},
        {"ZOOM_3_4_MITEM", (XtPointer)	ZOOM_3_4_MITEM},
        /*Menu Rescale Values*/
        {"RESCALE_05_MITEM", (XtPointer)	RESCALE_05_MITEM},
        {"RESCALE_06_MITEM", (XtPointer)	RESCALE_06_MITEM},
        {"RESCALE_07_MITEM", (XtPointer)	RESCALE_07_MITEM},
        {"RESCALE_08_MITEM", (XtPointer)	RESCALE_08_MITEM},
        {"RESCALE_09_MITEM", (XtPointer)	RESCALE_09_MITEM},
        {"RESCALE_10_MITEM", (XtPointer)	RESCALE_10_MITEM},
        {"RESCALE_11_MITEM", (XtPointer)	RESCALE_11_MITEM},
        {"RESCALE_12_MITEM", (XtPointer)	RESCALE_12_MITEM},
        {"RESCALE_13_MITEM", (XtPointer)	RESCALE_13_MITEM},
        {"RESCALE_14_MITEM", (XtPointer)	RESCALE_14_MITEM},
        {"RESCALE_15_MITEM", (XtPointer)	RESCALE_15_MITEM},
        {"RESCALE_16_MITEM", (XtPointer)	RESCALE_16_MITEM},
        {"RESCALE_17_MITEM", (XtPointer)	RESCALE_17_MITEM},
        {"RESCALE_18_MITEM", (XtPointer)	RESCALE_18_MITEM},
        {"RESCALE_19_MITEM", (XtPointer)	RESCALE_19_MITEM},
        {"RESCALE_20_MITEM", (XtPointer)	RESCALE_20_MITEM},
        /*Menu Net Values*/
        {"NET_STRUCT_PINV_MITEM", (XtPointer)	NET_STRUCT_PINV_MITEM},
        {"NET_STRUCT_TINV_MITEM", (XtPointer)	NET_STRUCT_TINV_MITEM},
        {"NET_STRUCT_MINDEAD_MITEM", (XtPointer)	NET_STRUCT_MINDEAD_MITEM},
        {"NET_STRUCT_MINTRAP_MITEM", (XtPointer)	NET_STRUCT_MINTRAP_MITEM},
        {"NET_STRUCT_ECS_MITEM", (XtPointer)	NET_STRUCT_ECS_MITEM},
        {"NET_STRUCT_STRUBOU_MITEM", (XtPointer)	NET_STRUCT_STRUBOU_MITEM},
        {"NET_SOLVE_TRG_MITEM", (XtPointer)	NET_SOLVE_TRG_MITEM},
        {"NET_SOLVE_EMC_MITEM", (XtPointer)	NET_SOLVE_EMC_MITEM},
        {"NET_SOLVE_GSPN_STST_MITEM", (XtPointer)	NET_SOLVE_GSPN_STST_MITEM},
        {"NET_SOLVE_GSPN_TRAN_MITEM", (XtPointer)	NET_SOLVE_GSPN_TRAN_MITEM},
        {"NET_SIMULATION_MITEM", (XtPointer)	NET_SIMULATION_MITEM},
        /****11/6/10 MDD by Marco***/
        {"GSPN_MDD", (XtPointer)	GSPN_MDD},
        /****11/6/10 MDD by Marco***/
        /*Menu SWN Values*/
        {"SWN_SYM_SIMULATION", (XtPointer)	SWN_SYM_SIMULATION},
        {"SWN_SYM_COMPUTERG", (XtPointer)	SWN_SYM_COMPUTERG},
        {"SWN_SYM_TRANSIENT", (XtPointer)	SWN_SYM_TRANSIENT},
        {"SWN_ORD_SIMULATION", (XtPointer)	SWN_ORD_SIMULATION},
        {"SWN_ORD_COMPUTERG", (XtPointer)	SWN_ORD_COMPUTERG},
        {"SWN_ORD_TRANSIENT", (XtPointer)	SWN_ORD_TRANSIENT},
        {"SWN_ORD_UNFOLD", (XtPointer)	SWN_ORD_UNFOLD},
        /****17/7/08 ESRG by Marco***/
        {"SWN_ESRG", (XtPointer)	SWN_ESRG},
        {"SWN_DSRG", (XtPointer)	SWN_DSRG},
        /****17/7/08 ESRG by Marco***/
        {"EGSPN_SIMULATION", (XtPointer)	EGSPN_SIMULATION},
        {"EGSPN_COMPUTERG", (XtPointer)	EGSPN_COMPUTERG},
        {"EGSPN_TRANSIENT", (XtPointer)	EGSPN_TRANSIENT},

        /*Pinv Popup Menu Values*/
        {"PINV_MITEM", (XtPointer)	PINV_MITEM},
        {"MINDEAD_MITEM", (XtPointer)	MINDEAD_MITEM},
        {"MINTRAPS_MITEM", (XtPointer)	MINTRAPS_MITEM},
        {"IMPPLACES_MITEM", (XtPointer)	IMPPLACES_MITEM},
        {"ABSMB_MITEM", (XtPointer)	ABSMB_MITEM},
        {"AVEMB_MITEM", (XtPointer)	AVEMB_MITEM},
        /*Tinv Popup Menu Values*/
        {"TINV_MITEM", (XtPointer)	TINV_MITEM},
        {"ECS_MITEM", (XtPointer)	ECS_MITEM},
        {"ME_MITEM", (XtPointer)	ME_MITEM},
        {"SC_MITEM", (XtPointer)	SC_MITEM},
        {"CC_MITEM", (XtPointer)	CC_MITEM},
        {"UNBSEQ_MITEM", (XtPointer)	UNBSEQ_MITEM},
        {"ACTLIVBOUND_MITEM", (XtPointer)	ACTLIVBOUND_MITEM},
        {"LIVBOUND_MITEM", (XtPointer)	LIVBOUND_MITEM},
        {"THRBOUND_MITEM", (XtPointer)	THRBOUND_MITEM},
        /*Menu Hep Values*/
        {"HELP_MOUSE_HELP_MITEM", (XtPointer)	HELP_MOUSE_HELP_MITEM},
        {"HELP_ABOUT_MITEM", (XtPointer)	HELP_ABOUT_MITEM}
    };
    static MrmCount		regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
        XtError("Sorry : can't register RegisterMenuValues names\n");
}
