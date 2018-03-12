#include "unfolding-obj.h"

struct trans_object *unfolding_transition(struct trans_object *t, int num, AssignmentPTR a, struct rpar_object **rpars) {
    struct trans_object *result;
    struct rpar_object  *rpar, *rpar_curr;
    char *string_num;
    bool Already_defined;

    result = (struct trans_object *) Emalloc(sizeof(struct trans_object));
    string_num = (char *) Ecalloc(TAG_SIZE, sizeof(char));
    sprintf(string_num, "_%d", num);
    result->tag = Estrdup(t->tag);
    result->tag = NewStringCat(result->tag, string_num);
    result->fire_rate.ff = t->fire_rate.ff;
    result->fire_rate.fp = t->fire_rate.fp;

    if (t->rpar != NULL)
        result->rpar = t->rpar;
    else {
        Already_defined = FALSE;
        rpar = (struct rpar_object *) malloc(RPAOBJ_SIZE);
        rpar->tag = EmptyString();
        rpar->tag = NewStringCat("Rate_", t->tag);

        for (rpar_curr = *rpars; rpar_curr != NULL && Already_defined == FALSE; rpar_curr = rpar_curr->next)
            if (strcmp(rpar_curr->tag, rpar->tag) == 0) {
                result->rpar = rpar_curr;
                Already_defined = TRUE;
            }


        if (Already_defined == FALSE) {
            rpar->layer = NewLayerList(WHOLENET, NULL);
            rpar->value = t->fire_rate.ff;
            rpar->center.x = t->center.x + 20;
            rpar->center.y = t->center.y + 20;
            rpar->next = *rpars;
            *rpars = rpar;
            result->rpar = rpar;
        }
    }

    result->enabl = t->enabl;
    result->kind = t->kind;
    result->mark_dep = t->mark_dep;
    result->center.x = t->center.x;
    result->center.y = t->center.y;
    result->tagpos.x = t->tagpos.x;
    result->tagpos.y = t->tagpos.y;
    result->colpos.x = t->colpos.x;
    result->colpos.y = t->colpos.y;
    result->ratepos.x = t->ratepos.x;
    result->ratepos.y = t->ratepos.y;
    result->orient = t->orient;
    result->arcs = NULL;
    result->color = NULL;
    result->lisp = NULL;
    result->layer = NewLayerList(WHOLENET, NULL);
    result->next = NULL;

    fprintf(gTr_Ind_fp, "Transition %s : %s\n", result->tag, print_assignment(a));

    return (result);
}





struct place_object *create_place(struct place_object *place, char *tag) {
    struct place_object *p;

    p = (struct place_object *) Emalloc(PLAOBJ_SIZE);
    p->tag = (char *) Estrdup(tag);
    p->color = NULL;
    p->lisp  = NULL;
    p->mpar  = NULL;
    p->layer  = NewLayerList(WHOLENET, NULL);
    p->cmark  = NULL;
    p->center.x = place->center.x;
    p->center.y = place->center.y;
    p->tagpos.x = place->tagpos.x;
    p->tagpos.y = place->tagpos.y;
    p->colpos.x = place->colpos.x;
    p->colpos.y = place->colpos.y;
    p->next = NULL;
    return (p);
}


list expand_arc(struct arc_object *arc, multisetPTR p_MS, set *p_S, struct net_object *unf_net) {
    int i, opd, vpd;
    list result;
    struct arc_object *new_arc;
    struct place_object *p = NULL;
    char *tag;

    opd = atoi(opt_list[OPT_ORIZ_PLACE_DISPLACEMENT].arg_val);
    vpd = atoi(opt_list[OPT_VERT_PLACE_DISPLACEMENT].arg_val);


    init_list(&result);
    for (i = 0; i < CARD(p_MS) ; i++)
        if (VALUE(SET(p_MS)[i]) > 0) {
            p = NULL;
            tag = NewStringCat(arc->place->tag, STR_INDEX(SET(p_MS)[i]));

            if (set_member(p_S, i) == FALSE) {
                set_insert(p_S, i);
                p = create_place(arc->place, tag);
                p->center.x += i * opd;
                p->center.y += i * vpd;


#if DEBUG_UNFOLD
                printf("Add place  %s\n\n", p->tag);
#endif
                if (unf_net->places != NULL)
                    p->next = unf_net->places;
                unf_net->places = p;


            }

            if (p == NULL)
                p = unf_net->places;

            while (p != NULL && (strcmp(p->tag, tag) != 0))
                p = p->next;

            if (p == NULL)
                Error(UNKN_PLACE_ERR, "expand_arc", tag);


            new_arc = (struct arc_object *) Emalloc(sizeof(struct arc_object));
            new_arc->type = arc->type;
            new_arc->mult = VALUE(SET(p_MS)[i]);
            new_arc->place = p;
            new_arc->colpos.x = 0;
            new_arc->colpos.y = 0;
            new_arc->point = arc->point;
            new_arc->color = NULL;
            new_arc->lisp = NULL;
            new_arc->layer = NewLayerList(WHOLENET, NULL);
            new_arc->next = NULL;
            append(&result, (generic_ptr) new_arc);

        }

    return (result);
}



