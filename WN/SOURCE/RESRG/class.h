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
/*! \file class.h
*/

//!Data structures requires by an efficient implementation of  Paige and Tarjan's algorithm with arcs weight

#include "macro.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <map>
#include <list>
#include <utility>
#include <stdlib.h>

#define TEST 0
#define TEST1 0
#define TESTSTAMPA 0
#define debug 1


using namespace std;

extern "C" {
#include "../../INCLUDE/const.libmcesrg"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/const.h"
//#include "../../INCLUDE/decl.h"
//#include "../../INCLUDE/macros.h"
//#include "../../INCLUDE/fun_ext.h"
#include "../../INCLUDE/ealloc.h"

    /************** extern function ************/

//extern void q_out();
//extern int print_marking ( );

// greaspn functions ////////

// extern char *emalloc ( size_t sz);
// extern FILE *efopen (const char * file,const char *mode );
// extern void *ecalloc ( size_t nitm, size_t sz );
    extern char *emalloc(size_t sz);
    extern FILE *efopen(const char *file, const char *mode);
    extern void *ecalloc(size_t nitm, size_t sz);
    extern Event_p get_new_event(int);
    extern TO_MERGEP *TO_MERGE(int, COLORS *);
    extern int  initialize(int  argc,  char **);
    extern void INIT_GLOBAL_VARS();
    extern void STORE_STATIC_CONF(STORE_STATICS_ARRAY *, COLORS *);
    extern void GROUPING_ALL_STATICS(TO_MERGEP *, COLORS *, int **);
    extern void NEW_SIM_STATIC_CONF(STORE_STATICS_ARRAY *);
    extern void MY_INIT_ALLOC();
    extern void create_canonical_data_structure();
    extern void CHAR_LOAD_COMPACT(unsigned long *);
    extern void POP_STATIC_CONF(STORE_STATICS_ARRAY, COLORS **);
    extern void string_to_marking(unsigned long,  unsigned long,  unsigned long);
    extern void GET_EVENTUALITIE_FROM_FILE(int, int);
    extern void COPY_CACHE(int);
    extern void STRING_TO_EVENTUALITIE();
    extern int  X_L_MOINS_1(int, int, int, int);
    extern int  C_L_PLUS_1(int, int, int);
    extern int  X_2(int, int, int, int);
    extern int  X_I_MOINS_1(int, int, int, int);
    extern void NEW_ASYM_MARKING(COLORS **, NET_MARKING **, int ** **, int ** *, int **,
                                 DECOMP_TO *, STORE_STATICS_ARRAY, int ** **);
    extern void my_en_list(int);
    extern void copy_event(Event_p,  Event_p);
    extern void fire_trans(Event_p);
    extern void SPECIAL_CANISATION();
    extern void STORE_CARD(int ** *, int ** **);
    extern void GET_SYMETRIC_MARKING(COLORS **, NET_MARKING **, int ** **, int ** *, int **,
                                     STORE_STATICS_ARRAY, TO_MERGEP *);
    extern void ALL_MY_GROUP(int ** *);
    extern void get_canonical_marking();
    extern void AFTER_CANONISATION(int **, int ** *);
    extern void GET_EVENT_FROM_MARKING(COLORS *tabc, int *, int **, int ** *, int **,
                                       STORE_STATICS_ARRAY, int ** *, TO_MERGEP *);
    extern void INIT_ARRAYS(TO_MERGEP *, int **, int ** *, STORE_STATICS_ARRAY);
    extern void dispose_old_event(Event_p);
    extern void EVENTUALITIE_TO_STRING();
    extern void ADD_EVENTUALITIE_TO_FILE(int);
    extern void MY_CHAR_STORE(unsigned long);
    extern void INIT_4D_MATRIX(MATRIX **);
    extern void INIT_ARRAYS(TO_MERGEP *, int **, int ** *, STORE_STATICS_ARRAY);

    extern void write_on_srg(FILE *, int);
    extern void write_ctrs(FILE *);
    extern void store_compact(int, FILE *);
    extern void store_double(double *, FILE *);
    extern void code_marking(void);
    extern int my_strcmp(unsigned long, unsigned long, unsigned long, unsigned long);
    extern unsigned long marking_to_string();
    extern void WRITE_ON_ESRG(FILE *);

/////////////////////////////

    /***************** global variables ********/
// greatspn variables //////
    extern Result_p enabled_head;
    extern Canonic_p sfl_h;
///////////////////////////

};





struct el {
    map <int, double> points;
};

//!Exception for Input/Output
struct ExceptionIO {};
//!Exception for operation
struct ExceptionOper {};


//!Enum Ins
/*!
Different type of Elements:
GENE, INS
*/
enum Ins {
    GENE = 0,
    INS  = 1
};

//!Enum Type
/*!
Different type of Elements:
INSTANCE, MACROC, MACROP, Unk, Dis
*/
enum Type {
    INSTANCE = 0,
    MACROC = 1,
    MACROP = 2,
    Unk = -1,
    Dis = -2
};

//!Enum Ins_Type
/*!
Different type of Arcs:
in_g, out_g, in_i, out_i
*/
enum Ins_Type {
    in_g = 0,
    out_g = 1,
    in_i = 2,
    out_i = 3
};

enum Direct_Type {
    in = 0,
    out = 1,
};
//!Enum Prt_Type
/*!
Different type of pointer:
nxt, prv, nxt_sb
*/
enum Ptr_Type {
    nxt = 0,
    prv = 1,
    nxt_sb = 2
};

class Marking;
class Q_list_el;
class X_list_el;
class X_list;
class Q_list;
//!Class Exception
/*!
This class is used for managing the exception
*/
class Exception {

    std::string mess;
public:
    //! Empty constructors
    Exception() {mess = "";};
    //! constructor takes in input a message
    Exception(std::string mess) {this->mess = mess;};
    //! \name Get of methods use to access at the data structures
    //@{
    //!it returns message
    std::string get(void) {return mess;};
    //@}
};


//!Class Instance
/*!
This class represents the firing of transitions that moves the system from a state (Marking) to another state (Marking) pointed by "des"
*/
class Instance {

public:
    //! Empty constructors
    Instance();
    //! Copy constructor
    Instance(const Instance &);

    //!Constructor used to construct the instances
    Instance(double weight, int trans, int card,
             int **col, Marking *dest);
    //!Constructor used to construct the instance
    Instance(double weight, int trans, int card, int **col);

    //!Destructor
    ~Instance();
    //! \name Set of methods use to access at the data structures
    //@{
    //!returns the number of  ordinary firing's instance
    int        get_card() const;
    //!returns the binding of color object firing's instance
    int      **get_col() const;
    //!returns a pointer to the reached marking by firing's instance
    Marking   *get_des() const;
    //!returns a pointer to the next  enabled instance from the same source marking
    Instance  *get_next() const;
    //!returns the transition id
    int        get_trans() const;
    //!returns the transition rate
    double     get_weight() const;

    //@}

    //! \name Set of methods use to set up  the data structures
    //@{


    //!sets up the number of  ordinary firing's ins
    int      &set_card();
    //!sets up the binding of color object firing's instance
    void      set_col(int **);
    //!sets up the reached marking by firing's instance
    void      set_des(Marking *);
    //!sets up a pointer to the next  enabled instance from the same source marking
    void      set_next(Instance *);
    //!sets up the transition id
    int      &set_trans();
    //!sets up the transition rate
    double   &set_weight();
    //@}

private :
    //!rate transition
    double      weight;
    //!transition id
    int         trans;
    //!number ordinary instance
    int         card;
    //!binding color object
    int       **col;
    //!reached marking by firing's instance
    Marking    *des;
    //!next enabled instance from the same source marking
    Instance   *next;
};


//!Class Marking
/*!
This class represents a Marking. Three different types of Marking can be possible: INSTANCE (eventuality), MACROC (uniform or saturate symmetric) ,MACROP (saturate asymmetric)
*/
class Marking {

public:

    //!Empty constructor
    Marking();
    //! Constructor
    /*! Used to create a new marking.
     It takes in input the block Q containing the element, the marking id and the type.
     It sets up the fields: id, type, part_q
    */
    Marking(Q_list_el *,
            int ,
            unsigned long,
            unsigned long,
            unsigned long,
            unsigned long,
            unsigned long,
            Type);
    //!Copy constructor
    Marking(const Marking &);
    //!destructor;
    ~Marking();

    //! \name Set of methods use to access at the data structures
    //@{
    //!returns a pointer to arc
    /*!The parameter Ins_Type is used to select the type of arc (in_generic, out_generic, in_inst, out_inst)*/
    Instance  *get_arc(Ins_Type) const;
    //!returns marking id
    int             get_id() const;
    unsigned long   get_srpos() const;
    unsigned long   get_srlength() const;
    unsigned long   get_srptr() const;
    unsigned long   get_evpos() const;
    unsigned long   get_evlength() const;
    const double    get_denom() {return denom;};

    //!returns type of marking
    Type       get_type() const;
    //!returns the pointer to the Q block that contains the marking
    Q_list_el *&get_q_part();
    //!returns a pointer to a marking
    /*!The parameter Ptr_Type is used to select between next, previous, next_sb marking*/
    Marking   *get_pointer(Ptr_Type) const;
    //@}

    //! \name set of methods use to change the data structures
    //@{
    //!sets up the marking id
    void       set_id(int, unsigned long,
                      unsigned long,
                      unsigned long,
                      unsigned long,
                      unsigned long);
    //!sets up the marking type
    void       set_type(Type);
    //!sets up  the  pointer to an arc
    /*!The parameter Ins_Type is used to select the type of arc (in_generic, out_generic, in_inst, out_inst), while the  parameter Instance* is a pointer to the firing instance*/
    void       set_arc(Instance *, Ins_Type);
    //!sets up the pointer to the block Q witch contains the marking
    void       set_q_part(Q_list_el *);
//!sets up a pointer to a marking
    /*!The parameter Ptr_Type is used to select between next, previous, next_sb marking, while the parameter Marking* is a pointer to another marking*/
    void       set_pointer(Marking *, Ptr_Type);
    void 	set_denom(double denom)
    {this->denom = denom;};

//@}

private :
    //!id marking
    int id;
    //!file information
    unsigned long file_sr;
    unsigned long file_srlg;
    unsigned long file_srptr;

    unsigned long file_ev;
    unsigned long file_evlg;

    //!type marking: INSTANCE (eventuality), MACROC (uniform or saturate symmetric) ,MACROP (saturate asymmetric)
    Type type;
    //!input  inst. arc
    Instance   *in_inst;
    //!input gen.  arc
    Instance   *in_gen;
    //!output inst arc
    Instance   *out_inst;
    //!output gen. arc
    Instance   *out_gen;
    //! Q's block contains this marking
    Q_list_el  *part_q;
    //! next marking in the same block Q
    Marking    *next;
    //! previous marking in the same block Q
    Marking    *prev;
    //! necessary  to know the eventualities already found
    Marking    *next_ev;
    double denom;
};


//!Class Q_list_el
/*!
This class  represents  a current aggregation of some  markings;
*/
class  Q_list_el {

public :

    //!Empty constructor;
    Q_list_el();
    //!Constructor
    /*! The identifier of the new element
    is passed as a parameter */
    Q_list_el(int);
    //!Constructor
    /*! The identifier of the new element
    is passed as a parameter with a new list
     of Markings*/
    Q_list_el(int, Marking *);
    //!Constructor
    /*! The number of elements of the new element
    is passed as a parameter with a new list
     of Markings*/
    Q_list_el(Marking *, int);
    Q_list_el(Q_list_el &);
    //! \name Set of methods use to access at the data structures
    //@{
    //! returns the number of elements of the block
    int get_num_elem() const;
    //! returns the identifier of the block
    int get_id() const;
    //! returns a pointer to the list of markings
    Marking *get_list_pmark() const;
    //!returns a pointer to a Q's block
    /*!The parameter Prt_Type is used to select between next, previous, next_subset marking*/
    Q_list_el *get_pointer(Ptr_Type) const;
    //!returns a pointer to the X's block containing the Q's block
    X_list_el *get_partx()const;
    //!returns a map represented the e_b list
    map<class Marking *, double> creat_E_B(Ins_Type, Ins_Type);
#ifdef debug
    //! returns the total element number of the block befor any split
    int get_tot_num_elem() const;
#endif
    //@}

    //! \name set of methods use to change the data structures
    //@{
    //!sets up the elements number
    void set_num_elem(int);
    //!increases of one the elements number
    void num_elem_plusplus();
    //!sets up the id
    void set_id(int);
    //    void discard(Marking*);

#ifdef debug
    //!sets up the total elements number of a block  before any split
    void set_tot_num_elem(int);
#endif
    //!set up the pointer pmark to the marking
    void set_pmark(Marking *);

    //!adds a marking in the top of the list
    void add_pmark_top(Marking &, Type);

    //!adds to add a marking and set the Type
    void add_pmark(Marking &, Type);

    //!adds to add a marking
    void add_pmark(Marking *);

    //!Remove the  marking with id = int and type= Type from the block
    Marking *remove(int, Type);

    //!sets up a pointer to Q's block
    /*!The parameter Prt_Type is used to select between next, previous, next_subset marking*/
    void set_pointer(Q_list_el *, const Ptr_Type);

    //!sets up a pointer to the X's block containing the Q's block
    void set_partx(X_list_el *);

    //!phase preslit (exact lumpability)
    //!It similar to presplit() function but it computer only the total in/out rate for every markings in the Qblock
    void presplit_rateout(map <int, double> &, Ins_Type);

    //!phase presplit
    void presplit(list <int> &, map <int, struct el> &, Ins_Type);

    //!phase split (exact lumpability)
    //!This method is similar to split but it split the block only whit respet to total in/out rate
    void split_rateout(map <int, double> m_out, Q_list_el *&temp);

    //!phase split
    void  split(list <int> &, map <int, struct el> &, Q_list_el *&);

    //!operator presplit
    void op_presplit(Marking *l_split, Q_list_el *t_block,
                     int level, X_list *xblock);
    //!Redefinition print operator
    friend ostream &operator<<(ostream &out, pair <class  Q_list_el *, Direct_Type>);
    //@}
private :
    //! id block
    int        id;
    //! number of marking in this block
    int        num_elem;

#ifdef debug
    //! number of total  marking in this block before any split
    int	tot_num_elem;

#endif
    //! list of marking contained in the block
    Marking   *pmark;
    //! X's block containing this block
    X_list_el    *part_x;
    //! the next  block
    Q_list_el *next;
    //! the previous block
    Q_list_el *prev;
    //! the next block contained in the same X's block
    Q_list_el *next_subset;

};

//!Class Q_list
/*!
represents the current partition; every element of the list, called block, is an aggregate
*/
class Q_list {

public:

    //!Empty constructor
    Q_list();
    //!Constructor
    //!It takes in in input   the net name
    Q_list(const std::string &);
    //!returns the fisrt Q's block
    Q_list_el *get_block() const;
    //!returns  the number of elements
    int get_num_elem() const;

    //! adds a new element in the Q list  at the bottom
    inline  void insert_elem(Q_list_el *const &);

    //! search the marking in every block  that with id = dest
    Marking *search(int dest);

    //!search the marking with id = des and update the list out
    Marking *search(int dest, int trans, double
                    weight, Marking *p_elem);

    //!removes the block with id = int (not efficient)
    void removeI(int);
//!removes the block using the pointer el
    void remove(Q_list_el *&el);
    //!check the lumpability
    int  lumpability();
    //!create the liste E_B
    Instance *creat_E_B();
    //!return the number of ESMs+EVs in the Q list
    int Compute_ESMEV();
    //!compute the computation peak
    int Compute_peak();
    //!
#ifdef debug
    //!compute the computation peak considering cache
    int Compute_Peak_withCache();
#endif
private :

    Q_list_el *head;  //!<the first Q's block
    int num_elem;     //!<the number of element in the Q list

};


//!Class X_list_el
/*!
This class  represents  a  block of the list X
*/
class X_list_el {
public:

    //!Empty constructor
    X_list_el();
    //!Constructor
    //!It takes in in input   a Q's block
    X_list_el(int, Q_list_el *);
    //! \name Set of methods use to access at the data structures
    //@{
    //! returns the number of elements of the block
    int get_num_elem() const;
    //! returns the identifier of the block
    int get_id() const;
    //!returns a pointer to a block
    /*!The parameter Prt_Type is used to select between next, previous*/
    X_list_el *get_pointer(Ptr_Type) const;
    //!returns a pointer to the first Q's block contained in this X block
    Q_list_el *get_subset() const;
    //@}
    //! \name set of methods use to change the data structures
    //@{
    //!sets up the elements number
    void set_num_elem(int);
    //!increases of one the elements number
    void num_elem_plusplus();
    //!sets up the subset pointer
    void set_subset(Q_list_el *);

    //!add a new  Q's block in the X's block
    void add_subset(Q_list_el *&);

    //!sets up a pointer to X's block
    /*!The parameter Prt_Type is used to select between next, previous, next_subset marking*/
    void set_pointer(X_list_el *, const Ptr_Type);

    //!removes from the X's block a Q'block with  id = int
    void remove(int);
    // //!sets up a new elements
    /// void set_elem(int,Q_list_el*);
    //@}


private :
    int id; //!< id X's block
    int num_elem; //!<number of Q's block in this X's block
    X_list_el  *next; //!<next X's block
    X_list_el  *prev;  //!<previus X's block
    Q_list_el  *subset;  //!<Q's blocks in this X's block

};

//!Class X_list
/*!
in addition to the current partition Q, the algorithm maintains another partition X
such that Q is a refinement of X and Q satisfies lumpability condition with respect to every block of
X.
*/
class X_list {
public:
    //!Empty constructor
    X_list();
    //! returns the number of elements of the block
    int get_num_elem() const;
    //! returns the number of elements increased of one unit
    int get_num_elem_plus();
    //! adds a new element in the Q list  at the bottom
    void insert_elem(X_list_el *const &);

private :

    X_list_el *head;  //!<the first X's block
    int num_elem;     //!<the number of element in the X list
};


//!Class C_list
/*!
 This class maintains the blocks of $X$, which are compound.
*/
class C_list {

public:
    C_list();//!>Empty constructor
    C_list(X_list_el *);
    //!>Constructor
    //!>create one block of list C which contains the X's block (var subset)
    void insert_elem(X_list_el *const &);//insert a new block of C containing the X'block passed by parameter
    //Q_list * find_b(int* delete_c); //find the block b
    //!merges a C'block to the list of C
    friend void insert_block(C_list *&, C_list *const &);
    //sets up the pointer to the next block
    void set_pointer(C_list *&);
    //!return the first element in the list (this element is removed by the list)
    friend C_list *top(C_list *&);
//! \name Set of methods use to access at the data structures
//@{
    //!returns a pointer to the next block
    C_list *get_pointer() const;
    //!returns a pointer to a X'sblock
    X_list_el *get_subset() const;
//@}
//!removes a X's block with id = int
    friend void remove_x(int, C_list *&);
private :
    C_list  *next;  //next C block
    X_list_el  *subset;  //X blocks in this C block
};



//!This function parses the binding of a transition
Instance *parse_binding(istringstream &,
                        int *, int *, int *, int *,
                        int *, int *, int *);
//!This function parses the binding of a transition
void initial_global_variables(char *net_name);

//!This function adds a new symmetric
/*! according to the  "re_sr_id" identifier,
 "add_sym" adds a new symmetric representation
 to the "sym_rep" array.*/
Q_list_el *add_sym(int, unsigned long,
                   unsigned long,
                   unsigned long);

//!This function inserts a new eventuality
//! according to the "re_sr_id" and  the "re_ev_id" identifiers,
//! "add_event" adds a new eventuality
//! to the "events" array.
Q_list_el *add_event(int,
                     unsigned long,
                     unsigned long,
                     unsigned long,
                     int,
                     unsigned long ,
                     unsigned long);

//!This function creates an arc between SR
//! according to the "source" and the "reached" ESMs,
//! "join_sr_to_sr" creates the correct arc
//! joining their symmetrical representations (sr).
void join_sr_to_sr(int source , int reached,
                   Instance *arc, Instance *rev_arc);

//!This function creates an arc between eventualities
//! according to the "source" and the "reached" ESMs,
//! "join_ev_to_ev" creates the correct arc
//! joining two from their eventualities (sr).
void join_ev_to_ev(int source , int reached,
                   Instance *arc, Instance *rev_arc) ;

//!This function creates an arc between an eventuality and a SR
//! according to the "source" and the "reached" ESMs,
//! "join_ev_to_sr" creates the correct arc
//! joining an eventuality (ev) of the "source" node
//! to the symmetrical representation (sr) of the reached ESM.
void join_ev_to_sr(int source , int reached,
                   Instance *arc, Instance *rev_arc);

//!This function inserts an arc
//! "add_arcs" treats the different cases of arcs :
//! ev --> ev, ev --> sr or sr --> sr.
void add_arcs(int source,
              int  re_sr_id ,
              unsigned long srpos,
              unsigned long srlg,
              unsigned long srptr,
              int  re_ev_id,
              unsigned long evpos,
              unsigned long evlg,
              Instance *arc, Instance *rev_arc, int flag);

//!This function uses to build Q
//! this function parses the binding string, creates the arcs and adds them
//! in the structure (Q).
void parse_binding_and_add_(ifstream &, int, int, Ins);

//!This function uses to treat a special case
//! this function is used to treat the cases
//! when the ESM is not initialy saturated and
//! becomes by construction.
void treat_special_cases(int sr);


//!This function is used to close
//!correctly all opened files.
void finalize_session();

//!This function build Q list
//! After the parse, the Q list is constructed
//! by using the function Create_Q.
Q_list *Create_Q(char *net_name);

//!Parser function
//! This function represents the main part of the parser
void  OutPutParser(char *net_name);

//! This function load the symmetrical representation
//! load the symmetrical representation
//! from the ".mark" file using the traditional
//! params. stored  in the ".off_sr" file.
void load_sym_rep(unsigned long  mark_pos,
                  unsigned long  length,
                  unsigned long  d_ptr);

//!This function load the eventualities
//! load an eventuality from the .event
//! file, using the "pos" position in the "off_ev"
//! file to get the position "d_ptr" and the length
//! "length" of the eventuality.
int load_event(unsigned long  llength,
               unsigned long  ld_ptr,
               int flag);

//!This function is used to compare a non ordinary strings
int strings_compare(int length_to_compare);

//!This function instances a generic arc
void copy_and_inst_arcs();

void add_event_mod(Marking *macrox,
                   int re_ev_id,
                   unsigned long pos,
                   unsigned long lg);

//!This function is used to compute all eventualities of the macro node "macro".
void split_macro(int clas, int sbc, int lin, int col,
                 Marking * (*traitement)(Marking *),
                 Marking *macro);


//!This function updates the the input arcs
//! updates the input (generic and inst) arcs
//! for the a constructed eventualities after
//! the split of "macro" by use of the
//! two following functions.
void change_instance_nodes(Instance *, Instance *);

//!This function is a special instanced case
//!the special treatment associated
//!to an eventuality constructed by split
//!of the marco node "macrox".
Marking *inst_treatement(Marking *);

//! This function updates the output arcs
//! the function updates the ouput arcs
//! for the a constructed eventuality after
//! the split of "macrox" by use of the
//! two following functions.
void search_events_and_add();

//QUI
void post_treatement(Marking *);
//QUI
//!Instances a symmetrical representation
//! This function instantiates a symmetrical representation
//! to all it eventualities and applies the "function treatment"
//! on each constructed eventuality.
void instance_macro(Marking *);

//!This function is the core of refinement algorithm
//!it takes in input the lists q,x,c and writes the final partition and the number of steps to obtant this refinement.
void Lump_test(Q_list *&, C_list *&, X_list *&);

//!This function is used to refine a partition
//!the variable multimap is use to store  all  markings with their rates (double) which reach this block
void split(Q_list *&, C_list *&, X_list *&, multimap<double, Marking *> &);


//!This function is used before the split function
//!it builds the multimap variable from the e_b (map)
void presplit(map <class Marking *, double> &, multimap<double, Marking *> &);


//!This function makes the lists x and c for strong lumpability
X_list *Create_X_C(Q_list *&, C_list *&);

//!This function makes the lists x and c for exact lumpability
X_list *Create_X_C_exact(Q_list *&, C_list *&);

//!This function makes the lists x and c for strong lumpability
//!it builds the list X in this way: all MACROC are in the same X block while
//!every MACROP (split) are inserted in a  new  X block
X_list *Create_X_C_all(Q_list *&, C_list *&);

//!This function makes the lists x and c for exact lumpability
X_list *Create_X_C_all_exact(Q_list *&, C_list *&);

//!This function makes a Q list graph
void draw_graph(std::string, Q_list *&);

//!This function orders the id of Q's blocks
//!It is used to generate the Mark file
void order_id_block(Q_list *&);

//!This function builds the DTMC from the Refinement ESRG w.r.t exact condition.
void genEX_DTMC(std::string, Q_list *&);
//!This function builds the DTMC from the Refinement ESRG w.r.t strong condition.
void genST_DTMC(std::string, Q_list *&);
//!This function prints in the  net_name.mark file  the corresponding  SR or  Eventuality for every element in a Qblock and either return this in the string
int print_state_ctmc(Marking *, char **);
//!This function prints the file ctrs
int print_ctrs(Marking *, FILE *);
//!This function creates the file wngr and ctrs used to solve the CTMC
void gen_wngr_ctrs(std::string, Q_list *&);
//!This function computes the peak
void Compute_peak(Q_list *&q);
//!This functions computes denominator for DTMC (on all Q)
void Compute_denom(Q_list *&q);
//!This functions computes denominator for DTMC (on a block)
void Compute_denom(Q_list_el *&block);
//!This functions write in a file (fp) the block elements in the following format:<idBlock, element number,list elements>
void write_block(ofstream &, Q_list *&);

