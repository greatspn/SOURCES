/*
 * highlight.c
 */

/*
#define DEBUG
#define DEBUG_GC
*/

#include "global.h"
#include "draw.h"
#include "Canvas.h"
#include "select.h"

extern Boolean redrawPending;

int      blink_on = FALSE;
struct coordinate *ll = NULL;
struct arc_object *aa = NULL;
int      rr;

static int      flash_on = TRUE;
int      flash_anyway = FALSE;
// static Widget flash_item;
static XtIntervalId  tID = (XtIntervalId)NULL;
struct coordinate *Hlist;

void highlight_item(DrawingSurfacePun ds,
                    float           x, float y,
                    int             rad) {
    int             nw_x, nw_y;

    nw_x = (int)(x * zoom_level - rad);
    nw_y = (int)(y * zoom_level - rad);
    rad += rad;
    SetFunction(XOR, ds);
    XFillRectangle(ds->display, ds->drawable, ds->gc, nw_x, nw_y /*, diam, diam*/, rad, rad); /* TBD*/

}

void RescaleList(double howmuch) {
    struct coordinate *list;

    for (list = ll; list != NULL; list = list->next) {
        list->x = (list->x * howmuch) ;
        list->y = (list->y * howmuch);
    }
}


static void show_list(DrawingSurfacePun ds,
                      struct coordinate *list,
                      int             rad,
                      struct arc_object *arc,
                      int op) {
    int             nw_x, nw_y;
    int             diam = (rad + rad) * zoom_level;

    for (; list != NULL; list = list->next) {
        nw_x = (int)((list->x - rad) * zoom_level);
        nw_y = (int)((list->y - rad) * zoom_level);
        SetFunction(op, ds);
        XFillRectangle(ds->display, ds->drawable, ds->gc, nw_x, nw_y, diam, diam);
    }
    for (; arc != NULL; arc = arc->next) {
        if (highlighted)
            PutArc(arc, SET, ds, 0, 0);
        else
            PutArc(arc, CLEAR, ds, 0, 0);
    }

}

/* static struct itimerval blink_timer; */

#define NULL_TIM ((struct itimerval *)0)
extern XtAppContext	appContext;

static void
my_blinker(XtPointer closure, XtIntervalId *id) {
    static int interval;
#ifdef DEBUG
    puts("my_blinker");
#endif // DEBUG
    if (blink_on) {
        if (tID != (XtIntervalId)NULL)
            XtRemoveTimeOut(tID);

        if (highlighted) {
            interval = 500;
            highlighted = FALSE;
        }
        else {
            interval = 200;
            highlighted = TRUE;

        }
        show_list(&mainDS, ll, rr, aa, XOR);
        if (!highlighted && redrawPending) {
            show_list(&mainDS, ll, rr, aa, CLEAR);
            redisplay_canvas();
            redraw_selected();
            redrawPending = FALSE;
        }
        if (flash_on || flash_anyway || !highlighted)
            tID = XtAppAddTimeOut(appContext, interval, my_blinker, NULL);
        else
            tID = (XtIntervalId)NULL;

    }
    else {
        if (highlighted) {
            highlighted = FALSE;
            show_list(&mainDS, ll, rr, aa, XOR);
#ifdef DEBUG
            puts("Cancellato");
#endif // DEBUG
        }
        tID = (XtIntervalId)NULL;
    }
}

void PauseBlink() {

    XtIntervalId id;
    blink_on = FALSE;
    my_blinker(NULL, &id);
}

void ResumeBlink() {
    XtPointer closure = NULL;
    XtIntervalId id;
    blink_on = TRUE;
    my_blinker(closure, &id);
}

//static
//set_flash_proc(item, value, event)
//Widget      item;
//unsigned int    value;
//XEvent          *event;
//{
//    XtPointer closure=NULL;
//    XtIntervalId id;
//    int             prev_flash = flash_on;
//
//    if (inib_flag)
//	return;
//    if ((flash_on = (int) (value & 0x1)) && !prev_flash)
//	my_blinker(closure, &id);
//}
//

void highlight_list(struct coordinate *list,
                    int    rad, int flashing,
                    struct arc_object *arc) {
    XtPointer closure = NULL;
    XtIntervalId id;

    if (light_on)
        return;
    ll = list;
    rr = rad / zoom_level;
    aa = arc;
    light_on = TRUE;
    blink_on = TRUE;
    flash_anyway = flashing;
    tID = (XtIntervalId)NULL;
    highlighted = FALSE;
    my_blinker(closure, &id);

    /*
    	xv_set(flash_item, XV_SHOW, TRUE, 0);
    */
}

void dehighlight_list(struct coordinate *list,
                      int rad) {
    XtPointer closure = NULL;
    XtIntervalId id;
    /*
    Notify_client   me = (Notify_client) frame_w;
    (void) notify_set_itimer_func(me, my_blinker,
    ITIMER_REAL, NULL_TIM, NULL_TIM);
    */


    if (tID != (XtIntervalId)NULL)
        XtRemoveTimeOut(tID);
    tID = (XtIntervalId)NULL;
    light_on = FALSE;
    ll = list;
    rr = rad / zoom_level;
    blink_on = FALSE;
    my_blinker(closure, &id);
    aa = NULL;
    /*	xv_set(flash_item, XV_SHOW, FALSE, 0);*/
    flash_anyway = FALSE;
    ll = NULL;
    /*   clear_canvas();
       redisplay_canvas();	*/

}


void init_highlight() {
    /*
    flash_item = xv_create(menu_sw, PANEL_TOGGLE,
    		PANEL_LABEL_STRING, "",
    		PANEL_CHOICE_STRINGS, "flash", 0,
    XY_POS( X_Iflash.x, X_Iflash.y ),
    			PANEL_NOTIFY_PROC, set_flash_proc,
    			XV_SHOW, FALSE,
    			PANEL_FEEDBACK, PANEL_INVERTED,
    			0);

     xv_set(flash_item, PANEL_TOGGLE_VALUE, 0, TRUE, 0);
     */
}
