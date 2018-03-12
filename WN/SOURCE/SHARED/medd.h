#ifndef MEDD_H_
#define MEDD_H_

#include <stdio.h>
#include <stdlib.h>


#ifndef __STD_H__
#define __STD_H__
#include <stdio.h>
#endif

#ifndef __STDL_H__
#define __STDL_H__
#include <stdlib.h>
#endif

#ifdef OVERFLOW
#undef OVERFLOW
#endif

#include <iostream>
#include <meddly.h>
#include <meddly_expert.h>
#include <sys/resource.h>


#ifndef __PRS_H__
#define __PRS_H__
#include "../AUTOMA/parser.h"
#endif

extern "C" {
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/fun_ext.h"
#include "../../INCLUDE/macros.h"
}


#define NDEBUG 1

#ifndef __ASS_H__
#define __ASS_H__
#include <assert.h>
#endif


#ifndef __EXC_H__
#define __EXC_H__
#include <exception>
#endif

#ifndef __GNR_H__
#define __GNR_H__
#include "general.h"
#endif

#ifndef __SSTR_H__
#define __SSTR_H__
#include <sstream>
#endif

#ifndef __FSTR_H__
#define __FSTR_H__
#include <fstream>
#endif

#ifndef __MATH_H__
#define __MATH_H__
#include <math.h>
#endif

#ifndef __LIM_H__
#define __LIM_H__
#include <limits>
#endif

#ifndef __IOM_H__
#define __IOM_H__
#include <iomanip>
#endif

#ifndef __MAP_H__
#define __MAP_H__
#include <map>
#endif

#ifndef __LST_H__
#define __LST_H__
#include <list>
#endif

#ifndef __PRS_H__
#define __PRS_H__
#include "../AUTOMA/parser.h"
#endif


#include <time.h>

#ifdef MCC
# error "Do not compile with -DMCC. Use the environment variable MCC=1 to turn on MCC mode."
#endif

#define  DEBUG 0
#define  DEBUG1 0
#define  DEBUG2 0
#define  DEBUG3 0
#define  DEBUG4 0
#define  DEBUGPARSER 0
#define  MAX_ID 32
#define  IOPRECISION 12
#define  MAXSERV 255
#define  MAXPRIO 127
//Use to endoce the id transition in the MTMDD for the AUTOMATON
#define  INCTRAN 1000
//predefined "machine epsilon", a numeric value representing the difference between 1 and the least value greater than 1 that is representable [1]
#define _DIFFMIN(T) numeric_limits<T>::epsilon()

extern "C" { int *sort_according_to_pinv(); };

using namespace std;

//!Exception for Input/Output
class ExceptionIO {

    std::string mess;
public:
    //! Empty constructors
    ExceptionIO() {mess = "";};
    //! constructor takes in input a message
    ExceptionIO(std::string mess) {this->mess = mess;};
    //! \name Get of methods use to access at the data structures
    //@{
    //!it returns message
    std::string get(void) {return mess;};
    //@}
    friend ostream &operator<<(ostream &out, class ExceptionIO d) {out << d.mess << endl; return out;};
};

// Is the application called from the new Java GUI?
bool invoked_from_gui();
extern "C" int print_stat_for_gui();
bool set_print_stat_for_gui();

// Is running for the model checking contests?
// defined in rg_files.c for compatibility
extern "C" int running_for_MCC();

// Namespace of the desicion diagram data structures.
namespace dddRS {

using namespace std;
using namespace MEDDLY;

enum VariableOrderCriteria {
    VOC_NO_REORDER = 1,        // Take the same order of the .net/.def places
    VOC_FROM_FILE,             // Read the order from a file
    VOC_PINV_HEURISTIC,        // Heuristic that uses P-invariants
    VOC_FORCE_HEURISTIC,       // Use FORCE heuristic, with place span metric
    VOC_FORCE_NES_HEURISTIC,   // Use FORCE heuristic, with normalized event span metric
    VOC_FORCE_WES1_HEURISTIC,  // Use FORCE heuristic, with weighted event span metric
    VOC_FORCE_PINV_HEURISTIC,  // Use FORCE + PINV heuristic, with WES(1) metric
};

// Terminal node ID for Binary DD
inline MEDDLY::node_handle getMeddlyTerminalNodeID(bool booleanValue) {
    return booleanValue ? -1 : 0;
}

//!Class RS
/*!
This class is used to manage the RS and compute performance indexes
*/


class RSRG {
protected:
    domain *d;

//compute_manager* cm;

    forest *fRS; //RS forest
    expert_forest *expfRS; //Expert RS forest
    forest *fTranB; //transitions forest
    forest *fTranR; //transitions + rates forest
    forest *fEV; //RS + id states forest

    dd_edge *rs;
    dd_edge *indexrs;
    dd_edge *NSFt;//timed transitions
    dd_edge *NSFtReal;//timed transitions with rates
    dd_edge *DiagReal;//Matrix diagonal
    dd_edge *NSFi;//immediate transitions
    dd_edge **VectNSFtReal;//vector of timed transitions with rates X throughput

//to manage priorities
    dd_edge *prioNSFt[MAXPRIO];
    dd_edge *defNSFt[MAXPRIO];
//to manage priorities

    double cardinality;
    std::string netname;
    std::string propname;
    dd_edge *initMark;
    int npl; //Places' number

    // Contains, for each variable, its effective bound in the TRG
    // This vector is computed *after* the TRG construction, and reflects
    // the real maximums of the variables.
    int *realBounds;
    // The maximum number of tokens in a marking of the TRG
    int maxSumTokens;
    // Upper bound of each variable, known a-priori
    int *bounds;

    // from place order in .net file to place levels in the MDD
    std::vector<int> net_to_mddLevel;

    map <std::string, int> S2Ipl; //It maps place name with its position in tabp
    map <std::string, int> S2Itr; //It maps transition name with its position in tabt

    //! Load the net_to_mddLevel[] vector from file
    void load_var_order_from_file();
private:
    //!compute the effective variable bounds associated with the current RS.
    void computeRealBounds();

    //!It reads the bound from file .bnd
    void read_bound(int *bounds);

    bool visitXBounds(int node, std::vector<bool> &, std::vector<bool> &, std::vector<int> &);

    inline int getbound(const int i) {if (bounds != NULL) return bounds[i]; else return -1;};

    int visitXBoundOfPlaces(const int node, const std::vector<bool> &selected_places,
                            std::vector<int> &cache) const;
public:

    int computeRealBoundOfPlaces(const std::vector<bool> &selected_places) const;

    //! Empty constructor
    RSRG() {
        realBounds = NULL; d = NULL; fRS = fTranB = fTranR = fEV = NULL; indexrs = rs = NULL; //cm=NULL;
        NSFt = NSFtReal = DiagReal = NSFi = NULL; VectNSFtReal = NULL; cardinality = 0.0; //ins = to = pfrom = from = NULL;
        for (int i = 0; i < MAXPRIO; i++) {
            prioNSFt[i] = defNSFt[i] = NULL;
        }
    }
    //!Constructor: it takes in input the  place number, the net name and the cache size;
    // and initializes the domain and the forest.
    RSRG(const int &, std::string, const long meddly_cache_size);
    //!it takes in input the initial marking (in GreatSPN format) and  encodes it in DD. For each place a variable is inserted.
    bool init_RS(const Net_Mark_p &);
    //!destructor;
    ~RSRG();
    //! \name Methods use to build the NEXT STATE FUNCTIONS
    //@{
    //! it takes in input matrices I,O,H for a transition and its type and computes its next state function.
    bool MakeNextState(const int *F, const int *T, const int *H, const int tt);
    //! it takes in input matrices I,O,H for a transition and its type and computes recursively its next state function. It returns -1 in case of error 0 otherwise.
    // void MakeNextState(int *f, int *t, int *h, const int &tt, int pl);
    //@}

    //! \name Methods use to generate the RS
    //@{
    //! it generates the RS using the next state function for models with only timed transitions. It returns -1 in case of error 0 otherwise.
    bool genRSTimed();
    //! it generates the RS using the next state function for models with timed and immediate transitions.  It returns -1 in case of error 0 otherwise.
    bool genRSAll();
    //@}

    //! \name Methods use to index the RS
    //@{
    //! it creates an RS EV+MDD from the RS MDD. It returns -1 in case of error 0 otherwise.
    bool IndexRS();
    //@}

    //! \name Methods use to solve the underlying CTMC
    //@{
    //!It computes the probability vector and the transition throughput using Jacobi's method. The results are saved in <netname>.prob and <netname>.sta. It returns -1 in case of error 0 otherwise.
    bool JacobiSolver();
    //@}

    //! it prints the RS
    friend ostream &operator<<(ostream &out, class RSRG &rs) {
        //dd_edge::iterator i = rs.rs->begin();
        enumerator i(*(rs.rs));
        int val, z = 0;
        while (i != 0) {
            out << "M" << z << "\n\t";
            for (int j = 1; j <= rs.npl; j++) {
                val = *(i.getAssignments() + j);
                if ((val != 0))
                    out << rs.getPL(j - 1) << "(" << val << ")";
            }//per ogni posto
            out << endl;
            ++i;
            ++z;
        }//per ogni marcatura
        return out;
    }//fine stampa




    //! \name Methods use to produce output
    //@{
    //!It returns the RS cardinality
    double getSizeRS() {return rs->getCardinality(); };
    //!It prints the RS and the domain info.
    void statistic() {
        cout << "\n-----------RS-----------\n";
        rs->show(stdout, 2);

        cout << "\n---------RG---------\n";
        NSFt->show(stdout, 2);
        //d->showInfo(stdout);
    }
    //@}
    //! \name Methods use to get from the transition/place name its corresponding integer.
    //@{
    //!It takes in input a place name and returns its corresponding interger.
    int getPL(const std::string &name) {
        if (S2Ipl.find(name) == S2Ipl.end())
            return -1;
        else
            return S2Ipl[name];
    }
    //!It takes in input a integer and returns its corresponding place name.
    std::string getPL(const int id) {
        return std::string(tabp[id].place_name);
    }
    //!It takes in input an integer and returns its corresponding  real variable bound in the current RS.
    int getRealBound(const int id) {
        if (realBounds == NULL)
            computeRealBounds();
        if ((id <= 0) || (id > npl)) { // variables are 1-based
            cerr << "id = " << id << ", npl = " << npl << endl;
            throw;
        }
        // cout << "getRealBound " << id << "(" << getPL(id-1) << ") = " << realBounds[id] << endl;
        return realBounds[id];
    }
    // return the maximum number of tokens that can be found in any TRG marking
    int get_token_sum_bound() const { return maxSumTokens; }
    //!It takes in input a transition name and returns its corresponding interger.
    int getTR(const std::string &name) {
        if (S2Itr.find(name) == S2Itr.end())
            return -1;
        else
            return S2Itr[name];
    }
    //!It takes in input a integer and returns its corresponding transition name.
    std::string getTR(const int id) {
        return std::string(tabt[id].trans_name);
    }
    //@}
    //! \name Methods use to get/set file name.
    //@{
    //!It returns the net name
    std::string getNetName() { return netname;}
    //!It returns the property file name
    std::string getPropName() { return propname;}
    //!It sets the property file name
    void setPropName(std::string propname) {
        this->propname = propname;
    }
    //@}

    //! \name Methods use to return the DD data structures.
    //@{
    //!It returns the MDD encoding the RS
    dd_edge *getRS()  { return rs;}
    //!It returns the MDD encoding the initial marking
    dd_edge *getInitMark()  { return initMark;}
    //!It returns the MDD encoding the RS + its enumeration
    dd_edge *getIndexrs() { return indexrs;}
    //!It returns the MDD encoding the RG
    dd_edge *getNSFt()  { return NSFt;}
    //!It returns the DD domain
    domain *getDomain()  { return d;}

    //!It returns the vector encoding the place ordering
    const int *getPlaceOrd() const { return net_to_mddLevel.data(); }
    //@}

};




//!Class RSRGAuto
/*!
This class is used to manage the RS augmented with the Automaton
*/
// class RSRGAuto : public RSRG {
//     int Top;
//     int Bot;
// //op_info* OpPOST;

// //expert_compute_manager* expcm;
//     forest *fRG;
// public:
//     //! Empty constructor
//     RSRGAuto() { }
//     //!Constructor: it takes in input the  place number and the net name; and initializes the domain and the forest. Observe that the last level (|P|+2) is used to encode the automaton state.
//     RSRGAuto(const int &, std::string);
//     bool init_RS(const Net_Mark_p &net_mark);
//     //! it takes in input matrices I,O,H for a transition and its type and computes its next state function.
//     bool MakeNextState(const int *F, const int *T, const int *H, const int tt, int AA, map <int, map <int, class ATRANSITION> > &automa);
//     //! it takes in input matrices I,O,H for a transition and its type and computes its next state function  considering the automanton state.
//     bool MakeNextState(const int *F, const int *T, const int *H, const int tt, int AA);
//     //! it computes the RG and saves it in the wngr file according to GreatSPN format. Variable outState  enables the saving in the file *.outT
//     bool ComputingRG(const bool &output, FILE *, FILE *, const int &Top, const int &Bot, set <int> &InitSet, bool outState);
//     //! it prints the RS
//     friend ostream &operator<<(ostream &out, class RSRGAuto &rs) {
//         //dd_edge::iterator i = rs.rs->begin();
//         enumerator i(*(rs.rs));
//         int nvar = rs.fRS->getDomain()->getNumVariables();
//         int val;
//         while (i != 0) {
//             for (int j = 1; j <= nvar; j++) {
//                 val = *(i.getAssignments() + j);
//                 if ((val != 0) && (j != nvar))
//                     out << rs.getPL(j - 1) << "(" << val << ")";
//                 if (j == nvar)
//                     out << " - L" << "(" << val << ")\n";
//             }//per ogni posto
//             ++i;
//         }//per ogni marcatura
//         return out;
//     }//fine stampa
//     //!it clears the next state functions
//     void clearNSF() {
//         NSFt->clear();//timed transitions
//         NSFtReal->clear();//timed transitions with rates
//         DiagReal->clear();//Matrix diagonal
//         NSFi->clear();//immediate transitions
//         for (int i = 0; i < ntr; i++) {
//             VectNSFtReal[i]->clear();
//         }
//     }
//     //!It takes in input the transition id and its rate encoded in a double and compute the corresponding rate for marking dependent.
//     void  checkRate(float &val, const int *plmark);
// };

class QUASIPRODFOR {
private:
    domain *d;
    forest *fRS;
    forest *fEV;
    dd_edge *rs;
    dd_edge *indexrs;
    dd_edge *Mark;
    int **to;
    enumerator *it;
    int nvar;

public:
    QUASIPRODFOR(dd_edge *rs, dd_edge *indexrs, domain *d) {
        fRS = rs->getForest();
        fEV = indexrs->getForest();
        this->d = d;
        this->rs = rs;
        this->indexrs = indexrs;
        nvar = d->getNumVariables();
        //to = (int **) malloc(sizeof(int *));
        //to[0] = (int*) malloc(sizeof(int) * (nvar + 1));
        it = NULL;
        Mark = new dd_edge(fRS);
    }
    ~QUASIPRODFOR() {
        // free(to[0]);
        // free(to);
        if (it != NULL)
            delete it;
        delete Mark;
    }

    //! \name Methods use to quasi product form
    //@{
    //!It computes the intersection between input MDD and the MDD encoding the RS. The ouput is encoded on the input MDD.
    inline bool getState(int **ins) {

        fRS->createEdge(ins, 1, *Mark);
        try {
            apply(INTERSECTION, *Mark, *rs, *Mark);
        }
        catch (...) {
            cerr << "Error  INTERSECTION" << endl;
            return false;
        }
        return true;
    };


    inline int initIterator() {
        it = new enumerator(*Mark);
        return Mark->getCardinality();
    };

    inline bool getMarking(int *vmark, int &idm) {
        const int *plmark;
        if (it == NULL || *it == 0)
            return false;
        else {
            plmark = it->getAssignments();
            fEV->evaluate((*indexrs), plmark, idm);
            memcpy(vmark, plmark, nvar * sizeof(int));
            ++(*it);
            return true;
        }
    }

    //@}
};


};
#endif /* MEDD_H_ */

