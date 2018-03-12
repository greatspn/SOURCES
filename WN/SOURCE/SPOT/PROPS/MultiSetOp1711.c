#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/SCONSSPOT.h"
#include "../../../INCLUDE/struct.h"
#include  "../../../INCLUDE/SSTRUCTSPOT.h"
#include "../../../INCLUDE/var_ext.h"
#include  "../../../INCLUDE/SVAR_EXTSPOT.h"
#include "../../../INCLUDE/macros.h"

extern void SymRepOfToken();
extern int TestSymRepWtToken();
extern void DynDistOfObjects();

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

                (*ptr)->tok_dom.Token = realloc((*ptr)->tok_dom.Token, (c + 1) * sizeof(int));
                (*ptr)->tok_dom.Token[c] = marking->id[i];
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
            (*ptr)->tok_dom.Token = realloc((*ptr)->tok_dom.Token, (i + 1) * sizeof(int));
            (*ptr)->tok_dom.Token[i] = marking->id[i];

        }
        ptr = &((*ptr)->next);
        marking = marking->next;
    }
    return head;
}

CToken_p SymRepOfTokenList(CToken_p Token_p, int **DynDis) {
    CToken_p *Arr = NULL;
    CToken_p *pt;
    CToken_p ptr = Token_p, Head = NULL, tmp = NULL;

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
                if ((Arr[j] != NULL) && (TestSymRepWtToken(Arr[i]->tok_dom, Arr[j]->tok_dom, DynDis) == true))
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

CToken_p MultSetsAndOp(CToken_p MultSet1, CToken_p MultSet2) {
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
                if (memcmp(Arr[i]->tok_dom.Token, Arr[j]->tok_dom.Token, Arr[i]->tok_dom.NbCl * sizeof(int)) == 0) {
                    Arr[i]->mult = min(Arr[i]->mult, Arr[j]->mult);
                    free(Arr[j]);
                    Arr[j] = NULL;
                    flag = true;
                }

        if (!flag) {
            free(Arr[i]);
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
            if (memcmp(Arr[i]->tok_dom.Token, Arr[j]->tok_dom.Token, Arr[i]->tok_dom.NbCl * sizeof(int)) == 0) {
                Arr[i]->mult = Arr[i]->mult + Arr[j]->mult;
                free(Arr[j]);
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
            if (Arr[j])
                if (memcmp(Arr[i]->tok_dom.Token, Arr[j]->tok_dom.Token, Arr[i]->tok_dom.NbCl * sizeof(int)) == 0 && Arr[i]->mult == Arr[j]->mult) {
                    free(Arr[i]); free(Arr[j]);
                    Arr[i] = Arr[j] = NULL;
                    flag = true;
                }

    }
    flag = false;
    for (i = 0; i < nMs2; i++)
        if (Arr[i]) {
            flag = true;
            free(Arr[i]);
        }

    free(Arr);
    if (flag) return false;
    else return true  ;
}

int MultSetNEQOp(CToken_p MultSet1, CToken_p MultSet2) {
    if (MultSetEQOp(MultSet1, MultSet2))return false;
    else return true;
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
            if (Arr[j])
                if (memcmp(Arr[i]->tok_dom.Token, Arr[j]->tok_dom.Token, Arr[i]->tok_dom.NbCl * sizeof(int)) == 0 && Arr[i]->mult <= Arr[j]->mult) {
                    free(Arr[i]); free(Arr[j]);
                    Arr[i] = Arr[j] = NULL;
                    flag = true;
                }
    }

    flag = false;
    for (i = 0; i < nMs2; i++)
        if (Arr[i]) {
            free(Arr[i]);
            if (i < nMs1) flag = true;
        }
    free(Arr);

    if (flag) return false;
    else return true  ;

}
int MultSetNLTEQOp(CToken_p MultSet1, CToken_p MultSet2) {
    if (MultSetLTEQOp(MultSet1, MultSet2)) return false;
    else return true;
}

int MultSetGTEQOp(CToken_p MultSet1, CToken_p MultSet2) {

    if (MultSetLTEQOp(MultSet2, MultSet1)) return true;
    else return false ;
}


int MultSetNGTEQOp(CToken_p MultSet1, CToken_p MultSet2) {
    if (MultSetGTEQOp(MultSet1, MultSet2)) return false;
    else return true;

}

int MultSetLTOp(CToken_p MultSet1, CToken_p MultSet2) {
    if (MultSetLTEQOp(MultSet1, MultSet2) == true &&  MultSetNGTEQOp(MultSet1, MultSet2) == true)
        return true;
    else return false;
}

int MultSetNLTOp(CToken_p MultSet1, CToken_p MultSet2) {
    if (MultSetEQOp(MultSet1, MultSet2) == true ||  MultSetNLTEQOp(MultSet1, MultSet2) == true)
        return true;
    else return false;
}

int MultSetGTOp(CToken_p MultSet1, CToken_p MultSet2) {
    if (MultSetGTEQOp(MultSet1, MultSet2) == true &&  MultSetNLTEQOp(MultSet1, MultSet2) == true)
        return true;
    else return false;
}

int MultSetNGTOp(CToken_p MultSet1, CToken_p MultSet2) {
    if (MultSetEQOp(MultSet1, MultSet2) == true ||  MultSetNGTEQOp(MultSet1, MultSet2) == true)
        return true;
    else return false;
}

CToken_p ParcourtArbreProp(TreeNode_p root, int **DynDis) {
    CToken_p ptr1, ptr2;
    if (root->op != NOP) {
        ptr1 = ParcourtArbreProp(root->filsg, DynDis);
        ptr2 = ParcourtArbreProp(root->filsd, DynDis);
        switch (root->op) {
        case ANDOP: return  MultSetsAndOp(ptr1, ptr2);
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








int Satisfaction(int *_CONS_, int Nb_CONS_, TYPE_P **PM, TYPE *NbElPM, int *** Card, int **num, int **DynDis) {
    int i, OP;
    CToken_p ptr1, ptr2;

    DynDistOfObjects(PM, NbElPM, Card, num, DynDis);

    for (i = 0; i < Nb_CONS_ ; i++)
        if (_CONS_[i] != -1) {
            ptr1 = ParcourtArbreProp(PROPOSITIONS[i].prop->filsg, DynDis);
            ptr2 = ParcourtArbreProp(PROPOSITIONS[i].prop->filsd, DynDis);
            OP = PROPOSITIONS[i].prop->op;

            if (_CONS_[i] == 0)
                switch (OP) {
                case EQOP : OP = NEQOP; break;
                case LQOP : OP = NLQOP; break;
                case LOP  : OP = NLOP; break;
                case GQOP : OP = NGQOP; break;
                case GOP  : OP = NGOP; break;
                }

            switch (OP) {
            case EQOP  :   if (!MultSetEQOp(ptr1, ptr2))    return false; break;
            case LQOP  :   if (!MultSetLTEQOp(ptr1, ptr2))  return false; break;
            case LOP   :   if (!MultSetLTOp(ptr1, ptr2))    return false; break;
            case GQOP  :   if (!MultSetGTEQOp(ptr1, ptr2))  return false; break;
            case GOP   :   if (!MultSetGTOp(ptr1, ptr2))    return false; break;
            case NEQOP :   if (!MultSetNEQOp(ptr1, ptr2))   return false; break;
            case NLQOP :   if (!MultSetNLTEQOp(ptr1, ptr2)) return false; break;
            case NLOP  :   if (!MultSetNLTOp(ptr1, ptr2))   return false; break;
            case NGQOP :   if (!MultSetNGTEQOp(ptr1, ptr2)) return false; break;
            case NGOP  :   if (!MultSetNGTOp(ptr1, ptr2))   return false; break;
            }
        }


}
return true;
}
