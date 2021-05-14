#include <stdlib.h>
#include "defines.h"
#include "unfolding-obj.h"
#include "var-lex.h"
#include "var-yacc.h"


#define READ_DOMAIN(pl) ( ((pl)->lisp != NULL) ? ((pl)->lisp)->text : (pl)->color)


extern list gListSubClasses;
list gListDomain;
list gListMarking;
list gListParsedArcs;
extern OptionType opt_list[];
FILE *gTr_Ind_fp;
FILE *gPl_Ind_fp;

/**************************************************************/
/* NAME :            DefParam                                 */
/* DESCRIPTION :     define marking and rate parameter for    */
/*                   unfolded net                             */
/* PARAMETERS :      netobj originary net                     */
/*                   unf_netobj unfolded net                  */
/**************************************************************/
static void DefParam(struct net_object *netobj, struct net_object *unf_netobj) {

    struct mpar_object *mpar;       /* temp. mpar_obj */
    struct mpar_object *unf_mpar;   /* temp. mpar_obj */
    struct mpar_object *unf_prev_mpar;  /* temp. mpar_obj */
    struct rpar_object *rpar;       /* temp. mpar_obj */
    struct rpar_object *unf_rpar;   /* temp. mpar_obj */
    struct rpar_object *unf_prev_rpar;  /* temp. mpar_obj */

    int i;

    for (mpar = netobj->mpars, i = 1; mpar != NULL; mpar = mpar->next, i++) {
        unf_mpar = mpar;
        if (i == 1)
            unf_netobj->mpars = unf_mpar;
        else
            unf_prev_mpar->next = unf_mpar;
        unf_prev_mpar = unf_mpar;

    }

//   for (rpar = netobj->rpars, i=1; rpar != NULL; rpar = rpar->next, i++) {
//     unf_rpar = (struct rpar_object *) Emalloc(RPAOBJ_SIZE);
//     unf_rpar->tag = (char *) Estrdup(rpar->tag);
//     unf_rpar->value = rpar->value;
//     unf_rpar->center.x = rpar->center.x;
//     unf_rpar->center.y = rpar->center.y;
//     unf_rpar->layer = NewLayerList(WHOLENET,NULL);
//     unf_rpar->next = NULL;
//     if (i == 1)
//         unf_netobj->rpars = unf_rpar;
//     else
//         unf_prev_rpar->next = unf_rpar;
//     unf_prev_rpar = unf_rpar;
//
//   }

    unf_netobj->rpars = netobj->rpars;
//   unf_netobj->mpars = netobj->mpars;
}



/**************************************************************/
/* NAME :            DefLispObjects                           */
/* DESCRIPTION :     parse and collect in gListClasses and    */
/*                   gListSubClasses definition of Classes    */
/*                   and SubClasses, respectively             */
/* PARAMETERS :      netobj originary net                     */
/*                   unf_netobj unfolded net                  */
/* NOTES :           parsing of marking is posticipated       */
/*                   because needs domain definition          */
/**************************************************************/
static void DefLispObjects(struct net_object *netobj, struct net_object *unf_netobj) {
    struct lisp_object *lisp;
    list curr = NULL;
    int i;
    markPTR m;



    init_list(&gListClasses);
    for (lisp = netobj->lisps; lisp != NULL; lisp = lisp->next)
        switch (lisp->type) {
        case 'c': {
            YACCobj_name = lisp->tag;
            LEXtoParsifyString = NewStringCat("~c ", lisp->text);
            EraseFinalCR(LEXtoParsifyString);

            yyparse();

            if (YACCparsedClass->type == SUBCLASS_TYPE)
                head_insert(&gListSubClasses, (generic_ptr) YACCparsedClass);
            else
                head_insert(&gListClasses, (generic_ptr) YACCparsedClass);


#if DEBUG_UNFOLD
            if (YACCparsedClass->type == SUBCLASS_TYPE) {
                printf("SubClass : %s\n", YACCparsedClass->name);
                printf("Elements : \n");
                for (i = 0; i < YACCparsedClass->num_el; i++)
                    printf("%s  ", (YACCparsedClass->elements)[i]->name);
                printf("\n");
            }
            else {
                printf("Class %s\n", YACCparsedClass->name);
                printf("SubClasses : \n");
                curr = NULL;
                while ((curr = list_iterator(YACCparsedClass->sub_classes, curr)) != NULL)
                    printf("%s  ", ((ClassObjPTR)DATA(curr))->name);
                printf("\n");
            }
#endif

            break;
        }
        case 'm': {
            break;
        }
        case 'f': {
            break;
        }
        default :
            Error(UNKN_LISP_ERR, "DefLispObjects", lisp->tag);
        }

}


/**************************************************************/
/* NAME :            DefClasses                               */
/* DESCRIPTION :     build structure of classes; i.e.         */
/*                   class P1 and P2 are static subclasses    */
/*                   of ordered classe P.                     */
/* PARAMETERS :      netobj originary net                     */
/*                   unf_netobj unfolded net                  */
/* NOTES :                  */
/*                          */
/**************************************************************/
static void DefClasses(struct net_object *netobj, struct net_object *unf_netobj) {

    list curr1 = NULL;
    list curr2 = NULL;
    list FoundSubClass;
    list lSubClasses;
    ClassObjPTR c;
    ClassObjPTR SubClass;
    char *class_name = UNCOLORED_CLASS_NAME;
    int i, j;

    while ((curr1 = list_iterator(gListClasses, curr1)) != NULL) {
        c = (ClassObjPTR) DATA(curr1);
        switch (c->type) {
        case UNORD_CLASS_TYPE:
        case ORD_CLASS_TYPE : {
            c->elements = NULL;
            lSubClasses = NULL;
            c->num_el = 0;
            c->min_idx = 0;
            c->max_idx = 0;
            i = 0;
            while ((curr2 = list_iterator(c->sub_classes, curr2)) != NULL) {
                char *subclassname;

                subclassname = ((ClassObjPTR) DATA(curr2))->name;
                if ((find_key(gListSubClasses, (generic_ptr) subclassname, CmpClassName, &FoundSubClass)) == ERROR)
                    Error(SUBCLASS_DEF_ERR, "DefClasses", NULL);
                else {
                    SubClass = (ClassObjPTR) DATA(FoundSubClass);
                    head_insert(&lSubClasses, SubClass);
                    c->elements = (Class_elementPTR *) Erealloc((generic_ptr) c->elements,
                                  (c->num_el + SubClass->num_el) *  sizeof(Class_elementPTR));


                    SubClass->min_idx = i;
                    SubClass->max_idx = i + SubClass->num_el;

                    c->max_idx += SubClass->num_el;
                    c->num_el += SubClass->num_el;

                    for (j = 0; j < SubClass->num_el; j++) {
                        c->elements[i++] = SubClass->elements[j];
                    }

                }

            }
            destroy(&(c->sub_classes), free);   /* Destroy List of Name of SubClasses */
            c->sub_classes = lSubClasses;       /* Replace it with list of SubClasses*/
#if DEBUG_UNFOLD
            printf("\nClass %s with %d elements:", c->name, c->num_el);
            for (i = 0; i < c->num_el; i++)
                printf("%s ", (c->elements[i])->name);
            printf("\n");
#endif

            break;
        }
        default :
            Error(UNKN_CLASS_TYPE_ERR, "DefClasses", NULL);
        }
    }
    head_insert(&gListClasses, (generic_ptr) NewUncoloredClass(class_name)); /*Add Uncolored Class*/

}


/**************************************************************/
/* NAME :            DefDomain                                */
/* DESCRIPTION :     for every place, parse and collect       */
/*                   in gListDomain definition of domain      */
/* PARAMETERS :      netobj originary net                     */
/*                   unf_netobj unfolded net                  */
/* NOTES :           gListDomain includes the uncolored class */
/**************************************************************/
static void DefDomain(struct net_object *netobj, struct net_object *unf_netobj) {
    struct place_object *place;
    char *great_domain, *class_name = UNCOLORED_CLASS_NAME;
    list curr = NULL, l = NULL;
    int i = 1;
    int cardinality;
    ClassObjPTR c;
    DomainObjPTR d;
    markPTR m;


    init_list(&gListDomain);
    init_list(&gListMarking);


    for (place = netobj->places; place != NULL; place = place->next) {
        YACCobj_name = place->tag;
        /* definizione dominio */
        if ((great_domain = READ_DOMAIN(place)) != NULL) { // se dominio colorato
            LEXtoParsifyString = NewStringCat("~w ", great_domain);
            EraseFinalCR(LEXtoParsifyString);

            yyparse();

#if DEBUG_UNFOLD
//    TEST PLACE DOMAIN
            curr = NULL;
            printf("\n%s) ", YACCparsedDomain->place_name);
            while ((curr = list_iterator(YACCparsedDomain->class_list, curr)) != NULL) {
                printf("%s ", ((ClassObjPTR) DATA(curr))->name);
            }
            printf("\n");
#endif

            cardinality = 1;
            curr = NULL;
            while ((curr = list_iterator(YACCparsedDomain->class_list, curr)) != NULL)
                cardinality *= ((ClassObjPTR) DATA(curr))->num_el;

            init_set(YACCparsedDomain->create_place, cardinality);
            append(&gListDomain, (generic_ptr) YACCparsedDomain);


            if (place->cmark != NULL) {

                YACCobj_name = place->cmark->tag;
                LEXtoParsifyString = NewStringCat("~m ", place->cmark->text);
                EraseFinalCR(LEXtoParsifyString);
                yyparse();
                YACCparsedMarking->place = place;
//      evalMarking(YACCparsedMarking, &p_MS);
                append(&gListMarking, (generic_ptr) YACCparsedMarking);


#if DEBUG_UNFOLD
                printf("Marking: %s\n", PrintMarking(YACCparsedMarking));
//        printMultiset(p_MS);
#endif

            }
        }
        else { // dominio non colorato
            init_list(&l);

            if ((find_key(gListClasses, (generic_ptr) class_name, CmpClassName, &curr)) == ERROR)
                Error(CLASS_DEF_ERR, "DefDomain", NULL);
            else {
                head_insert(&l, DATA(curr));
                YACCparsedDomain = NewDomain(place->tag, l, 1);
                init_set(YACCparsedDomain->create_place, 1);
                append(&gListDomain, (generic_ptr) YACCparsedDomain);
            }
#if DEBUG_UNFOLD
//  TEST PLACE DOMAIN
            curr = NULL;
            printf("\n%s) ", YACCparsedDomain->place_name);
            while ((curr = list_iterator(YACCparsedDomain->class_list, curr)) != NULL) {
                printf("%s ", ((ClassObjPTR) DATA(curr))->name);
            }
            printf("\n");
#endif

            if (place->mpar != NULL) {

                YACCparsedMarking = NewMarking(place->mpar->tag, (generic_ptr) place->mpar);
                YACCparsedMarking->type = 1;

            }
            else {
                YACCobj_name = "";
                LEXtoParsifyString = EmptyString();

                sprintf(LEXtoParsifyString, "~m <%d S>", place->m0);
                yyparse();
                YACCparsedMarking->type = 0;
            }
            YACCparsedMarking->place = place;
//     evalMarking(YACCparsedMarking, &p_MS);
            head_insert(&gListMarking, (generic_ptr) YACCparsedMarking);

        }


    }



}

/**************************************************************/
/* NAME :            ParseArc                                 */
/* DESCRIPTION :     parse arc expression                     */
/* PARAMETERS :      function, expression to parse            */
/* RETURN VALUE :    pointer to arc                           */
/**************************************************************/
arcPTR ParseArc(char *function) {

    YACCobj_name = EmptyString();
    LEXtoParsifyString = NewStringCat("~f ", function);
    EraseFinalCR(LEXtoParsifyString);
//  printf("Primapars %s \n", LEXtoParsifyString);
    yyparse();
//  printf("Dopopars\n");
    return (YACCparsedArc);

}

/**************************************************************/
/* NAME :            Free_Var_Name                            */
/* DESCRIPTION :     pick a free variable name                */
/* PARAMETERS :                                               */
/* RETURN VALUE :    variable name                            */
/* NOTES :                                                    */
/**************************************************************/
char *FreeVarName() {
    char *default_name = "xyzj";
    char *var_name;
    char *suffix = (char *)Emalloc(5);
    static int i = 1;
    list l = NULL;

    sprintf(suffix, "%d", i++);
    var_name = NewStringCat(default_name, suffix);
    while (find_key(YACCParsedVarList, (generic_ptr) var_name, CmpVarName, &l) == OK) {
        sprintf(suffix, "%d", i++);
        var_name = NewStringCat(default_name, suffix);
    }

    return (var_name);
}


/**************************************************************/
/* NAME :            NoColoredArc                             */
/* DESCRIPTION :     parse arc, without arc expression        */
/* PARAMETERS :      arc                                      */
/* RETURN VALUE :    pointer to arc                           */
/* NOTES :           gListDomain includes the uncolored class */
/**************************************************************/
arcPTR ParseNoColoredArc(struct arc_object *arc, char *new_var) {
    list l = NULL;
    DomainObjPTR d;
    char *class_name = UNCOLORED_CLASS_NAME;


    if (arc->place->color != NULL)
        if ((find_key(gListDomain, (generic_ptr)(arc->place)->tag , CmpDomainName, &l)) == OK) {
            d = (DomainObjPTR) DATA(l);
            if (d->num_el == 1) {

                YACCobj_name = EmptyString();
                LEXtoParsifyString = EmptyString();
                sprintf(LEXtoParsifyString, "~f %d<%s>", arc->mult, new_var);
                yyparse();
            }
            else
                Error(ERROR_REPETITIONS_AND_NO_FUNCTION, "NoColoredArc", (arc->place)->tag);
        }
        else
            Error(UNDEF_DOM_ERR, "NoColoredArc", (arc->place)->tag);
    else if ((find_key(gListDomain, (generic_ptr)(arc->place)->tag, CmpDomainName, &l)) == OK) {
        d = (DomainObjPTR) DATA(l);


        YACCobj_name = EmptyString();
        LEXtoParsifyString = EmptyString();
        sprintf(LEXtoParsifyString, "~f %d<S>", arc->mult);
        yyparse();
    }
    else
        Error(UNKN_CLASS_ERR, "NoColoredArc", class_name);

    return (YACCparsedArc);
}


/**************************************************************/
/* NAME :            ParseGuard                               */
/* DESCRIPTION :     parse guard expression                   */
/* PARAMETERS :      trans                                    */
/* RETURN VALUE :    pointer to guard expression              */
/**************************************************************/
guard_exprPTR ParseGuard(struct trans_object *trans, struct net_object *net) {
    bool Found;
    struct lisp_object *lisp;


    Found = FALSE;
    if (trans->color != NULL) {
        for (lisp = net->lisps; lisp != NULL; lisp = lisp->next)
            if (lisp->type == 'f' && (strcmp(trans->color, lisp->tag) == 0)) {
                LEXtoParsifyString = NewStringCat("~k ", lisp->text);
                EraseFinalCR(LEXtoParsifyString);
                Found = TRUE;
            }

        if (Found == FALSE) {
            LEXtoParsifyString = NewStringCat("~k ", trans->color);
            EraseFinalCR(LEXtoParsifyString);
        }

        YACCobj_name = EmptyString();
        yyparse();
#if DEBUG_UNFOLD
        printf("Guard expression: %s\n", PrintGuardExpr(YACCparsedGuard));
#endif
        return (YACCparsedGuard);
    }
    else if (trans->lisp != NULL) {
        YACCobj_name = EmptyString();
        LEXtoParsifyString = NewStringCat("~k ", (trans->lisp)->text);
        EraseFinalCR(LEXtoParsifyString);
        yyparse();
#if DEBUG_UNFOLD
        printf("Guard expression: %s\n", PrintGuardExpr(YACCparsedGuard));
#endif
        return (YACCparsedGuard);
    }
    else
        return (NULL);

    return (NULL);
}



static list expandtransition(struct trans_object *trans_el, struct net_object *net, struct net_object *unf_net) {
    struct trans_object *unf_trans;
    bool Found;
    struct lisp_object *lisp;
    struct arc_object *arc, *new_arc, *prev_arc;
    guard_exprPTR g;
    arcPTR *parsedArcs;
    operandPTR op;
    list arcs, l = NULL, curr = NULL;
    int i, j, noarc, otd, vtd;
    multisetPTR p_MS;
    AssignmentPTR gamma;
    status state;
    char *new_var;

    init_list(&gListParsedArcs);
    init_list(&YACCParsedVarList);


#if DEBUG_UNFOLD
    printf("Expand transition %s\n\n", trans_el->tag);
#endif

    new_var = FreeVarName();

    for (arc = trans_el->arcs, noarc = 0; arc != NULL; arc = arc->next, noarc++);

    parsedArcs = (arcPTR *) Ecalloc(noarc , sizeof(arcPTR));

    init_list(&YACCParsedVarList);

    i = 0;
    for (arc = trans_el->arcs; arc != NULL; arc = arc->next) {

        init_list(&l);

        if ((find_key(gListDomain, (generic_ptr)(arc->place)->tag , CmpDomainName, &l)) == OK) {
            YACCparsedDomain = ((DomainObjPTR) DATA(l));
            Found = FALSE;
            if (arc->color != NULL) {
                for (lisp = net->lisps; lisp != NULL; lisp = lisp->next) {
                    if (lisp->type == 'f' && (strcmp(arc->color, lisp->tag) == 0)) {
                        parsedArcs[i++] = ParseArc(lisp->text);
                        Found = TRUE;
                    }
                }
                if (Found == FALSE) {
                    parsedArcs[i++] = ParseArc(arc->color);
                    Found = TRUE;
                }
            }
            else if (Found == FALSE && arc->lisp != NULL)
                parsedArcs[i++] = ParseArc(arc->lisp->text);
            else
                parsedArcs[i++] = ParseNoColoredArc(arc, new_var);
        }
        else
            Error(UNDEF_DOM_ERR, "DefTransitions", (arc->place)->tag);

    }


    g = ParseGuard(trans_el, net);
    create_assignment(&gamma, YACCParsedVarList);


    otd = atoi(opt_list[OPT_ORIZ_TRANS_DISPLACEMENT].arg_val);
    vtd = atoi(opt_list[OPT_VERT_TRANS_DISPLACEMENT].arg_val);

    l = NULL;
    j = 0;
    init_list(&l);
    for (state = first_assignment(gamma) ; state != ERROR ; j++) {
#if DEBUG_UNFOLD
        printf("Assignment %s\n%", print_assignment(gamma));
#endif
        if (eval_guard_expr(g, gamma) == TRUE) {
            unf_trans = unfolding_transition(trans_el, j, gamma, &(unf_net->rpars));
            unf_trans->center.x += j * otd;
            unf_trans->center.y += j * vtd;
            head_insert(&l, (generic_ptr) unf_trans);


            i = 0;
            for (arc = trans_el->arcs; arc != NULL; arc = arc->next) {
#if DEBUG_UNFOLD
                printf("Arc %s \n", PrintArcExpression(parsedArcs[i]));
#endif


                eval_arc(parsedArcs[i], gamma, &p_MS);


                arcs = expand_arc(arc, p_MS, parsedArcs[i++]->domain->create_place, unf_net);
// 	destroy_multiset(p_MS);
                curr = NULL;
                while ((curr = list_iterator(arcs, curr)) != NULL) {
                    new_arc = (struct arc_object *) DATA(curr);
                    if (unf_trans->arcs == NULL)
                        unf_trans->arcs = new_arc;
                    else
                        prev_arc->next = new_arc;
                    prev_arc = new_arc;

                }
// 	destroy(&arcs, NULL);
            }

        }
        state = next_assignment(gamma);
    }

    return (l);
}



static void DefTransitions(struct net_object *netobj, struct net_object *unf_netobj) {
    struct trans_object *trans_el, *trans, *prev_trans;
    struct group_object *pri_group, *unf_new_group, *prev_group;
    list l, curr;
    int i;

    /* Immediate transition */
    for (pri_group = netobj->groups; pri_group != NULL; pri_group = pri_group->next) {


//    Create new group
        unf_new_group = (struct group_object *) Emalloc(sizeof(struct group_object));
        unf_new_group->tag = Estrdup(pri_group->tag);
        unf_new_group->pri = pri_group->pri;
        unf_new_group->trans = NULL;
        unf_new_group->center.x = pri_group->center.x;
        unf_new_group->center.y = pri_group->center.y;
        unf_new_group->movelink = NULL;
        // unf_new_group->next = unf_netobj->groups;
        unf_new_group->next = NULL;

        i = 0;
        for (trans_el = pri_group->trans;  trans_el != NULL; trans_el = trans_el->next) {
            l = expandtransition(trans_el, netobj, unf_netobj);
            curr = NULL;
            while ((curr = list_iterator(l, curr)) != NULL) {
                trans = (struct trans_object *) DATA(curr);
                trans->next = unf_new_group->trans;
                unf_new_group->trans = trans;
            }

        }
        destroy(&l, NULL);
        // unf_netobj->groups = unf_new_group;

        // Elvio, 11 June 2015 : now groups are copied without being reversed.
        // Copying groups in reversed order caused a priority inversion
        // in WNRG/WNSRG, which were unabled to compute the correct solution.
        if (pri_group == netobj->groups) // First group
            unf_netobj->groups = unf_new_group;
        else 
            prev_group->next = unf_new_group;
        prev_group = unf_new_group;
    }

    /* Exponential and Deterministic Transition */
    i = 0;
    for (trans_el = netobj->trans; trans_el != NULL; trans_el = trans_el->next) {
        l = expandtransition(trans_el, netobj, unf_netobj);
        curr = NULL;
        while ((curr = list_iterator(l, curr)) != NULL) {
            trans = (struct trans_object *) DATA(curr);
            trans->next =  unf_netobj->trans;
            unf_netobj->trans = trans;
        }
        destroy(&l, NULL);
    }

}


void DefMarking(struct net_object *netobj, struct net_object *unf_netobj) {
    struct place_object *place;
    list curr = NULL;
    markPTR m;
    multisetPTR p_MS;
    int i;
    char *tag;

    while ((curr = list_iterator(gListMarking, curr)) != NULL) {
        m = DATA(curr);
        if (m->type == 0) {
            evalMarking(m, &p_MS);
            for (i = 0; i < CARD(p_MS) ; i++)
                if (VALUE(SET(p_MS)[i]) > 0) {
                    tag = NewStringCat(m->place->tag, STR_INDEX(SET(p_MS)[i]));
                    for (place = unf_netobj->places; place != NULL && (strcmp(place->tag, tag) != 0) ; place = place->next);
                    if (place == NULL)
                        Error(UNKN_PLACE_ERR, "DefMarking", tag);
                    else
                        place->m0 = VALUE(SET(p_MS)[i]);
                }
        }
        else {
            tag = NewStringCat(m->place->tag, "_");
            for (place = unf_netobj->places; place != NULL && (strcmp(place->tag, tag) != 0) ; place = place->next);
            if (place == NULL)
                Error(UNKN_PLACE_ERR, "DefMarking", tag);
            else
                place->mpar = m->mrk;
        }
    }


}

void WritePlaceIdx(struct net_object *netobj, struct net_object *unf_netobj) {
    struct place_object *place_SWN;
    struct place_object *place_GSPN;

    for (place_SWN = netobj->places; place_SWN != NULL; place_SWN = place_SWN->next) {
        fprintf(gPl_Ind_fp, "Colored Place %s translated in:\n", place_SWN->tag);

        for (place_GSPN = unf_netobj->places; place_GSPN != NULL; place_GSPN = place_GSPN->next) {
            if (strstr(place_GSPN->tag, place_SWN->tag) != NULL)
                fprintf(gPl_Ind_fp, "   %s with M0 = %d \n", place_GSPN->tag, place_GSPN->m0);
        }
    }
}


int unfold(struct net_object *netobj, struct net_object *unf_netobj) {


    if ((gTr_Ind_fp = fopen(opt_list[OPT_TR_IDX_NAME].arg_val, "w")) == NULL) {
        printf("\nCan't open file %s\n", opt_list[OPT_TR_IDX_NAME].arg_val);
        return (1);
    }

    if (opt_list[OPT_PL_IDX_NAME].arg_val != NULL)
        if ((gPl_Ind_fp = fopen(opt_list[OPT_PL_IDX_NAME].arg_val, "w")) == NULL) {
            printf("\nCan't open file %s\n", opt_list[OPT_PL_IDX_NAME].arg_val);
            return (1);
        }

    DefParam(netobj, unf_netobj);
    DefLispObjects(netobj, unf_netobj);
    DefClasses(netobj, unf_netobj);
    DefDomain(netobj, unf_netobj);
//   DefDynamicSubClasses(netobj, unf_netobj);



    DefTransitions(netobj, unf_netobj);


    DefMarking(netobj, unf_netobj);

    if (opt_list[OPT_PL_IDX_NAME].arg_val != NULL) {
        WritePlaceIdx(netobj, unf_netobj);

    }
    fclose(gPl_Ind_fp);
    fclose(gTr_Ind_fp);
}
