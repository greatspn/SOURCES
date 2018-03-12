/*
 * search.c
 */


/*
#define DEBUG
*/


#include	"global.h"
#include	<ctype.h>

//
//search_proc(item, event)
//Widget     item;
//XEvent          *event;
//{
//	/*
//	int             found = FALSE, number = FALSE, s_num;
//	float           s_float;
//	struct mpar_object *mpar;
//	struct place_object *place;
//	struct rpar_object *rpar;
//	struct trans_object *trans;
//	struct group_object *group;
//	struct arc_object *arc, *arcnx;
//	struct res_object *result;
//	struct lisp_object *lisp;
//
//	if (inib_flag)
//	return;
//
//	strcpy(s_string, (char *) xv_get(search_string, PANEL_VALUE));
//
//	if( strlen(s_string) == 0 )
//	return;
//
//	dehighlight_search();
//
//	switch (cur_object) {
//	case MPAR:
//	if ( isdigit(s_string[0]) ) {
//		number = TRUE;
//		s_num = atoi(s_string);
//	}
//	for (place = netobj->places; place != NULL; place = place->next)
//		if (IsLayerListVisible(place->layer)) {
//		if (number) {
//			if (place->mpar == NULL && place->m0 == s_num ||
//			place->mpar != NULL && place->mpar->value == s_num) {
//			found = TRUE;
//			highlight_search(place->center);
//			}
//		}
//		else {
//			if (place->mpar != NULL &&
//			strcmp(place->mpar->tag, s_string) == 0
//			|| place->cmark != NULL &&
//			strcmp(place->cmark->tag, s_string) == 0) {
//			found = TRUE;
//			highlight_search(place->center);
//			}
//		}
//		}
//	if (found) {
//		put_msg(1,"Search for places with initial marking '%s' succeeded", s_string);
//		highlight_list(s_list, (int) (place_radius + place_radius), FALSE, NULL);
//	}
//	else
//		put_msg(1,"No place with initial marking '%s' !", s_string);
//	break;
//	case PLACE:
//	for (place = netobj->places; place != NULL; place = place->next)
//		if ( IsLayerListVisible(place->layer))
//		if (strcmp(place->tag, s_string) == 0) {
//			found = TRUE;
//			highlight_search(place->center);
//		}
//	if (found) {
//		put_msg(1,"Search for place '%s' succeeded", s_string);
//		highlight_list(s_list, (int) (place_radius + place_radius), FALSE, NULL);
//	}
//	else
//		put_msg(1,"No such place '%s' !", s_string);
//	break;
//	case RPAR:
//	if (s_string[0] >= '0' && s_string[0] <= '9' || s_string[0] == '.') {
//		number = TRUE;
//		s_float = atof(s_string);
//	}
//	for (trans = netobj->trans; trans != NULL; trans = trans->next)
//		if (IsLayerListVisible(trans->layer)) {
//		if (number) {
//			if (trans->mark_dep == NULL && (
//							trans->rpar == NULL && (
//			 trans->enabl >= 0 && trans->fire_rate.ff == s_float
//									  ||
//			   trans->enabl < 0 && trans->fire_rate.fp[0] == s_float
//									) ||
//			trans->rpar != NULL && trans->rpar->value == s_float
//							)) {
//			found = TRUE;
//			highlight_search(trans->center);
//			}
//		}
//		else {
//			if (trans->mark_dep == NULL && trans->rpar != NULL &&
//			strcmp(trans->rpar->tag, s_string) == 0) {
//			found = TRUE;
//			highlight_search(trans->center);
//			}
//		}
//		}
//	for (group = netobj->groups; group != NULL; group = group->next)
//		for (trans = group->trans; trans != NULL; trans = trans->next)
//		if (IsLayerListVisible(trans->layer)) {
//			if (number) {
//			if (trans->mark_dep == NULL &&
//				( trans->rpar == NULL &&
//				  trans->fire_rate.ff == s_float ||
//				  trans->rpar != NULL &&
//				  trans->rpar->value == s_float
//				)) {
//			    found = TRUE;
//			    highlight_search(trans->center);
//			}
//		    }
//		    else {
//			if (trans->mark_dep == NULL && trans->rpar != NULL &&
//			    strcmp(trans->rpar->tag, s_string) == 0) {
//			    found = TRUE;
//			    highlight_search(trans->center);
//			}
//		    }
//		}
//	if (found) {
//	    put_msg(1,"Search for transitions with rate '%s' succeeded", s_string);
//	    highlight_list(s_list, (int) (place_radius + place_radius), FALSE, NULL);
//	}
//	else
//	    put_msg(1,"No transition with rate '%s' !", s_string);
//	break;
//    case IMTRANS:
//    case DETRANS:
//    case EXTRANS:
//	for (trans = netobj->trans; trans != NULL; trans = trans->next)
//	    if (IsLayerListVisible(trans->layer))
//		if (strcmp(trans->tag, s_string) == 0) {
//			found = TRUE;
//		    highlight_search(trans->center);
//		}
//	for (group = netobj->groups; group != NULL; group = group->next)
//	    for (trans = group->trans; trans != NULL; trans = trans->next)
//		if (IsLayerListVisible(trans->layer))
//		    if (strcmp(trans->tag, s_string) == 0) {
//			found = TRUE;
//			highlight_search(trans->center);
//			}
//	if (found) {
//	    put_msg(1,"Search for transition '%s' succeeded", s_string);
//	    highlight_list(s_list, (int) (place_radius + place_radius), FALSE, NULL);
//	}
//	else
//	    put_msg(1,"No such transition '%s' !", s_string);
//	break;
//    case RESULT:
//	for (result = netobj->results; result != NULL; result = result->next)
//	    if (strcmp(result->tag, s_string) == 0) {
//		found = TRUE;
//		highlight_search(result->center);
//	    }
//	if (found) {
//	    put_msg(1,"Search for performance figure '%s' succeeded", s_string);
//	    highlight_list(s_list, (int) (place_radius + place_radius), FALSE, NULL);
//	}
//	else
//	    put_msg(1,"No such performance figure '%s' !", s_string);
//	break;
//    case COLOR:
//	for (place = netobj->places; place != NULL; place = place->next)
//	    if (IsLayerListVisible(place->layer )) {
//		if (place->color != NULL
//		    && strcmp(place->color, s_string) == 0
//			|| place->lisp != NULL
//		    && strcmp(place->lisp->tag, s_string) == 0) {
//		    found = TRUE;
//		    highlight_search(place->center);
//		}
//	    }
//	for (trans = netobj->trans; trans != NULL; trans = trans->next)
//	    if (IsLayerListVisible(trans->layer) {
//		if (trans->color != NULL
//			&& strcmp(trans->color, s_string) == 0
//		    || trans->lisp != NULL
//		    && strcmp(trans->lisp->tag, s_string) == 0) {
//		    found = TRUE;
//		    highlight_search(trans->center);
//		}
//	    }
//	for (group = netobj->groups; group != NULL; group = group->next)
//	    for (trans = group->trans; trans != NULL; trans = trans->next)
//		if (IsLayerListVisible(trans->layer)) {
//		    if (trans->color != NULL
//			&& strcmp(trans->color, s_string) == 0
//			|| trans->lisp != NULL
//			&& strcmp(trans->lisp->tag, s_string) == 0) {
//			found = TRUE;
//			highlight_search(trans->center);
//		    }
//		}
//	for (arc = netobj->arcs; arc != NULL; arc = arcnx) {
//	    arcnx = arc->next;
//	    if (IsLayerListVisible(arc->layer)) {
//		if (arc->color != NULL
//		    && strcmp(arc->color, s_string) == 0
//		    || arc->lisp != NULL
//		    && strcmp(arc->lisp->tag, s_string) == 0) {
//		    found = TRUE;
//			highlight_arc(arc);
//		}
//	    }
//	}
//	if (found) {
//	    if (a_list != NULL) {
//		if (netobj->arcs == NULL)
//		    netobj->arcs = a_list;
//		else {
//			for (arc = netobj->arcs; arc->next != NULL;
//			 arc = arc->next);
//		    arc->next = a_list;
//		}
//	    }
//	    put_msg(1,"Search for objects with color '%s' succeeded", s_string);
//	    highlight_list(s_list, (int) (place_radius + place_radius), FALSE, a_list);
//	}
//	else
//	    put_msg(1,"No object with color '%s' !", s_string);
//	break;
//    default:
//	for (mpar = netobj->mpars; mpar != NULL; mpar = mpar->next)
//	    if (IsLayerListVisible(mpar->layer))
//		if (strcmp(mpar->tag, s_string) == 0) {
//		    found = TRUE;
//		    highlight_search(mpar->center);
//		}
//	for (place = netobj->places; place != NULL; place = place->next)
//	    if (IsLayerListVisible(place->layer))
//		if (strcmp(place->tag, s_string) == 0) {
//		    found = TRUE;
//		    highlight_search(place->center);
//		}
//	for (rpar = netobj->rpars; rpar != NULL; rpar = rpar->next)
//	    if (IsLayerListVisible(rpar->layer))
//		if (strcmp(rpar->tag, s_string) == 0) {
//		    found = TRUE;
//		    highlight_search(rpar->center);
//		}
//	for (trans = netobj->trans; trans != NULL; trans = trans->next)
//	    if (IsLayerListVisible(trans->layer))
//		if (strcmp(trans->tag, s_string) == 0) {
//		    found = TRUE;
//		    highlight_search(trans->center);
//		}
//	for (group = netobj->groups; group != NULL; group = group->next)
//	    for (trans = group->trans; trans != NULL; trans = trans->next)
//		if (IsLayerListVisible(trans->layer))
//		    if (strcmp(trans->tag, s_string) == 0) {
//			found = TRUE;
//			highlight_search(trans->center);
//		    }
//	for (result = netobj->results; result != NULL; result = result->next)
//	    if (strcmp(result->tag, s_string) == 0) {
//		found = TRUE;
//		highlight_search(result->center);
//	    }
//	for (lisp = netobj->lisps; lisp != NULL; lisp = lisp->next)
//	    if (strcmp(lisp->tag, s_string) == 0) {
//		found = TRUE;
//		highlight_search(lisp->center);
//	    }
//	if (found) {
//	    put_msg(1,"Search for name '%s' succeeded", s_string);
//	    highlight_list(s_list, (int) (place_radius + place_radius), FALSE, NULL);
//	}
//	else
//	    put_msg(1,"No such object '%s' !", s_string);
//	break;
//	}
//	*/
//}
