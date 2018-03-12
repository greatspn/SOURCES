/***************************************************************************
 *   Copyright (C) 2006 by Marco Beccuti   *
 *   beccuti@mfn.unipmn.it   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include "class.h"


extern int DTMC;
extern int STRONG;
extern int ESRGtoUNIVERSO;
extern int CTMC_GEN;
extern int PEAK;
int call_instance_macro = 0;

extern Q_list *q;

// To treat the problem of
// double representations
// of markings
extern char cache_string [];
typedef struct Mark {
    Marking *event;
    char *marking;
    int length;

} MarkingStruct;

MarkingStruct *tabtemp = NULL;
int tabsize = 0;
////////

inline  int get_min(int first, int second) {
    return ((first <= second) ? first : second);
}
inline  int get_max(int first, int second) {
    return ((first >= second) ? first : second);
}

Q_list_el  **elem_set_index = NULL;
Marking    **sym_rep       = NULL;
Marking    **events        = NULL;
Instance   **ingeneric     = NULL;
Instance   **outgeneric    = NULL;

int size_comp = 0, nb_esms = 0,
    cmp_stored_ev = 0,
    nb_events = 0;

char net[MAXSTRING];
char cmd[MAXSTRING];

ifstream esrg, off_sr;
fstream off_ev;



void test(Q_list_el *&qblock) {
    cout << "\nStart\n";
    while (qblock != NULL) {
        cout << "\tElements number before split:" << qblock->get_num_elem() << endl;
        instance_macro(qblock->get_list_pmark());
        cout << "\tElements number after split" << qblock->get_num_elem() << endl;
        qblock = qblock->get_pointer(nxt);
    }
    cout << "End\n";
    exit(1);
}
/*******************************************/

Marking *sr;




Instance::Instance() {

    this->weight = 0;
    this->trans = 0;
    this->card = 0;
    this->col = NULL;
    this->des = NULL;
    this->next = NULL;
}

Instance::Instance(const Instance &ins) {

}

Instance::Instance(double weight, int trans, int card, int **col, Marking *dest) {

    this->weight = weight;
    this->trans = trans;
    this->card = card;
    this->col = col;
    this->des = dest;
    this->next = NULL;
}

Instance::Instance(double weight, int trans, int card, int **col) {

    this->weight = weight;
    this->trans = trans;
    this->card = card;
    this->col = col;
    this->des = NULL;
    this->next = NULL;


}
Instance::~Instance() {

//  if(  tabt[trans].comp_num )
//     {
//       delete []  col[0];
//       delete []  col[1];
//       delete []  col;
//     }

}


inline double Instance:: get_weight() const {
    return  weight;
}
inline int Instance:: get_trans() const {
    return trans;
}
inline int Instance:: get_card() const {
    return card;
}
inline int  **Instance:: get_col() const {
    return col;
}
inline Marking  *Instance:: get_des() const {
    return  des;
}
inline Instance  *Instance:: get_next() const {
    return next;
}
inline double &Instance::set_weight() {
    return weight;
}
inline int  &Instance::set_trans() {
    return trans;
}
inline void Instance::set_col(int **co) {
    col = co;
}
inline void Instance::set_des(Marking *m) {
    des = m;
}
inline void Instance :: set_next(Instance *ins) {
    next = ins;
}
inline int &Instance::set_card() {
    return card;
}


Marking::Marking() {
    type = Unk;
    in_inst = in_gen = out_inst = out_gen = NULL;
//11/7	Q_list_el*  part_q=NULL;
    part_q = NULL;
    next = prev = next_ev = NULL;
    denom = 1;
}

Marking:: Marking(Q_list_el *el, int i,
                  unsigned long srpos,
                  unsigned long srleng,
                  unsigned long srptr,
                  unsigned long evpos,
                  unsigned long evleng,
                  Type ty) {

    id = i;
    type = ty;
    part_q = el;
    in_inst = in_gen = out_inst = out_gen = NULL;
    next = prev = next_ev = NULL;
    file_sr = srpos;
    file_srlg = srleng;
    file_srptr = srptr;
    file_ev = evpos;
    file_evlg = evleng;
    denom = 1;


}

Marking::Marking(const Marking &m) {

}

Marking::~Marking() {


}

inline int  Marking::get_id() const {
    return id;
}

inline unsigned long Marking::  get_srpos() const {
    return file_sr;
};
inline unsigned long Marking::  get_srlength() const {
    return file_srlg;
};
inline unsigned long  Marking:: get_srptr() const {
    return file_srptr;
};

inline unsigned long  Marking:: get_evpos() const {
    return file_ev;
};

inline unsigned long Marking::  get_evlength() const {
    return file_evlg;
};


inline Type Marking::get_type() const {
    return type;
}

Instance  *Marking:: get_arc(Ins_Type is_t) const {

    switch (is_t) {
    case  in_g  : return this->in_gen; break;
    case  out_g : return this->out_gen; break;
    case  in_i  : return this->in_inst; break;
    case  out_i : return this->out_inst; break;
    default:
        printf("Error type does not define\n");
        exit(1);
    }
}

inline Q_list_el *&Marking::get_q_part() {
    return  part_q;
}

Marking   *Marking::get_pointer(Ptr_Type pt_t) const {

    switch (pt_t) {
    case nxt  :  return next; break;
    case prv  :  return prev; break;
    case  nxt_sb :  return next_ev; break;
    default:
        printf("Error type does not define\n");
        exit(1);
    }
}


inline void   Marking:: set_id(int i,
                               unsigned long srpos,
                               unsigned long srleng,
                               unsigned long srptr,
                               unsigned long evpos,
                               unsigned long evleng) {
    id = i;
    file_sr = srpos;
    file_srlg = srleng;
    file_srptr = srptr;
    file_ev = evpos;
    file_evlg = evleng;
}





inline void   Marking:: set_type(Type ty) {
    type = ty;
}
void  Marking:: set_arc(Instance *ins, Ins_Type is_t) {

    switch (is_t) {
    case    in_g  : in_gen = ins; break;
    case    out_g : out_gen = ins; break;
    case    in_i  : in_inst = ins;  break;
    case    out_i : out_inst = ins; break;
    default:
        printf("Error type does not define\n");
        exit(1);
    }

}

inline void Marking :: set_q_part(Q_list_el *el) {
    part_q = el;
}


void Marking ::set_pointer(Marking *m, Ptr_Type pt_t) {

    switch (pt_t) {
    case nxt  : next = m; break;
    case prv  : prev = m; break;
    case  nxt_sb : next_ev = m; break;
    default:
        printf("Error type does not define\n");
        exit(1);
    }

}

Q_list_el::Q_list_el() {
    num_elem = 0;
#ifdef debug
    tot_num_elem = 0;
#endif
    pmark = NULL;
    part_x = NULL;
    next = NULL;
    prev = NULL;
    next_subset = NULL;
}

Q_list_el::Q_list_el(int i) {
    id = i;
    num_elem = 0;
#ifdef debug
    tot_num_elem = 0;
#endif
    pmark = NULL;
    part_x = NULL;
    next = NULL;
    prev = NULL;
    next_subset = NULL;
}

Q_list_el::Q_list_el(int i, Marking *m) {
    id = i;
    num_elem = 0;
#ifdef debug
    tot_num_elem = 0;
#endif
    pmark = m;
    part_x = NULL;
    next = NULL;
    prev = NULL;
    next_subset = NULL;
}

Q_list_el::Q_list_el(Marking *m, int i) {
    id = 0;
#ifdef debug
    tot_num_elem = 0;
#endif
    num_elem = i;
    pmark = m;
    part_x = NULL;
    next = NULL;
    prev = NULL;
    next_subset = NULL;
}

inline int Q_list_el::get_num_elem() const {
    return  num_elem;
}

inline int Q_list_el::get_id() const {
    return id;
}

inline void   Q_list_el::set_id(int i) {
    id = i;
}


inline Marking *Q_list_el::get_list_pmark() const {
    return  pmark;
}




Q_list_el *Q_list_el::get_pointer(Ptr_Type pt_t) const {

    switch (pt_t) {
    case nxt  :  return next; break;
    case prv  :  return prev; break;
    case  nxt_sb :  return next_subset; break;
    default:
        printf("Error type does not define\n");
        exit(1);
    }
}

inline void Q_list_el:: set_num_elem(int num) {
    num_elem = num;
}



inline void  Q_list_el::num_elem_plusplus() {
    num_elem++;
}

inline X_list_el *Q_list_el::get_partx() const {
    return part_x;
}

// void  Q_list_el::discard(Marking* macro){

//   Marking* pt=pmark;

//   this->pmark=pmark->get_pointer(nxt);
//   (this->pmark)->set_pointer(NULL,prv);
//   this->num_elem--;
//   pt->set_type(Dis);
// }


inline void  Q_list_el::set_pmark(Marking *m) {
    pmark = m;
}



#ifdef debug
inline int Q_list_el::get_tot_num_elem() const {
    return tot_num_elem;
}
inline void Q_list_el::set_tot_num_elem(int num) {
    tot_num_elem = num;
}
#endif



void  Q_list_el::add_pmark_top(Marking &m, Type ty) {
    m.set_pointer(pmark, nxt);
    pmark = &m; pmark->set_type(ty);
}


void Q_list_el::add_pmark(Marking &m, Type ty) {

    m.set_type(ty);

    if (pmark) {
        m.set_pointer(pmark->get_pointer(nxt), nxt);
        m.set_pointer(pmark, prv);
        if (pmark->get_pointer(nxt) != NULL)
            (pmark->get_pointer(nxt))->set_pointer(&m, prv);
        pmark->set_pointer(&m, nxt);
    }
    else
        pmark = &m;
}

void Q_list_el::add_pmark(Marking  *m) {

    if (pmark) {
        m->set_pointer(pmark->get_pointer(nxt), nxt);
        m->set_pointer(pmark, prv);
        if (pmark->get_pointer(nxt) != NULL)
            (pmark->get_pointer(nxt))->set_pointer(m, prv);
        pmark->set_pointer(m, nxt);
    }
    else
        pmark = m;
    m->set_q_part(this);

}

void Q_list_el::set_pointer(Q_list_el *el, const Ptr_Type pt_t) {

    switch (pt_t) {

    case nxt :  this->next = el; break;
    case prv :  this->prev = el; break;
    case nxt_sb : this-> next_subset = el; break;

    }

}





inline void Q_list_el::set_partx(X_list_el *x_block) {
    part_x = x_block;
}



void Q_list_el:: op_presplit(Marking *l_split, Q_list_el *t_block,
                             int level, X_list *xblock) {
}


Marking *Q_list_el::remove(int id, Type type) {

    Marking *m;
    m = this->get_list_pmark();
    int cicla = 1;
    while ((m) && (cicla)) {
        if ((m->get_id() == id) && (m->get_type() == type))
            cicla = 0;
        else
            m = m->get_pointer(nxt);
    }
    if (cicla == 1) return NULL;
    else {
        if (m->get_pointer(prv) != NULL) {
            //caso elemento non intesta
            m->get_pointer(prv)->set_pointer(m->get_pointer(nxt), nxt);
        }
        else {
            //caso elemento in testa
            m->get_q_part()->set_pmark(m->get_pointer(nxt));
        }

        if (m->get_pointer(nxt) != NULL) {
            m->get_pointer(nxt)->set_pointer(m->get_pointer(prv), prv);
        }
        this->num_elem--;
    }
    //setto i puntatori a nul dell'elemento perche viene tolto dalla lista
    m->set_pointer(NULL, nxt);
    m->set_pointer(NULL, prv);
    return m;
}



void Q_list_el::presplit_rateout(map <int, double> &m_out, Ins_Type type) {

    Marking *tmp_marking;
/// tmp_marking=this->pmark;
    if (this->pmark->get_type() != INSTANCE)
        tmp_marking = this->pmark->get_pointer(nxt);
    else
        tmp_marking = this->pmark;
    while (tmp_marking != NULL) {
        Instance *tmp_instance;
        tmp_instance = tmp_marking->get_arc(type);
        while (tmp_instance != NULL) {
            m_out[tmp_marking->get_id()] += tmp_instance->get_card() * tmp_instance->get_weight() / tmp_marking->get_denom();
            //m_out[tmp_marking->get_id()] +=tmp_instance->get_card()*tmp_instance->get_weight()*tmp_marking->get_denom();
            tmp_instance = tmp_instance->get_next();
        }
        tmp_marking = tmp_marking->get_pointer(nxt);
    }
}

void Q_list_el::presplit(list <int> &part_out, map <int, struct el> &m_out, Ins_Type type) {

    Marking *tmp_marking;
    /// tmp_marking=this->pmark;
    if (this->pmark->get_type() != INSTANCE)
        tmp_marking = this->pmark->get_pointer(nxt);
    else
        tmp_marking = this->pmark;
    while (tmp_marking != NULL) {
        Instance *tmp_instance;
        tmp_instance = tmp_marking->get_arc(type);
        while (tmp_instance != NULL) {
            if ((type != out_i) || ((type == out_i) && (tmp_instance->get_des())->get_q_part() != tmp_marking->get_q_part())) {
                part_out.push_back((tmp_instance->get_des())->get_q_part()->get_id());
                m_out[tmp_marking->get_id()].points[(tmp_instance->get_des())->get_q_part()->get_id()] =
                    m_out[tmp_marking->get_id()].points[(tmp_instance->get_des())->get_q_part()->get_id()] +
                    (tmp_instance->get_card() * tmp_instance->get_weight() / tmp_marking->get_denom());
            }
            /***********************************************************************************/
#if TEST
            cout << m_out[tmp_marking->get_id()].points[(tmp_instance->get_des())->get_q_part()->get_id()] <<
                 " - " << (tmp_instance->get_des())->get_q_part()->get_id() << endl;
#endif
            /***********************************************************************************/
            tmp_instance = tmp_instance->get_next();
        }
        tmp_marking = tmp_marking->get_pointer(nxt);
    }
    part_out.unique();
}


void Q_list_el::split_rateout(map <int, double> m_out, Q_list_el *&temp) {

    Marking *tmp_m, *ric_mark, *ric_tmp;
    int mark = this->get_list_pmark()->get_id();
    ric_mark = this->get_list_pmark()->get_pointer(nxt);
    while (ric_mark != NULL) {
        if (m_out[mark] != m_out[ric_mark->get_id()]) {
            if (temp == NULL) {
                temp = new Q_list_el;
            }
            ric_tmp = ric_mark->get_pointer(nxt);
            tmp_m = this->remove(ric_mark->get_id(), ric_mark->get_type());
            temp->add_pmark(tmp_m);
            temp->num_elem_plusplus();
            ric_mark = ric_tmp;
        }
        else
            ric_mark = ric_mark->get_pointer(nxt);
    }
}


void Q_list_el::split(list <int> &part_out , map <int, struct el> &m_out, Q_list_el *&temp) {

    list<int>::iterator iter;
    iter = part_out.begin();
    //Q_list_el  *temp=NULL;
    Marking *tmp_m, *ric_mark, *ric_tmp;
    int mark = this->get_list_pmark()->get_id();
    while ((iter != part_out.end()) && (this->num_elem != 1)) {
        ric_mark = this->get_list_pmark()->get_pointer(nxt);
        while (ric_mark != NULL) {
            if (m_out[mark].points[*iter] != m_out[ric_mark->get_id()].points[*iter]) {
                if (temp == NULL) {
                    temp = new Q_list_el;
                }
                ric_tmp = ric_mark->get_pointer(nxt);
                tmp_m = this->remove(ric_mark->get_id(), ric_mark->get_type());
                temp->add_pmark(tmp_m);
                temp->num_elem_plusplus();
                ric_mark = ric_tmp;

            }
            else
                ric_mark = ric_mark->get_pointer(nxt);
        }
        iter++;
    }
}


map<class Marking *, double> Q_list_el::creat_E_B(Ins_Type type, Ins_Type type1) {
    Marking *m = this->get_list_pmark();
    map <class Marking *, double> e_b;

    //NUOVO

    if (m->get_type() != INSTANCE) {
        instance_macro(m);
        exit(-1);
    }
    //NUOVO
    while (m) {
        Instance *i = m->get_arc(type1);
        //if(m->get_arc(type)==NULL)
        while (i) {
            //NON BELLO COME SOLUZIONE PER VEDERE SE GIA instanziato
            if ((i->get_des()->get_pointer(nxt_sb) == NULL) ||
                    (i->get_des()->get_pointer(nxt_sb)->get_q_part()->get_list_pmark()->get_type() != INSTANCE)) {
                instance_macro((i->get_des()));
                if (DTMC)
                    Compute_denom(i->get_des()->get_q_part());
                //***********
                //Marking *tmp=instVSmacro[m->get_id()];
                //s post_treatement(i->get_des());
                //**************
            }
            m->set_arc(i->get_next(), type1);
            //delete(i);
            i = m->get_arc(type1);
        }
        i = m->get_arc(type);
        while (i) {
            if ((!STRONG) && (i->get_des()->get_q_part() != NULL) && (i->get_des()->get_q_part()->get_list_pmark() != NULL))
                e_b[i->get_des()] = e_b[i->get_des()] + (i->get_card() * i->get_weight()) / m->get_denom();
            if ((!STRONG) && (i->get_des()->get_q_part() != NULL) && (i->get_des()->get_q_part()->get_list_pmark() != NULL) && (i->get_des()->get_q_part()->get_list_pmark()->get_type() != INSTANCE)) {
                instance_macro(i->get_des()->get_q_part()->get_list_pmark());
                if (DTMC)
                    Compute_denom(i->get_des()->get_q_part());
            }
            i = i->get_next();
        }
        m = m->get_pointer(nxt);
    }
    return e_b;
}

Q_list :: Q_list() {
    head = NULL;
    num_elem = 0;
}

Q_list ::Q_list(const std::string &str) {

}




void Q_list::insert_elem(Q_list_el *const &el) {
    if (head == NULL) {
        head = el;
    }
    else {
        head->set_pointer(el, prv);
        el->set_pointer(head, nxt);
        head = el;
    }
    num_elem++;
}


Q_list_el *Q_list::get_block() const {
    return head;
}

int Q_list::get_num_elem() const {
    return num_elem;
}
Marking *Q_list ::search(int dest) {

    Q_list_el *hd = head;
    Marking *m;

    while (hd) {

        m = hd->get_list_pmark();

        while (m) {
            if (m->get_id() == dest)
                return m;
            else
                m = m->get_pointer(nxt);
        }

        hd = hd-> get_pointer(nxt);
    }
    return NULL;
}

Marking *Q_list ::search(int dest, int trans, double
                         weight, Marking *p_elem) {
    return NULL;
    // to be written
}

void Q_list::removeI(int id) {

    Q_list_el *tmp;
    int fine = 0;
    if (head->get_id() == id) {
        if (head->get_pointer(nxt) != NULL)
            (head->get_pointer(nxt))->set_pointer(NULL, prv);
        tmp = head;
        head = head->get_pointer(nxt);
        num_elem--;
        delete(tmp);
    }
    else {
        Q_list_el *hd = head->get_pointer(nxt);
        while ((hd) && !(fine)) {
            if (hd->get_id() == id) {
                fine = 1;
                tmp = hd;
                if (hd->get_pointer(nxt) != NULL)
                    (hd->get_pointer(nxt))->set_pointer(hd->get_pointer(prv), prv);
                if (hd->get_pointer(prv) != NULL)
                    (hd->get_pointer(prv))->set_pointer(hd->get_pointer(nxt), nxt);
                else
                    hd = hd->get_pointer(nxt);
                num_elem--;
                delete(tmp);
            }
            else
                hd = hd-> get_pointer(nxt);
        }
    }
}
void Q_list::remove(Q_list_el *&el) {
    if (el->get_pointer(prv) == NULL) {
        if (head->get_pointer(nxt) != NULL)
            (head->get_pointer(nxt))->set_pointer(NULL, prv);
        head = head->get_pointer(nxt);
        num_elem--;
        //delete(el);
    }
    else {
        if (el->get_pointer(nxt) != NULL)
            (el->get_pointer(nxt))->set_pointer(el->get_pointer(prv), prv);
        if (el->get_pointer(prv) != NULL)
            (el->get_pointer(prv))->set_pointer(el->get_pointer(nxt), nxt);
        num_elem--;
        //delete(el);
    }
}


int  Q_list ::lumpability() {
    return 0;
}

Instance  *Q_list ::  creat_E_B() {
    return NULL;
}


X_list_el::X_list_el() {
    num_elem = 0;
    id = 0;
    subset = NULL;
    next = NULL;
    prev = NULL;
}


X_list_el::X_list_el(int i, Q_list_el *m) {
    id = i;
    num_elem = 1;
    subset = m;
    next = NULL;
    prev = NULL;
}



inline int X_list_el:: get_num_elem() const {
    return  num_elem;
}

inline int X_list_el::get_id() const {
    return id;
}


inline Q_list_el *X_list_el::get_subset() const {
    return  subset;
}



X_list_el *X_list_el::get_pointer(Ptr_Type pt_t) const {

    switch (pt_t) {
    case nxt  :  return next; break;
    case prv  :  return prev; break;
    case  nxt_sb : exit(1); break;
    default:
        printf("Error type does not define\n");
        exit(1);
    }
}





inline void X_list_el:: set_num_elem(int num) {
    num_elem = num;
}


inline void  X_list_el::num_elem_plusplus() {
    num_elem++;
}


void X_list_el::set_subset(Q_list_el *el) {
    subset = el;
}


void X_list_el::add_subset(Q_list_el *&m) {


    if (subset) {
        m->set_pointer(subset->get_pointer(nxt_sb), nxt_sb);
        subset->set_pointer(m, nxt_sb);
    }
    else
        subset = m;
    num_elem++;
}




void X_list_el::set_pointer(X_list_el *el, const Ptr_Type pt_t) {

    switch (pt_t) {

    case nxt :  this->next = el; break;
    case prv :  this->prev = el; break;
    case nxt_sb : exit(1); break;

    }

}


void  X_list_el::remove(int id) {

    Q_list_el *m;

    int cicla = 1;
    if ((this->get_subset())->get_id() == id) {
        this->set_subset((this->get_subset())->get_pointer(nxt_sb));
        this->num_elem--;
    }
    else {
        m = this->get_subset();
        while ((m->get_pointer(nxt_sb)) && (cicla)) {
            if (m->get_pointer(nxt_sb)->get_id() == id) {
                cicla = 0;
                //if (m->get_pointer(nxt_sb)->get_pointer(nxt_sb)!=NULL)
                m->set_pointer(m->get_pointer(nxt_sb)->get_pointer(nxt_sb), nxt_sb);
                //else
                // m->set_pointer(NULL,nxt_sb);
                this->num_elem--;
            }
            else
                m = m->get_pointer(nxt_sb);
        }

    }
}



X_list :: X_list() {
    head = NULL;
    num_elem = 0;

}


inline int X_list:: get_num_elem() const {
    return  num_elem;
}

inline int X_list::get_num_elem_plus() {
    return ++num_elem;
}


void X_list::insert_elem(X_list_el *const  &el) {
    if (head == NULL) {
        head = el;
    }
    else {
        head->set_pointer(el, prv);
        el->set_pointer(head, nxt);
        head = el;
    }
//  num_elem++;
}

C_list::C_list() {
    next = NULL;
    subset = NULL;
}

C_list::C_list(X_list_el  *m) {
    next = NULL;
    subset = m;
}

void C_list::insert_elem(X_list_el *const  &el) {
    subset = el;
}

void insert_block(C_list *&list, C_list *const  &el) {

    if (list == NULL)
        list = el;
    else
        el->set_pointer(list);
    list = el;

}

inline C_list *C_list::get_pointer()const {
    return next;
}

inline X_list_el *C_list::get_subset()const {
    return subset;
}

inline void C_list::set_pointer(C_list *&m) {
    next = m;
}

C_list *top(C_list *&c) {
    C_list *tmp = c;
    if (c != NULL) {
        c = c->get_pointer();
        //free del(c);
    }
    return tmp;
}




void remove_x(int id, C_list *&c) {

    //X_list_el* m,*tmp_m;
    C_list *m, *tmp;
    int cicla = 1;
    if (c != NULL) {
        if (c->get_subset()->get_id() == id) {
            tmp = c;
            c = c->get_pointer();
            delete(tmp);
        }
        else {
            m = c;
            while ((m->get_pointer()) && (cicla)) {
                if (m->get_pointer()->get_subset()->get_id() == id) {
                    cicla = 0;
                    tmp = (m->get_pointer());
                    (*m).next = m->get_pointer()->get_pointer();
                    //else
                    // m->set_pointer(NULL,nxt_sb);
                    delete(tmp);
                }
                else
                    m = m->get_pointer();
            }

        }
    }
}



/*************************************************************************/




// This function parses the string corresponding to an instance of transition
Instance *parse_binding(istringstream &str ,
                        int *re_sr_id,
                        unsigned long *srpos,
                        unsigned long *srlg,
                        unsigned long 	*srptr,
                        int *re_ev_id,
                        unsigned long *evpos,
                        unsigned long 	*evlg) {
    Instance *ins = new Instance();
    int **col;
    int i;

    // parses the transtion id
    str >> ins->set_trans();
    //std::cout<< "str:"<<str<< std::endl;
    int comp_num = tabt[ins->get_trans()].comp_num;
    //std::cout<< " comp_num :" <<comp_num << std::endl;
    //std::cout<< " trans:" <<ins->get_trans() << std::endl;
    // getchar();
    // init. the weight of the transtion
    ins->set_weight() = tabt[ins->get_trans()].mean_t;

    if (comp_num) {
        col    = new int *[2] ;
        col[0] =  new  int[comp_num];
        col[1] =  new  int[comp_num];
        // parses the binding
        for (i = 0; i < comp_num; i++)
            str >> col[0][i] >> col[1][i];;
        // parses the number of ordinary instances
        //str >> ins->set_card() ;
        ins->set_col(col);
    }
    // parses the id of the reached sr and event
    str >> ins->set_card() >> (*re_sr_id) >> (*srpos) >> (*srlg) >> (*srptr) >> (*re_ev_id) >> (*evpos) >> (*evlg);
    return ins;
}

// initialise the greatSPN structures
void initial_global_variables(const char *net_name) {
    char **argv;
    argv = new char *[2];
    argv[1] = (char *) net_name;
    initialize(2, argv);

    INIT_GLOBAL_VARS();
    STORE_STATIC_CONF(&ASYM_STATIC_STORE, tabc);
    MERG = TO_MERGE(ncl, tabc);
    GROUPING_ALL_STATICS(MERG, tabc, num);
    NEW_SIM_STATIC_CONF(& SYM_STATIC_STORE);
    MY_INIT_ALLOC();

    delete [](argv);

}

// according to the  "re_sr_id" identifier,
// "add_sym" adds a new symmetric representation
// to the "sym_rep" array.
Q_list_el *add_sym(int re_sr_id,
                   unsigned long srpos,
                   unsigned long srlg,
                   unsigned long srptr) {
    Q_list_el *qu_el = NULL;

    if (!elem_set_index[re_sr_id]) {
        qu_el = elem_set_index[re_sr_id] =
                    new Q_list_el(re_sr_id);
    }
    else
        qu_el = elem_set_index[re_sr_id];

    qu_el->num_elem_plusplus();
    sym_rep[re_sr_id] = new Marking(qu_el, re_sr_id, srpos, srlg, srptr, 0, 0, Unk);
    qu_el->add_pmark_top(*sym_rep[re_sr_id], Unk);

    return qu_el;
}

// according to the "re_sr_id" and  the "re_ev_id" identifiers,
// "add_event" adds a new eventuality
// to the "events" array.
Q_list_el *add_event(int re_sr_id,
                     unsigned long srpos,
                     unsigned long srlg,
                     unsigned long srptr,
                     int re_ev_id,
                     unsigned long evpos,
                     unsigned long evlg) {
    Q_list_el *qu_el = NULL;

    if (!elem_set_index[re_sr_id])
        qu_el = elem_set_index[re_sr_id] =
                    new Q_list_el(re_sr_id);
    else
        qu_el = elem_set_index[re_sr_id];

    qu_el->num_elem_plusplus();

    events[re_ev_id] = new Marking(qu_el,
                                   re_ev_id, srpos, srlg, srptr,
                                   evpos, evlg,
                                   INSTANCE);

    // The ESM is saturated ==>
    // there is a MACROx node.
    if (sym_rep[re_sr_id]) {
        qu_el->add_pmark(*events[re_ev_id], INSTANCE);
        if (events[re_ev_id]->get_pointer(prv)) {
            events[re_ev_id]->get_pointer(prv)->set_pointer(events[re_ev_id], nxt_sb);
        }
        if (events[re_ev_id]->get_pointer(nxt)) {
            events[re_ev_id]->set_pointer(events[re_ev_id]->get_pointer(nxt), nxt_sb);
        }
    }
    // The ESM is'nt saturated.
    else {
        qu_el->add_pmark_top(*events[re_ev_id], INSTANCE);
        /*we have changed it in 21/6/07
        if (events[re_ev_id]->get_pointer(prv)!=NULL)
          {
           events[re_ev_id]->get_pointer(prv)->set_pointer(events[re_ev_id],nxt_sb);
           if  (events[re_ev_id]->get_pointer(nxt))
              {
               events[re_ev_id]->set_pointer(events[re_ev_id]->get_pointer(nxt),nxt_sb);
              }
           }*/
        if (events[re_ev_id]->get_pointer(nxt) != NULL) {
            events[re_ev_id]->set_pointer(events[re_ev_id]->get_pointer(nxt), nxt_sb);
            events[re_ev_id]->get_pointer(nxt)->set_pointer(events[re_ev_id], prv);
        }

    }
    if (re_ev_id >= size_comp) {

        events = (Marking **)realloc(events, (size_comp + 10) * sizeof(Marking **));

        size_comp += 10;
    }

    return qu_el;
}


// according to the "source" and the "reached" ESMs,
// "join_sr_to_sr" creats the correct arc
// joining their symmetrical representations (sr).
void join_sr_to_sr(int source , int reached,
                   Instance *arc, Instance *rev_arc) {
    arc->set_des(sym_rep[reached]);
    rev_arc->set_des(sym_rep[source]);

    arc->set_next(outgeneric[source]);
    sym_rep[source]->set_arc(arc, out_g);
    outgeneric[source] = sym_rep[source]->get_arc(out_g);

    rev_arc->set_next(ingeneric[reached]);
    sym_rep[reached]->set_arc(rev_arc, in_g);
    ingeneric[reached] = sym_rep[reached]->get_arc(in_g);

}

// according to the "source" and the "reached" ESMs,
// "join_ev_to_ev" creats the correct arc
// joining two from their eventualities (sr).
void join_ev_to_ev(int source , int reached,
                   Instance *arc, Instance *rev_arc) {
    arc->set_des(events[reached]);
    rev_arc->set_des(events[source]);

    arc->set_next(events[source]->get_arc(out_i));
    events[source]->set_arc(arc, out_i);

    rev_arc->set_next(events[reached]->get_arc(in_i));
    events[reached]->set_arc(rev_arc, in_i);
}

// according to the "source" and the "reached" ESMs,
// "join_ev_to_sr" creats the correct arc
// joining an eventuality (ev) of the "source" node
// to the symmetrical representation (sr) of the reached ESM.
void join_ev_to_sr(int source , int reached,
                   Instance *arc, Instance *rev_arc) {
    arc->set_des(sym_rep[reached]);
    rev_arc->set_des(events[source]);

    arc->set_next(events[source]->get_arc(out_i));
    events[source]->set_arc(arc, out_i);

    rev_arc->set_next(sym_rep[reached]->get_arc(in_i));
    sym_rep[reached]->set_arc(rev_arc, in_i);
}


// "add_arcs" treats the diffrent cases of arcs :
// ev --> ev, ev --> sr or sr --> sr.
void add_arcs(int source,
              int  re_sr_id,
              unsigned long srpos,
              unsigned long srlg,
              unsigned long srptr,
              int  re_ev_id,
              unsigned long evpos,
              unsigned long evlg,
              Instance *arc, Instance *rev_arc, int flag) {

    // Treating a generic arc("GEN").
    if (flag == GEN) {
        // if the reached ESM does'nt exist
        if (!sym_rep[re_sr_id])
            (void) add_sym(re_sr_id, srpos, srlg, srptr);

        // set the info. of the arcs. Here we join sr to sr.
        join_sr_to_sr(source, re_sr_id, arc, rev_arc);
    }

    // Treating an instantiated arc("INST").
    if (flag == INST) {
        // The source and the reached eventualities are defined.
        if (re_ev_id != Unk) {
            if (!events[re_ev_id])
                (void)add_event(re_sr_id,
                                srpos, srlg, srptr,
                                re_ev_id,
                                evpos, evlg);
            join_ev_to_ev(source , re_ev_id  , arc, rev_arc);
        }
        // The reached eventuality is not defined because of the
        // satuartion of the reached ESM.
        else {
            if (!sym_rep[re_sr_id])
                (void)add_sym(re_sr_id,
                              srpos, srlg, srptr);
            join_ev_to_sr(source , re_sr_id, arc, rev_arc);
        }
    }
}

// this function parses the binding string, creats the arcs and adds them
// in the structure (Q).
void parse_binding_and_add_(ifstream &f, int source, int nb_inst, Ins type) {
    int  re_sr_id , re_ev_id ;
    unsigned long srpos, srlg, srptr,
             evpos, evlg;
    Instance *arc;
    Instance *rev_arc;
    char str[MAXSTRING];
    int i;

    for (i = 0; i < nb_inst; i++) {
        f.getline(str, MAXSTRING - 1);
        istringstream st(str);
        arc = parse_binding(st , &re_sr_id ,
                            &srpos, & srlg, & srptr,
                            &re_ev_id ,
                            &evpos, &evlg);
        rev_arc = new Instance(arc-> get_weight(), arc->get_trans(),
                               arc->get_card(), arc->get_col())   ;
        add_arcs(source, re_sr_id ,
                 srpos, srlg, srptr,
                 re_ev_id,
                 evpos, evlg,
                 arc, rev_arc, type);
    }

}

// this function is used to treat the cases
// when the ESM is not initialy saturated and
// becomes by construction.
void treat_special_cases(int sr) {
//if(outgeneric[sr])
    //{
    Marking  *event =
        sym_rep[sr]->get_pointer(nxt);
    sym_rep[sr]->set_pointer(event, nxt_sb);
    if (event != NULL) {
        event->set_pointer(sym_rep[sr], prv);
        sym_rep[sr]->set_type(MACROP);
    }
    Marking *tmp_event = NULL;
    while (event) {
        event->set_arc(outgeneric[sr], out_g);
        tmp_event = event;
        event = event->get_pointer(nxt);
        if (event != NULL) {
            event->set_pointer(tmp_event, prv);
            if (tmp_event != NULL) {
                tmp_event->set_pointer(event, nxt_sb);
            }
        }
    }
    //}
}


//This function is used to close
//correctly all opened files.
void finalize_session() {
    esrg.close();
    off_sr.close();
    off_ev.close();
    system("/bin/rm -f tmp.off_ev");
}

void Parser(std::string net_name) {

    // read the statistiques from "net_name.stat" file //
    ifstream stat;
    std::string net = net_name + ".cmst";
    stat.open(net.c_str(), ifstream::in);
    if (!stat) {
        cerr << "Error opening input stream" << endl;
        exit(1);
    }
    stat >> nb_esms >> nb_events >>  COMPT_STORED_EVENT ;
    size_comp = nb_events;
    stat.close();
    ////////////////////////////////////////////////////
    net = net_name + ".esrg";
    esrg.open(net.c_str(), ifstream::in);
    //  esrg=fopen(net.c_str(),"r");
    if (!esrg) {
        cerr << "Error opening input stream" << endl;
        exit(1);
    }

    elem_set_index = new Q_list_el* [nb_esms + 1];
    for (int i = 0; i < nb_esms + 1; elem_set_index[i] = NULL, ++i);
    sym_rep        = new Marking*   [nb_esms + 1];
    for (int i = 0; i < nb_esms + 1; sym_rep[i] = NULL, ++i);

    events         = (Marking **)calloc(nb_events, sizeof(Marking *));

    ingeneric      = new Instance*  [nb_esms + 1];
    for (int i = 0; i < nb_esms + 1; ingeneric[i] = NULL, ++i);
    outgeneric     = new Instance*  [nb_esms + 1];
    for (int i = 0; i < nb_esms + 1; outgeneric[i] = NULL, ++i);
    initial_global_variables(net_name.c_str());

}


// This function represents the main part of the parser
// --> the parse starts here.
void OutPutParser(char *net_name) {
    int sr_id, sat, gen_inst,
        even_id, inst, nb_event;
//11,7 real_number ;
    unsigned long srmark, srlen, srptr, evmark, evlen;

    char  tmp[MAXSTRING];

    Parser((std::string) net_name);
    create_canonical_data_structure();

    while (esrg.getline(tmp, MAXSTRING - 1)) {

        (istringstream) tmp >> sr_id >> srmark >> srlen >>
                            srptr >> sat >> gen_inst >> nb_event ;
        // if the ESM is saturated.
        if (sat == SR_SAT) {
            // if the ESM does'nt exist,
            // it must be add to the "sym_rep" array.
            if (!sym_rep[sr_id])
                (void)add_sym(sr_id,
                              srmark, srlen, srptr);


            // if the saturated ESM enables
            // asymmetric transitions,
            // the type field must be set to "MACROP".
            if (nb_event && sym_rep[sr_id]->get_type() != MACROP)
                sym_rep[sr_id]->set_type(MACROP);
            else
                sym_rep[sr_id]->set_type(MACROC);

            // the parse of the generic instances of the ESM.
            // parse_binding_and_add( esrg,sr_id,gen_inst, GEN );
            // if the ESM is saturated.
            parse_binding_and_add_(esrg, sr_id, gen_inst, GENE);

            // the ESM already exist by was not saturated,
            // the output of the constructed eventualities
            // must be update by the generic arcs.
            treat_special_cases(sr_id);
        }


        while (esrg.getline(tmp, MAXSTRING - 1) && tmp[0] != '#') {

            (istringstream)tmp  >> even_id >> evmark >> evlen >> inst;

            // if the eventuality does'nt exist.
            if (!events[even_id])
                (void)add_event(sr_id,
                                srmark, srlen, srptr,
                                even_id,
                                evmark, evlen);


            // if the ESM is saturated,
            // the new construted eventuality node,
            // must be update by the generic instances.
            if (sym_rep[sr_id]) {
                events[even_id]->set_arc(outgeneric[sr_id], out_g);
                events[even_id]->set_arc(ingeneric[sr_id], in_g);
            }

            // The parse of the instances.
            parse_binding_and_add_(esrg, even_id, inst, INS);
        }
    }
}


//Used to parse files and creates
// the Q structure
Q_list *Create_Q(char *net_name) {
    int i;
    Q_list *Q = new Q_list();

    OutPutParser(net_name);

    for (i = nb_esms; i > 0; i--) {
        Q->insert_elem(elem_set_index[i]);
    }
    return Q;
}



// load the symmetrical representation
// from the ".mark" file using the traditional
// params. stored  in the ".off_sr" file.
void load_sym_rep(unsigned long  mark_pos,
                  unsigned long  length,
                  unsigned long  d_ptr) {

    // load the correct symbolic marking from
    // the ".mark" file.
    POP_STATIC_CONF(SYM_STATIC_STORE, & tabc);
    string_to_marking(mark_pos, d_ptr, length);

}

// load an eventuality from the .event
// file, using the "pos" position in the "off_ev"
// file to get the position "d_ptr" and the length
// "length" of the eventuality.
int load_event(unsigned long  llength,
               unsigned long  ld_ptr,
               int flag) {


    // load the string form of the eventuality
    // from the ".event" file in "compare" cache string.
    GET_EVENTUALITIE_FROM_FILE(ld_ptr, llength);
    if (flag) {
        COPY_CACHE(llength);
        STRING_TO_EVENTUALITIE();
    }
    return 0;
}





int strings_compare(int length_to_compare) {
    unsigned long ii;
    unsigned long op1;
    unsigned long op2;
    char *cp;
    char *pp;

    if (LEGTH_OF_CACHE != length_to_compare) return NOT_EQUAL ;
    else {
        cp = compare;
        pp = CACHE_STRING_ESRG;
        for (ii = length_to_compare; ii; ii--) {
            op1 = *cp; cp++;
            op2 = *pp; pp++;
            if (op1 != op2) return NOT_EQUAL;
        }
    }

    return EQUAL;
}



//This function is used to compute
// all eventualities of the macro node "macro".
void split_macro(int clas, int sbc, int lin, int col,
                 Marking * (*traitement)(Marking *), Marking *macro) {
    int Inf_born = 0;
    int Sup_born = 0;
    int i;

    int XLMOINS1 = X_L_MOINS_1(col, lin, clas, sbc);
    Inf_born = get_max(0, DYNAMIC[clas][sbc][lin - 1] -
                       XLMOINS1 - C_L_PLUS_1(col, clas, sbc) +
                       X_2(col, lin, clas, sbc));
    Sup_born = get_min(STATICS[clas][sbc][col - 1] -
                       X_I_MOINS_1(col, lin, clas, sbc),
                       DYNAMIC[clas][sbc][lin - 1] - XLMOINS1);

    for (i = Sup_born; i >= Inf_born; i--) {

        RESULT[clas][sbc][lin - 1][col - 1] = i;
        if ((lin == NB_DSC[clas][sbc]) && (col == NB_SSBC[clas][sbc])) {
            if ((clas == ncl - 1) && (sbc == SYM_STATIC_STORE[clas].sbc_num - 1))
                (*traitement)(macro);
            if ((clas != ncl - 1) && (sbc == SYM_STATIC_STORE[clas].sbc_num - 1))
                split_macro(clas + 1, 0, 1, 1, (*traitement), macro);
            if (sbc != SYM_STATIC_STORE[clas].sbc_num - 1)
                split_macro(clas, sbc + 1, 1, 1, (*traitement), macro);
        }
        if ((lin != NB_DSC[clas][sbc]) && (col == NB_SSBC[clas][sbc]))
            split_macro(clas, sbc, lin + 1, 1, (*traitement), macro);
        if (col != NB_SSBC[clas][sbc])
            split_macro(clas, sbc, lin, col + 1, (*traitement), macro);
    }
}

void copy_and_inst_arcs(Marking *macrox) {
    Marking *event = NULL;
    Instance *next;
    Instance *in_gen = macrox->get_arc(in_g);
    Instance *in_inst = macrox->get_arc(in_i);

    if (in_gen) {
        event = macrox->get_pointer(nxt_sb);
        while (event) {
            event->set_arc(in_gen, in_g);
            event = event->get_pointer(nxt_sb);
        }
    }

    while (in_inst) {

        next = in_inst->get_next();
        Marking *event = in_inst->get_des();
        Instance *tmp = event->get_arc(out_i);
        while (tmp->get_col() != in_inst->get_col()) tmp = tmp->get_next();
        change_instance_nodes(tmp , in_inst);
        delete  in_inst;
        in_inst = next;
    }
    macrox->set_arc(NULL, in_i);
}

///     a revoir /////////////////////
void add_event_mod(Marking *macrox,
                   int re_ev_id,
                   unsigned long pos,
                   unsigned long lg) {

    if (re_ev_id >= size_comp) {
        events = (Marking **)realloc(events, (size_comp + 10) * sizeof(Marking *));
        size_comp += 10;
    }

    events[re_ev_id] = new Marking(macrox->get_q_part(),
                                   re_ev_id,
                                   macrox->get_srpos(),
                                   macrox-> get_srlength(),
                                   macrox->get_srptr(),
                                   pos,
                                   lg,
                                   INSTANCE);

    events[re_ev_id]->set_pointer(macrox->get_pointer(nxt), nxt);
    events[re_ev_id]->set_pointer(macrox->get_pointer(nxt_sb), nxt_sb);
    events[re_ev_id]->set_pointer(macrox, prv);

    if (macrox->get_pointer(nxt) != NULL) {
        macrox->get_pointer(nxt)->set_pointer(events[re_ev_id], prv);
    }
//if (events[re_ev_id]->get_pointer(nxt)!=NULL)
//    events[re_ev_id]->get_pointer(nxt)->set_pointer(events[re_ev_id],prv);

    macrox->set_pointer(events[re_ev_id], nxt);
    macrox->set_pointer(events[re_ev_id], nxt_sb);
    (macrox->get_q_part())->set_num_elem((macrox->get_q_part())->get_num_elem() + 1) ;

}
////////////////////////////////////////////

Marking *
_seach_event_marking(MarkingStruct *tab,
                     int tab_size,
                     char *s_string,
                     int s_size) {

    int i;
    unsigned long ii;
    unsigned long op1;
    unsigned long op2;
    char *cp;
    char *pp;

    for (i = 0; i < tab_size; i++)
        if (s_size == tab[i].length) {
            cp = tab[i].marking ;
            pp = s_string ;
            for (ii = s_size; ii; ii--) {
                op1 = *cp; cp++;
                op2 = *pp; pp++;
                if (op1 != op2)
                    break;
            }
            if (ii == 0) {
                return tab[i].event;
            }
        }

    return NULL;
}

void
_free_event_marking(MarkingStruct *tab,
                    int tab_size) {
    int i;
    for (i = 0; i < tab_size; i++)
        free(tab[i].marking);
    free(tab);
}


int
_copy_cache_string(char **new_string) {
    int i;

    (*new_string) = (char *)calloc(length, sizeof(char));

    for (i = 0; i < length; i++)
        (*new_string)[i] = cache_string[i];
    return length;
}


void
init_tabtemp_(MarkingStruct **tab,
              int *tab_size,
              Marking *macrox) {

    Marking *event = NULL;

    if ((macrox->get_pointer(nxt_sb) != NULL))
        event = macrox->get_pointer(nxt_sb);

    while (event) {
        load_sym_rep(macrox->get_srpos(),
                     macrox->get_srlength(),
                     macrox->get_srptr());
        load_event(event->get_evlength(),
                   event->get_evpos(), 1);

        NEW_ASYM_MARKING(&tabc, &net_mark, &card, &num, &tot,
                         MERG, ASYM_STATIC_STORE, RESULT);

        (*tab) = (MarkingStruct *)realloc((*tab), ((*tab_size) + 1) *
                                          sizeof(MarkingStruct));
        (*tab)[(*tab_size)].length =
            _copy_cache_string(&(*tab)[(*tab_size)].marking);
        (*tab)[(*tab_size)].event = event;
        (*tab_size)++;

        event = event->get_pointer(nxt_sb);
    }
}

//the special treatement associated
//to an eventuality constructed by split
//of the marco node "macrox".
Marking *
inst_treatement(Marking *macrox) {

    Marking *event = NULL;
    // the macro node is asymmetric.

    if ((macrox->get_pointer(nxt_sb) != NULL)) {
        event = macrox->get_pointer(nxt_sb);

        while (event) {

            int len = (int)event->get_evlength();
            load_event(event->get_evlength(),
                       event->get_evpos(), 0);
            EVENTUALITIE_TO_STRING();

            if (strings_compare(len) == EQUAL)
                break;

            event = event->get_pointer(nxt_sb);
        }
    }

    if (!event) {
        load_sym_rep(macrox->get_srpos(),
                     macrox->get_srlength(),
                     macrox->get_srptr());
        NEW_ASYM_MARKING(&tabc, &net_mark, &card,
                         &num, &tot, MERG,
                         ASYM_STATIC_STORE, RESULT);
        if ((event = _seach_event_marking(tabtemp, tabsize,
                                          cache_string,
                                          length)) == NULL) {

            tabtemp = (MarkingStruct *)realloc(tabtemp, (tabsize + 1) *
                                               sizeof(MarkingStruct));
            tabtemp[tabsize].length =
                _copy_cache_string(&tabtemp[tabsize].marking);
            tabsize++;
        }

    }

    // if the computed eventuality does not exist.
    if (!event) {
        // add the new event. to the ".event" file.
        EVENTUALITIE_TO_STRING();
        ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
        // create and init the node for the new event.
        (void)add_event_mod(macrox,
                            COMPT_STORED_EVENT + 1,
                            FILE_POS_PTR,
                            LEGTH_OF_CACHE);
        tabtemp[tabsize - 1].event = events[COMPT_STORED_EVENT + 1];
        fflush(EVENT_MARK);
        return events[COMPT_STORED_EVENT + 1];
    }

    return event;
}

//the special treatement associated
//to an eventuality constructed by split
//of the marco node "macrox".
Marking *
inst_treatement1(Marking *macrox) {

    Marking *event = NULL;

    // the macro node is asymmetric.
    if ((macrox->get_pointer(nxt_sb) != NULL)) {
        event = macrox->get_pointer(nxt_sb);

        while (event) {
            load_event(event->get_evlength(),
                       event->get_evpos(), 0);
            EVENTUALITIE_TO_STRING();

            if (strings_compare(LEGTH_OF_CACHE) == EQUAL)
                break;

            event = event->get_pointer(nxt_sb);
        }
    }

    // if the computed eventuality does not exist.
    if (!event) {

        // add the new event. to the ".event" file.
        EVENTUALITIE_TO_STRING();
        ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
        // create and init the node for the new event.
        (void)add_event_mod(macrox,
                            COMPT_STORED_EVENT + 1,
                            FILE_POS_PTR,
                            LEGTH_OF_CACHE);

        fflush(EVENT_MARK);
        return events[COMPT_STORED_EVENT + 1];
    }

    return event;
}

void change_instance_nodes(Instance *inst , Instance *dest) {
    int ii;
//11/7 remove sr
    Event_p  nev_p , ev_p, copy_of_ev_p;

    load_event((dest->get_des())->get_evlength(),
               (dest->get_des())->get_evpos(), 1);
    load_sym_rep((dest->get_des())->get_srpos(),
                 (dest->get_des())->get_srlength(),
                 (dest->get_des())->get_srptr());
    NEW_ASYM_MARKING(& tabc, &net_mark, &card, &num, &tot
                     , MERG, ASYM_STATIC_STORE, RESULT);

    my_en_list(dest->get_trans() + 1);
    copy_of_ev_p = get_new_event(dest->get_trans());
    ev_p = enabled_head->list;

    while (ev_p != NULL) {


        nev_p = ev_p->next;
        copy_event(copy_of_ev_p, ev_p);

        fire_trans(ev_p);

        for (ii = 0;
                ii < tabt[inst->get_trans()].comp_num &&
                ev_p->npla[ii] == (dest->get_col())[0][ii] &&
                ev_p->split[ii] == (dest->get_col())[1][ii];
                ii++);


        if (ii == tabt[dest->get_trans()].comp_num) {

            SPECIAL_CANISATION();
            STORE_CARD(card, &STORED_CARD);
            GET_SYMETRIC_MARKING(&tabc, &net_mark, &card, &num,
                                 &tot, SYM_STATIC_STORE, MERG);
            ALL_MY_GROUP(&MERGING_MAPPE);
            get_canonical_marking();
            (void)marking_to_string();

            if (my_strcmp(inst->get_des()->get_srpos(),
                          inst->get_des()->get_srlength(), 0, length) == 0) {

                AFTER_CANONISATION(sfl_h->min, &MERGING_MAPPE);
                GET_EVENT_FROM_MARKING(tabc, tot, num, card, MERGING_MAPPE,
                                       ASYM_STATIC_STORE, STORED_CARD, MERG);
                INIT_ARRAYS(MERG, num, card, ASYM_STATIC_STORE);

                Marking *el = inst_treatement1(inst->get_des());

                inst->set_des(el);
                Instance *tmp = new Instance(dest->get_weight(),
                                             dest->get_trans(),
                                             dest->get_card(),
                                             dest->get_col(),
                                             dest->get_des());
                tmp->set_next(el->get_arc(in_i));
                el->set_arc(tmp, in_i);
            }

        }

        load_event((dest->get_des())->get_evlength(),
                   (dest->get_des())->get_evpos(), 1);
        load_sym_rep((dest->get_des())->get_srpos(),
                     (dest->get_des())->get_srlength(),
                     (dest->get_des())->get_srptr());

        NEW_ASYM_MARKING(& tabc, &net_mark, &card, &num, &tot
                         , MERG, ASYM_STATIC_STORE, RESULT);
        ev_p = nev_p;
    }

    dispose_old_event(copy_of_ev_p);

}





// update the ouput arcs
// for the a constructed eventuality after
// the split of "macrox" by use of the
// two following functions.
void search_events_and_add(Marking *macrox,
                           Marking *event, Instance *gen) {
    int tr;
    Event_p  nev_p, ev_p,
             copy_of_ev_p;
    //11/7Marking *tmp;

    load_sym_rep(macrox->get_srpos(),
                 macrox->get_srlength(),
                 macrox->get_srptr());
    load_event(event->get_evlength(),
               event->get_evpos(), 1);
    NEW_ASYM_MARKING(&tabc, &net_mark, &card, &num, &tot,
                     MERG, ASYM_STATIC_STORE, RESULT);
    tr = gen->get_trans() + 1 ;
    my_en_list(tr);
    copy_of_ev_p = get_new_event(tr - 1);
    ev_p = enabled_head->list;

    while (ev_p != NULL) {
        nev_p = ev_p->next;
        copy_event(copy_of_ev_p, ev_p);
        fire_trans(ev_p);
        SPECIAL_CANISATION();
        STORE_CARD(card, &STORED_CARD);
        GET_SYMETRIC_MARKING(&tabc, &net_mark, &card, &num,
                             &tot, SYM_STATIC_STORE, MERG);
        ALL_MY_GROUP(&MERGING_MAPPE);
        get_canonical_marking();
        (void)marking_to_string();

        if (my_strcmp(gen->get_des()->get_srpos(),
                      gen->get_des()->get_srlength(), 0, length) == 0) {
            AFTER_CANONISATION(sfl_h->min, &MERGING_MAPPE);
            GET_EVENT_FROM_MARKING(tabc, tot, num, card, MERGING_MAPPE,
                                   ASYM_STATIC_STORE, STORED_CARD, MERG);
            INIT_ARRAYS(MERG, num, card, ASYM_STATIC_STORE);

            Marking *el = inst_treatement1(gen->get_des());
            int **col = NULL;
            int ii, comp_num = tabt[tr - 1].comp_num;

            if (comp_num) {
                col = (int **)calloc(2, sizeof(int *));
                col[0] = (int *) calloc(comp_num, sizeof(int));
                col[1] = (int *) calloc(comp_num, sizeof(int));
            }

            for (ii = 0; ii < tabt[tr - 1].comp_num; ii++) {
                col[0][ii] = ev_p->npla[ii];
                col[1][ii] = ev_p->split[ii];
            }

            Instance *arc = new Instance(tabt[tr - 1].mean_t , tr - 1,
                                         ev_p->ordinary_instances, col);
            Instance *rev_arc = new Instance(tabt[tr - 1].mean_t , tr - 1,
                                             ev_p->ordinary_instances, col);
            add_arcs(event->get_id(),
                     (gen->get_des())->get_id(),
                     (gen->get_des())-> get_srpos(),
                     (gen->get_des())-> get_srlength(),
                     (gen->get_des())->get_srptr(),
                     el->get_id(),
                     el->get_evpos(),
                     el-> get_evlength(),
                     arc, rev_arc, INST);
        }

        load_sym_rep(macrox->get_srpos(),
                     macrox->get_srlength(),
                     macrox->get_srptr());
        load_event(event->get_evlength(),
                   event->get_evpos(), 1);

        NEW_ASYM_MARKING(& tabc, &net_mark, &card, &num,
                         &tot, MERG, ASYM_STATIC_STORE, RESULT);
        ev_p = nev_p;
    }
    dispose_old_event(copy_of_ev_p);
}


void post_treatement(Marking *macrox) {
    Marking *event = NULL;
    Instance *out_ge = macrox->get_arc(out_g);

    while (out_ge) {
        event = macrox->get_pointer(nxt_sb);
        while (event) {
            //if ((macrox->get_type()!=MACROC)||(!STRONG))
            search_events_and_add(macrox, event, out_ge);
            event = event->get_pointer(nxt_sb);
        }
        out_ge = out_ge->get_next();
    }
}

// This function instantiates a symmetrical representation
// to all it eventualities and applies the "function treatement"
// on each constructed eventuality.
void instance_macro(Marking *macrox) {
    //numero di volte che viene chiamata
    call_instance_macro++;
    //numero di volte che viene chiamata

    tabtemp = NULL; tabsize = 0;
    init_tabtemp_(&tabtemp,
                  &tabsize,
                  macrox);

    load_sym_rep(macrox->get_srpos(),
                 macrox->get_srlength(),
                 macrox->get_srptr());

    INIT_4D_MATRIX(RESULT);
    INIT_ARRAYS(MERG, num, card, ASYM_STATIC_STORE);
    split_macro(0, 0, 1, 1, inst_treatement, macrox);
    _free_event_marking(tabtemp,
                        tabsize);
    copy_and_inst_arcs(macrox);
    post_treatement(macrox);
#ifdef debug
    macrox->get_q_part()->set_tot_num_elem(macrox->get_q_part()->get_num_elem() - 1);
#endif
    Marking *tmp = macrox->get_pointer(nxt);
    (macrox->get_q_part())->set_pmark(tmp);
    (macrox->get_q_part())->set_num_elem(
        (macrox->get_q_part())->get_num_elem() - 1);
    if (tmp != NULL)
        tmp->set_pointer(NULL, prv);
    // (macrox->get_q_part())->discard(macrox);

}


X_list *Create_X_C_all(Q_list *&q, C_list *&c) {

    Q_list_el *new_block, *Q_block = q->get_block();
    Q_list_el *tmp_block;//11/7*tmp1_block;
    X_list *tmp_Xlist = new X_list();
    X_list_el *tmp_Xblock = NULL;
    list <int> part_out;
    map <int, double> out;
    struct el tmp1;

    while (Q_block != NULL) {
        //case MACROP
#if TEST
        cout << "ID BLOCCO" << Q_block->get_id() << "\n";
#endif
        if ((Q_block->get_list_pmark())->get_type() == MACROP) {
            X_list_el *tmp_Xblock1 = NULL;
            //rimuovo dal blocco il macro elemento e lo inserisco in un nuovo blocco
            Marking *m = (Q_block->get_list_pmark()->get_pointer(nxt));
            int num_elem = Q_block->get_num_elem();
            map <int, struct el> m_out;
            //init. sturtture per split
            Q_block->presplit(part_out, m_out, out_i);
            instance_macro(Q_block->get_list_pmark());
            num_elem = Q_block->get_num_elem() - num_elem;
            if (num_elem >= 0) {
                m->get_pointer(prv)->set_pointer(NULL, nxt);
                m->set_pointer(NULL, prv);
                new_block = new Q_list_el(Q_block->get_list_pmark(), num_elem + 1);
                Q_block->set_pmark(m);
                Q_block->set_num_elem(Q_block->get_num_elem() - (num_elem + 1));
                q->insert_elem(new_block);
                if (tmp_Xblock1 == NULL)
                    tmp_Xblock1 = new X_list_el((tmp_Xlist->get_num_elem_plus()), new_block);
                else
                    tmp_Xblock1->add_subset(new_block);
                new_block->set_partx(tmp_Xblock1);
            }
            if (Q_block->get_num_elem() > 1) {
                //dopo instanziazine contiene piu di un elemento
                //tmp_block=NULL;
#if TEST
                cout << "\tNumero elementi MAPS" << m_out.size() << endl;
#endif
                //init. strutture per split
                //11/7 int num_elem;
                //11/7 int fine=0;
                tmp_block = Q_block;
                if (tmp_Xblock1 == NULL)
                    tmp_Xblock1 = new X_list_el((tmp_Xlist->get_num_elem_plus()), Q_block);
                else
                    tmp_Xblock1->add_subset(Q_block);
                Q_block->set_partx(tmp_Xblock1);
                tmp_Xlist->insert_elem(tmp_Xblock1);
                new_block = NULL;
                do {
                    if ((tmp_block == NULL) || (tmp_block->get_num_elem() > 1))
                        tmp_block->split(part_out, m_out, new_block);
                    if (new_block != NULL) {
                        //tmp1_block=new_block->get_pointer(nxt);
                        new_block->set_id((q->get_block()->get_id()) > (q->get_num_elem()) ? q->get_block()->get_id() + 1 : (q->get_num_elem() + 1));
                        q->insert_elem(new_block);
                        tmp_Xblock1->add_subset(new_block);
                        new_block->set_partx(tmp_Xblock1);
                        //Q_block=tmp_block;
                        //new_block=tmp1_block;
                        //new_block=new_block->get_pointer(nxt);
                    }
                    tmp_block = new_block;
                    new_block = NULL;
                }
                while ((tmp_block != NULL));
                if (tmp_Xblock1->get_num_elem() > 1) {
                    C_list *tmp = new C_list(tmp_Xblock1);
                    insert_block(c, tmp);
                }
                tmp_Xblock1 = NULL;
            }//fine caso dopo split piu di un elemento
            else {
                //caso dopo split un solo elemento
                if (tmp_Xblock1 == NULL)
                    tmp_Xblock1 = new X_list_el((tmp_Xlist->get_num_elem_plus()), Q_block);
                else
                    tmp_Xblock1->add_subset(Q_block);
                tmp_Xlist->insert_elem(tmp_Xblock1);
                Q_block->set_partx(tmp_Xblock1);
            }//fine caso dopo split un solo elemento
            //tmp_Xblock=NULL;
        }
        //end  case MACROP
        //case MACROC
        else {
            if (tmp_Xblock == NULL)
                tmp_Xblock = new X_list_el((tmp_Xlist->get_num_elem_plus()), Q_block);
            else
                tmp_Xblock->add_subset(Q_block);
            // tmp_Xlist->insert_elem(tmp_Xblock);
            Q_block->set_partx(tmp_Xblock);
        }
        //end case MACROC
        Q_block = Q_block->get_pointer(nxt);
    }
    //if I have  X block witch contains more than one Q's block
    tmp_Xlist->insert_elem(tmp_Xblock);
    if (tmp_Xblock->get_num_elem() > 1) {
        C_list *tmp = new C_list(tmp_Xblock);
        insert_block(c, tmp);
    }
    //end if I have  X block witch contains more than one Q's block
    return tmp_Xlist;
}

//ricontrollare se funziona ora devo interrompere

X_list *Create_X_C_all_exact(Q_list *&q, C_list *&c) {

    Q_list_el *new_block, *Q_block = q->get_block();
    Q_list_el *tmp_block;//11/7*tmp1_block;
    X_list *tmp_Xlist = new X_list();
    X_list_el *tmp_Xblock = NULL;

    list <int> part_out;
    map <int, double> out;
    struct el tmp1;
    while (Q_block != NULL) {
#if TEST
        cout << "ID BLOCCO" << Q_block->get_id() << "\n";
#endif
        if ((Q_block->get_list_pmark())->get_arc(in_i) != NULL) {
            //case MACRO con archi in_i
            map <int, struct el> m_out;
            if (Q_block->get_list_pmark()->get_type() != INSTANCE)
                instance_macro(Q_block->get_list_pmark());
            Q_block->presplit(part_out, m_out, in_i);
            if (Q_block->get_num_elem() > 1) {
                //dopo instanziazine contiene piu di un lemento
                tmp_block = NULL;
#if TEST
                cout << "\tNumero elementi MAPS" << m_out.size() << endl;
#endif
                //init. strutture per split
                //11/7 int num_elem;
                //11/7 int fine=0;
                tmp_block = Q_block;
                if (tmp_Xblock == NULL)
                    tmp_Xblock = new X_list_el((tmp_Xlist->get_num_elem_plus()), Q_block);
                else
                    tmp_Xblock->add_subset(Q_block);
                Q_block->set_partx(tmp_Xblock);
                //tmp_Xlist->insert_elem(tmp_Xblock);
                new_block = NULL;
                do {
                    if ((tmp_block == NULL) || (tmp_block->get_num_elem() > 1))
                        tmp_block->split(part_out, m_out, new_block);
                    if (new_block != NULL) {
                        //end case no split
                        //tmp1_block=new_block->get_pointer(nxt);
                        new_block->set_id((q->get_block()->get_id()) > (q->get_num_elem()) ? q->get_block()->get_id() + 1 : (q->get_num_elem() + 1));
                        q->insert_elem(new_block);
                        tmp_Xblock->add_subset(new_block);
                        new_block->set_partx(tmp_Xblock);
                        //Q_block=tmp_block;
                        //new_block=tmp1_block;
                        //new_block=new_block->get_pointer(nxt);
                    }
                    tmp_block = new_block;
                    new_block = NULL;
                }
                while ((tmp_block != NULL));
            }//fine caso dopo split piu di un elemento
            else {
                //caso dopo split un solo elemento
                if (tmp_Xblock == NULL)
                    tmp_Xblock = new X_list_el((tmp_Xlist->get_num_elem_plus()), Q_block);
                else
                    tmp_Xblock->add_subset(Q_block);
                Q_block->set_partx(tmp_Xblock);
            }//fine caso dopo split un solo elemento
        }
        //end  case MACRO  con archi in_i
        //case MACRO senza archi in_i
        else {
            if ((Q_block->get_list_pmark())->get_type() == MACROP) {
                //instance_macro(Q_block->get_list_pmark());
                Marking *tmp = (Q_block->get_list_pmark())->get_pointer(nxt);
                Marking *tmp_next = NULL;
                tmp_block = NULL;
                while (tmp != NULL) {
                    //controllo se in un eventualita ha instanziato un arco in_i
                    if (tmp->get_arc(in_i) != NULL) {
                        tmp_next = tmp->get_pointer(nxt);
                        tmp->get_q_part()->remove(tmp->get_id(), tmp->get_type()); //rimuovo elemento dal blocco e
                        if (tmp_block == NULL)
                            tmp_block = new Q_list_el;
                        tmp_block->add_pmark(tmp);
                        tmp_block->num_elem_plusplus();
                        tmp = tmp_next;
                    }
                    else
                        tmp = tmp->get_pointer(nxt);
                }//end controllo se in un eventualita ha instanziato un arco in_i
                instance_macro(Q_block->get_list_pmark());
                if (Q_block->get_num_elem() == 0) {
                    Q_list_el *tmpQ_block = Q_block->get_pointer(prv); //se no perdo il successivo Q_block punterebbe sbagliato
                    q->remove(Q_block);
                    //riabilittare se si cancella nel modo  giusto!!!
                    //Q_block=tmpQ_block;
                }
                //inserisco il blocco di q in X
                else {
                    if (tmp_Xblock == NULL)
                        tmp_Xblock = new X_list_el((tmp_Xlist->get_num_elem_plus()), Q_block);
                    else
                        tmp_Xblock->add_subset(Q_block);
                    //inserisco il blocco di q in X
                    Q_block->set_partx(tmp_Xblock);
                }
                if ((tmp_block != NULL) && (tmp_block->get_num_elem() > 0)) {
                    tmp_block->set_id((q->get_block()->get_id()) > (q->get_num_elem()) ? q->get_block()->get_id() + 1 : (q->get_num_elem() + 1));
                    q->insert_elem(tmp_block);
                    if (tmp_Xblock == NULL)
                        tmp_Xblock = new X_list_el((tmp_Xlist->get_num_elem_plus()), tmp_block);
                    else
                        tmp_Xblock->add_subset(tmp_block);
                    //setto partx
                    tmp_block->set_partx(tmp_Xblock);
                    //setto partx
                    new_block = NULL;
                    map <int, struct el> m_out;
                    tmp_block->presplit(part_out, m_out, in_i);
                    do {
                        if ((tmp_block == NULL) || (tmp_block->get_num_elem() > 1))
                            tmp_block->split(part_out, m_out, new_block);
                        if (new_block != NULL) {
                            //end case no split
                            //tmp1_block=new_block->get_pointer(nxt);
                            new_block->set_id((q->get_block()->get_id()) > (q->get_num_elem()) ? q->get_block()->get_id() + 1 : (q->get_num_elem() + 1));
                            q->insert_elem(new_block);
                            tmp_Xblock->add_subset(new_block);
                            new_block->set_partx(tmp_Xblock);
                            //Q_block=tmp_block;
                            //new_block=tmp1_block;
                            //new_block=new_block->get_pointer(nxt);
                        }
                        tmp_block = new_block;
                        new_block = NULL;
                    }
                    while ((tmp_block != NULL));
                }
                //if I have  X block witch contains more than one Q's block
            }
            else {
                ;			if (tmp_Xblock == NULL)
                    tmp_Xblock = new X_list_el((tmp_Xlist->get_num_elem_plus()), Q_block);
                else
                    tmp_Xblock->add_subset(Q_block);
                Q_block->set_partx(tmp_Xblock);
                //tmp_Xblock=NULL; inserisco sempre nello stesso
            }
        }
        //end case MACROC   senza archi in_i
        Q_block = Q_block->get_pointer(nxt);
    }
    //inserisco il blocco in c se  composto
    if (tmp_Xblock->get_num_elem() > 1) {
        C_list *tmp = new C_list(tmp_Xblock);
        insert_block(c, tmp);
    }
    //inserisco il blocco in c se  composto
    //inserisco il blocco unico di X nella lista
    tmp_Xlist->insert_elem(tmp_Xblock);
    return tmp_Xlist;
}
//ricontrollare se funziona ora devo interrompere


X_list *Create_X_C_exact(Q_list *&q, C_list *&c) {

    Q_list_el *new_block, *Q_block = q->get_block();
    Q_list_el *tmp_block;//11/7 *tmp1_block;
    X_list *tmp_Xlist = new X_list();
    X_list_el *tmp_Xblock = NULL;

    struct el tmp1;
    while (Q_block != NULL) {
#if TEST
        cout << "ID BLOCCO: " << Q_block->get_id() << "\n";
        cout << Q_block << endl;
#endif
        if ((Q_block->get_list_pmark())->get_arc(in_i) != NULL) {
            //case MACRO con archi in_i
            list <int> part_out;
            map <int, struct el> m_out;
            map <int, double> rate_out;
#if TEST
            cout << "\tpart_out:" << part_out.size() << " rate_out:" << rate_out.size() << "\n";
#endif
            if (Q_block->get_list_pmark()->get_type() != INSTANCE) {
                //instance_macro(Q_block->get_list_pmark()->get_arc(in_g)->get_des());
                instance_macro(Q_block->get_list_pmark());
                if (DTMC)
                    Compute_denom(Q_block);
            }

            if (Q_block->get_num_elem() > 1) {
                //dopo instanziazine contiene piu di un elemento
                tmp_block = NULL;
#if TEST
                cout << "\tNumero elementi" << m_out.size() << endl;

#endif
                //init. strutture per split
                //11/7 int num_elem;
                //11/7 int fine=0;
                tmp_block = Q_block;
                if (tmp_Xblock == NULL)
                    tmp_Xblock = new X_list_el((tmp_Xlist->get_num_elem_plus()), Q_block);
                else
                    tmp_Xblock->add_subset(Q_block);
                Q_block->set_partx(tmp_Xblock);
                tmp_Xlist->insert_elem(tmp_Xblock);
                new_block = NULL;
                Q_block->presplit(part_out, m_out, in_i);
                Q_block->presplit_rateout(rate_out, out_i);
                do {
                    if ((tmp_block == NULL) || (tmp_block->get_num_elem() > 1)) {
                        tmp_block->split(part_out, m_out, new_block);
                        //In order to cover the problem find by Serge
                        if (tmp_block->get_num_elem() > 1) {
                            Q_list_el *new_block1 = NULL;
                            do {
                                if ((tmp_block == NULL) || (tmp_block->get_num_elem() > 1)) {
                                    tmp_block->split_rateout(rate_out, new_block1);
                                }
                                if (new_block1 != NULL) {
                                    new_block1->set_id((q->get_block()->get_id()) > (q->get_num_elem()) ? q->get_block()->get_id() + 1 : (q->get_num_elem() + 2));
                                    q->insert_elem(new_block1);
                                    tmp_Xblock->add_subset(new_block1);
                                    new_block1->set_partx(tmp_Xblock);
                                }
                                tmp_block = new_block1;
                                new_block1 = NULL;
                            }
                            while ((tmp_block != NULL));
                        }
                        //In order to cover the problem find by Serge
                    }
                    if (new_block != NULL) {
                        //end case no split
                        //tmp1_block=new_block->get_pointer(nxt);
                        new_block->set_id((q->get_block()->get_id()) > (q->get_num_elem()) ? q->get_block()->get_id() + 1 : (q->get_num_elem() + 2));
                        q->insert_elem(new_block);
                        tmp_Xblock->add_subset(new_block);
                        new_block->set_partx(tmp_Xblock);
                        //Q_block=tmp_block;
                        //new_block=tmp1_block;
                        //new_block=new_block->get_pointer(nxt);
                    }
                    tmp_block = new_block;
                    new_block = NULL;
                }
                while ((tmp_block != NULL));
                //if I have  X block witch contains more than one Q's block
                if (tmp_Xblock->get_num_elem() > 1) {
                    C_list *tmp = new C_list(tmp_Xblock);
                    insert_block(c, tmp);
                }
                //end if I have  X block witch contains more than one Q's block
            }//fine caso dopo split piu di un elemento
            else {
                //caso dopo split un solo elemento
                if (tmp_Xblock == NULL)
                    tmp_Xblock = new X_list_el((tmp_Xlist->get_num_elem_plus()), Q_block);
                else
                    tmp_Xblock->add_subset(Q_block);
                tmp_Xlist->insert_elem(tmp_Xblock);
                Q_block->set_partx(tmp_Xblock);
            }//fine caso dopo split un solo elemento
            tmp_Xblock = NULL;
            part_out.clear();
            m_out.clear();
            rate_out.clear();
        }
        //end  case MACRO  con archi in_i
        //case MACRO senza archi in_i
        else {
            if ((Q_block->get_list_pmark())->get_type() == MACROP) {
                //instance_macro(Q_block->get_list_pmark());
                Marking *tmp = (Q_block->get_list_pmark())->get_pointer(nxt);
                Marking *tmp_next = NULL;
                tmp_block = NULL;
                while (tmp != NULL) {
                    //controllo se in un eventualita ha instanziato un arco in_i e la torlgo dal blocco
                    if ((!DTMC) || (tmp->get_arc(in_i) != NULL)) {
                        tmp_next = tmp->get_pointer(nxt);
                        tmp->get_q_part()->remove(tmp->get_id(), tmp->get_type()); //rimuovo elemento dal blocco e
                        if (tmp_block == NULL)
                            tmp_block = new Q_list_el;
                        tmp_block->add_pmark(tmp);
                        tmp_block->num_elem_plusplus();
                        tmp = tmp_next;
                    }
                    else
                        tmp = tmp->get_pointer(nxt);
                }//end controllo se in un eventualita ha instanziato un arco in_i e la torlgo dal blocco
                Marking *tmp_marking = Q_block->get_list_pmark();
                list <int> part_out;
                map <int, struct el> m_out;
                map <int, double> rate_out;
                if (tmp_block != NULL) {
                    tmp_block->presplit(part_out, m_out, in_i);
                    tmp_block->presplit_rateout(rate_out, out_i);
#if TEST
                    cout << "\tpart_out:" << part_out.size() << " rate_out:" << rate_out.size() << "\n";
#endif
                }
                instance_macro(Q_block->get_list_pmark());
                if (DTMC)
                    Compute_denom(Q_block);
                if (Q_block->get_num_elem() == 0) {
                    Q_list_el *tmpQ_block = Q_block->get_pointer(prv); //se no perdo il successivo Q_block punterebbe sbagliato
                    tmp_marking->set_q_part(NULL);
                    q->remove(Q_block);
                    //Q_block=tmpQ_block;
                }
                //inserisco il blocco di q in X
                else {
                    if (tmp_Xblock == NULL)
                        tmp_Xblock = new X_list_el((tmp_Xlist->get_num_elem_plus()), Q_block);
                    else
                        tmp_Xblock->add_subset(Q_block);
                    //inserisco il blocco di q in X
                    Q_block->set_partx(tmp_Xblock);
                }
                if ((tmp_block != NULL) && (tmp_block->get_num_elem() > 0)) {
                    tmp_block->set_id((q->get_block()->get_id()) > (q->get_num_elem()) ? q->get_block()->get_id() + 1 : (q->get_num_elem() + 2));
                    q->insert_elem(tmp_block);
                    if (tmp_Xblock == NULL)
                        tmp_Xblock = new X_list_el((tmp_Xlist->get_num_elem_plus()), tmp_block);
                    else
                        tmp_Xblock->add_subset(tmp_block);
                    //setto partx
                    tmp_block->set_partx(tmp_Xblock);
                    //setto partx
                    new_block = NULL;
                    do {
                        if ((tmp_block == NULL) || (tmp_block->get_num_elem() > 1)) {
                            tmp_block->split(part_out, m_out, new_block);
                            //In order to cover the problem find by Serge
                            if (tmp_block->get_num_elem() > 1) {
                                Q_list_el *new_block1 = NULL;
                                do {
                                    if ((tmp_block == NULL) || (tmp_block->get_num_elem() > 1)) {
                                        tmp_block->split_rateout(rate_out, new_block1);
                                    }
                                    if (new_block1 != NULL) {
                                        new_block1->set_id((q->get_block()->get_id()) > (q->get_num_elem()) ? q->get_block()->get_id() + 1 : (q->get_num_elem() + 2));
                                        q->insert_elem(new_block1);
                                        tmp_Xblock->add_subset(new_block1);
                                        new_block1->set_partx(tmp_Xblock);
                                    }
                                    tmp_block = new_block1;
                                    new_block1 = NULL;
                                }
                                while ((tmp_block != NULL));

                            }
                            //In order to cover the problem find by Serge
                        }
                        if (new_block != NULL) {
                            //end case no split
                            //tmp1_block=new_block->get_pointer(nxt);
                            new_block->set_id((q->get_block()->get_id()) > (q->get_num_elem()) ? q->get_block()->get_id() + 1 : (q->get_num_elem() + 2));
                            q->insert_elem(new_block);
                            tmp_Xblock->add_subset(new_block);
                            new_block->set_partx(tmp_Xblock);
                            //Q_block=tmp_block;
                            //new_block=tmp1_block;
                            //new_block=new_block->get_pointer(nxt);
                        }
                        tmp_block = new_block;
                        new_block = NULL;
                    }
                    while ((tmp_block != NULL));
                }
                //if I have  X block witch contains more than one Q's block
                if (tmp_Xblock->get_num_elem() > 1) {
                    C_list *tmp = new C_list(tmp_Xblock);
                    insert_block(c, tmp);
                }
                tmp_Xlist->insert_elem(tmp_Xblock);
                part_out.clear();
                m_out.clear();
                rate_out.clear();
            }
            else {
                tmp_Xblock = new X_list_el((tmp_Xlist->get_num_elem_plus()), Q_block);
                tmp_Xlist->insert_elem(tmp_Xblock);
                Q_block->set_partx(tmp_Xblock);
            }
        }
        //end case MACROC   senza archi in_i
        tmp_Xblock = NULL;
        Q_block = Q_block->get_pointer(nxt);
    }
    return tmp_Xlist;
}



X_list *Create_X_C(Q_list *&q, C_list *&c) {

    Q_list_el *new_block, *Q_block = q->get_block();
    Q_list_el *tmp_block;//11/7 *tmp1_block;
    X_list *tmp_Xlist = new X_list();
    X_list_el *tmp_Xblock = NULL;

    //map <int,double> out;
    struct el tmp1;
    while (Q_block != NULL) {
        //case MACROP
#if TEST
        cout << "ID BLOCCO" << Q_block->get_id() << "\n";
#endif
        if ((Q_block->get_list_pmark())->get_type() != MACROC) {
            if ((Q_block->get_list_pmark())->get_type() == MACROP) {
                //rimuovo dal blocco il macro elemento e lo inserisco in un nuovo blocco
                Marking *m = (Q_block->get_list_pmark()->get_pointer(nxt));
                int num_elem = Q_block->get_num_elem();
                list <int> part_out;
                map <int, struct el> m_out;
                Q_block->presplit(part_out, m_out, out_i);
                instance_macro(Q_block->get_list_pmark());
                if (DTMC)
                    Compute_denom(Q_block);
                num_elem = Q_block->get_num_elem() - num_elem;
                if (num_elem >= 0) {
                    m->get_pointer(prv)->set_pointer(NULL, nxt);
                    m->set_pointer(NULL, prv);
                    new_block = new Q_list_el(Q_block->get_list_pmark(), num_elem + 1);
                    Q_block->set_pmark(m);
                    Q_block->set_num_elem(Q_block->get_num_elem() - (num_elem + 1));
                    q->insert_elem(new_block);
                    tmp_Xblock = new X_list_el((tmp_Xlist->get_num_elem_plus()), new_block);
                    new_block->set_partx(tmp_Xblock);
                    Marking *tmp_marking = new_block->get_list_pmark();
                    //patch
                    while (tmp_marking != NULL) {
                        tmp_marking->set_q_part(new_block);
                        tmp_marking = tmp_marking->get_pointer(nxt);
                    }
                    new_block->set_id((q->get_block()->get_id()) > (q->get_num_elem()) ? q->get_block()->get_id() + 1 : (q->get_num_elem() + 2));
                }
                //patch
                if (Q_block->get_num_elem() > 1) {
                    //dopo instanziazine contiene piu di un elemento
                    tmp_block = NULL;
                    //map <int,struct el> m_out;
                    //init. sturtture per split
                    //Q_block->presplit(part_out,m_out,out_i);
#if TEST
                    cout << "\tNumero elementi MAPS" << m_out.size() << endl;
#endif
                    //init. strutture per split
                    //11/7 int num_elem;
                    //11/7 int fine=0;
                    tmp_block = Q_block;
                    if (tmp_Xblock == NULL)
                        tmp_Xblock = new X_list_el((tmp_Xlist->get_num_elem_plus()), Q_block);
                    else
                        tmp_Xblock->add_subset(Q_block);
                    Q_block->set_partx(tmp_Xblock);
                    tmp_Xlist->insert_elem(tmp_Xblock);
                    new_block = NULL;
                    do {
                        //num_elem=tmp_block->get_num_elem();
                        //tmp1_block=tmp_block;
                        if ((tmp_block == NULL) || (tmp_block->get_num_elem() > 1))
                            tmp_block->split(part_out, m_out, new_block);
                        if (new_block != NULL) {
                            //end case no split
                            //tmp1_block=new_block->get_pointer(nxt);
                            new_block->set_id((q->get_block()->get_id()) > (q->get_num_elem()) ? q->get_block()->get_id() + 1 : (q->get_num_elem() + 2));
                            q->insert_elem(new_block);
                            tmp_Xblock->add_subset(new_block);
                            new_block->set_partx(tmp_Xblock);
                        }
                        tmp_block = new_block;
                        new_block = NULL;
                    }
                    while ((tmp_block != NULL));
                    //if I have  X block witch contains more than one Q's block
                    if (tmp_Xblock->get_num_elem() > 1) {
                        C_list *tmp = new C_list(tmp_Xblock);
                        insert_block(c, tmp);
                    }
                    //end if I have  X block witch contains more than one Q's block
                }//fine caso dopo split piu di un elemento
                else {
                    //caso dopo split un solo elemento
                    if (tmp_Xblock == NULL)
                        tmp_Xblock = new X_list_el((tmp_Xlist->get_num_elem_plus()), Q_block);
                    else
                        tmp_Xblock->add_subset(Q_block);
                    tmp_Xlist->insert_elem(tmp_Xblock);
                    Q_block->set_partx(tmp_Xblock);
                    if (tmp_Xblock->get_num_elem() > 1) {
                        C_list *tmp = new C_list(tmp_Xblock);
                        insert_block(c, tmp);
                    }
                }//fine caso dopo split un solo elemento
            }
            else {
                if (Q_block->get_num_elem() > 1) {
                    list <int> part_out;
                    map <int, struct el> m_out;
                    Q_block->presplit(part_out, m_out, out_i);
                    tmp_block = NULL;
#if TEST
                    cout << "\tNumero elementi MAPS" << m_out.size() << endl;
#endif
                    //11/7 int num_elem;
                    //11/7 int fine=0;
                    tmp_block = Q_block;
                    if (tmp_Xblock == NULL)
                        tmp_Xblock = new X_list_el((tmp_Xlist->get_num_elem_plus()), Q_block);
                    else
                        tmp_Xblock->add_subset(Q_block);
                    Q_block->set_partx(tmp_Xblock);
                    tmp_Xlist->insert_elem(tmp_Xblock);
                    new_block = NULL;
                    do {
                        if ((tmp_block == NULL) || (tmp_block->get_num_elem() > 1))
                            tmp_block->split(part_out, m_out, new_block);
                        if (new_block != NULL) {
                            new_block->set_id((q->get_block()->get_id()) > (q->get_num_elem()) ? q->get_block()->get_id() + 1 : (q->get_num_elem() + 2));
                            q->insert_elem(new_block);
                            tmp_Xblock->add_subset(new_block);
                            new_block->set_partx(tmp_Xblock);
                        }
                        tmp_block = new_block;
                        new_block = NULL;
                    }
                    while ((tmp_block != NULL));
                    //if I have  X block witch contains more than one Q's block
                    if (tmp_Xblock->get_num_elem() > 1) {
                        C_list *tmp = new C_list(tmp_Xblock);
                        insert_block(c, tmp);
                    }
                }
                else {
                    //caso dopo split un solo elemento
                    if (tmp_Xblock == NULL)
                        tmp_Xblock = new X_list_el((tmp_Xlist->get_num_elem_plus()), Q_block);
                    else
                        tmp_Xblock->add_subset(Q_block);
                    tmp_Xlist->insert_elem(tmp_Xblock);
                    Q_block->set_partx(tmp_Xblock);
                    if (tmp_Xblock->get_num_elem() > 1) {
                        C_list *tmp = new C_list(tmp_Xblock);
                        insert_block(c, tmp);
                    }
                }//fine caso dopo split un solo elemento
            }
            tmp_Xblock = NULL;
        }
        //end  case MACROP
        //case MACROC
        else {
            tmp_Xblock = new X_list_el((tmp_Xlist->get_num_elem_plus()), Q_block);
            tmp_Xlist->insert_elem(tmp_Xblock);
            Q_block->set_partx(tmp_Xblock);
            tmp_Xblock = NULL;
        }
        //end case MACROC
        Q_block = Q_block->get_pointer(nxt);
    }
    return tmp_Xlist;
}

void presplit(map <class Marking *, double> &e_b, multimap<double, Marking *> &multi_split) {
    map <class Marking *, double>::iterator iter;
    iter = e_b.begin();
    while (iter != e_b.end()) {
        multi_split.insert(make_pair((*iter).second, (*iter).first));
        e_b.erase((*iter).first);
        iter++;
    }
}


void split(Q_list *&q, C_list *&c, X_list *&x, multimap<double, Marking *> &multi_split) {
    map <int, class Q_list_el *> tmp_part;
    multimap <double, Marking *>::iterator iter;
    iter = multi_split.begin();
    map <int, class Q_list_el *>::iterator iter2;
    double current = 0;
    // Q_list_el tmp_qblock=NULL;
    current = (*iter).first;
    while (iter != multi_split.end()) {
        if (current != (*iter).first) {
            iter2 = tmp_part.begin();
            while (iter2 != tmp_part.end()) {
                (*iter2).second->set_id((q->get_block()->get_id() + 1) > (q->get_num_elem() + 1) ? q->get_block()->get_id() + 1 : (q->get_num_elem() + 1));
                q->insert_elem((*iter2).second);
                if ((*iter2).second->get_partx()->get_num_elem() == 2) {
                    C_list *tmp = new C_list((*iter2).second->get_partx());
                    insert_block(c, tmp);
                }
                tmp_part.erase((*iter2).first);
                iter2++;
            }
            current = (*iter).first;
        }
        ((*iter).second->get_q_part())->remove((*iter).second->get_id(), (*iter).second->get_type());
        //gestire caso in cui rimuovo tutti gli elementi di una partizione
        int id = (*iter).second->get_q_part()->get_id();
        X_list_el *tmp_x = (*iter).second->get_q_part()->get_partx();
        if (((*iter).second->get_q_part()->get_num_elem() == 0)) {
            ((*iter).second->get_q_part()->get_partx())->remove((*iter).second->get_q_part()->get_id());
            if ((*iter).second->get_q_part()->get_partx()->get_num_elem() == 1) {
                //devo togliere da C se no poi lo inserisco due volte
                remove_x((*iter).second->get_q_part()->get_partx()->get_id(), c);
            }//devo togliere da C se no poi lo inserisco due volte
            q->remove((*iter).second->get_q_part());
        }
        //gestire caso in cui rimuovo tutti gli elementi di una partizione
        if (tmp_part[id] == NULL) {
            Q_list_el *tmp_q_block = new Q_list_el(0);
            tmp_part[id] = tmp_q_block;
            //fa puntareil nuovo blocco al blocco X che dovrebbe contenerlo
            tmp_part[id]->set_partx(tmp_x);
            //inserisce il nuovo blocco nel blocco X che dovrebbe contenerlo
            tmp_x->add_subset(tmp_q_block);
        }
        tmp_part[id]->num_elem_plusplus();
        tmp_part[id]->add_pmark((*iter).second);
        //tmp_part[(*iter).second->get_q_part()->get_id()]->num_elem_plusplus();
        //multi_split.erase((*iter).first);
        iter++;
    }
    iter2 = tmp_part.begin();
    while (iter2 != tmp_part.end()) {
        (*iter2).second->set_id((q->get_block()->get_id() + 1) > (q->get_num_elem() + 1) ? q->get_block()->get_id() + 1 : (q->get_num_elem() + 1));
        q->insert_elem((*iter2).second);
        if ((*iter2).second->get_partx()->get_num_elem() == 2) {
            C_list *tmp = new C_list((*iter2).second->get_partx());
            insert_block(c, tmp);
        }
        tmp_part.erase((*iter2).first);
        iter2++;
    }

}


void Lump_test(Q_list *&q, C_list *&c, X_list *&x) {
    C_list *first_el = NULL;
    Q_list_el *b = NULL;
    int step = 1;
    while (c != NULL) {
        map <class Marking *, double> e_b;
        multimap<double, Marking *> multi_split;
        first_el = top(c);
        if (first_el->get_subset()->get_num_elem() > 1) { //non bello devo pensare a restrutturare la lista C???????????? quando split non posso controllare se aggungo due volte il blocco!!!
            b = (first_el->get_subset())->get_subset();
            (b->get_partx())->remove(b->get_id());
            //inserisco b in una nuovo elemento di X
            X_list_el *tmp_Xblock = new X_list_el((x->get_num_elem_plus()), b); //creo blocco
            x->insert_elem(tmp_Xblock);//inserisco blocco nella lista
            if ((b->get_partx())->get_num_elem() > 1) {
                //inserisco il vecchio blocco di X che conteneva b  in C se contiene pi di 1 elemento
                C_list *tmp = new C_list(b->get_partx());
                insert_block(c, tmp);
            }//inserisco il vecchio blocco di X che conteneva b  in C se contiene p di 1 elemento
            b->set_partx(tmp_Xblock);//set partx correttamente
            b->set_pointer(NULL, nxt_sb);
            //end inserisco b in un nuovo elemento di X
            //creo E_B
            if ((b->get_list_pmark()->get_type() == INSTANCE)) {
                if (!STRONG)
                    e_b = b->creat_E_B(out_i, out_g);
                else
                    e_b = b->creat_E_B(in_i, in_g);
                //split
                presplit(e_b, multi_split);
                split(q, c, x, multi_split);
            }
            delete(first_el);
            //if (b->get_num_elem()==0)
            //  {
            //   q->remove(b);
            //delete(b);
            //}
#if TEST
            cout << step << "Passo \n";
#endif
            step++;
        }
    }
    cout << "\n\tNumber of steps: " << step << "\n\n";
}





void order_id_block(Q_list *&q) {

    Q_list_el *tmp = q->get_block();
    int num = 1;
    while (tmp != NULL) {
        tmp->set_id(num);
        tmp = tmp->get_pointer(nxt);
        num++;
    }
}


void gen_wngr_ctrs(std::string net_name, Q_list *&q) {
    FILE *wngr, *ctrs, *fthrou;
    ofstream rgr_aux;
    std::string net = net_name + ".wngr";
    map <int, double>::iterator iter;
    wngr = fopen(net.c_str(), "w+");
    if (!wngr) {
        cerr << "Error opening output stream wngr" << endl;
        throw ExceptionIO();
    }
    fprintf(wngr, "1\n");
    std::string net1 = net_name + ".ctrs";
    ctrs = fopen(net1.c_str(), "w+");
    if (!ctrs) {
        cerr << "Error opening output stream ctrs" << endl;
        throw ExceptionIO();
    }
    net = net_name + ".rgr_aux";
    rgr_aux.open(net.c_str(), ofstream::out);
    if (!rgr_aux) {
        cerr << "Error opening output stream rgr_aux" << endl;
        throw ExceptionIO();
    }
    rgr_aux << "toptan= " << q->get_num_elem() << "\ntopvan= 0\n";
    rgr_aux.close();

    net1 = net_name + ".throu";
    fthrou = fopen(net1.c_str(), "w+");
    if (!fthrou) {
        cerr << "Error opening output stream throu" << endl;
        throw ExceptionIO();;
    }
    fprintf(fthrou, "%d\n", ntr);

    Q_list_el *tmp = q->get_block();
    Instance *tmp_arc = NULL;
    while (tmp != NULL) {
        map<int, double> reach;
        map<int, double> throu; //int id transition double total rate
        print_ctrs(tmp->get_list_pmark(), ctrs);
        if (tmp->get_list_pmark()->get_type() == MACROC) {
            tmp_arc = tmp->get_list_pmark()->get_arc(out_g);
            while (tmp_arc != NULL) {
                //test if the generic arc point to a macroc that has already been instanced.
                if ((tmp_arc->get_des()->get_q_part() != NULL) && (tmp_arc->get_des()->get_id() == tmp_arc->get_des()->get_q_part()->get_list_pmark()->get_id())) {
                    reach[tmp_arc->get_des()->get_q_part()->get_id()] += tmp_arc->get_card() * tmp_arc->get_weight();
                }
                else {
                    //case macroc has already instanced

                    int card_mark = 0;; //number of SM contains in the MacroC
                    map<int, int> tmp_reach; //int block reached, int number of element;
                    Marking *tmp_marking = tmp_arc->get_des()->get_pointer(nxt_sb);
                    while (tmp_marking != NULL) {
                        //get the information about the block reach
                        card_mark++;
                        tmp_reach[tmp_marking->get_q_part()->get_id()] = tmp_marking->get_q_part()->get_num_elem();
                        tmp_marking = tmp_marking->get_pointer(nxt_sb);
                    }//get the information about the block reach
                    map <int, int>::iterator iter1;
                    for (iter1 = tmp_reach.begin(); iter1 != tmp_reach.end(); iter1++) {
                        reach[iter1->first] += tmp_arc->get_card() * tmp_arc->get_weight() * iter1->second / card_mark;
                    }
                }//case macroc has already instanced
                throu[tmp_arc->get_trans()] += tmp_arc->get_card() * tmp_arc->get_weight();
                tmp_arc = tmp_arc->get_next();
            }
        }
        else {
            Marking *tmp_elem = tmp->get_list_pmark();
            while (tmp_elem != NULL) {
                tmp_arc = tmp_elem->get_arc(out_i);
                map<int, double> out_arc;
                //norm=0;
                while (tmp_arc != NULL) {
                    out_arc[tmp_arc->get_des()->get_q_part()->get_id()] += tmp_arc->get_card() * tmp_arc->get_weight();
                    throu[tmp_arc->get_trans()] += tmp_arc->get_card() * tmp_arc->get_weight();
                    tmp_arc = tmp_arc->get_next();
                }
                for (iter = out_arc.begin(); iter != out_arc.end(); iter++) {
                    reach[iter->first] += (iter->second) / (tmp->get_num_elem());
                }
                tmp_elem = tmp_elem->get_pointer(nxt);
            }
        }
        //write wngr
        store_compact(reach.size(), wngr);
        iter = reach.begin();
        while (iter != reach.end()) {
            store_compact(iter->first, wngr);
            store_double(&(iter->second), wngr);
            iter++;
        }

        //write wngr
        store_compact(throu.size(), fthrou);
        iter = throu.begin();
        while (iter != throu.end()) {
            store_compact(iter->first, fthrou);
            store_double(&(iter->second), fthrou);
            cout << iter->second << "," << iter->first << endl;
            iter++;
        }
        tmp = tmp->get_pointer(nxt);
    }
    fprintf(fthrou, "%d\n", ntr);
    for (int st = 0; st < ntr; st++)
        fprintf(fthrou, "%s\n", tabt[st].trans_name);
    fprintf(wngr, "%d\n", q->get_num_elem() + 1);
    fclose(wngr);
    fclose(ctrs);
    fclose(fthrou);
}

/*
void gen_ctmc(std::string net_name,Q_list*&q){
  ofstream fp;
  std::string net=net_name+".CTMC";
  map <int,double>::iterator iter;
  fp.open(net.c_str(),ofstream::out);
  if( !fp )
    {
      cerr << "Error opening output stream .CTMC" << endl;
      throw ExceptionIO();
    }
  //graphic
  ofstream fp1;
  std::string net1=net_name+".graph1";
  fp1.open(net1.c_str(),ofstream::out);
  if(!fp1)
    {
      cerr << "Error opening output stream .graph1" << endl;
      throw ExceptionIO();
    }
  fp1<<"digraph ESRG {\n  nodesep=.05;\n rankdir=LR;node [shape=record, width=0.1,height=0.1];\n";
  Q_list_el *tmp=q->get_block();
  while (tmp!=NULL)
    {
      fp1<<"QBLOCK"<<tmp->get_id()<<"[shape=record,label=\" <Q"<<tmp->get_id()<<"> Q ID:"<<tmp->get_id();
      if (tmp->get_list_pmark()->get_type()==MACROC)
	{
	  fp1<<"|<M"<<tmp->get_list_pmark()->get_id()<<"> ID:"<<tmp->get_list_pmark()->get_id()<< " Macro\"];\n";
	}
      else
	{
	  Marking *tmp_elem=tmp->get_list_pmark();
	  //int i=0;
	  while (tmp_elem!=NULL)
	    {
	      //	if (i>0) fp1<<"|";
	      fp1<<"|<I"<<tmp_elem->get_id()<<"> ID:"<<tmp_elem->get_id()<< " INST";
	      tmp_elem=tmp_elem->get_pointer(nxt);
	      //	i++;
	    }
	  fp1<<"\"];\n";
	}
      tmp=tmp->get_pointer(nxt);
    }
  //graphic
  tmp=q->get_block();
  Instance* tmp_arc=NULL;
  while (tmp!=NULL)
    {
      map<int,double> reach;
      double norm=0;
      if (tmp->get_list_pmark()->get_type()==MACROC)
	{
	  tmp_arc=tmp->get_list_pmark()->get_arc(out_g);
	  while (tmp_arc!=NULL)
	    {
	      //test if the generic arc point to a macroc that has already been instanced.
	      if((tmp_arc->get_des()->get_q_part()!=NULL)&&(tmp_arc->get_des()->get_id()==tmp_arc->get_des()->get_q_part()->get_list_pmark()->get_id()))
		{
		  norm+=tmp_arc->get_card()*tmp_arc->get_weight();
		  reach[tmp_arc->get_des()->get_q_part()->get_id()]+=tmp_arc->get_card()*tmp_arc->get_weight();
		}
	      else
		{//case macroc has already instanced
		  int card_mark=0;; //number of SM contains in the MacroC
		  map<int,int> tmp_reach; //int block reached, int number of element;
		  Marking *tmp_marking=tmp_arc->get_des()->get_pointer(nxt_sb);
		  while (tmp_marking!=NULL)
		    {//get the information about the block reach
		      card_mark++;
		      tmp_reach[tmp_marking->get_q_part()->get_id()]=tmp_marking->get_q_part()->get_num_elem();
		      tmp_marking=tmp_marking->get_pointer(nxt_sb);
		    }//get the information about the block reach
		  map <int,int>::iterator iter1;
		  for (iter1=tmp_reach.begin();iter1!=tmp_reach.end();iter1++)
		    {
		      reach[iter1->first]+=tmp_arc->get_card()*tmp_arc->get_weight()*iter1->second/card_mark;
		      norm+=tmp_arc->get_card()*tmp_arc->get_weight()*iter1->second/card_mark;
		    }
		}//case macroc has already instanced
	      tmp_arc=tmp_arc->get_next();
	    }
	  for(iter=reach.begin();iter!=reach.end();iter++)
	    {
	      iter->second=iter->second/norm;
	    }
	}
      else
	{
	  Marking *tmp_elem=tmp->get_list_pmark();
	  while (tmp_elem!=NULL)
	    {
	      tmp_arc=tmp_elem->get_arc(out_i);
	      map<int,double> out_arc;
	      norm=0;
	      while(tmp_arc!=NULL)
		{
		  norm+=tmp_arc->get_card()*tmp_arc->get_weight();
		  out_arc[tmp_arc->get_des()->get_q_part()->get_id()]+=tmp_arc->get_card()*tmp_arc->get_weight();
		  tmp_arc=tmp_arc->get_next();
		}
	      for(iter=out_arc.begin();iter!=out_arc.end();iter++)
		{
		  //if(tmp->get_list_pmark()->get_type()==MACROC)
		  //	{
		  // 	reach[iter->first]+=iter->second/norm;
		  //	}
		  //else
		  //	{
		  reach[iter->first]+=(iter->second)/(tmp->get_num_elem()*norm);
		  //	}
		}
	      tmp_elem=tmp_elem->get_pointer(nxt);
	    }
	}
      fp<<"#reached elements: "<<reach.size()<<endl;
      iter=reach.begin();
      while(iter!=reach.end())
	{
	  fp<<"\t reach id:"<<iter->first<<" rate:";
	  fp<<iter->second<<endl;
	  fp1<<"QBLOCK"<<tmp->get_id()<<":Q"<<tmp->get_id()<<"->QBLOCK"<<iter->first<<":Q"<<iter->first<<"[label=\""<<iter->second<<"\"]\n";
	  iter++;
	}
      tmp=tmp->get_pointer(nxt);

    }
  fp1<<"}\n";
  fp1.close();
  fp.close();
}
*/

// void draw_graph(std::string net_name,Q_list*& q){
//   ofstream fp;
//   int i;
//   std::string net=net_name+".graph";
//   fp.open(net.c_str(),ofstream::out);
//   if(!fp)
//     {
//       cerr << "Error opening output stream" << endl;
//       throw ExceptionIO();
//     }
//   fp<<"digraph ESRG {\n  nodesep=.05;\n rankdir=LR;node [shape=record, width=0.1,height=0.1];\n node0 [ label=\"";
//   for (i=1;i<=q->get_num_elem();i++)
//     {
//       fp<<"<f"<<i<<"> "<<i <<" |";
//     }
//   fp<<"\", height=1.5];\n node[width=1.5];\n";
//   Q_list_el *block=q->get_block();
//   i=1;
//   while (block!=NULL)
//     {
//       Marking *m=block->get_list_pmark();
//       //save the number of elements in the block
//       if (m->get_type()==INSTANCE)
// 	PEAK+=block->get_num_elem();
//       else
// 	PEAK+=1;
//       //save the number of elements in the block
//       while(m!=NULL)
// 	{
// 	  fp<<"m"<<m->get_id()<<"t"<<m->get_type()<<"[label=\"M "<<m->get_id()<<"  T="<<m->get_type()<<"\"];\n";
// 	  if (m->get_pointer(prv)==NULL)
// 	    fp<<"node0:f"<<i<<" ->m"<<m->get_id() <<"t"<<m->get_type()<<";\n";
// 	  else
// 	    fp<<"m"<<m->get_pointer(prv)->get_id()<<"t"<<m->get_pointer(prv)->get_type()<<"->m"<<m->get_id()<<"t"<<m->get_type() <<";\n";
// 	  if(m->get_type()==MACROC) break;
// 	  m=m->get_pointer(nxt);
// 	}
//       i++;
//       block=block->get_pointer(nxt);
//     }
//   fp<<"}\n";
//   fp.close();
// }

int print_sr_ctmc(Marking *s, char **st) {

    FILE *fd;
    int pos;
    char  *mark = "sr.mark";
    fd = fopen(mark, "w+");
    if (!fd) {
        cerr << "Error opening output stream" << endl;
        throw ExceptionIO();
    }

    load_sym_rep(s->get_srpos(),
                 s->get_srlength(),
                 s->get_srptr());
    get_canonical_marking();
    WRITE_ON_ESRG(fd);
    pos = ftell(fd);
    *st = (char *)malloc((pos + 1) * sizeof(char));
    fseek(fd, 0, SEEK_SET);
    fread(*st, sizeof(char), pos * sizeof(char), fd);
    (*st)[pos] = '\0';
    fclose(fd);
    return 0;

}


// void draw_graph(std::string net_name,Q_list*& q){
//   ofstream fp;
//   int i=0;

//   std::string net=net_name+".assciiesrg";
//   fp.open(net.c_str(),ofstream::out);

//   if(!fp)
//     {
//       cerr << "Error opening output stream" << endl;
//       throw ExceptionIO();
//     }

//   fp << "digraph ESRG {" << endl << "nodesep=.05;" << endl;
//   fp << "node[shape=record,fontsize=10];" << endl;


//   Q_list_el *block=q->get_block();
//   Marking *m =NULL;

//   while (block!=NULL)
//     {
//       m=block->get_list_pmark();
//       //save the number of elements in the block

//       fp << "ESM" ;
//       fp <<  block->get_id() ;
//       fp << "[shape=record, tailport=e, headport=w, label=\"{";


//       char* st;
//       print_sr_ctmc (m,&st);
//       fp << "<esm" << block->get_id() << "> " ;
//       fp << "ESM" << block->get_id() << ": "<<st;
//       free(st);

//       if (m->get_type()!=INSTANCE)
// 	m=m->get_pointer(nxt);
//       if (m) fp << " | ";

//       while(m!=NULL) {
// 	print_state_ctmc (m, &st);
// 	fp << "<e" << m->get_id() << "> " << st ;
// 	free(st);
// 	m=m->get_pointer(nxt);
// 	if (m) fp << " | ";
//       }

//       fp << "}\"];" << endl ;

//       block=block->get_pointer(nxt);
//     }

//   block=q->get_block();

//   while (block!=NULL)
//     {
//       m=block->get_list_pmark();

//       while(m!=NULL)
// 	{
// 	  Instance * ins=NULL;
// 	  if (m->get_type()!=INSTANCE)
// 	    ins= m->get_arc(out_g);
// 	  else
// 	    ins= m->get_arc(out_i);

// 	  while (ins){

// 	    fp << "ESM" ;
// 	    fp <<  block->get_id() ;

// 	    if (m->get_type()==INSTANCE)
// 	      fp << ":e" << m->get_id() << "->";
//             else
// 	      fp << ":esm" << block->get_id() << "->";

// 	    fp << "ESM" << ins->get_des()->get_q_part()->get_id();

// 	    if(ins->get_des()->get_type()!=INSTANCE){
// 	      fp << ":esm" << ins->get_des()->get_q_part()->get_id() ;
// 	      fp << "[style=bold];"<< endl;
// 	    }
// 	    else
// 	      fp << ":e" << ins->get_des()->get_id() << ";" << endl ;

// 	    ins=ins->get_next();
// 	  }
// 	  m=m->get_pointer(nxt);
// 	}

//       block=block->get_pointer(nxt);
//     }
//   fp<<"}" << endl;
//   fp.close();

// }

void draw_graph(std::string net_name, Q_list *&q) {
    ofstream fp;
    int i = 0;

    std::string net = net_name + ".assciiesrg";
    fp.open(net.c_str(), ofstream::out);

    if (!fp) {
        cerr << "Error opening output stream" << endl;
        throw ExceptionIO();
    }

    fp << "digraph ESRG {" << endl << "nodesep=.1;" << endl;
    fp << "node [shape=box,fontsize=9];" << endl;
    fp << "compound=true;" << endl;
    fp << "ranksep=.1 ;" << endl;

    Q_list_el *block = q->get_block();
    Marking *m = NULL;

    while (block != NULL) {
        m = block->get_list_pmark();
        //save the number of elements in the block

        fp << "subgraph cluster" ;
        fp <<  block->get_id() ;
        fp << "{" << endl;
        fp << "node [style=filled,goup=" <<  block->get_id();
        fp << "];" << endl;

        char *st;
        print_sr_ctmc(m, &st);
        fp << "ESM" << block->get_id() << "[color=lightgrey, label=\"" << st;
        fp << "\"];" << endl;
        free(st);

        if (m->get_type() != INSTANCE) {
            fp << "label = \"ESM" << block->get_id() << ": SAT \";" << endl;
            m = m->get_pointer(nxt);
        }
        else {
            fp << "label = \"ESM" << block->get_id() << "\";" << endl;
        }

        while (m != NULL) {
            print_state_ctmc(m, &st);
            fp << "e" << m->get_id() << "[color=white, label=\"" << st ;
            fp << "\"];" << endl;
            free(st);
            m = m->get_pointer(nxt);
        }


        fp << "color = blue;" << endl;
        fp << "};" << endl ;

        block = block->get_pointer(nxt);
    }

    block = q->get_block();

    while (block != NULL) {
        m = block->get_list_pmark();

        while (m != NULL) {
            Instance *ins = NULL;
            if (m->get_type() != INSTANCE)
                ins = m->get_arc(out_g);
            else
                ins = m->get_arc(out_i);

            while (ins) {

                if (m->get_type() == INSTANCE)
                    fp << "e" << m->get_id() << "->";
                else
                    fp << "ESM" << block->get_id() << "->";


                if (m->get_type() != INSTANCE &&
                        ins->get_des()->get_type() != INSTANCE) {
                    fp << "ESM" << ins->get_des()->get_q_part()->get_id() ;
                    fp << "[ltail=cluster" << block->get_id() << "," ;
                    fp << " lhead=cluster" << ins->get_des()->get_q_part()->get_id() << "," ;
                    fp << "style=bold];" << endl;
                }
                else
                    fp << "e" << ins->get_des()->get_id() << ";" << endl ;

                ins = ins->get_next();
            }
            m = m->get_pointer(nxt);
        }

        block = block->get_pointer(nxt);
    }
    fp << "}" << endl;
    fp.close();

}



int Q_list::Compute_ESMEV() {
    int PEAK = 0;
    Q_list_el *block = q->get_block();
    while (block != NULL) {
        PEAK += block->get_num_elem();
        block = block->get_pointer(nxt);
    }
    return PEAK;
}


int Q_list::Compute_peak() {
    int PEAK = 0;

    Q_list_el *block = q->get_block();
    while (block != NULL) {
        Marking *m = block->get_list_pmark();
        //save the number of elements in the block
        if (m != NULL) {
            if ((m->get_type() == INSTANCE))
                PEAK += block->get_num_elem();
            else
                PEAK += 1;
        }
        block = block->get_pointer(nxt);
    }
    return PEAK;
}




#ifdef debug
int Q_list::Compute_Peak_withCache() {
    int PEAK = 0;

    Q_list_el *block = q->get_block();
    while (block != NULL) {
        Marking *m = block->get_list_pmark();
        //save the number of elements in the block
        if (m != NULL) {
            if ((m->get_type() == INSTANCE) && (block->get_num_elem() != block->get_tot_num_elem()))
                PEAK += block->get_num_elem();
            else
                PEAK += 1;
        }
        block = block->get_pointer(nxt);
    }
    return PEAK;
}
#endif




int print_state_ctmc(Marking *s, char **st) {
    FILE *fd;
    int pos;
    char  *mark = "states.mark";
    fd = fopen(mark, "w+");
    if (!fd) {
        cerr << "Error opening output stream" << endl;
        throw ExceptionIO();
    }
    load_sym_rep(s->get_srpos(), s->get_srlength(), s->get_srptr());
    if (s->get_type() == INSTANCE) {
        load_event(s->get_evlength(), s->get_evpos(), 1);
        NEW_ASYM_MARKING(&tabc, &net_mark, &card, &num, &tot,
                         MERG, ASYM_STATIC_STORE, RESULT);
    }
    get_canonical_marking();
    //write_on_srg(fd, (s == initial_marking ? 0 : 1) );
    write_on_srg(fd, 1);
    pos = ftell(fd);
    *st = (char *)malloc((pos + 1) * sizeof(char));
    fseek(fd, 0, SEEK_SET);
    fread(*st, sizeof(char), pos * sizeof(char), fd);
    (*st)[pos] = '\0';
    fclose(fd);
    return 0;
}



int print_ctrs(Marking *s, FILE *fd) {
    load_sym_rep(s->get_srpos(), s->get_srlength(), s->get_srptr());
    if (s->get_type() == INSTANCE) {
        load_event(s->get_evlength(), s->get_evpos(), 1);
        NEW_ASYM_MARKING(&tabc, &net_mark, &card, &num, &tot,
                         MERG, ASYM_STATIC_STORE, RESULT);
    }
    get_canonical_marking();
    code_marking();
    write_ctrs(fd);
    return 0;
}

void test_stampa1(std::string net_name, Q_list *&q, int type_arc) {

    Q_list_el *tmp = q->get_block();
    Marking *mk = NULL;
    Instance *tmp_arc = NULL;
    char *st;
    ofstream fp, fp1;

    std::string net = net_name + ".RESRG";
    std::string net1 = net_name + ".RESRS";
    fp.open(net.c_str(), ofstream::out);
    if (!fp) {
        cerr << "Error opening output stream" << endl;
        throw ExceptionIO();
    }
    fp1.open(net1.c_str(), ofstream::out);
    if (!fp1) {
        cerr << "Error opening output stream" << endl;
        throw ExceptionIO();
    }
    if (type_arc)
        fp << "\n\nINPUT ARCS\n\n";
    else
        fp << "\n\nOUTPUT ARCS\n\n";
    while (tmp) {
        fp << "Begin block:" << tmp->get_id() << " elements number: " << tmp->get_num_elem() << endl;
        mk = tmp->get_list_pmark();
        //int step=1; //per genrare il file ctrs
        while (mk) {
            print_state_ctmc(mk, &st);
            //step++;
            fp << "\n\t Node id: " << mk->get_id() << " type:" << mk->get_type();
            fp1 << "Node id: " << mk->get_id() << " type" << mk->get_type() << "\n" << st;
            if (type_arc)
                tmp_arc = mk->get_arc(in_i);
            else
                tmp_arc = mk->get_arc(out_i);
            if (tmp_arc == NULL) {
                fp << "Type list: gen\n";
                if (type_arc)
                    tmp_arc = mk->get_arc(in_g);
                else
                    tmp_arc = mk->get_arc(out_g);
            }
            else {
                fp << "Type list: inst\n";
            }
            while (tmp_arc != NULL) {
                fp << "\n\t\tQ block: " << tmp_arc->get_des()->get_q_part()->get_id() << " elem id:" << tmp_arc->get_des()->get_id() << " with weight:" << tmp_arc->get_card()*tmp_arc->get_weight();
                tmp_arc = tmp_arc->get_next();
            }
            fp << endl;
            mk = mk->get_pointer(nxt);
        }
        tmp = tmp->get_pointer(nxt);
        fp << "\nend block\n";
    }

}




void Compute_denom(Q_list *&q) {
    Q_list_el *block = q->get_block();
    double denom;
    double gen_denom;
    while (block != NULL) {
        Marking *m = block->get_list_pmark();
        //save the number of elements in the block
        if (m != NULL) {
            Instance *tmp_arc = m->get_arc(out_g);
            while (tmp_arc != NULL) {
                gen_denom += tmp_arc->get_card() * tmp_arc->get_weight();
                tmp_arc = tmp_arc->get_next();
            }
        }
        while (m != NULL) {
            Instance *tmp_arc = m->get_arc(out_i);
            denom = gen_denom;
            while (tmp_arc != NULL) {
                denom += tmp_arc->get_card() * tmp_arc->get_weight();
                tmp_arc = tmp_arc->get_next();
            }
            m->set_denom(denom);
            m = m->get_pointer(nxt);
        }
        block = block->get_pointer(nxt);
        gen_denom = 0;
    }

}

void Compute_denom(Q_list_el *&block) {
    double denom = 0.0;
    double gen_denom = 0.0;
    Marking *m = block->get_list_pmark();
    //save the number of elements in the block
    if (m != NULL) {
        Instance *tmp_arc = m->get_arc(out_g);
        while (tmp_arc != NULL) {
            gen_denom += tmp_arc->get_card() * tmp_arc->get_weight();
            tmp_arc = tmp_arc->get_next();
        }
    }
    while (m != NULL) {
        Instance *tmp_arc = m->get_arc(out_i);
        denom = gen_denom;
        while (tmp_arc != NULL) {
            denom += tmp_arc->get_card() * tmp_arc->get_weight();
            tmp_arc = tmp_arc->get_next();
        }
        m->set_denom(denom);
        m = m->get_pointer(nxt);
    }
}

ostream &operator<<(ostream &out, pair < class Q_list_el *, Direct_Type> pr) {
    out << "-----------------------------------------";
    out << "\nNumber of element: " << pr.first->get_num_elem() << endl;
    Marking *temp = pr.first->get_list_pmark();
    while (temp != NULL) {
        out << "\nElem id: " << temp->get_id();
        switch (temp->get_type()) {
        case 0:
            out << "\tType: INST" << endl;
            break;
        case 1:
            out << "\tType: MACROC" << endl;
            break;
        case 2:
            out << "\tType: MACROP" << endl;
            break;
        default:
            out << "Type: UNK" << endl;
            Exception obj("\n\n KNOWN TYPE!!!\n\n");
            throw (obj);
            break;
        }
        //print marking
        char *s = (char *) calloc(1000, sizeof(char));
        print_state_ctmc(temp, &s);
        cout << "Marking:\n" << s;
        free(s);
        //print markig
        Ins_Type inst_arc, gen_arc;
        if (pr.second == in) {
            inst_arc = in_i;
            gen_arc = in_g;
        }
        else {
            inst_arc = out_i;
            gen_arc = out_g;
        }
        if (temp->get_type() != 0) {
            if (pr.second == in)
                cout << "\n****************\nGeneric Input:\n";
            else
                cout << "\n****************\nGeneric Output:\n";
            class Instance *tempI = temp->get_arc(gen_arc);
            while (tempI != NULL) {
                if (tempI->get_des() == NULL) {
                    Exception obj("\n\n ERROR ELEMENT\n\n");
                    throw (obj);
                }
                if ((tempI->get_des())->get_q_part() == NULL) {
                    Exception obj("\n\n ERROR PARTITION\n\n");
                    throw (obj);
                }
                cout << "\tMarking: " << (tempI->get_des())->get_id() << " Id Block: " << ((tempI->get_des())->get_q_part())->get_id() << endl;
                cout << "\t rate: " <<  tempI->get_weight()*(tempI->get_card()) << " trans :" << tabt[tempI->get_trans()].trans_name;
                tempI = tempI->get_next();
                cout << endl;
            }
            cout << "\n****************\n";
        }
        if (pr.second == in)
            cout << "\n****************\nInst. Input:\n";
        else
            cout << "\n****************\nInst. Output:\n";
        class Instance *tempI = temp->get_arc(inst_arc);
        while (tempI != NULL) {
            cout << "\tMarking reached: " << (tempI->get_des())->get_id() << " Id Block: " << ((tempI->get_des())->get_q_part())->get_id() << endl;
            cout << "\t rate: " <<  tempI->get_weight()*(tempI->get_card()) << " trans :" << tabt[tempI->get_trans()].trans_name;
            tempI = tempI->get_next();
            cout << endl;
        }
        cout << "\n****************\n";
        temp = temp->get_pointer(nxt);
    }
    out << "\n-----------------------------------------\n";
    return out;
}

void write_block(ofstream &fp, Q_list *&q) {
    Q_list_el *block = q->get_block();
    double denom;
    double gen_denom;
    fp << q->get_num_elem() << endl;
    while (block != NULL) {
        Marking *m = block->get_list_pmark();
        if (m != NULL) {
            fp << block->get_id() << " ";
            if (m->get_type() == MACROC) {
                fp << "-1\n" << m->get_id() << " " << m->get_srpos() << " " << m->get_srlength() << " " << m->get_srptr()     << endl;
            }
            else {
                fp << block->get_num_elem() << endl;
                while (m != NULL) {
                    fp << m->get_id() << " " << m->get_srpos() << " " << m->get_srlength() << " " << m->get_srptr()     << " " << m->get_evpos() << " " << m-> get_evlength() << endl;
                    m = m->get_pointer(nxt);
                }
            }
        }
        else {
            Exception obj("\n\n Error empty block \n\n");
            throw (obj);
        }
        block = block->get_pointer(nxt);
    }
}

void genEX_DTMC(std::string net_name, Q_list *&q) {
    ofstream fp;
    std::string net = net_name + ".resrg";
    map <int, double>::iterator iter;
    fp.open(net.c_str(), ofstream::out);
    if (!fp) {
        Exception obj("\n\n Error on the DTMC output file\n\n");
        throw (obj);
    }
    Q_list_el *tmp = q->get_block();
    Instance *tmp_arc = NULL;
    while (tmp != NULL) {
        map<int, double> reach;
        double norm = 0;
        if (tmp->get_list_pmark()->get_type() == MACROC) {
            tmp_arc = tmp->get_list_pmark()->get_arc(out_g);
            while (tmp_arc != NULL) {
                //test if the generic arc point to a macroc that has not already been instanced.
                if ((tmp_arc->get_des()->get_q_part() != NULL) &&
                        (tmp_arc->get_des()->get_q_part()->get_num_elem() != 0) &&
                        (tmp_arc->get_des()->get_id() ==
                         tmp_arc->get_des()->get_q_part()->get_list_pmark()->get_id())) {
                    norm += tmp_arc->get_card() * tmp_arc->get_weight();
                    reach[tmp_arc->get_des()->get_q_part()->get_id()] += tmp_arc->get_card() * tmp_arc->get_weight();
                }
                else {
                    //case macroc has already instanced
                    int card_mark = 0;; //number of SM contains in the MacroC
                    map<int, int> tmp_reach; //int block reached, int number of element;
                    Marking *tmp_marking = tmp_arc->get_des()->get_pointer(nxt_sb);
                    while (tmp_marking != NULL) {
                        //get the information about the block reach
                        card_mark++;
                        tmp_reach[tmp_marking->get_q_part()->get_id()] = tmp_marking->get_q_part()->get_num_elem();
                        tmp_marking = tmp_marking->get_pointer(nxt_sb);
                    }//get the information about the block reach
                    map <int, int>::iterator iter1;
                    for (iter1 = tmp_reach.begin(); iter1 != tmp_reach.end(); iter1++) {
                        reach[iter1->first] += tmp_arc->get_card() * tmp_arc->get_weight() * iter1->second / card_mark;
                        norm += tmp_arc->get_card() * tmp_arc->get_weight() * iter1->second / card_mark;
                    }
                }//case macroc has already instanced
                tmp_arc = tmp_arc->get_next();
            }
            for (iter = reach.begin(); iter != reach.end(); iter++) {
                iter->second = iter->second / norm;
            }
        }
        else {
            Marking *tmp_elem = tmp->get_list_pmark();
            while (tmp_elem != NULL) {
                tmp_arc = tmp_elem->get_arc(out_i);
                map<int, double> out_arc;
                norm = 0;
                while (tmp_arc != NULL) {
                    norm += tmp_arc->get_card() * tmp_arc->get_weight();
                    out_arc[tmp_arc->get_des()->get_q_part()->get_id()] += tmp_arc->get_card() * tmp_arc->get_weight();
                    tmp_arc = tmp_arc->get_next();
                }
                for (iter = out_arc.begin(); iter != out_arc.end(); iter++) {
                    reach[iter->first] += (iter->second) / (tmp->get_num_elem() * norm);
                }
                tmp_elem = tmp_elem->get_pointer(nxt);
            }
        }
        //	fp<<tmp->get_id()<<" "<<reach.size()<<endl;
        iter = reach.begin();
        while (iter != reach.end()) {
            fp << tmp->get_id() << " " << iter->second << " " << iter->first << endl;
            iter++;
        }
        tmp = tmp->get_pointer(nxt);
    }
    fp.close();
}

void genST_DTMC(std::string net_name, Q_list *&q) {
    ofstream fp;
    std::string net = net_name + ".resrg";
    map <int, double>::iterator iter;
    fp.open(net.c_str(), ofstream::out);
    if (!fp) {
        Exception obj("\n\n Error on the DTMC output file\n\n");
        throw (obj);
    }
    Q_list_el *tmp = q->get_block();
    Instance *tmp_arc = NULL;
    while (tmp != NULL) {
        map<int, double> reach;
        double norm = 0;
        if (tmp->get_list_pmark()->get_type() == MACROC) {
            tmp_arc = tmp->get_list_pmark()->get_arc(out_g);
            while (tmp_arc != NULL) {
                //test if the generic arc point to a macroc that has not already been instanced.
                if ((tmp_arc->get_des()->get_q_part() != NULL) && (tmp_arc->get_des()->get_q_part()->get_num_elem() != 0) && (tmp_arc->get_des()->get_id() == tmp_arc->get_des()->get_q_part()->get_list_pmark()->get_id())) {
                    norm += tmp_arc->get_card() * tmp_arc->get_weight();
                    reach[tmp_arc->get_des()->get_q_part()->get_id()] += tmp_arc->get_card() * tmp_arc->get_weight();
                }
                else {
                    //case macroc has already instanced
                    /*Marking *tmp_elem=tmp->get_list_pmark()->get_pointer(nxt_sb);
                    if (tmp_elem==NULL)
                    	{
                    	Exception obj("\n\n Error block \n\n");
                    	throw (obj);
                    	}
                    else
                    	{*/
                    norm += tmp_arc->get_card() * tmp_arc->get_weight();
                    reach[tmp_arc->get_des()->get_pointer(nxt_sb)->get_q_part()->get_id()] += tmp_arc->get_card() * tmp_arc->get_weight();
                    /*
                    tmp_arc=tmp_elem->get_arc(out_i);
                    map<int,double> out_arc;
                    norm=0;
                    while(tmp_arc!=NULL)
                    	{
                    	norm+=tmp_arc->get_card()*tmp_arc->get_weight();
                    	out_arc[tmp_arc->get_des()->get_q_part()->get_id()]+=tmp_arc->get_card()*tmp_arc->get_weight();
                    	tmp_arc=tmp_arc->get_next();
                    	}
                    for(iter=out_arc.begin();iter!=out_arc.end();iter++)
                    	{
                    	reach[iter->first]+=(iter->second)/norm;
                    	}

                    }*/

                }//case macroc has already instanced
                tmp_arc = tmp_arc->get_next();
            }
            for (iter = reach.begin(); iter != reach.end(); iter++) {
                iter->second = iter->second / norm;
            }
        }
        else {
            Marking *tmp_elem = tmp->get_list_pmark();
            tmp_arc = tmp_elem->get_arc(out_i);
            map<int, double> out_arc;
            norm = 0;
            while (tmp_arc != NULL) {
                norm += tmp_arc->get_card() * tmp_arc->get_weight();
                out_arc[tmp_arc->get_des()->get_q_part()->get_id()] += tmp_arc->get_card() * tmp_arc->get_weight();
                tmp_arc = tmp_arc->get_next();
            }
            for (iter = out_arc.begin(); iter != out_arc.end(); iter++) {
                reach[iter->first] += (iter->second) / norm;
            }
        }
        //fp<<tmp->get_id()<<" "<<reach.size()<<endl;
        iter = reach.begin();
        while (iter != reach.end()) {
            fp << tmp->get_id() << " " << iter->second << " " << iter->first << endl;
            iter++;
        }
        tmp = tmp->get_pointer(nxt);
    }
    fp.close();
}


