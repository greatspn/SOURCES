#include <stdlib.h>
#include "object_old.h"

coordinatePTR create_coordinate(float x, float y) {
    coordinatePTR c;

    c = (coordinatePTR) Emalloc(COORD_SIZE);
    c->x    = x;
    c->y    = y;
    c->next = NULL;
    return (c);

}

positionPTR create_position(float x, float y) {
    positionPTR p;

    p = (positionPTR) Emalloc(POS_SIZE);
    p->x = x;
    p->y = y;
    return (p);

}

tagPTR create_tag(char *name, positionPTR pos) {
    tagPTR t;

    t = (tagPTR) Emalloc(TAG_SIZE);
    t->name = name;
    t->pos  = pos;
    return (t);

}


mpar_objectPTR create_mpar(tagPTR tag, LayerPTR layer, short value) {
    mpar_objectPTR m;

    m = (mpar_objectPTR) Emalloc(MPAOBJ_SIZE);
    m->tag = tag;
    m->layer = layer;
    m->value = value;
    m->next = NULL;
    return (m);

}


lisp_objectPTR create_lisp(tagPTR tag, char type, char *text) {
    lisp_objectPTR l;

    l       = (lisp_objectPTR) Emalloc(LISPOBJ_SIZE);
    l->tag    = tag;
    l->type = type;
    l->text = text;
    l->next = NULL;
    return (l);

}

place_objectPTR create_place(tagPTR tag, tagPTR color, lisp_objectPTR  lisp,
                             LayerPTR layer, positionPTR center) {
    place_objectPTR p;

    p         = (place_objectPTR) Emalloc(PLAOBJ_SIZE);
    p->tag      = tag;
    p->color  = color;
    p->lisp   = lisp;
    p->layer  = layer;
    p->center = center;
    p->tokens = p->m0 = 0;
    p->mpar   = NULL;
    p->cmark = NULL;
    p->next = NULL;
    return (p);

}


rpar_objectPTR create_rpar(tagPTR tag, LayerPTR layer, float value) {
    rpar_objectPTR r;

    r         = (rpar_objectPTR) Emalloc(RPAOBJ_SIZE);
    r->tag    = tag;
    r->layer  = layer;
    r->value  = value;
    r->next   = NULL;
    return (r);

}


com_objectPTR create_com(char *line) {
    com_objectPTR c;

    c         = (com_objectPTR) Emalloc(CMMOBJ_SIZE);
    c->line   = line;
    c->next   = NULL;
    return (c);

}

trans_objectPTR create_trans(tagPTR tag, tagPTR pri , tagPTR color, lisp_objectPTR  lisp,
                             LayerPTR layer, short kind, f_fpPTR fire_rate, positionPTR ratepos,
                             positionPTR center, short orient) {
    trans_objectPTR t;

    t         = (trans_objectPTR) Emalloc(TRNOBJ_SIZE);
    t->tag    = tag;
    t->color  = color;
    t->pri    = pri;
    t->lisp   = lisp;
    t->layer  = layer;
    t->kind   = kind;
    t->layer  = layer;
    t->fire_rate  = fire_rate;
    t->ratepos = ratepos;
    t->center  = center;
    t->orient  = orient;
    t->enabl = 0;
    t->rpar   =  NULL;
    t->mark_dep = NULL;
    t->next = NULL;
    return (t);

}

group_objectPTR create_group(tagPTR tag, short pri) {
    group_objectPTR g;

    g         = (group_objectPTR) Emalloc(GRPOBJ_SIZE);
    g->tag    = tag;
    g->pri  = pri;
    g->trans = NULL;
    g->next = NULL;
    return (g);

}

arc_objectPTR create_arc(char type, LayerPTR layer, short mult, place_objectPTR place,
                         coordinatePTR point, tagPTR color, lisp_objectPTR lisp) {
    arc_objectPTR a;

    a         = (arc_objectPTR) Emalloc(ARCOBJ_SIZE);
    a->type    = type;
    a->layer  = layer;
    a->mult = mult;
    a->place = place;
    a->point = point;
    a->color = color;
    a->lisp = lisp;
    return (a);

}





