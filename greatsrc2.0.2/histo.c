/*
 * histo.c
 */
#include "global.h"
#include "draw.h"
#include "showgdi.h"

#define HISTO_HEIGHT 80
#define HISTO_WIDTH 620

struct histo_list {
    DrawingSurfacePun      pw;
    struct place_object *place;
    int             height;
    struct histo_list *next;
};

static struct histo_list *head = NULL;

/** predeclaration */
void histogram_list(struct histo_list *pp);


void histogram_list_new() {
    struct histo_list *p_h;

    for (p_h = head; p_h != NULL; p_h = p_h->next) {
        gdiClearRect(p_h->pw, 0, 0, HISTO_WIDTH, p_h->height);
        histogram_list(p_h);
    }
}

void histogram_list(struct histo_list *pp) {
    char            string[LINEMAX];
    int             i;
    struct probability *prob;

    sprintf(string, "TOKENS DISTRIBUTION IN PLACE %s, mean=%g +/-%.2g",
            pp->place->tag, pp->place->distr->prob, pp->place->distr->next->prob);
    gdiDrawText(pp->pw, 5, 15, SET, cour_font, string);


    /* pp->pw !!!!!!!!!!!!!! deve essere una DrawingSurface !!!! */

    DrawIntVector(5, 20, HISTO_WIDTH - 5, 20, SET, pp->pw);
    DrawIntVector(100, 20, 100, pp->height - 5, SET, pp->pw);

    for (i = 35, prob = pp->place->distr->next->next; prob != NULL; i += 15, prob = prob->next) {
        sprintf(string, "%2d  %7.5f", prob->val, prob->prob);
        gdiDrawText(pp->pw, 5, i, SET, cour_font, string);
        gdiSetRect(pp->pw, 100, i - 10, (int)(500 * prob->prob + 0.5), 10);
    }
}


static void repaint_histo(Widget canvas,
                          XtPointer closure,
                          XtPointer call_data) {
    DrawingSurfacePun      paint_window = (DrawingSurfacePun) closure;
    struct histo_list *hl = head;

    while (hl->pw != paint_window) hl = hl->next;
    histogram_list(hl);
}


DrawingSurfacePun new_popup(int, int, int, int, int, int, void(*)(), char *);

void histogram(struct place_object *place,
               XButtonEvent          *ie) {

    DrawingSurfacePun		poppw;
    char            string[LINEMAX];
    int             histo_height;
    struct probability *prob;
    struct histo_list *new_histo;
    int             x, y;
    int p1, p2, p3, p4;

    XmScrollBarGetValues(sb1, &p1, &p2, &p3, &p4);
    x = ie->x_root;

    XmScrollBarGetValues(sb2, &p1, &p2, &p3, &p4);
    y = ie->y_root;

    sprintf(string, "TOKENS DISTRIBUTION IN PLACE %s", place->tag);
    for (histo_height = 35, prob = place->distr->next->next;
            prob != NULL;
            histo_height += 15, prob = prob->next);
    if (histo_height < HISTO_HEIGHT)
        histo_height = HISTO_HEIGHT;
    poppw = new_popup(x, y,
                      HISTO_WIDTH, histo_height,
                      HISTO_WIDTH, HISTO_HEIGHT,
                      repaint_histo, string);
    new_histo = (struct histo_list *) emalloc(sizeof(struct histo_list));
    new_histo->next = head;
    head = new_histo;
    new_histo->pw = poppw;
    new_histo->place = place;
    new_histo->height = histo_height;
    histogram_list(new_histo);

}

void remove_histogram(DrawingSurfacePun   pw_p) {
    struct histo_list *l_p, *p_l_p;

    for (p_l_p = NULL, l_p = head;
            l_p != NULL && l_p->pw != pw_p;
            p_l_p = l_p, l_p = l_p->next) ;

    if (l_p != NULL) {
        if (p_l_p != NULL)
            p_l_p->next = l_p->next;
        else
            head = l_p->next;
        free(l_p);
    }
}
