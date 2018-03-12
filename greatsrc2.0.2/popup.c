/*
 * popup.c
 */

/*
#define DEBUG
*/

#include "global.h"
#include "funct.h"
#include "showgdi.h"

struct f_list {
    Widget f_p;
    Widget c_p;
    Widget pw_p;
    DrawingSurface ds;
    struct f_list  *next;

};


static struct f_list *free_f = NULL;
static struct f_list *shown_f = NULL;



static void
UnmanageProcCB(Widget w, struct f_list *client_data, XmDrawingAreaCallbackStruct *call_data) {

    struct f_list  *p_p, *a_p;
    Arg alist[8];

    if (call_data->event->type != ButtonRelease)
        return;

    for (p_p = NULL, a_p = shown_f; a_p != client_data; p_p = a_p, a_p = a_p->next);

    if (p_p == NULL)
        shown_f = a_p->next;
    else
        p_p->next = a_p->next;

    a_p->next = free_f;
    free_f = a_p;

    XtUnmanageChild(XtParent(XtParent(XtParent(w))));
    XtUnmapWidget(XtParent(XtParent(XtParent(XtParent(w)))));
    remove_histogram(a_p->pw_p);
    remove_ind_prestazione(a_p->pw_p);

}


DrawingSurfacePun new_popup(int nw_x, int  nw_y, int  width, int  height, int  tot_w, int tot_h,
                            void (*repaint_proc)(),
                            char *frame_name) {

    Widget  popup_f;
    Widget  popup_w = NULL;
    Widget  popup_pw;
    DrawingSurfacePun dsp;
    struct f_list  *p_p;
    Arg alist[8];
    XmString title;

    title = XmStringCreate(frame_name, XmSTRING_DEFAULT_CHARSET);

    if (width < tot_w)
        width = tot_w;

    if (height < tot_h)
        height = tot_h;

    if (free_f == NULL) {

        popup_f = FetchWidget(frame_w, "HistoDialog");
        popup_pw = XtNameToWidget(popup_f, "*HistoDialog_Draw");

        XtManageChild(popup_f);

        XtSetArg(alist[0], XmNheight, height);
        XtSetArg(alist[1], XmNwidth, width);
        XtSetValues(popup_pw, alist, 2);

        XtSetArg(alist[0], XmNheight, height + 6);
        XtSetArg(alist[1], XmNwidth, width + 6);
        XtSetArg(alist[2], XmNdialogTitle, title);
        XtSetValues(popup_f, alist, 3);

        XtSetArg(alist[0], XmNmaxHeight, height + 6);
        XtSetArg(alist[1], XmNmaxWidth, width + 6);
        XtSetArg(alist[2], XmNx, nw_x);
        XtSetArg(alist[3], XmNy, nw_y);
        XtSetValues(XtParent(popup_f), alist, 4);

        printf("nw_y %d  nw_y %d\n", nw_y, nw_y);

        p_p = (struct f_list *) emalloc(sizeof(struct f_list));
        p_p->next = shown_f;
        shown_f = p_p;
        p_p->f_p = popup_f;
        p_p->c_p = popup_w;
        p_p->pw_p = popup_pw;
        dsp = &(p_p->ds);
        InitDS(popup_pw, dsp);
        XtAddCallback(popup_pw, XmNinputCallback, (XtCallbackProc)UnmanageProcCB, p_p);
#ifdef DEBUG
        fprintf(stderr, "Nuovo");
#endif
        XtMapWidget(popup_f);
        XtMapWidget(XtParent(popup_f));
    }
    else {


        p_p = free_f;
        free_f = free_f->next;
        p_p->next = shown_f;
        shown_f = p_p;
        popup_f = p_p->f_p;
        popup_w = p_p->c_p;
        popup_pw = p_p->pw_p;
        dsp = &(p_p->ds);

        printf("Width %d  Height %d\n", width, height);


        XtManageChild(popup_f);
        XtSetArg(alist[0], XmNheight, height + 6);
        XtSetArg(alist[1], XmNwidth, width + 6);
        XtSetArg(alist[2], XmNdialogTitle, title);
        XtSetValues(popup_f, alist, 3);

        XtSetArg(alist[0], XmNmaxHeight, height + 6);
        XtSetArg(alist[1], XmNmaxWidth, width + 6);
        XtSetArg(alist[2], XmNx, nw_x);
        XtSetArg(alist[3], XmNy, nw_y);
        XtSetValues(XtParent(popup_f), alist, 4);

        XtSetArg(alist[0], XmNheight, height);
        XtSetArg(alist[1], XmNwidth, width);
        XtSetValues(popup_pw, alist, 2);

        XtRemoveAllCallbacks(popup_pw, XmNexposeCallback);
#ifdef DEBUG
        fprintf(stderr, "Usato");
#endif
        XtMapWidget(popup_f);
        XtMapWidget(XtParent(popup_f));

    }
    XtAddCallback(popup_pw, XmNexposeCallback, repaint_proc, dsp);
    XmStringFree(title);
    SetBackground(popup_pw, "white");
    SetForeground(popup_pw, "black");
    XClearWindow(mainDS.display, XtWindow(popup_pw));


#ifdef DEBUG
    fprintf(stderr, "\nExit NewPopup\n");
#endif
    return (dsp);

}

void quit_popup(Widget pw) {

    struct f_list  *p_p, *a_p;
//	Arg alist[8];

    if (pw == NULL)
        return;
    for (p_p = NULL, a_p = shown_f; a_p != NULL && a_p->pw_p != pw;
            p_p = a_p, a_p = a_p->next);
    if (a_p == NULL)
        return;
    if (p_p == NULL)
        shown_f = a_p->next;
    else
        p_p->next = a_p->next;
    a_p->next = free_f;
    free_f = a_p;
    XtRemoveAllCallbacks(a_p->pw_p, XmNexposeCallback);

    XtUnmapWidget(XtParent(a_p->f_p));

}







/*
 XtSetArg(alist[0],XmNdialogTitle,title);

 popup_f= XmCreateFormDialog(frame_w,"ThePopup",alist,1);
 XtManageChild(popup_f);




#ifdef	DEBUG
fprintf(stderr,"tot_h = %d   tot_w = %d  \n",tot_h,tot_w);
#endif	DEBUG
*/
/*   XtSetArg(alist[2],XmNheight,tot_h+4);
    XtSetArg(alist[1],XmNwidth,tot_w+4);
    XtSetArg(alist[0],XmNscrollingPolicy,XmAUTOMATIC);

    popup_w = XmCreateScrolledWindow(popup_f,"",alist,1);
    XtManageChild(popup_w);

    SetBackground(popup_w,"gray50");


#ifdef	DEBUG
fprintf(stderr,"width = %d   height = %d  \n",width,height);
#endif	DEBUG

    XtSetArg(alist[0],XmNheight,height);
    XtSetArg(alist[1],XmNwidth,width);
    popup_pw = XmCreateDrawingArea(popup_w,"DrawingPopup",alist,2);
    XtManageChild(popup_pw);

    SetBackground(popup_pw,"white");
    SetForeground(popup_pw,"black");
*/

/*	    	    XmScrolledWindowSetAreas(popup_w,NULL,NULL,popup_pw);*/

/*	    XtSetArg(alist[0],XmNx,30);
	    XtSetArg(alist[1],XmNy,30);
	    XtSetValues(popup_f,alist,2);
	    XtMapWidget(popup_f);
	    XtMapWidget(XtParent(popup_f));
	    XtMapWidget(XtParent(XtParent(popup_f)));


        XtAddCallback(popup_f,XmNpopdownCallback,(XtCallbackProc)HideProcCB,NULL);
	*/
