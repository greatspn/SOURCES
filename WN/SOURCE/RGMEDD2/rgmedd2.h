#ifndef MEDD_H_
#define MEDD_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef OVERFLOW
#undef OVERFLOW
#endif

#include <assert.h>
#include <exception>
#include <sstream>
#include <fstream>
#include <math.h>
#include <limits>
#include <iomanip>
#include <map>
#include <list>
#include <time.h>

#include <iostream>
#include <sys/resource.h>

// Optionally include <gmpxx.h> (if available on the platform)
#ifdef HAS_GMP_LIB
#include <gmpxx.h>
// The cardinality type is a multi-precision integer
typedef mpz_class cardinality_t;
// Get the mpz_t& needed by Meddly from a mpz_class& object
inline mpz_t& cardinality_ref(cardinality_t& c) { return *reinterpret_cast<mpz_t*>(&c); }
// The Meddly operand type
#define cardinality_operant_type  MEDDLY::HUGEINT
#else
// The cardinality type is just a double
typedef double cardinality_t;
inline double& cardinality_ref(double& c) { return c; }
#define cardinality_operant_type  MEDDLY::INTEGER
#endif

// Include meddly only after having included <gmpxx.h>
#include <meddly.h>
#include <meddly_expert.h>

#include "general.h"

// #ifndef __PRS_H__
// #define __PRS_H__
// #include "../AUTOMA/parser.h"
// #endif

extern "C" {
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/fun_ext.h"
#include "../../INCLUDE/macros.h"
}

#define EXIT_FAILURE_MEDDLY    80
#define EXIT_FAILURE_RGMEDD    81
#define EXIT_FAILURE_GENERIC   81
#define EXIT_FAILURE_GMP       82


// #ifndef __PRS_H__
// #define __PRS_H__
// #include "../AUTOMA/parser.h"
// #endif



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

extern "C" {
    int read_PIN_file();
    void free_PIN_file();
    int* sort_according_to_pinv();
};
extern void print_banner(const char* title);

using namespace std;

//-----------------------------------------------------------------------------
// Exception class used bu RGMEDD
//-----------------------------------------------------------------------------

class rgmedd_exception : public std::exception {
    std::string msg; // message carried in this exception
public:
    rgmedd_exception() {}
    rgmedd_exception(const std::string& m) : msg(m) {}
    rgmedd_exception(std::string&& m) : msg(m) {}
    rgmedd_exception(const char* m) : msg(m) {}

    virtual const char* what() const throw() override;
};

//-----------------------------------------------------------------------------

// Is the application called from the new Java GUI?
bool invoked_from_gui();
extern "C" int print_stat_for_gui();
bool set_print_stat_for_gui();

// Is running for the model checking contests?
// defined in rg_files.c for compatibility
extern "C" int running_for_MCC();

//-----------------------------------------------------------------------------

using namespace std;
using namespace MEDDLY;

//-----------------------------------------------------------------------------

enum VariableOrderCriteria {
    VOC_NO_REORDER = 1,        // Take the same order of the .net/.def places
    VOC_FROM_FILE,             // Read the order from a file
    VOC_DFS,                   // Depth-first Visit order
    VOC_BFS,                   // Breadth-first Visit order
    VOC_PINV,                  // Heuristic that uses P-invariants
    VOC_FORCE,                 // Use original FORCE implementation
    VOC_FORCE_PINV,            // Use original FORCE implementation with P-semiflows as clusters
    VOC_FORCE_NU,              // Use original FORCE implementation with Nested Units as clusters
    VOC_MEAS_FORCE,            // Use FORCE heuristic, with place span metric
    VOC_MEAS_FORCE_NES,        // Use FORCE heuristic, with normalized event span metric
    VOC_MEAS_FORCE_WES1,       // Use FORCE heuristic, with weighted event span metric
    VOC_MEAS_FORCE_PINV,       // Use FORCE + PINV heuristic, with WES(1) metric
    VOC_MEAS_FORCE_NU,
    VOC_CUTHILL_MCKEE,         // Cuthill-Mckee algorithm
    // VOC_MINIMUM_DEGREE,        // Use minimum degree ordering (fill-in reduction matrix reordering)
    VOC_KING,                  // Use King ordering (less efficient than Cuthill-Mckee)
    VOC_SLOAN,                 // Use sloan ordering with W1=1, W2=2
    VOC_SLOAN_1_16,            // Use sloan ordering with W1=1, W2=16
    VOC_NOACK,                 // Use Noack order ("A ZBDD package for efficient model checking of Petri nets")
    VOC_TOVCHIGRECHKO,         // Use Tovchigrechko method.
    VOC_GRADIENT_P,
    VOC_MARKOV_CLUSTER,
    VOC_GRADIENT_NU,
    // VOC_NOACK_NU,
    // VOC_TOVCHIGRECHKO_NU,
    VOC_VCL_CUTHILL_MCKEE,
    VOC_VCL_ADVANCED_CUTHILL_MCKEE,
    VOC_VCL_GIBBS_POOLE_STOCKMEYER,
    VOC_PRINT_METRICS_AND_QUIT,  // Do not order anything, just print graph metrics and quit
    // Meta-heuristics
    VOC_META_BY_SCORE,         // Select method with best NES score
    // VOC_META_BY_SCORE_WES1,    // Select method with best WES(1) score
    // VOC_META_BY_SCORE_NLWES,   // Select method with best WES(1) score
    // VOC_META_BY_SCORE_CRS,     // Select method with best Cross score
    // VOC_META_BY_SCORE_CRS1,    // Select method with best Cross(1) score
    // VOC_META_USE_ANN,          // Use Artificial Neural Network
    //
    VOC_TOTAL_CRITERIAS,       // Must be the last entry of this enum
};

//-----------------------------------------------------------------------------

enum class BoundGuessPolicy {
    M0_LINEAR, // each variable starts at its initial marking value or 1, then underestimated variables increase by their max_var_incr times restarts
    M0_EXP, // each variable starts at its initial marking value or 1, then underestimated variables increase by their max_var_incr times 2^restarts
    MAX0_LINEAR, // all variables start with the max token count in m0, then underestimated variables increase by their max_var_incr times restarts
    MAX0_EXP, // all variables start with the max token count in m0, then underestimated variables increase by their max_var_incr times 2^restarts
    M0_MAX0_EXP, // each variable starts at its initial marking value or 1, underestimated variables start with the max token count in m0,
                 // then underestimated variables increase by their max_var_incr times 2^(restarts-1)
    LOAD_FROM_FILE // loads the bounds from the bnd file; if a bound of a place is not available m0 or 1 is used.
                   // The underestimated variables increase by their max_var_incr times 2^restarts
};

//-----------------------------------------------------------------------------
// Main class of RGMEDD. Holds the Meddly objects, and builds the RS.
//-----------------------------------------------------------------------------

class RSRG {
protected:
    domain *dom = nullptr;

    forest *forestMDD = nullptr; //RS forest (Boolean MDD)
    forest *forestMxD = nullptr; // transitions forest (Boolean MxD - double levels)
    forest *forestMTMxD = nullptr; // transitions + rates forest (Multi-terminal MxD)
    forest *forestEVplMDD = nullptr; // RS + id states forest (EV+MDD)

    satpregen_opname::pregen_relation* nsf_collection = nullptr;

    dd_edge rs;
    dd_edge indexrs;
    dd_edge NSF;         // Monolithic Next State Function
    dd_edge NSFReal;//timed transitions with rates
    dd_edge DiagReal;//Matrix diagonal
    // dd_edge *NSFi = nullptr;//immediate transitions
    std::vector<dd_edge> VectNSFReal;//vector of timed transitions with rates X throughput

    std::string netname;
    std::string propname;
    dd_edge initMark;

    // Enabling conditions of each event as MxDs (only input/inhibitor arcs are encoded)
    vector<dd_edge> all_enabling_events;
    // Firing of each event (enabling conditions + output arcs)
    vector<dd_edge> events;

    // Stores if the stored bounds are exact (i.e. are the bounds of the full RS)
    // or if they are a guess made during RS generation. This variable starts at true,
    // and becomes false after RS construction.
    bool guessing_bounds = true;

    // Contains, for each variable, its effective max value in the RG
    // This vector is computed *after* the RG construction, and reflects
    // the real maximum value of each variable.
    std::vector<int> realBounds;

    // The maximum number of tokens in any marking of the RG
    int maxSumTokens = -1;

    // Variable order: from place order in .net file to variables levels in the MDD
    std::vector<int> net_to_mddLevel;
    // Inverse relation (useful to print markings from dd_edge iterators)
    std::vector<int> mddLevel_to_net;

    // maximum increment a variable could receive from the firing of any event
    std::vector<int> maxVarIncr;

    // our current estimate of each variable max value (inclusive)
    std::vector<int> guessedBounds;

    // The domain bound for variable i (exclusive) that will be used for Meddly variable domains.
    int domainBounds(int i) const;

    BoundGuessPolicy bound_policy = BoundGuessPolicy::MAX0_EXP;

    bool use_monolithic_nsf = true;

    map<std::string, int> S2Ipl; //It maps place name with its position in tabp
    map<std::string, int> S2Itr; //It maps transition name with its position in tabt

private:
    int visitXBounds(const node_handle node, int visit_level, std::vector<bool> &visited, std::vector<int> &nodeMaxSumTokens);

    /*inline int getbound(const int i) {if (bounds != NULL) return bounds[i]; else return -1;};*/

    int visitXBoundOfVariables(const node_handle node, int visit_level, const std::vector<bool> &selected_vars,
                               std::vector<int> &cache) const;
    // Initializes guessedBound according to the policy. Called before domain initialisaztion
    void initializeGuessedBounds();
    //!compute the effective variable bounds associated with the current RS.
    void computeRealBounds();

    // const cardinality_t&
    // visitXTrnEnabling(const node_handle node, int visit_level, const std::vector<pair<int, int>> &enabling,
    //                   std::vector<cardinality_t> &cache, std::vector<cardinality_t> &cache2) const;

    struct MakeNsfHelper;
    friend struct MakeNsfHelper;

public:
    // Make a new estimate of the guessed bounds, using the realBounds[] values
    bool updateGuessedBounds(int restart_count);
    // Compute the bound of a set of places
    int computeRealBoundOfVariables(const std::vector<bool> &selected_vars) const;

    // Count # of fired transitions in RS
    // const cardinality_t&
    // count_markings_enabling(int tr, std::vector<pair<int, int>>& enabling,
    //                         std::vector<cardinality_t>& cache,
    //                         std::vector<cardinality_t> &cache2) const;
    // cardinality_t count_num_fired_transitions() const;

    cardinality_t count_num_fired_transitions_by_events() const;

    // Get the MxD of a single event
    const dd_edge& getEventMxD(int tr) const { return events[tr]; }

    inline bool useMonolithicNSF() const { return use_monolithic_nsf; }
    inline bool useByEventsNSF() const { return !use_monolithic_nsf; }

    RSRG();
    ~RSRG();

    // Initialize Meddly, computes the variable order, initializes the guessed bounds
    void initialize(bool _use_monolithic_nsf,
                    const VariableOrderCriteria voc,
                    bool save_ordering_image,
                    bool reorder_SCC,
                    const long meddly_cache_size,
                    BoundGuessPolicy bound_policy);

    // Initialize the dd_edges of the initial marking
    bool init_RS(const Net_Mark_p &);
    //! \name Methods use to build the NEXT STATE FUNCTIONS
    //@{
    // //! it takes in input matrices I,O,H for a transition and its type and computes its next state function.
    // bool MakeNextState(const std::vector<int>& F, const std::vector<int>& T,
    //                    const std::vector<int>& H, const int tt);
    // //! it takes in input matrices I,O,H for a transition and its type and computes recursively its next state function. It returns -1 in case of error 0 otherwise.
    // void MakeNextState(int *f, int *t, int *h, const int &tt, int pl);
    // //@}

    // build all the NSF of all the events
    void MakeAllNsfs();

    //! \name Methods use to generate the RS
    //@{
    //! it generates the RS using the next state function for models with only timed transitions. It returns -1 in case of error 0 otherwise.
    void genRSTimed();
    //! it generates the RS using the next state function for models with timed and immediate transitions.  It returns -1 in case of error 0 otherwise.
    bool genRSAll();
    //@}

    //! \name Methods use to index the RS
    //@{
    //! it creates an RS EV+MDD from the RS MDD.
    void IndexRS();
    //@}

    //! \name Methods use to solve the underlying CTMC
    //@{
    //!It computes the probability vector and the transition throughput using Jacobi's method. The results are saved in <netname>.prob and <netname>.sta. It returns -1 in case of error 0 otherwise.
    bool JacobiSolver();
    //@}

    // Prints any dd_edge in readable format
    void show_markings(ostream& out, const dd_edge& e, int max_markings=100);


    //! \name Methods use to produce output
    //@{
    //!It returns the RS cardinality
    void getSizeRS(cardinality_t& card) {
        MEDDLY::apply(CARDINALITY, rs, cardinality_ref(card));
    }

    //!It prints the RS and the domain info.
    void statistic() {
        MEDDLY::ostream_output stdout_wrap(cout);
        cout << "\n-----------RS-----------\n";
        rs.show(stdout_wrap, 2);

        if(useMonolithicNSF()){
            cout << "\n---------RG---------\n";
            NSF.show(stdout_wrap, 2);
        }
        //d->showInfo(stdout);
    }
    //@}
    //! \name Methods use to get from the transition/place name its corresponding integer.
    //@{
    //!It takes in input a place name and returns its corresponding interger.
    int findPlaceIdByName(const std::string &name) {
        if (S2Ipl.find(name) == S2Ipl.end())
            return -1;
        else
            return S2Ipl[name];
    }
    // //!It takes in input a integer and returns its corresponding place name.
    // const char* getPL(const int id) {
    //     return tabp[id].place_name;
    // }

    int getMaxValueOfPlaceRS(const int place) {
        if (realBounds.empty())
            throw rgmedd_exception("Compute the RS first");
        assert(place >= 0 && place < npl);
        return realBounds[net_to_mddLevel[place]];

        // if ((var <= 0) || (var > npl)) { // variables are 1-based
        //     cerr << "var = " << var << ", npl = " << npl << endl;
        //     throw rgmedd_exception("variable index out-of-bound.");
        // }
        // return realBounds[id - 1];
    }

    // return the maximum number of tokens that can be found in any TRG marking
    int get_token_sum_bound() const { return maxSumTokens; }

    //!It takes in input a transition name and returns its corresponding interger.
    int findTransitionIdByName(const std::string &name) {
        if (S2Itr.find(name) == S2Itr.end())
            return -1;
        else
            return S2Itr[name];
    }
    //!It takes in input a integer and returns its corresponding transition name.
    // const char* getTR(const int id) {
    //     return tabt[id].trans_name;
    // }
    //@}
    //! \name Methods use to get/set file name.
    //@{
    //!It returns the net name
    const std::string& getNetName() { return netname;}
    //!It returns the property file name
    const std::string& getPropName() { return propname;}
    //!It sets the property file name
    void setPropName(std::string propname) {
        this->propname = propname;
    }


    // It returns the MDD encoding the RS
    const dd_edge& getRS() const  { return rs;}
    // It returns the MDD encoding the initial marking
    const dd_edge& getInitMark() const  { return initMark;}
    // It returns the MDD encoding the RS + its enumeration
    const dd_edge& getIndexRS() const { return indexrs;}
    // It returns the MXD encoding the Next State Function
    const dd_edge& getNSF() const  { return NSF;}
    // It returns the MDDs encoding the RG if it is not monolithic
    satpregen_opname::pregen_relation *getNSFCollection() const { return nsf_collection;}
    // It resets the MDDs encoding the RG if it is not monolithic
    void resetNSFCollection();

    // It returns the DD domain
    domain *getDomain() const  { return dom;}

    forest* getForestMDD() const { return forestMDD; }
    forest* getForestMxD() const { return forestMxD; }

    // It returns the vector encoding the place ordering
    const int *getPlaceOrd() const { return net_to_mddLevel.data(); }
    //@}

    // Converts a place id into its corresponding MDD variable (0-based)
    int convertPlaceToMDDLevel(int place) const { return net_to_mddLevel.at(place); }

    // Converts a (0-based) MDD variable level into the place name it is encoding
    const char* placeNameOfMDDVar(int var) const { return tabp[mddLevel_to_net.at(var)].place_name; }
};

// Prints the RS for the model.rg file
ostream &operator<<(ostream &out, class RSRG &rs);


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
//         NSF.clear();//timed transitions
//         NSFReal->clear();//timed transitions with rates
//         DiagReal->clear();//Matrix diagonal
//         NSFi->clear();//immediate transitions
//         for (int i = 0; i < ntr; i++) {
//             VectNSFReal[i]->clear();
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
        apply(INTERSECTION, *Mark, *rs, *Mark);
        return true;
    };


    inline int initIterator() {
        it = new enumerator(*Mark);
        return Mark->getCardinality();
    };

    template<typename ID>
    inline bool getMarking(int *vmark, ID& idm) {
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


#endif /* MEDD_H_ */

