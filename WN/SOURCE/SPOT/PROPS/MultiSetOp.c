#include <stdlib.h>
#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/SCONSSPOT.h"
#include "../../../INCLUDE/struct.h"
#include  "../../../INCLUDE/SSTRUCTSPOT.h"
#include "../../../INCLUDE/var_ext.h"
#include "../../../INCLUDE/fun_ext.h"
#include  "../../../INCLUDE/SVAR_EXTSPOT.h"
#include "../../../INCLUDE/macros.h"
#include "../../../INCLUDE/service.h"
#include "../../../INCLUDE/gspnlib.h"

extern void SymRepOfToken();
extern int TestSymRepWtToken();
extern void DynDistOfObjects();
extern int get_static_subclass();

void Free(CToken_p tk) {
    free(tk->tok_dom.Domain);
    free(tk->tok_dom.Token);
    free(tk);
}
void FreeList(CToken_p tk) {
    CToken_p ptr = tk, ptr1;
    while (ptr) {
        ptr1 = ptr->next;
        Free(ptr);
        ptr = ptr1;
    }
}
CToken_p Projection(CToken_p tk) {
    int place = tk->mult, i, c;
    CToken_p head = NULL;
    CToken_p *ptr;
    Token_Domains Proj = tk->tok_dom;
    Token_p marking = net_mark[place].marking;
    ptr = &head;

    while (marking) {
        (*ptr) = (CToken_p)malloc(sizeof(CToken));
        (*ptr)->next = NULL;
        (*ptr)->mult = marking->molt;
        (*ptr)->tok_dom.NbCl = Proj.NbCl;
        (*ptr)->tok_dom.Domain = NULL;
        (*ptr)->tok_dom.Token = NULL;
        c = 0;
        for (i = 0; i < tabp[place].comp_num; i++)
            if (Proj.Token[i] == PROJECT) {
                (*ptr)->tok_dom.Domain = realloc((*ptr)->tok_dom.Domain,
                                                 (c + 1) * sizeof(int));
                (*ptr)->tok_dom.Token = realloc((*ptr)->tok_dom.Token,
                                                (c + 1) * sizeof(int));
                (*ptr)->tok_dom.Token[c] = marking->id[i];
                (*ptr)->tok_dom.Domain[c] = tabp[place].dominio[i];
                c++;
            }
        ptr = &((*ptr)->next);
        marking = marking->next;
    }
    return head;
}

CToken_p Identity(CToken_p tk) {
    int place = tk->mult, i;
    CToken_p head = NULL;
    CToken_p *ptr;
    Token_p marking = net_mark[place].marking;

    ptr = &head;
    while (marking) {
        (*ptr) = (CToken_p)malloc(sizeof(CToken));
        (*ptr)->next = NULL;
        (*ptr)->mult = marking->molt;
        (*ptr)->tok_dom.NbCl = tabp[place].comp_num;
        (*ptr)->tok_dom.Domain = NULL;
        (*ptr)->tok_dom.Token = NULL;

        for (i = 0; i < tabp[place].comp_num; i++) {
            (*ptr)->tok_dom.Domain = realloc((*ptr)->tok_dom.Domain, (i + 1) * sizeof(int));
            (*ptr)->tok_dom.Token = realloc((*ptr)->tok_dom.Token, (i + 1) * sizeof(int));
            (*ptr)->tok_dom.Token[i] = marking->id[i];
            (*ptr)->tok_dom.Domain[i] = tabp[place].dominio[i];
        }
        ptr = &((*ptr)->next);
        marking = marking->next;
    }
    return head;
}

CToken_p
SymRepOfTokenList(CToken_p Token_p,
                  int **DynDis) {
    CToken_p *Arr = NULL;
    CToken_p *pt;
    CToken_p ptr = Token_p,
             Head = NULL, tmp = NULL;

    int NbEl = 0, i, j;

    while (ptr) {
        Arr = realloc(Arr, (NbEl + 1) * sizeof(Token_p));
        Arr[NbEl] = ptr;
        NbEl++;
        ptr = ptr->next;
    }

    pt = &Head;

    for (i = 0; i < NbEl; i++)
        if (Arr[i] != NULL) {
            tmp = (CToken_p)malloc(sizeof(CToken));
            SymRepOfToken(Arr[i]->tok_dom, &(tmp->tok_dom), DynDis);
            tmp->mult = Arr[i]->mult;
            tmp->next = NULL;
            Arr[i] = tmp;
            (*pt) = Arr[i];
            pt = &(Arr[i]->next);
            for (j = i + 1; j < NbEl; j++)
                if (Arr[j] &&
                        (TestSymRepWtToken(Arr[i]->tok_dom,
                                           Arr[j]->tok_dom, DynDis) == true
                        )
                   )
                    Arr[j] = NULL;
        }
    free(Arr);
    return Head;
}

int next(int j, int nMs2,  CToken_p  *Arr) {
    int k;
    for (k = j + 1; k < nMs2; k++)
        if (Arr[k]) return k;
    return nMs2;

}

CToken_p
MultSetsAndOp(CToken_p MultSet1, CToken_p MultSet2) {
    CToken_p ptr, Head;
    CToken_p *Arr = NULL;
    CToken_p *pt;
    int i, j, nMs1, nMs2, flag;

    for (ptr = MultSet1, nMs1 = 0; ptr != NULL; ptr = ptr->next, nMs1++) {
        Arr = realloc(Arr, (nMs1 + 1) * sizeof(CToken_p));
        Arr[nMs1] = ptr;
    }
    for (ptr = MultSet2, nMs2 = nMs1; ptr != NULL; ptr = ptr->next, nMs2++) {
        Arr = realloc(Arr, (nMs2 + 1) * sizeof(CToken_p));
        Arr[nMs2] = ptr;
    }

    for (i = 0; i < nMs1; i++) {
        flag = false;
        for (j = nMs1; j < nMs2 && !flag; j++)
            if (Arr[j] != NULL)
                if (memcmp(Arr[i]->tok_dom.Token,
                           Arr[j]->tok_dom.Token,
                           Arr[i]->tok_dom.NbCl * sizeof(int)) == 0) {
                    Arr[i]->mult = min(Arr[i]->mult, Arr[j]->mult);
                    Free(Arr[j]);
                    Arr[j] = NULL;
                    flag = true;
                }

        if (!flag) {
            Free(Arr[i]);
            Arr[i] = NULL;
        }
    }
    pt = &Head;

    for (i = 0; i < nMs2; i++)
        if (Arr[i] && i < nMs1) {
            (*pt) = Arr[i];
            pt = &(Arr[i]->next);
        }
        else if (Arr[i] && i >= nMs1)
            free(Arr[i]);

    (*pt) = NULL;
    free(Arr);
    return Head;
}


CToken_p MultSetsOrOp(CToken_p MultSet1, CToken_p MultSet2) {
    CToken_p ptr, Head;
    CToken_p *Arr = NULL;
    CToken_p *pt;
    int i, j, nMs1, nMs2, flag;

    for (ptr = MultSet1, nMs1 = 0; ptr != NULL; ptr = ptr->next, nMs1++) {
        Arr = realloc(Arr, (nMs1 + 1) * sizeof(CToken_p));
        Arr[nMs1] = ptr;
    }
    for (ptr = MultSet2, nMs2 = nMs1; ptr != NULL; ptr = ptr->next, nMs2++) {
        Arr = realloc(Arr, (nMs2 + 1) * sizeof(CToken_p));
        Arr[nMs2] = ptr;
    }

    for (i = 0; i < nMs1; i++) {
        flag = false;
        for (j = nMs1; j < nMs2 && !flag; j = next(j, nMs2, Arr))
            if (memcmp(Arr[i]->tok_dom.Token,
                       Arr[j]->tok_dom.Token,
                       Arr[i]->tok_dom.NbCl * sizeof(int)) == 0)	{
                Arr[i]->mult = Arr[i]->mult + Arr[j]->mult;
                Free(Arr[j]);
                Arr[j] = NULL;
                flag = true;
            }
    }

    pt = &Head;
    for (i = 0; i < nMs2; i++)
        if (Arr[i]) {
            (*pt) = Arr[i];
            pt = &(Arr[i]->next);
        }
    (*pt) = NULL;
    free(Arr);
    return Head;
}


int MultSetEQOp(CToken_p MultSet1, CToken_p MultSet2) {
    CToken_p ptr, Head;
    CToken_p *Arr = NULL;
    CToken_p *pt;
    int i, j, nMs1, nMs2, flag;

    for (ptr = MultSet1, nMs1 = 0; ptr != NULL; ptr = ptr->next, nMs1++) {
        Arr = realloc(Arr, (nMs1 + 1) * sizeof(CToken_p));
        Arr[nMs1] = ptr;
    }
    for (ptr = MultSet2, nMs2 = nMs1; ptr != NULL; ptr = ptr->next, nMs2++) {
        Arr = realloc(Arr, (nMs2 + 1) * sizeof(CToken_p));
        Arr[nMs2] = ptr;
    }

    for (i = 0; i < nMs1; i++) {
        flag = false;
        for (j = nMs1; j < nMs2 && !flag; j++)
            if (Arr[j] &&
                    memcmp(Arr[i]->tok_dom.Token,
                           Arr[j]->tok_dom.Token,
                           Arr[i]->tok_dom.NbCl * sizeof(int)) == 0 &&
                    Arr[i]->mult == Arr[j]->mult) {
                Arr[i] = Arr[j] = NULL;
                flag = true;
            }

    }

    flag = false;

    for (i = 0; i < nMs2; i++)
        if (Arr[i]) {
            flag = true;
            Arr[i] = NULL;
        }

    free(Arr);

    return (!flag);
}



int MultSetLTEQOp(CToken_p MultSet1, CToken_p MultSet2) {
    CToken_p ptr, Head;
    CToken_p *Arr = NULL;
    CToken_p *pt;
    int i, j, nMs1, nMs2, flag;

    for (ptr = MultSet1, nMs1 = 0; ptr != NULL; ptr = ptr->next, nMs1++) {
        Arr = realloc(Arr, (nMs1 + 1) * sizeof(CToken_p));
        Arr[nMs1] = ptr;
    }
    for (ptr = MultSet2, nMs2 = nMs1; ptr != NULL; ptr = ptr->next, nMs2++) {
        Arr = realloc(Arr, (nMs2 + 1) * sizeof(CToken_p));
        Arr[nMs2] = ptr;
    }

    for (i = 0; i < nMs1; i++) {
        flag = false ;
        for (j = nMs1; j < nMs2 && !flag; j++)
            if (Arr[j] &&
                    memcmp(Arr[i]->tok_dom.Token,
                           Arr[j]->tok_dom.Token,
                           Arr[i]->tok_dom.NbCl * sizeof(int)) == 0 &&
                    Arr[i]->mult <= Arr[j]->mult) {
                Arr[i] = Arr[j] = NULL;
                flag = true;
            }
    }

    flag = false;
    for (i = 0; i < nMs2; i++)
        if (Arr[i]) {
            Arr[i] = NULL;
            if (i < nMs1)
                flag = true;
        }
    free(Arr);

    return (!flag);
}

int MultSetLTOp(CToken_p MultSet1, CToken_p MultSet2) {
    CToken_p ptr, Head;
    CToken_p *Arr = NULL;
    CToken_p *pt;
    int i, j, nMs1, nMs2, flag;

    for (ptr = MultSet1, nMs1 = 0; ptr != NULL; ptr = ptr->next, nMs1++) {
        Arr = realloc(Arr, (nMs1 + 1) * sizeof(CToken_p));
        Arr[nMs1] = ptr;
    }
    for (ptr = MultSet2, nMs2 = nMs1; ptr != NULL; ptr = ptr->next, nMs2++) {
        Arr = realloc(Arr, (nMs2 + 1) * sizeof(CToken_p));
        Arr[nMs2] = ptr;
    }

    for (i = 0; i < nMs1; i++) {
        flag = false ;
        for (j = nMs1; j < nMs2 && !flag; j++)
            if (Arr[j] &&
                    memcmp(Arr[i]->tok_dom.Token,
                           Arr[j]->tok_dom.Token,
                           Arr[i]->tok_dom.NbCl * sizeof(int)) == 0 &&
                    Arr[i]->mult < Arr[j]->mult) {
                Arr[i] = Arr[j] = NULL;
                flag = true;
            }
    }

    flag = false;
    for (i = 0; i < nMs2; i++)
        if (Arr[i]) {
            Arr[i] = NULL;
            if (i < nMs1)
                flag = true;
        }
    free(Arr);

    return (!flag);

}

/*** c'est bon ********/

int MultSetNEQOp(CToken_p MultSet1, CToken_p MultSet2) {
    return (!MultSetEQOp(MultSet1, MultSet2));
}

int MultSetNLTEQOp(CToken_p MultSet1, CToken_p MultSet2) {
    return (!MultSetLTEQOp(MultSet1, MultSet2));
}

int MultSetGTEQOp(CToken_p MultSet1, CToken_p MultSet2) {
    return MultSetLTEQOp(MultSet2, MultSet1);
}

int MultSetNGTEQOp(CToken_p MultSet1, CToken_p MultSet2) {
    return (!MultSetGTEQOp(MultSet1, MultSet2));
}


int MultSetNLTOp(CToken_p MultSet1, CToken_p MultSet2) {
    return (!MultSetLTOp(MultSet1, MultSet2));
}

int MultSetGTOp(CToken_p MultSet1, CToken_p MultSet2) {
    return MultSetLTOp(MultSet2, MultSet1);
}

int MultSetNGTOp(CToken_p MultSet1, CToken_p MultSet2) {
    return (!MultSetGTOp(MultSet1, MultSet2));
}

/********************/

CToken_p ParcourtArbreProp(TreeNode_p root, int **DynDis) {
    CToken_p ptr1, ptr2;
    if (root->op != NOP) {
        ptr1 = ParcourtArbreProp(root->filsg, DynDis);
        ptr2 = ParcourtArbreProp(root->filsd, DynDis);
        switch (root->op) {
        case ANDOP: return MultSetsAndOp(ptr1, ptr2);
        case OROP : return MultSetsOrOp(ptr1, ptr2);
        }
    }
    else
        switch (root->type) {
        case IDE: return Identity(root->FONCT);
        case TOK: return SymRepOfTokenList(root->FONCT, DynDis);
        case PRO: return Projection(root->FONCT);
        }
}

int my_get_ordinary_tokens(CToken_p tk_p)

{
    int ret_value = 1;
    int ii = tk_p->tok_dom.NbCl;
    int cl, ss, ds;

    for (; ii ; ii--) {
        cl = tk_p->tok_dom.Domain[ii - 1];
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1)) {
            ds = tk_p->tok_dom.Token[ii - 1];
            ss = get_static_subclass(cl, ds);
            ret_value *= GET_CARD(cl, ss, ds);
        }
    }
    return (ret_value);
}

int  ParcourtArbrePropDEC(TreeNode_p root, int **DynDis) {
    int NbToken1, NbToken2;
    CToken_p ptr1, ptr2;
    if (root->op != NOP)
        switch (root->op) {
        case DEC:
            ptr2 = ptr1 = ParcourtArbreProp(root->filsd, DynDis);
            /* Compter le nombre de jetons ordinaires*/
            NbToken1 = 0;
            while (ptr1) {

                NbToken1 += ptr1->mult * my_get_ordinary_tokens(ptr1);
                ptr1 = ptr1->next;
            }
            FreeList(ptr2);
            return NbToken1;
        case OROP:
            NbToken1 = ParcourtArbrePropDEC(root->filsg, DynDis);
            NbToken2 = ParcourtArbrePropDEC(root->filsd, DynDis);
            return  NbToken1 + NbToken2;
        }
    else
        return root->FONCT->mult;
}

int Satisfaction(int *_CONS_, int Nb_CONS_, TYPE_P **PM,
                 TYPE *NbElPM, int *** Card, int **num, int **DynDis) {
    int i, OP, tok1, tok2;
    CToken_p ptr1, ptr2;

    DynDistOfObjects(PM, NbElPM, Card, num, DynDis);

    int fl = true;

    for (i = 0; i < Nb_CONS_ ; i++)
        if (_CONS_[i] != -1) {
            OP = PROPOSITIONS[i].prop->op;

            if (_CONS_[i] == 0)
                switch (OP) {
                case EQOP : OP = NEQOP; break;
                case LQOP : OP = NLQOP; break;
                case LOP  : OP = NLOP; break;
                case GQOP : OP = NGQOP; break;
                case GOP  : OP = NGOP; break;
                }
            if (PROPOSITIONS[i].prop->type != DEC) {

                ptr1 = ParcourtArbreProp(PROPOSITIONS[i].prop->filsg, DynDis);
                ptr2 = ParcourtArbreProp(PROPOSITIONS[i].prop->filsd, DynDis);
                switch (OP) {
                case EQOP  :   if (!MultSetEQOp(ptr1, ptr2))    fl = false; break;
                case LQOP  :   if (!MultSetLTEQOp(ptr1, ptr2))  fl = false; break;
                case LOP   :   if (!MultSetLTOp(ptr1, ptr2))    fl = false; break;
                case GQOP  :   if (!MultSetGTEQOp(ptr1, ptr2))  fl = false; break;
                case GOP   :   if (!MultSetGTOp(ptr1, ptr2))    fl = false; break;
                case NEQOP :   if (!MultSetNEQOp(ptr1, ptr2))   fl = false; break;
                case NLQOP :   if (!MultSetNLTEQOp(ptr1, ptr2)) fl = false; break;
                case NLOP  :   if (!MultSetNLTOp(ptr1, ptr2))   fl = false; break;
                case NGQOP :   if (!MultSetNGTEQOp(ptr1, ptr2)) fl = false; break;
                case NGOP  :   if (!MultSetNGTOp(ptr1, ptr2))   fl = false; break;
                }
                FreeList(ptr1);
                FreeList(ptr2);

            }
            else {
                tok1 = ParcourtArbrePropDEC(PROPOSITIONS[i].prop->filsg, DynDis);
                tok2 = ParcourtArbrePropDEC(PROPOSITIONS[i].prop->filsd, DynDis);
                switch (OP) {

                case EQOP  :   if (!(tok1 == tok2))   fl = false; break;
                case LQOP  :   if (!(tok1 <= tok2))  fl = false;  break;
                case LOP   :   if (!(tok1 < tok2))    fl = false;  break;
                case GQOP  :   if (!(tok1 >= tok2))   fl = false; break;
                case GOP   :   if (!(tok1 > tok2))    fl = false; break;
                case NEQOP :   if (!(tok1 != tok2))   fl = false; break;
                case NLQOP :   if (tok1 <= tok2)      fl = false; break;
                case NLOP  :   if (tok1 < tok2)       fl = false; break;
                case NGQOP :   if (tok1 >= tok2)      fl = false; break;
                case NGOP  :   if (tok1 > tok2)       fl = false ; break;
                }
            }
            if (fl == false)
                break;
        }


    return fl;
}

void add_to_ev_list(Event_p *list, Event_p ev) {
    ev->next = (*list);
    (*list) = ev;
}

void free_ev_list(Event_p list) {
    Event_p ptr;
    while (list) {
        ptr = list->next;
        dispose_old_event(list);
        list = ptr;
    }
}

Event_p join_two_lists(Event_p list1, Event_p list2) {
    Event_p ptr = list1;

    if (ptr)
        while (ptr->next) ptr = ptr->next;
    else
        return list2;

    ptr->next = list2 ;

    return list1;
}

void
MAXIMUM_FUNCT(int comp, int cl,
              Event_p list, Event_p *sat,
              Event_p *notsat) {

    Event_p ptr = list, next ;
    int sbc = UNKNOWN, new_sbc = UNKNOWN;
    int *static_arry = NULL;
    int nb_elem = 0, i;

    (*sat) = (*notsat) = NULL;

    while (ptr) {
        static_arry = realloc(static_arry, (nb_elem + 1) * sizeof(int));
        new_sbc = static_arry[nb_elem] = get_static_subclass(cl, ptr->npla[comp - 1]);
        if (new_sbc > sbc) sbc = new_sbc;
        nb_elem++;
        ptr = ptr->next;
    }
    i = 0;
    ptr = list;
    while (ptr) {
        next = ptr->next;
        if (static_arry[i] < sbc)
            add_to_ev_list(notsat , ptr);
        else
            add_to_ev_list(sat , ptr);
        i++;
        ptr = next;
    }

    free(static_arry);
}

int
max(Token_Domains td, int comp,
    int cl, Event_p ptr) {

    int i, max_comp = get_static_subclass(cl, ptr->npla[td.Token[0] - 1]);

    for (i = 1; i < td.NbCl; i++)
        if (get_static_subclass(cl, ptr->npla[td.Token[i] - 1]) < max_comp)
            max_comp = get_static_subclass(cl, ptr->npla[td.Token[i] - 1]);

    if (get_static_subclass(cl, ptr->npla[comp - 1]) == max_comp)
        return TRUE;

    return FALSE;

}

void
MAX_COMP_FUNCT(Token_Domains td, int comp,
               int cl, Event_p list,
               Event_p *sat, Event_p *notsat) {
    Event_p ptr = list, next ;
    (*sat) = (*notsat) = NULL;
    while (ptr) {
        next = ptr->next;

        if (max(td, comp, cl, ptr))
            add_to_ev_list(sat , ptr);
        else
            add_to_ev_list(notsat , ptr);

        ptr = next;
    }

}

void
INCLUSION_FUNCT_bis(int comp, int cl,
                    CToken_p fonct ,
                    PART_MAT_P mark_part,
                    Event_p list,
                    Event_p *sat,
                    Event_p *notsat) {
    Event_p ptr = list, next ;
    int i, sbc = UNKNOWN;
    TYPE_P var = (TYPE_P)AllocateBV();
    (*sat) = (*notsat) = NULL;

    while (ptr) {
        next = ptr->next;
        for (i = 0; i < (fonct->tok_dom).NbCl; i++) {
            Add(var, (fonct->tok_dom).Token[i]) ;

            if (!INCLUDE(var ,
                         mark_part->PART_MAT[cl]
                         [get_static_subclass(cl, ptr->npla[comp - 1]) ]))
                add_to_ev_list(notsat , ptr);
            else
                add_to_ev_list(sat , ptr);

            VideC(var);
        }
        ptr = next;
    }
    free(var);
}

void
INCLUSION_FUNCT(int comp, int cl, CToken_p fonct ,
                PART_MAT_P mark_part, Event_p list,
                Event_p *sat, Event_p *notsat) {
    Event_p ptr = list, next ;
    int i, sbc = UNKNOWN;
    TYPE_P var = (TYPE_P)AllocateBV();
    (*sat) = (*notsat) = NULL;

    while (ptr) {
        next = ptr->next;

        for (i = 0; i < (fonct->tok_dom).NbCl; i++)
            Add(var, (fonct->tok_dom).Token[i]) ;

        if (!INCLUDE(mark_part->PART_MAT[cl]
                     [get_static_subclass(cl, ptr->npla[comp - 1])],
                     var))
            add_to_ev_list(notsat , ptr);

        else {
            add_to_ev_list(sat , ptr);

        }
        VideC(var);
        ptr = next;
    }
    free(var);
}

void
STATIC_EQUA_FUNCT(int comp1, int comp2, int cl,
                  Event_p list,
                  Event_p *sat, Event_p *notsat) {

    Event_p ptr = list, next;
    int sbc = UNKNOWN;
    while (ptr) {
        if (get_static_subclass(cl, ptr->npla[comp1 - 1]) !=
                get_static_subclass(cl, ptr->npla[comp2 - 1]))
            add_to_ev_list(notsat , ptr);
        else
            add_to_ev_list(sat , ptr);
        ptr = next;
    }
}

void
treat_guard(TreeNode_p root, Event_p list,
            Event_p *sat, Event_p *notsat,
            PART_MAT_P mark_part) {

    Event_p  satg = NULL, notsatg = NULL,
             satd = NULL, notsatd = NULL;

    (* sat) = (* notsat) = NULL;

    switch (root->type) {

    case NOTYPE :

        switch (root->op) {
        case ANDOP :
            treat_guard(root->filsg , list , & satg, & notsatg, mark_part);
            if (satg) {
                treat_guard(root->filsd , satg , & satd, & notsatd , mark_part);
                (*sat) = satd; (*notsat) = join_two_lists(notsatg, notsatd);
            }
            else
                (*notsat) = notsatg ;

            break;
        case OROP  :
            treat_guard(root->filsg , list , & satg, & notsatg, mark_part);
            if (notsatg)
                treat_guard(root->filsd , notsatg, &satd, & notsatd , mark_part);
            (*sat) = join_two_lists(satg, satd); (*notsat) = notsatd;
            break;

        }
        break;

    case MAXIMUM  :
        MAXIMUM_FUNCT(root->op, tabt[list->trans].dominio[(root->op) - 1],
                      list, sat, notsat);
        break;

    case MAX_COMP :
        MAX_COMP_FUNCT(root->FONCT->tok_dom , root->FONCT->mult,
                       tabt[list->trans].dominio[(root->FONCT->mult) - 1], list,
                       sat, notsat)  ;
        break;

    case INCLUDED :
        INCLUSION_FUNCT(root->FONCT->mult, tabt[list->trans].dominio[(root->FONCT->mult) - 1],
                        root->FONCT, mark_part, list, sat, notsat);
        break;

    case DF      :
        STATIC_EQUA_FUNCT((root->FONCT->tok_dom).Token[0],
                          (root->FONCT->tok_dom).Token[1],
                          tabt[list->trans].dominio[(root->FONCT->tok_dom).Token[0] - 1] ,
                          list, sat, notsat);
        break;

    }
}


Result_p
trait_gurded_transitions(Result_p enabled_head,
                         PART_MAT_P mark_part) {

    Result_p enabled, prec, cur, head;
    Event_p sat;
    Event_p notsat;


    enabled = prec = head = enabled_head;

    while (enabled) {
        if (tabt[enabled->list->trans].dyn_guard) {
            treat_guard(tabt[enabled->list->trans].dyn_guard,
                        enabled->list, &sat, &notsat, mark_part);
            free_ev_list(notsat);
            enabled->list = sat;
        }

        cur = enabled->next;

        if (!enabled->list) {
            if (enabled == head)
                head = prec = cur;
            else
                prec->next = cur;
            push_result(enabled);
        }
        else
            prec = enabled;

        enabled = cur;

    }

// this section re-orders enabling
// list according to a symmetry property

    Result_p sym = NULL, asym = NULL,
             ptr = head, swap = NULL,
             last = NULL;

    while (ptr) {
        swap = ptr->next;

        if (tabt[ptr->list->trans].dyn_guard) {
            ptr->next = asym;
            asym = ptr;
        }
        else {
            if (!sym) last = ptr;
            ptr->next = sym;
            sym = ptr;
        }

        ptr = swap;
    }

    if (last) last->next = asym;
    if (sym) head = sym; else head = asym;


    return head;
}


int adjust_priority(int pri , Result_p enabled) {

    //int prio=pri;
    int prio = tabt[enabled->list->trans].pri ;
    Result_p ptr = enabled;
    while (ptr) {
        if (prio < tabt[ptr->list->trans].pri)
            prio = tabt[ptr->list->trans].pri;
        ptr = ptr->next;
    }
    return prio;
}

