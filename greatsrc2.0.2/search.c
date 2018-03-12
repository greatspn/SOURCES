/*
 * search.c
 */


/*
#define DEBUG
*/


#include "global.h"
#include "showgdi.h"
#include "arc.h"
#include "draw.h"

//static Widget search_button;


char            s_string[LINEMAX];

struct coordinate *s_list = NULL;

struct arc_object *a_list = NULL;


void highlight_search(struct coordinate coor) {
    struct coordinate *c_p;

    c_p = (struct coordinate *) emalloc(COORD_SIZE);
    c_p->next = s_list;
    s_list = c_p;
    c_p->x = coor.x;
    c_p->y = coor.y;
}


void dehighlight_search() {
    struct coordinate *n_p;
    struct coordinate *point;
    float           xloc, yloc;
    int             diam = place_radius + place_radius;

    if (s_list == NULL && a_list == NULL)
        return;

#ifdef DEBUG
    fprintf(stderr, "Dehighlighting search list\n");
#endif
    dehighlight_list(s_list, (int)(place_radius + place_radius));
    for (; s_list != NULL; s_list = n_p) {
        n_p = s_list->next;
        free((char *) s_list);
    }
    for (; a_list != NULL; a_list = a_list->next) {
        for (point = a_list->point; point->next->next != NULL;
                point = point->next);
        if (a_list->color != NULL || a_list->lisp != NULL) {
            xloc = ((point->x + point->next->x) * 0.5
                    + a_list->colpos.x) * zoom_level - 3;
            yloc = ((point->y + point->next->y) * 0.5
                    + a_list->colpos.y) * zoom_level - diam + 2;
            gdiDrawingAreaInvertRect((int)xloc, (int)yloc, diam + diam, diam);
        }
        DrawArc(a_list, SET);
    }
}


void highlight_arc(struct arc_object *arc) {
    struct coordinate *point;
    float           xloc, yloc;
    int             diam = place_radius + place_radius;

    RemoveArc(arc);
    arc->next = a_list;
    a_list = arc;
    DrawArc(arc, CLEAR);
    for (point = arc->point; point->next->next != NULL;
            point = point->next);
    if (arc->color != NULL || arc->lisp != NULL) {
        xloc = ((point->x + point->next->x) * 0.5
                + arc->colpos.x) * zoom_level - 3;
        yloc = ((point->y + point->next->y) * 0.5
                + arc->colpos.y) * zoom_level - diam + 2;
        gdiDrawingAreaInvertRect((int)xloc, (int)yloc, diam + diam, diam);
    }
}


//void init_search()
//{
//	/*
//	search_string = xv_create( menu_sw, PANEL_TEXT,
//			PANEL_LABEL_STRING, ":",
//			PANEL_VALUE_DISPLAY_LENGTH, 16,
//			PANEL_VALUE, s_string,
//			FONT_NAME, gachab_font,
//	XY_POS( X_Ssearch.x, X_Ssearch.y ),
//			PANEL_NOTIFY_PROC, search_proc,
//			MENU_STRING, "Type here the string to be searched for", 0,
//			NULL);
//	search_button = xv_create( menu_sw, PANEL_BUTTON,
//				PANEL_LABEL_STRING,"search",
//	XY_POS( X_Bsearch.x, X_Bsearch.y ),
//				PANEL_NOTIFY_PROC, search_proc,
//				MENU_STRING,
//		"Highlight objects having the following string as attribute", 0,
//				0);
//				*/
//}
