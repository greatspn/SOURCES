/*
 *  canvas.c
 */


/*
#define DEBUG
#define DEBUG_GC
*/

#include "global.h"
#include "Canvas.h"
#include "draw.h"
#include "liveness.h"
#include "res.h"
#include "rate.h"
#include "grid.h"
#include "printarea.h"

extern void gdiClearDrawingArea();
extern int showison;



static int      oldx = 0, oldy = 0;
/* TBD */
static int      canvas_tracker = FALSE ;

//static short    pi_image_data[16] = {
///* Format_version=1, Width=16, Height=16, Depth=1, Valid_bits_per_item=16 */
//	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
//	0x0000, 0x7800, 0xAF00, 0x2400, 0x2400, 0x2400, 0x2400, 0x6600
//};

/**************************************************/
XImage    pi_im, shadow;
/**************************************************/
extern Widget sb1, sb2, TinvPopup, PinvPopup, EndShowPopup;

#define	LOC_MD	LOC_MOVE	/* LOC_DRAG */



void xv_writebackground(Widget       pw,
                        int    dx, int   dy, int   w, int   h, int   op) {
    /*
    Display        *display;
    Drawable        drawable;
    GC              gc;
    XGCValues       gc_val;
    XtGCMask        gcm;

    display = (Display *) XtDisplay(pw);
    drawable = (Drawable) XtWindow(pw);
    gc_val.function = op;

    #ifdef DEBUG_GC
    fprintf(stderr,"   ...xv_writebackground:  creating gc\n");
    #endif DEBUG

    gc = XCreateGC(display, drawable, GCFunction, &gc_val);
    XFillRectangle(display, drawable, gc, dx, dy, w, h);
    XFreeGC(display,gc);

    */
}



void init_canvas() {
    /*

    reader_left = reader_middle = reader_right = null_proc;
    move = left = middle = null_proc;

    init_help_menu();
    */

}

void reset_canvas_scroll() {
    int p1, p2, p3, p4;

    nw_x = 0;
    nw_y = 0;
    oldx = 0;
    oldy = 0;
    XmScrollBarGetValues(sb1, &p1, &p2, &p3, &p4);
    XmScrollBarSetValues(sb1, 0, p2, p3, p4, TRUE);
    XmScrollBarGetValues(sb2, &p1, &p2, &p3, &p4);
    XmScrollBarSetValues(sb2, 0, p2, p3, p4, TRUE);
}

//static void overview_tracker(int x, int y)
//{
//
//#ifdef DEBUG
//  fprintf(stderr,"  start overview_tracker(%d,%d)\n", x, y );
//#endif
//
///*
//	int             nnw_x, nnw_y;
//
//	nnw_x = (int) xv_get(h_sb, SCROLLBAR_VIEW_START);
//	nnw_y = (int) xv_get(v_sb, SCROLLBAR_VIEW_START);
//
//	move_ovbr_box(nnw_x, nnw_y);
//
//
//	oldx = x * z_coef;
//	oldy = y * z_coef;
//
//
//#ifdef DEBUG
//  fprintf(stderr,"  end overview_tracker(%d,%d)\n", x, y );
//#endif DEBUG
//*/
//}


#define	EID_LOC_MD (!(ie->type == ButtonRelease)) || (ie->type == MotionNotify)

void PopPopupEH(Widget	    cc, caddr_t	    client_data, XAnyEvent    *ie) {
    int     x = 0;
    int     y = 0;

    int a, b;



    if (receiving_msg)
        return;

    if (canvas_tracker) {
        if (ie->type == MotionNotify) {
            x = ((XMotionEvent *)ie)->x;
            y = ((XMotionEvent *)ie)->y;
        }
        else if (ie->type == ButtonPress) {
            x = ((XButtonEvent *)ie)->x;
            y = ((XButtonEvent *)ie)->y;
        }

        if (GetGridLevel() > 1) {
            a = x / GetGridLevel();
            b = x % GetGridLevel();
            if ((float)(b) > (float)(GetGridLevel()) * 0.5)
                x = (a + 1) * GetGridLevel();
            else
                x = a * GetGridLevel();
            a = y / GetGridLevel();
            b = y % GetGridLevel();
            if ((float)(b) > (float)(GetGridLevel()) * 0.5)
                y = (a + 1) * GetGridLevel();
            else
                y = a * GetGridLevel();
        }
        if (ie->type == MotionNotify) {

            /*overview_tracker(x, y);*/
#ifdef DEBUG
            fprintf(stderr, "\nMoving  x=%d   y=%d", x, y);
#endif
            (*move)(x, y);
        }
        else if (ie->type == ButtonPress) {
#ifdef DEBUG
            puts("TRUE ButtonPress\n");
#endif
            switch (((XButtonEvent *)ie)->button) {
            case Button1  :
                (*left)(x, y);
                break;
            case Button2:
                (*middle)(x, y);
                break;
            case Button3 :
                XmMenuPosition(popup, (XButtonPressedEvent *) ie);
                XtManageChild(popup);
                break;
            }
        }
    }
    else if (ie->type == ButtonPress) {
#ifdef DEBUG
        puts("FALSE ButtonPress\n");
#endif
        switch (((XButtonEvent *)ie)->button) {
        case Button3 :
            if (showison)
                switch (cur_object) {
                case PLACE:
                    XmMenuPosition(PinvPopup, (XButtonPressedEvent *) ie);
                    XtManageChild(PinvPopup);
                    break;
                case IMTRANS:
                case DETRANS:
                case EXTRANS:
                    XmMenuPosition(TinvPopup, (XButtonPressedEvent *) ie);
                    XtManageChild(TinvPopup);
                    break;
                default:
                    XmMenuPosition(EndShowPopup, (XButtonPressedEvent *) ie);
                    XtManageChild(EndShowPopup);
                }
            else {
                XmMenuPosition(popup, (XButtonPressedEvent *) ie);
                XtManageChild(popup);
            }
            break;
        case Button2 :
            (*reader_middle)(ie);
            break;
        case Button1 :
            (*reader_left)(ie);

        }
    }

}

void reset_canvas_reader() {
    action_on = FALSE;
    /* TBD */
    canvas_tracker = FALSE;
}

void set_canvas_tracker() {
    action_on = TRUE;
    canvas_tracker = TRUE;
}

void clear_canvas() {
    gdiClearDrawingArea();
}

extern struct coordinate *ll ;
extern struct arc_object *aa ;
extern int      rr;

extern int      flash_anyway ;



void redisplay_canvas() {
    SetWaitCursor(mainwin);

#ifdef DEBUG
    fprintf(stderr, "Start redisplay_canvas()\n");
#endif
    if (cur_command == FIRE_TRANS || cur_command == SIMULATE)
        dehighlight_trans();
#ifdef DEBUG
    fprintf(stderr, "  clearing throughputs\n");
#endif
    ClearThroughputs();
    /*SetThruVisible(FALSE);*/
#ifdef DEBUG
    fprintf(stderr, "  erasing live\n");
#endif
    EraseLive();
#ifdef DEBUG
    fprintf(stderr, "  clearing canvas\n");
#endif
    /*
    if(showison)
    {
    	clear_canvas();
    	highlighted = FALSE;
    }
    */
    highlighted = FALSE;
#ifdef DEBUG
    fprintf(stderr, "  displaying grid\n");
#endif
    DisplayGrid();
#ifdef DEBUG
    fprintf(stderr, "  redisplaying net\n");
#endif
    RedisplayNet();

#ifdef DEBUG
    fprintf(stderr, "  setting enabled\n");
#endif

    if (cur_command == FIRE_TRANS)
        set_enabled();
    else if (cur_command == SIMULATE)
        reset_simulation_screen();

#ifdef DEBUG
    fprintf(stderr, "  showing rates\n");
#endif

    if (rate_visible)
        ShowRate(TRUE);
    else if (res_visible)
        ShowThroughputs();
    else if (cur_command == SHOW)
        ShowLive();

#ifdef DEBUG
    fprintf(stderr, "  drawing rect_box\n");
#endif

    DrawPrintArea();

#ifdef DEBUG
    fprintf(stderr, "End redisplay_canvas()\n");
#endif

    ClearWaitCursor(mainwin);
}
