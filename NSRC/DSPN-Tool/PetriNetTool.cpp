/*
 *  PetriNetTool.cpp
 *
 *  Command Line interface of the Petri Net tool.
 *
 *  Created by Elvio Amparore
 *
 */
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <exception>
#include <algorithm>
#include <ctime>
#include <cmath>
#include <cfloat>
#include <cstring>
#include <climits>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <unistd.h>
#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>
using namespace std;

#include <boost/optional.hpp>

#include "../numeric/numeric.h"
using namespace numerical;

#include "utils.h"
#include "sparsevector.h"
#include "PetriNet.h"
#include "Semiflows.h"
#include "Measure.h"
#include "ReachabilityGraph.h"
#include "PackedMemoryPool.h"
#include "NewReachabilityGraph.h"
#include "PetriExport.h"
#include "SolveCTMC.h"
#include "MRP.h"
#include "MRP_Implicit.h"
#include "MRP_Explicit.h"
#include "MRP_SccDecomp.h"
#include "DTA.h"
#include "SynchProduct.h"
#include "SynchProductSCC.h"
#include "CSLTA.h"
#include "LinearProg.h"

#include "../platform/platform_utils.h"



// TODO: eliminare -old-mrp ed -isomorphism

//TODO: remove this
void WriteStatesInArffFormat(const PN &pn, const RG &rg, const char *netName,
                             bool onlyRecurrentStates, const char *arffName);

// TODO: and this function, too
void WriteEmcAndPreconditioner(const PN &pn, const RG &rg, const char *netName,
                               const SolverParams &spar);

// TODO: remove
void NewSolveTransientCTMC(const CTMC &ctmc, PetriNetSolution &sol, double t,
                           const SolverParams &spar, VerboseLevel verboseLvl);

// TODO: experiment: remove this
void PassageTimeForProbeAutomata(ifstream &lab, ifstream &ifrs,
                                 ifstream &ifrsXa, ofstream &hydra_pi0,
                                 const SolverParams &spar, double time,
                                 size_t numSamples, VerboseLevel verboseLvl);

/*void NewSolveSteadyStateCTMC(RG& rg, PetriNetSolution& sol, bool useIlu0,
						     const SolverParams& spar, VerboseLevel verboseLvl);*/

void Experiment1();

void Experiment2(const char *arg);

void Experiment3(const MRP &mrp, const SolverParams &spar);

//=============================================================================

void sig_alarm_handler(int signum) {
    cout << "Timeout expired. Exit..." << endl;
    exit(0);
}

//=============================================================================


//#ifdef USE_UNICODE_TTY
//#define LINESEP "\xE2\x94\x80"
//#else
#define LINESEP "="
//#endif


static const char *s_AppBanner =
    "{=}\n"
    "  {%DSPN-Tool} command line tool.\n"
    "{=}\n"
    "\nOptions:\n"
    "  {!-v} and {!-vv}       Verbose and very verbose modes.\n"
    "  {!-nv} and {!-nnv}     Basic verbose and quiet mode.\n"
    "  {!-pt}              Load nets as P/T nets (ignore delays).\n"
    "  {!-load} {$<file>}     Loads a GreatSPN Petri net {$<file>}.\n"
    "  {!-load-mrmc} {$<mod>} Loads a model in MRMC format ({$<mod>}.tra and {$<mod>}.lab).\n"
    "  {!-trg}             Builds the Tangible Reachability Graph.\n"
    "  {!-rg}              Builds the Reachability Graph (with vanishings).\n"
    "  {!-no-vpaths}       Don't include vanishing paths in the TRG.\n"
    //"  -isomorphism     Enable SMC isomorphism detection (MRP solution).\n"
    //"  -old-mrp         Dont' use the MRP initial transient analysis.\n"
    "  {!-scc}             Component-based analysis of MRP(implicit).\n"
    // "  {!-sccrep}          Component-based analysis of MRP(repeated products).\n"
    "  {!-noscc}           Don't use a component-based MRP analysis.\n"
    "  {!-no-c-aggreg}     Don't aggregate complex components (works with {!-scc}).\n"
    "  {!-greedy-scc}      Greedy component method.\n"
    "  {!-ilp-scc}         Use lp_solve to compute the components.\n"
    "  {!-optimal-scc}     Optimal component method (default).\n"
    "  {!-i}               Implicit MRP steady state solution method.\n"
    "  {!-e}               Explicit MRP steady state solution method.\n"
    "  {!-mpar} {$<P> <n>}    Set the marking parameter {$<P>} to {$<n>}.\n"
    "  {!-rpar} {$<P> <real>} Set the rate parameter {$<P>} to {$<real>}.\n"
    "  {!-show-pars}       Print the marking and rate parameters.\n"
    "  {!-measure} {$<M> <e>} Add a new measure {$<M>} with expression {$<e>}.\n"
    "  {!-all-measures}    Print all the basic measures of the Petri net.\n"
    "\n"
    "  {!-s}               Compute the steady state solution.\n"
    "  {!-t} {$<real>}        Compute a transient solution (CTMC only).\n"
    "  {!-b} {$<M>}           Compute the backward probability with measure {$<M>}.\n"
    "  {!-wr}              Write the measure values in a file <netname>.results \n"
    "  {!-wr-f} {$<file>}     Write the measure values in the specified file.\n"
    "  {!-wr-sol-f} {$<file>} Write the solution vector in the specified file.\n"
    "\n"
    "  {!-pinv} or {!-psfl}   Compute Place semiflows, saved as {$<file>.pin}.\n"
    "  {!-tinv} or {!-tsfl}   Compute Transition semiflows, saved as {$<file>.tin}.\n"
    "  {!-pfl}             Compute minimal Place flows, saved as {$<file>.pfl}.\n"
    "  {!-tfl}             Compute minimal Transition flows, saved as {$<file>.tfl}.\n"
    "  {!-pbasis}          Compute basis for Place invariants, saved as {$<file>.pba}.\n"
    "  {!-tbasis}          Compute basis for Transition invariants, saved as {$<file>.tba}.\n"
    "  {!-traps}/{!-siphons}  Compute minimal traps/siphons.\n"
    "  {!-imPxT}/{!-imTxP}    Save the PxT or TxP incidence matrix.\n"
    "  {!-bnd}             Compute place bounds from P-semiflows, saved as {$<file>.bnd}.\n"
    "  {!-detect-exp}      Limit exponential growth in P/T flow generation.\n"
    "  {!-strict-support}  Slack variables are excluded from the flow support.\n"    
    "  {!-timeout} {$<sec>}   Set a timeout of {$<sec>} seconds for the tool activity.\n"
    "\n"
    "  {!-dot}             Saves the (T)RG as a dot/pdf file.\n"
    "  {!-dot-open}        Like -dot, but also opens the pdf output.\n"
    "  {!-with-clusters}   Group together markings in the same state subset (MRP).\n"
    "  {!-max-dot-markings} {$n} Saved dot/pdf file will have at most {$n} markings.\n"
    "  {!-export-rg}       Exports the full (T)RG structure.\n"
    "  {!-export-spnica}   Exports the Petri Net in the SPNica format.\n"
    "  {!-export-cosmos}   Exports the Petri Net in COSMOS GSPN format.\n"
    "  {!-export-asmc}     Exports the TRG in the CslTA-Solver format.\n"
    "  {!-export-ctmc}     Exports the TRG of a CTMC in Prism format.\n"
    "\n"
    "  {!-cslta} {$<ex>}      Model check the CSL^TA expression {$<ex>}.\n"
    "  {!-cslta0} {$<ex>}     Model check the CSL^TA expr. {$<ex>} in the initial state.\n"
    "  {!-csltaN} {$<s>} {$<ex>} Model check the CSL^TA expr. {$<ex>} in state {$<s>}.\n"
    "  {!-dta} {$<file.dta>}  Loads the DTA {$<file.dta>} from the disk.\n"
    "  {!-zdta} and {!-no-zdta}  Use/don't use the zoned DTA construction.\n"
    "  {!-on-the-fly}      On the fly state space construction method.\n"
    "  {!-dta-path} {$<path>} Add {$<path>} to the list of DTA search paths.\n"
    "  {!-bind} {$<R>}        Rules {$<R>} to bind the DTA parameters to a Petri Net.\n"
    "  {!-fmc}             Forward model checking of the DTAxPN from the initial state.\n"
    "  {!-bmc}             Backward model checking.\n"
    "  {!-dot-dta}         Saves the DTA as a dot/pdf file.\n"
    "  {!-dot-dta-open}    Like -dot-dta, but also opens the pdf output.\n"
    "\n"
    "  {!-omega} {$<real>}    Set the SOR/JOR over-relaxation coefficient.\n"
    "  {!-epsilon} {$<real>}  Set the residual error used in numerical analysis.\n"
    "  {!-max-iters} {$<n>}   Maximum number of iterations for iterative algorithms.\n"
    "  {!-jor}             Using Jacobi/JOR for stationary analysis.\n"
    "  {!-forward-sor}     Using Forward SOR for stationary analysis.\n"
    "  {!-backward-sor}    Using Backward SOR for stationary analysis.\n"
    "  {!-ssor}            Using Symmetric SOR for stationary analysis.\n"
    "  {!-i-power}         Use Power Method for implicit steady state analysis.\n"
    "  {!-gmres}           Use GMRES for implicit steady state analysis.\n"
    "  {!-M} {$<n>}           Maximum number of Arnoldi iterations in GMRES.\n"
    "  {!-bicgstab}        Use BiCG-STAB for implicit steady state analysis.\n"
    "  {!-cgs}             Use CGS for implicit steady state analysis.\n"
    "\n"
    "  {!-no-prec}         Use no preconditioner strategy.\n"
    "  {!-diag-prec}       Use a diagonal (Jacobi) preconditioner.\n"
    "  {!-ilu0-prec}       Use the ILU(0) preconditioner.\n"
    "  {!-ilutk-prec} {$<T> <K>} ILUTK precondonditioner with threshold {$T} (real) and\n"
    "                   max entries per row {$K} (integer).\n"
    "  {!-i-no-inout}      No inner/outer preconditioner (implicit MRP only).\n"
    "  {!-i-prec-sim}      Preconditioner built directly from simulated EMC.\n"
    "  {!-i-prec-emc}      Preconditioner built directly from explicit EMC.\n"
    "  {!-i-inout-emc}     EMC matrix builds the preconditioner (implicit MRP only).\n"
    "  {!-i-inout-sim}     Simulated EMC for the precond. matrix (implicit MRP only).\n"
    "  {!-i-inout-epsilon} {$<real>}  Epsilon for the (F)GMRES solver used in\n"
    "                   the inner/outer preconditioner (implicit MRP only).\n"
    "  {!-i-inout-max-iters} {$<n>}   Max number of (F)GMRES solver iterations in.\n"
    "                   the inner/outer preconditioner (implicit MRP only).\n"
    "  {!-i-inout-M} {$<n>}   Arnoldi base in inner/outer preconditioner.\n"
    //"  -i-sim-emc-prec  Use a simulated EMC as preconditioner.\n"
    //"  -i-emc-prec      Use the full EMC as preconditioner.\n"
    "  {!-i-num-sim}       Number of simulations for the preconditioner.\n"
    "  {!-i-sim-samples}   Samples in each simulated preconditioner row.\n"
    "  {!-i-no-self-loops} No self loop in the simulated EMC preconditioner.\n"
    "  {!-i-self-loops}    Add self loops in the simulated EMC preconditioner.\n"
    //"  -i-diag-ilu-prec Use a diagonal ILU preconditioner over a simulated DTMC.\n"
    //"  -i-sim-ilu0-prec Use the ILU(0) preconditioner over a simulated EMC.\n"
    //"  -i-ilu0-prec     Use the ILU(0) preconditioner of the EMC.\n"
    "  {!-i-alpha}         Mixing coefficient for preconditioners (Google trick).\n"
    "  {!-i-prec-sum}      Preconditioner sum.\n"
    "  {!-i-seed}          Random seed.\n"
    //"  -standard-unif   Use standard Uniformization for transient analysis.\n"
    //"  -foxglynn-unif   Use Fox-Glynn Uniformization for transient analysis.\n"
    "\n"
    "Options are read and applied sequentially.\n"
    "{=}\n"
    "Possible usages of DSPN-Tool include:\n"
    "\n"
    "(T)RG construction of a Petri Net in DOT format:\n"
    "  {:DSPN-Tool}{! -load }{$<netname>} ({!-trg} or {!-rg}){! -dot-open}\n"
    "\n"
    "Petri Net file format conversion in COSMOS format:\n"
    "  {:DSPN-Tool}{! -load }{$<netname>}{! -export-cosmos}\n"
    "\n"
    "Steady state solution of a CTMC with GMRES and ILU0 preconditioner:\n"
    "  {:DSPN-Tool}{! -load }{$<netname>}{! -gmres -ilu0-prec -s}\n"
    "\n"
    "Transient solution of a CTMC at time 1.5, with marking parameter N set to 4:\n"
    "  {:DSPN-Tool}{! -load }{$<netname>}{! -mpar }{$N 4}{! -t }{$1.5}\n"
    "\n"
    "Steady state solution of an MRP with explicit EMC construction:\n"
    "  {:DSPN-Tool}{! -load }{$<netname>}{! -e -s}\n"
    "\n"
    "Steady state solution of an MRP with implicit method and GMRES, and\n"
    "accuracy (epsilon) set to 1.0e-10:\n"
    "  {:DSPN-Tool}{! -load }{$<netname>}{! -i -gmres -epsilon }{$1.0e-10}{! -s}\n"
    "\n"
    "Steady state solution of an MRP with implicit method, GMRES with inner/outer\n"
    "preconditioner, simulation approx. (100 simulations and 10 samples per row) of\n"
    "inner matrix and ILUTK inner preconditioner:\n"
    "  {:DSPN-Tool}{! -load }{$<netname>}{! -i -gmres -i-inout-sim -i-sim-samples }{$10}\n"
    "    {!-i-num-sim }{$100}{! -ilutk-prec }{$1.0e-4 5}{! -s}\n"
    "\n"
    "Evaluates the acceptance by a DTA in a CTMC using the implicit method:\n"
    "  {:DSPN-Tool}{! -load }{$<netname>}{! -dta }{$<name.dta>}{! -bind }{$<rules>}{! -i -bmc}\n"
    "\n"
    "Binding rules are written as a list:\n"
    "  \"{$clock-value1}, ... | {$transition1}, ... | {$marking-expression1}, ...\" \n"
    "of comma-separated real values, transition names and CSL^TA\n"
    "expressions, that are bind to the abstract clock values, action names and\n"
    "state propositions of the DTA.\n"
    "\n"
    "Evaluate a CSL^TA expression on a CTMC with on-the-fly state space generation:\n"
    " \"Probability > 50% that a failure state is reached in time [1,3].\"\n"
    "  {:DSPN-Tool}{! -load }{$<netname>}{! -on-the-fly}\n"
    "    {! -cslta }{$\"PROB_TA > 0.5 until (1, 3 | | True, #fail=1)\"}\n"
    "Use {!-cslta0 }{$N <expr>} to evaluate {$<expr>} only from state {$N}.\n"
    "{=}\n"
    ;

void PrintAppBanner() {
    const char *p = s_AppBanner;
    char opt = '\0';
    while (*p != '\0') {
        char ch = *p++;
        if (ch == '{') {
            assert(opt == '\0');
            opt = *p++;
            switch (opt) {
            case '%':	cout << console::beg_title();  break;
            case '!':   cout << console::cyan_fgnd();  break;
            case '$':   cout << console::yellow_fgnd();  break;
            case ':':   cout << console::bold_on();  break;
            case '=':   for (size_t i = 0; i < 79; i++) cout << LINESEP;  break;
            default: throw program_exception("PrintAppBanner: push()");
            }
        }
        else if (ch == '}') {
            assert(opt != '\0');
            switch (opt) {
            case '%':	cout << console::end_title();  break;
            case '!':   cout << console::default_disp();  break;
            case '$':   cout << console::default_disp();  break;
            case ':':   cout << console::bold_off();  break;
            case '=':   break;
            default: throw program_exception("PrintAppBanner: pop()");
            }
            opt = '\0';
        }
        else cout << ch;
    }
    cout << flush;
}

//=============================================================================

template<typename T> void set_null(shared_ptr<T> &ptr) {
    shared_ptr<T> empty;
    ptr.swap(empty);
}

//=============================================================================

bool is_invariants_cmd(std::string cmd, invariants_spec_t& is) {
    if (cmd.size() < 3 || cmd[0] != '-')
        return false;

    is.invknd = InvariantKind::PLACE;
    is.system_kind = SystemMatrixType::REGULAR;
    is.suppl_flags = 0;
    is.matk = FlowMatrixKind::SEMIFLOWS;

    // extract the nominal command
    size_t i=1;
    while (i < cmd.size() && std::isalpha(cmd[i]))
        i++;
    std::string cmdName = cmd.substr(1, i-1);
    // cout << "cmdName = " << cmdName << endl;

    if (cmdName == "traps"  || cmdName == "siphons") {
        is.system_kind = (cmdName == "traps" ? SystemMatrixType::TRAPS : SystemMatrixType::SIPHONS);
        return true;
    }
    
    if      (cmdName == "pinv" || cmdName == "psfl")  
    { is.invknd=InvariantKind::PLACE;      is.matk=FlowMatrixKind::SEMIFLOWS;  }
    else if (cmdName == "tinv" || cmdName == "tsfl")  
    { is.invknd=InvariantKind::TRANSITION; is.matk=FlowMatrixKind::SEMIFLOWS;  }
    else if (cmdName == "pfl")  
    { is.invknd=InvariantKind::PLACE;      is.matk=FlowMatrixKind::INTEGER_FLOWS;  }
    else if (cmdName == "tfl")  
    { is.invknd=InvariantKind::TRANSITION; is.matk=FlowMatrixKind::INTEGER_FLOWS;  }
    else if (cmdName == "pbasis")  
    { is.invknd=InvariantKind::PLACE;      is.matk=FlowMatrixKind::BASIS;  }
    else if (cmdName == "tbasis")  
    { is.invknd=InvariantKind::TRANSITION; is.matk=FlowMatrixKind::BASIS;  }
    else return false;

    // read extra flags
    while (i < cmd.size()) {
        // cout << "extra: " << cmd[i] << endl;
        switch (cmd[i]) {
            case '+': is.suppl_flags |= FM_POSITIVE_SUPPLEMENTARY; break;
            case '-': is.suppl_flags |= FM_NEGATIVE_SUPPLEMENTARY; break;
            case '*': is.suppl_flags |= FM_ON_THE_FLY_SUPPL_VARS | FM_POSITIVE_SUPPLEMENTARY | FM_NEGATIVE_SUPPLEMENTARY; break;
            case 'r': is.suppl_flags |= FM_REDUCE_SUPPLEMENTARY_VARS; break;
            default:  return false;
        }
        i++;
    } 
    return true;
}

//=============================================================================

struct ToolData {
    shared_ptr<string>	netName;
    bool                readAsPT = false;
    shared_ptr<PN>		pn;
    shared_ptr<PETLT>   petlt;
    shared_ptr<RG>   	rg;
    shared_ptr<RG2>   	rg2;
    shared_ptr<CTMC>    ctmc;
    shared_ptr<MRP>     mrp;
    shared_ptr<string>	dtaName;
    shared_ptr<DTA>     dta;
    shared_ptr<SAT>     sat;
    shared_ptr<flow_matrix_t> pinv, tinv;
    shared_ptr<flow_matrix_t> pbasis, tbasis;
    shared_ptr<flow_matrix_t> pminflows, tminflows;
    shared_ptr<flow_matrix_t> pnestflspan, tnestflspan;
    shared_ptr<place_bounds_t> pbounds;
    bool use_Colom_pivoting = true;
    bool extra_vars_in_support = true;
#ifdef HAS_LP_SOLVE_LIB
    shared_ptr<place_ilp_bounds_t> pIlpBounds;
#endif
    bool                detectExpFlows = false;
    bool 				withVanishings;
    bool 				withVPaths;
    shared_ptr<PetriNetSolution>  sol;
    bool 				drawClustersInDot;
    size_t              maxSavedMarkingsInDot;
    SolverParams 		spar;
    VerboseLevel 		verboseLvl;

    ToolData() {
        // Initialize the parameters with their default value
        withVanishings = false;
        withVPaths = false;
        drawClustersInDot = false;
        maxSavedMarkingsInDot = 80;
        verboseLvl = VL_BASIC;

        spar.epsilon		       = 1.0e-7;
        spar.omega			       = 0.95;
        spar.maxIters		       = 10000;
        spar.KrylovM		       = 30;
        spar.explicitAlgo	       = ELA_FORWARD_SOR;
        spar.implicitAlgo	       = ILA_GMRES_M;
        spar.precAlgo              = PA_NULL_PRECONDITIONER;
        spar.ilutkThreshold        = 1.0e-5;
        spar.ilutkMaxNumEntries    = 8;
        spar.implicitPrecStrategy  = IPS_NO_PRECONDITIONER;
        spar.inoutEpsilon          = 1.0e-7;
        spar.inoutKrylovM          = 25;
        spar.inoutMaxIters         = 50;
        spar.ipaNumSimulations     = 100;
        spar.ipaSamplesPerRow      = 7;
        spar.ipaAlpha		       = 1.0;
        spar.ipaPrecSum            = -0.001;
        spar.ipaOldPrec            = false;
        spar.ipaAddSelfLoop        = false;
        spar.rndSeed               = 0;
        spar.aggregateComplexComp  = true;
        spar.compMethod            = PCM_GREEDY;
        spar.randomizeSccSelection = false;
        spar.solMethod             = LSS_EXPLICIT;
        spar.dtaSearchPaths        = { ".", "DTA" };
        spar.onTheFlyModelChecking = false;
        spar.useSCCDecomp          = false;
        spar.useZonedDTA           = true;
        spar.onTheFlyCompRecursive = false;

    }

    void RequirePetriNet() const {
        if (pn == nullptr || netName == nullptr || petlt == nullptr)
            throw program_exception("Load a Petri Net with the -load command "
                                    "before issuing other commands.");
    }

    void RequireTRG_1(bool withVanishings = false) {
        RequirePetriNet();
        if (!ValidateMarkRatePars(*pn))
            throw program_exception("Missing values of parameters.");
        if (rg == nullptr || withVanishings) {
            set_null(sol);
            set_null(ctmc);
            set_null(mrp);
            set_null(rg2);

            cout << console::beg_title();
            if (withVanishings)
                cout << "BUILDING THE REACHABILITY GRAPH (WITH VANISHINGS)...";
            else
                cout << "BUILDING THE TANGIBLE REACHABILITY GRAPH...";
            cout << console::end_title() << endl;

            performance_timer timer;
            RGBuildStat buildStat;
            rg = make_shared<RG>();
            BuildRG(*pn, *petlt, *rg, withVanishings,
                    withVPaths, buildStat, verboseLvl);
            cout << "MARKING COUNT: " << rg->markSet.size() << endl;
            int cntNPExpTrn = rg->countNumOfTimedPaths(true, false, false);
            int cntPrExpTrn = rg->countNumOfTimedPaths(false, true, false);
            int cntDetTrn = rg->countNumOfTimedPaths(false, false, true);
            cout << "NON-PREEMPTIVE EXPONENTIAL ARCS: " << cntNPExpTrn << endl;
            cout << "PREEMPTIVE EXPONENTIAL ARCS: " << cntPrExpTrn << endl;
            cout << "DETERMINISTIC ARCS: " << cntDetTrn << endl;
            cout << "TRANSITION PATHS FOLLOWED: " << buildStat.numPathsFollowed << endl;
            cout << "MAX. CONCURRENTLY ENABLED DETERMINISTIC TRANSITIONS: ";
            cout << buildStat.maxDetTrnsConEnabl << endl;
            cout << "MAX. OUT ARCS FROM A MARKING: ";
            cout << buildStat.maxExitArcs << endl;
            if (!withVanishings) {
                if (withVPaths) {
                    cout << "MAX. NUMBER OF VANISHING PATHS IN A TRG ARC: ";
                    cout << buildStat.maxArcVanishPaths << endl;
                }
                cout << "MAX. VANISHING PATH LENGTH: ";
                cout << buildStat.maxVisitDepth << endl;
            }
            else {
                if (rg->hasMultipleInitMarks()) {
                    cout << "THE TRG HAS " << rg->initMark.timedPaths.size();
                    cout << " INITIAL MARKINGS.\n";
                }
                else
                    cout << "THE TRG HAS A SINGLE INITIAL MARKING.\n";
            }
            cout << "BUILD TIME: " << timer << "\n\n";
            cout << "\nTHE RG IS A " << console::beg_emph();
            cout << RGClass_Names[rg->rgClass] << console::end_emph() << endl;
            cout << endl;
            this->withVanishings = withVanishings;

            if (print_stat_for_gui()) {
                cout << "#{GUI}# RESULT STAT build_time " << timer << endl;
            }
        }
    }

    void RequireVRG_1() {
        RequireTRG_1(true);
    }

    void RequireTRG_2(bool withVanishings = false) {
        RequirePetriNet();
        if (!ValidateMarkRatePars(*pn))
            throw program_exception("Missing values of parameters.");
        if (rg2 == nullptr || withVanishings) {
            RequirePetriNet();
            set_null(sol);
            set_null(ctmc);
            set_null(mrp);
            set_null(rg);
            bool reduceImmediateTrns = !withVanishings;
            bool storeVanishingPaths = !withVPaths;
            bool eraseVanishingMarkings = true;
            withVanishings = !reduceImmediateTrns;
            performance_timer timer;
            RG2BuildStat buildStat;
            rg2 = make_shared<RG2>();
            BuildRG2(*pn, *petlt, *rg2, buildStat, reduceImmediateTrns,
                     storeVanishingPaths, eraseVanishingMarkings, verboseLvl);
            cout << "BUILD TIME: " << timer << "\n\n";
            cout << "\nTHE RG IS A " << console::beg_emph();
            cout << RGClass_Names[rg2->rgClass] << console::end_emph() << endl;
            cout << endl;
            this->withVanishings = withVanishings;

            if (print_stat_for_gui()) {
                cout << "#{GUI}# RESULT STAT build_time " << timer << endl;
                cout << "#{GUI}# RESULT STAT rg_class " << RGClass_Names[rg2->rgClass] << endl;
                cout << "#{GUI}# RESULT STAT num_tangible_markings " << rg2->stateSets[TANGIBLE].size() << endl;
                cout << "#{GUI}# RESULT STAT num_vanishing_markings " << rg2->stateSets[VANISHING].size() << endl;

                cout << "#{GUI}# RESULT STAT immediate_firings " << buildStat.numImmedFiredTrns << endl;
                cout << "#{GUI}# RESULT STAT timed_firings " << buildStat.numTimedFiredTrns << endl;
                cout << "#{GUI}# RESULT STAT visited_vanishing_markings " << buildStat.numVanishingStates << endl;

                if (rg2->initStates.size() > 1)
                    cout << "#{GUI}# RESULT STAT multiple_initial_tangible_markings " << rg2->initStates.size() << endl;
            }
        }
    }

    void RequireTRG_1or2() {
        RequirePetriNet();
        if ((rg == nullptr && rg2 == nullptr) || withVanishings)
            RequireTRG_2();
    }

    void Require_VRGorTRG_1() {
        RequirePetriNet();
        if (rg == nullptr)
            RequireTRG_1();
    }

    void Require_VRGorTRG_1or2() {
        RequirePetriNet();
        if (rg == nullptr && rg2 == nullptr)
            RequireTRG_2();
    }

    void RequireSolution() {
        RequireTRG_1or2();
        if (sol == nullptr)
            throw program_exception("Compute the solution of the PN before, using -s, -t or -b.");
    }

    void RequireDTA() const {
        if (dta == nullptr)
            throw program_exception("Load a DTA with -dta <filename.dta> before.");
    }

    void RequireSAT() const {
        if (sat == nullptr)
            throw program_exception("Compute the Model checking solution before with -cslta.");
    }

    void RequireOrBuildCTMC() {
        RequireTRG_1or2();
        if (RGC_CTMC != GetRGClass())
            throw program_exception("The reachability graph is not a CTMC.");
        if (ctmc == nullptr) {
            ctmc = make_shared<CTMC>();
            if (rg != nullptr)
                ExtractCTMC(*rg, *ctmc, verboseLvl);
            else
                ExtractCTMC(*rg2, *ctmc, verboseLvl);
        }
    }

    void RequireOrBuildMRP() {
        RequireTRG_1or2();
        if (RGC_MarkovRenewalProcess != GetRGClass())
            throw program_exception("The reachability graph is not a Markov Renewal Process.");
        if (mrp == nullptr) {
            mrp = make_shared<MRP>();
            if (rg != nullptr)
                ExtractMRP(*rg, *mrp, verboseLvl);
            else
                ExtractMRP(*rg2, *mrp, verboseLvl);
        }
    }

    void RequirePSemiflows() const {
        RequirePetriNet();
        if (pinv == nullptr)
            throw program_exception("Compute P-semiflows with the -psfl command before!");
    }

    RGClass GetRGClass() const {
        if (rg == nullptr && rg2 == nullptr)
            throw program_exception("GetRGClass() called without having constructted the (t)rg.");
        if (rg != nullptr)
            return rg->rgClass;
        else return rg2->rgClass;
    }

    int GenerateDotFile(const string &DotName, bool openIt) const {
        // use posix_spawn, which provides better compatibility across Unix and Cygwin
        string dot_fname = DotName + ".dot";
        string pdf_fname = DotName + ".pdf";

        int status = dot_to_pdf(dot_fname.c_str(), pdf_fname.c_str());

        if (0==status) {
            cout << "DOT FILEs SAVED AS " << DotName << ".dot ";
            cout << "AND " << DotName << ".pdf\n" << endl;
            if (openIt)
                open_file(pdf_fname.c_str());
            if (invoked_from_gui()) {
                cout << "#{GUI}# RESULT " << (withVanishings ? "RG" : "TRG") << endl;
            }
        }
        return status;
    }


    int ExecuteCommandLine(int argc, char *const *argv);
};

int main(int argc, char **argv) {
    ToolData td;
    return td.ExecuteCommandLine(argc, argv);
}

int ToolData::ExecuteCommandLine(int argc, char *const *argv) {
    int argNum = 1;

    if (argc == 1) {
        cout << "Nothing to do. Try with the \'-h\' argument." << endl;
    }
    while (argNum < argc) {
        string cmdArg(argv[argNum++]);
        size_t remainedArgs = argc - argNum;
        invariants_spec_t inv_spec;

        try {
            if (cmdArg == "-h" || cmdArg == "-help" ||
                    cmdArg == "-?" || cmdArg == "--help") {
                PrintAppBanner();
            }
            // else if (cmdArg == "-pmap") {
            // 	string pmap = "pmap -x ";
            // 	pmap += std::to_string(getpid());
            // 	cout << pmap << endl;
            // 	system(pmap.c_str());
            // }
            else if (cmdArg == "-v") {
                verboseLvl = VL_VERBOSE;
                withVPaths = true;
            }
            else if (cmdArg == "-vv") {
                verboseLvl = VL_VERY_VERBOSE;
                withVPaths = true;
            }
            else if (cmdArg == "-nv") {
                verboseLvl = VL_BASIC;
                withVPaths = false;
            }
            else if (cmdArg == "-nnv") {
                verboseLvl = VL_NONE;
                withVPaths = false;
            }
            else if (cmdArg == "-pt") {
                cout << "LOADING NETS AS P/T NETS." << endl;
                readAsPT = true;
            }
            else if (cmdArg == "-load" && remainedArgs >= 1) {
                netName = make_shared<string>(argv[argNum++]);
                pn = make_shared<PN>();
                petlt = make_shared<PETLT>();
                set_null(rg);
                set_null(rg2);
                set_null(sol);
                set_null(ctmc);
                set_null(mrp);

                // Read the .net/.def files
                string PNetName(*netName + ".net");
                string PDefName(*netName + ".def");
                if (verboseLvl >= VL_BASIC) {
                    cout << console::beg_title() << "LOADING PETRI NET " << *netName;
                    cout << " (net/def)..." << console::end_title() << endl;
                }
                ifstream net(PNetName.c_str());
                if (!net) {
                    cout << "Couldn't open file " << *netName << ".net!" << endl;
                    return -1;
                }
                ifstream def(PDefName.c_str());
                if (!def) {
                    cout << "Couldn't open file " << *netName << ".def!" << endl;
                    return -1;
                }
                performance_timer timer;
                ReadGreatSPN_File(net, def, *pn, readAsPT, verboseLvl);
                net.close();
                def.close();
                BuildPossiblyEnabledTransitionLookupTable(*pn, *petlt, verboseLvl);

                if (verboseLvl >= VL_BASIC) {
                    cout << "MARKING PAR: " << pn->mpars.size() << endl;
                    cout << "PLACES:      " << pn->plcs.size() << endl;
                    cout << "RATE PAR:    " << pn->rpars.size() << endl;
                    cout << "TRANSITIONS: " << pn->trns.size() << endl;
                    cout << "MEASURES:    " << pn->measures.size() << endl;
                    cout << "LOADING TIME: " << timer << "\n\n";
                    cout << endl;
                }
            }
            else if ((cmdArg == "-load-mrmc" || cmdArg == "-load-prism") && remainedArgs >= 1) {
                netName = make_shared<string>(argv[argNum++]);
                pn = make_shared<PN>();
                rg2 = make_shared<RG2>();
                petlt = make_shared<PETLT>();
                set_null(rg);
                set_null(sol);
                set_null(ctmc);
                set_null(mrp);

                // Read the MRMC/Prism model files
                performance_timer timer;
                bool isPrism = (cmdArg == "-load-prism");
                string TraName(*netName + (isPrism ? ".trns" : ".tra"));
                string LabName(*netName + (isPrism ? ".labels" : ".lab"));
                string StaName(*netName + (isPrism ? ".states" : ""));
                cout << console::beg_title() << "LOADING " << (isPrism ? "PRISM" : "MRMC") << " MODEL ";
                cout << *netName << " ..." << console::end_title() << endl;
                ifstream ifsTra(TraName.c_str());
                if (!ifsTra) {
                    cout << "Couldn't open file " << TraName << "!" << endl;
                    return -1;
                }
                ifstream ifsLab(LabName.c_str());
                if (!ifsLab) {
                    cout << "Couldn't open file " << LabName << "!" << endl;
                    return -1;
                }
                if (isPrism) {
                    ifstream ifsSta(StaName.c_str());
                    if (!ifsSta) {
                        cout << "Couldn't open file " << StaName << "!" << endl;
                        return -1;
                    }
                    LoadPrismModel(ifsTra, ifsLab, ifsSta, *pn, *rg2, verboseLvl);
                    ifsSta.close();
                }
                else
                    LoadMRMC_Model(ifsTra, ifsLab, *pn, *rg2, verboseLvl);
                BuildPossiblyEnabledTransitionLookupTable(*pn, *petlt, verboseLvl);
                ifsTra.close();
                ifsLab.close();
                cout << "\nLOADING TIME: " << timer << "\n\n";
                cout << endl;
            }
            else if (cmdArg == "-no-vpaths") {
                withVPaths = false;
            }
            //TODO: remove this switch clause
            else if (cmdArg == "-new-trg" || cmdArg == "-new-rg") {
                RequireTRG_2(cmdArg == "-new-rg");
            }
            else if (cmdArg == "-trg" || cmdArg == "-rg") {
                RequireTRG_1(cmdArg == "-rg");
            }
            else if (cmdArg == "-with-clusters") {
                drawClustersInDot = true;
            }
            else if (cmdArg == "-max-dot-markings" && remainedArgs >= 1) {
                maxSavedMarkingsInDot = atoi(argv[argNum++]);
            }
            else if (cmdArg == "-dot" || 
                    (cmdArg == "-dot-F" && remainedArgs >= 1) || 
                     cmdArg == "-dot-open") 
            {
                Require_VRGorTRG_1or2();
                string DotName;
                if (cmdArg == "-dot-F") {
                    DotName = argv[argNum++];
                } else DotName = *netName + (withVanishings ? "-RG" : "-TRG");
                ofstream dotFile((DotName + ".dot").c_str());
                if (rg != nullptr)
                    SaveRG_AsDotFile(*pn, *rg, drawClustersInDot, dotFile);
                else
                    SaveRG_AsDotFile(*pn, *rg2, drawClustersInDot, dotFile, maxSavedMarkingsInDot);
                dotFile.close();

                GenerateDotFile(DotName, (cmdArg == "-dot-open"));
            }
            else if (cmdArg == "-export-rg") {
                Require_VRGorTRG_1();

                string RgName = *netName + ".rg";
                ofstream rgFile(RgName.c_str());
                SaveReachabilityGraph(*pn, *rg, rgFile);
                rgFile.close();

                cout << "REACHABILITY GRAPH SAVED AS " << RgName << ".\n" << endl;
            }
            else if (cmdArg == "-dta" && remainedArgs >= 1) {
                // Read the .dta file
                dtaName = make_shared<string>(argv[argNum++]);
                cout << console::beg_title() << "LOADING DTA FILE " << *dtaName << console::end_title() << endl;
                ifstream dtaifs(dtaName->c_str());
                if (!dtaifs) {
                    cout << "Couldn't open file " << *dtaName << "!" << endl;
                    return -1;
                }
                performance_timer timer;
                dta = make_shared<DTA>();
                LoadDTAFromFile(dtaifs, *dta, verboseLvl);
                dtaifs.close();

                cout << "DTA NAME:     " << dta->name << endl;
                cout << "LOADING TIME: " << timer << "\n\n";
                cout << endl;
            }
            else if (cmdArg == "-bind" && remainedArgs >= 1) {
                //RequirePetriNet();
                RequireDTA();
                // Get the binding rules
                const char *rules = argv[argNum++];
                cout << "BINDING PARAMETERS OF THE \"" << dta->name << "\" DTA ... " << endl;

                bool isComplete = BindDTAToPN(*dta, pn.get(), rules, verboseLvl);

                cout << "DTA IS " << (isComplete ? "" : "PARTIALLY ") << "BOUND." << endl;
                cout << endl;
            }
            else if (cmdArg == "-zdta") {
                spar.useZonedDTA = true;
                cout << "USE ZONE DTA CONSTRUCTION ... " << endl;
                if (dta) {
                    shared_ptr<DTA> zdta = make_shared<DTA>();
                    MakeZonedDTA(*dta, *zdta, verboseLvl);
                    dta = zdta;
                }
            }
            else if (cmdArg == "-no-zdta") {
                spar.useZonedDTA = false;
                cout << "AVOID ZONE DTA CONSTRUCTION WHEN POSSIBLE... " << endl;
            }
            else if (cmdArg == "-dot-dta" || cmdArg == "-dot-dta-open") {
                RequireDTA();
                string DotName = *dtaName;
                ofstream dotFile((*dtaName + ".dot").c_str());
                SaveDTA_AsDotFile(*dta, dotFile);
                dotFile.close();
                GenerateDotFile(DotName, (cmdArg == "-dot-dta-open"));
            }
            else if (cmdArg == "-dta-path" && remainedArgs >= 1) {
                const char *newDtaPath = argv[argNum++];
                spar.dtaSearchPaths.push_back(newDtaPath);
                cout << "ADDING \"" << newDtaPath << "\" TO THE DTA SEARCH PATHs." << endl;
            }
            else if (cmdArg == "-on-the-fly") {
                spar.onTheFlyModelChecking = true;
                cout << "USING ON-THE-FLY STATE SPACE CONSTRUCTION OF CSL^TA MODEL CHECKING." << endl;
            }
            else if (cmdArg == "-on-the-fly-components") {
                spar.onTheFlyCompRecursive = true;
                cout << "APPLY COMPONENT METHOD ON EACH ON-THE-FLY COMPONENT." << endl;
            }
            else if (cmdArg == "-fmc" || cmdArg == "-bmc") { // Forward/backward model checking
                bool isForward = (cmdArg == "-fmc");
                RequirePetriNet();
                RequireTRG_2();
                RequireDTA();
                if (!dta->isBound)
                    throw program_exception("Bind the DTA with -bind before using the -mc command.");
                if (GetRGClass() != RGC_CTMC)
                    throw program_exception("Model checking requires a valid CTMC.");
                set_null(sat);

                cout << (isForward ? "FORWARD" : "BACKWARD") << " MODEL CHECKING";
                cout << " USING DTA " << dta->name << " ..." << endl;
                performance_timer timer;
                shared_ptr<SAT> mcsat = make_shared<SAT>();
                if (isForward)
                    mcsat->probs.resize(1);

                if (spar.onTheFlyModelChecking) { // Component method over the ZDTA
                    DTA zonedDta;
                    if (!dta->isZoned)
                        MakeZonedDTA(*dta, zonedDta, verboseLvl);
                    const DTA &zdta = (!dta->isZoned) ? zonedDta : *dta;
                    PrepareStatePropositions(*rg2, zonedDta, spar, verboseLvl);
                    if (isForward) {
                        double p = ModelCheck_CTMC_ZDTA_Forward_SCC(*rg2, zdta, marking_t(0),
                                   spar, verboseLvl);
                        mcsat->probs(0) = p;
                    }
                    else
                        ModelCheck_CTMC_ZDTA_Backward_SCC(*rg2, zdta, mcsat->probs, spar, verboseLvl);
                }
                else { // Construct the whole state space SxLxC
                    PrepareStatePropositions(*rg2, *dta, spar, verboseLvl);
                    if (isForward) {
                        double p = ModelCheck_CTMC_DTA_Forward(*rg2, *dta, marking_t(0),
                                                               spar, verboseLvl);
                        mcsat->probs(0) = p;
                    }
                    else // Backward whole-state-space model checking
                        ModelCheck_CTMC_DTA_Backward(*rg2, *dta, mcsat->probs,
                                                     spar, verboseLvl);
                }
                std::fill(mcsat->sat.begin(), mcsat->sat.end(), true);

                cout << console::beg_success() << "RESULT" << console::end_success() << ": ";
                if (isForward)
                    cout << "ACCEPTANCE PROBABILITY IS " << fmtdbl(mcsat->probs[0], "%.12lf");
                else { // backward
                    if (mcsat->probs.size() < 1000)
                        cout << print_vec(mcsat->probs);
                    else
                        cout << "...";
                    cout << "\nACCEPTANCE PROBABILITY OF s[0] IS " << fmtdbl(mcsat->probs[0], "%.12lf");
                }

                std::swap(sat, mcsat);
                cout << "\n\nSOLUTION TIME: " << timer;
                cout << "\nMODEL CHECKING COMPLETED.\n\n" << endl;
            }
            else if ((cmdArg == "-cslta" && remainedArgs >= 1) ||
                     (cmdArg == "-cslta0" && remainedArgs >= 1) ||
                     (cmdArg == "-cslta0-X" && remainedArgs >= 2) || // -cslta0-X <name> <expr>
                     (cmdArg == "-csltaN" && remainedArgs >= 2)) {
                RequirePetriNet();
                RequireTRG_2();
                if (GetRGClass() != RGC_CTMC)
                    throw program_exception("Model checking requires a valid CTMC.");
                set_null(sat);
                boost::optional<marking_t> state0;
                if (cmdArg == "-cslta0" || cmdArg == "-cslta0-X") {
                    if (rg2->initStates.size() != 1)
                        throw program_exception("Option -cslta0 can only be used when the "
                                                "CTMC has a single initial state. Use -csltaN <index> <expr>");
                    state0.reset(rg2->initStates[0].state.index());
                }
                else if (cmdArg == "-csltaN") {
                    const char *state0Index = argv[argNum++];
                    state0.reset(marking_t(atoi(state0Index)));
                    if (!state0 || size_t(*state0) > rg2->NumStates()) {
                        throw program_exception("Invalid initial CTMC state index.");
                    }
                }
                const char *measName = nullptr;
                if (cmdArg == "-cslta0-X")
                    measName = argv[argNum++];
                const char *exprText = argv[argNum++];
                performance_timer timer;
                shared_ptr<AtomicPropExpr> cslTaExpr = ParseCslTaExpr(*pn, exprText, verboseLvl);

                cout << console::beg_title() << "\n\nMODEL CHECKING CSLTA EXPRESSION:" << console::end_title() << "\n  ";
                if (state0)
                    cout << "STATE " << *state0 << " |= ";
                cout << print_petrinetobj(cslTaExpr) << "\n\n" << endl;

                ModelCheckCslTA(*rg2, cslTaExpr, state0, spar, verboseLvl);

                cout << console::beg_success() << "RESULT" << console::end_success() << ": ";
                shared_ptr<SAT> mcsat = make_shared<SAT>(cslTaExpr->GetStoredSAT());
                if (state0) {
                    cout << ((*mcsat)[0] ? "TRUE." : "FALSE.");
                    if (mcsat->probs.size() == 1)
                        cout << " ACCEPTANCE PROBABILITY IS " << fmtdbl(mcsat->probs[0], "%.12lf");
                }
                else {
                    if (rg2->NumStates() > 100) {
                        cout << "COMPUTED FOR ALL THE " << rg2->NumStates() << " INTIAL STATES.";
                        size_t numSat = std::count(mcsat->sat.begin(), mcsat->sat.end(), true);
                        cout << " (" << numSat << " states satisfy).";
                    }
                    else {
                        cout << print_vec(mcsat->sat);
                        if (mcsat->probs.size() == rg2->NumStates())
                            cout << "\n PROBS: " << print_vec(mcsat->probs);
                    }
                }
                if (measName != nullptr && invoked_from_gui()) {
                    cout << "\n#{GUI}# RESULT " << measName << " "
                         << ((*mcsat)[0] ? "true" : "false");
                    if (mcsat->probs.size() == 1) // stochastic probability
                        cout << " " << fmtdbl(mcsat->probs[0], "%.12lf");
                    cout << endl;
                }

                std::swap(sat, mcsat);
                cout << "\n\nSOLUTION TIME: " << timer;
                cout << "\nMODEL CHECKING COMPLETED.\n\n" << endl;
            }
            else if (cmdArg == "-mpar" && remainedArgs >= 2) {
                RequirePetriNet();
                if (rg != nullptr)
                    throw program_exception("Marking parameters can be changed "
                                            "only before the RG construction.");
                string name = argv[argNum++];
                int val = atoi(argv[argNum++]);
                if (!ChangeMarkPar(*pn, name, val))
                    throw program_exception("The specified marking parameter name does not exists.");
                cout << "MARKING PARAMETER \"" << name << "\" SET TO " << val << endl;
            }
            else if (cmdArg == "-rpar" && remainedArgs >= 2) {
                RequirePetriNet();
                if (rg != nullptr)
                    throw program_exception("Rate parameters can be changed "
                                            "only before the RG construction.");
                string name = argv[argNum++];
                double val = atof(argv[argNum++]);
                if (!ChangeRatePar(*pn, name, val))
                    throw program_exception("The specified rate parameter name does not exists.");
                cout << "RATE PARAMETER \"" << name << "\" SET TO " << val << endl;
            }
            else if (cmdArg == "-show-pars") {
                RequirePetriNet();
                PrintParameters(*pn);
                cout << endl;
            }
            else if (cmdArg == "-measure" && remainedArgs >= 2) {
                RequirePetriNet();
                const char *name = argv[argNum++];
                const char *expr = argv[argNum++];
                AddNewMeasure(*pn, name, expr, verboseLvl);
            }
            else if (cmdArg == "-all-measures") {
                RequireTRG_2();
                RequireSolution();
                ComputeAllBasicMeasures(*pn, *rg2, *sol);
                cout << "\nBASIC PETRI NET MEASURES COMPUTED." << endl;
            }
            else if (cmdArg == "-export-spnica") {
                RequirePetriNet();
                string NbFile(*netName + ".nb");
                ofstream nbf(NbFile.c_str());
                SaveAsSPNicaPetriNet(*pn, nbf);
                nbf.close();

                cout << "SPNICA NET FILE SAVED AS " << NbFile << ".\n" << endl;
            }
            else if (cmdArg == "-export-cosmos") {
                RequirePetriNet();
                string CosmosFile(*netName + ".gspn");
                ofstream cmf(CosmosFile.c_str());
                SaveAsCosmosPetriNet(*pn, *netName, cmf);
                cmf.close();

                cout << "COSMOS GSPN FILE SAVED AS " << CosmosFile << ".\n" << endl;
            }
            else if (cmdArg == "-export-asmc") {
                RequireTRG_1or2();
                string AsmcFile(*netName + ".cslta");
                ofstream asmc(AsmcFile.c_str());
                if (rg != nullptr)
                    SaveAsCsltaASMC(*pn, *rg, netName->c_str(), asmc);
                else
                    SaveAsCsltaASMC(*pn, *rg2, netName->c_str(), asmc);
                asmc.close();

                cout << "ASMC SAVED AS " << AsmcFile << ".\n" << endl;
            }
            else if (cmdArg == "-export-ctmc") {
                RequireTRG_1();
                string CtmcFile(*netName + ".ctmc");
                string StatesFile(*netName + ".states");
                string Pi0DistrFile(*netName + ".pi0_distr");
                ofstream ctmc(CtmcFile.c_str());
                ofstream states(StatesFile.c_str());
                SaveAsPrismCTMC(*pn, *rg, ctmc, states, Pi0DistrFile.c_str());
                ctmc.close();
                states.close();

                cout << "CTMC SAVED AS " << *netName << ".(ctmc,states) in Prism ";
                cout << "format.\n" << endl;
            }
            else if (cmdArg == "-detect-exp") {
                cout << "AVOID EXPONENTIAL GROWTH OF SEMIFLOWS." << endl;
                detectExpFlows = true;
            }
            else if (cmdArg == "-trivial-pivoting") {
                cout << "USING TRIVIAL PIVOTING." << endl;
                use_Colom_pivoting = false;
            }
            else if (cmdArg == "-strict-support") {
                cout << "EXCLUDING SLACK VARIABLES FROM FLOWS SUPPORT." << endl;
                extra_vars_in_support = false;
            }
            // else if (cmdArg == "-pinv"   || cmdArg == "-pinv+"   || cmdArg == "-pinv-"   || cmdArg == "-pinv+-"   || cmdArg == "-pinv*"   ||
            //          cmdArg == "-tinv"   || cmdArg == "-tinv+"   || cmdArg == "-tinv-"   || cmdArg == "-tinv+-"   || cmdArg == "-tinv*"   ||
            //          cmdArg == "-psfl"   || cmdArg == "-psfl+"   || cmdArg == "-psfl-"   || cmdArg == "-psfl+-"   || cmdArg == "-psfl*"   ||
            //          cmdArg == "-tsfl"   || cmdArg == "-tsfl+"   || cmdArg == "-tsfl-"   || cmdArg == "-tsfl+-"   || cmdArg == "-tsfl*"   ||
            //          cmdArg == "-pbasis" || cmdArg == "-pbasis+" || cmdArg == "-pbasis-" || cmdArg == "-pbasis+-" || cmdArg == "-pbasis*" ||
            //          cmdArg == "-tbasis" || cmdArg == "-tbasis+" || cmdArg == "-tbasis-" || cmdArg == "-tbasis+-" || cmdArg == "-tbasis*" ||
            //          cmdArg == "-pfl"    || cmdArg == "-pfl+"    || cmdArg == "-pfl-"    || cmdArg == "-pfl+-"    || cmdArg == "-pfl*"    ||
            //          cmdArg == "-tfl"    || cmdArg == "-tfl+"    || cmdArg == "-tfl-"    || cmdArg == "-tfl+-"    || cmdArg == "-tfl*"    ||
            //          cmdArg == "-traps"  || cmdArg == "-siphons") 
            else if (cmdArg == "-imPxT" || cmdArg == "-imTxP") {
                RequirePetriNet();
                shared_ptr<flow_matrix_t> psf;
                psf = ComputeFlows(*pn, (cmdArg == "-imPxT") ? InvariantKind::PLACE : InvariantKind::TRANSITION, 
                                   FlowMatrixKind::NONE, SystemMatrixType::REGULAR, false, 0, false, false, verboseLvl);

                string IncMatFile(*netName + ".mat");
                cout << "SAVING INCIDENCE MATRIX AS: "<<IncMatFile<<endl;
                ofstream im_os(IncMatFile.c_str());
                psf->save_matrix_A(im_os);
                im_os.close();
            }
            else if (is_invariants_cmd(cmdArg, inv_spec))
            {
                RequirePetriNet();
                // size_t suppl_flags = 0;
                // InvariantKind invknd = InvariantKind::PLACE;
                // SystemMatrixType system_kind = SystemMatrixType::REGULAR;
                // FlowMatrixKind matk = FlowMatrixKind::SEMIFLOWS;
                // if (cmdArg == "-traps"  || cmdArg == "-siphons") {
                //     system_kind = (cmdArg == "-traps" ? SystemMatrixType::TRAPS : SystemMatrixType::SIPHONS);
                // }
                // else {
                //     invknd = (cmdArg[1]=='p' ? InvariantKind::PLACE : InvariantKind::TRANSITION);
                //     if (cmdArg[2] == 'b') // [b]asis
                //         matk = FlowMatrixKind::BASIS;
                //     if (cmdArg[2] == 'f') // [f]l
                //         matk = FlowMatrixKind::INTEGER_FLOWS;
                //     switch (cmdArg[strlen(cmdArg.c_str()) - 1]) {
                //         case '+': suppl_flags |= FM_POSITIVE_SUPPLEMENTARY; break;
                //         case '-': suppl_flags |= FM_NEGATIVE_SUPPLEMENTARY; break;
                //         case '*': suppl_flags |= FM_ON_THE_FLY_SUPPL_VARS | FM_POSITIVE_SUPPLEMENTARY | FM_NEGATIVE_SUPPLEMENTARY; break;
                //         default:  suppl_flags = 0;  break;
                //     }
                //     if (cmdArg[strlen(cmdArg.c_str()) - 2] == '+')
                //         suppl_flags |= FM_POSITIVE_SUPPLEMENTARY;
                // }
                // if (cmdArg[3] == 'p') // s[p]an
                //     matk = FlowMatrixKind::NESTED_FLOW_SPAN;
                performance_timer timer;
                shared_ptr<flow_matrix_t> psf = ComputeFlows(*pn, inv_spec.invknd, inv_spec.matk, 
                                                             inv_spec.system_kind, detectExpFlows, 
                                                             inv_spec.suppl_flags, use_Colom_pivoting, 
                                                             extra_vars_in_support, verboseLvl);
                shared_ptr<flow_matrix_t> *dst;
                switch (inv_spec.matk) {
                    case FlowMatrixKind::SEMIFLOWS:
                        dst = (inv_spec.invknd == InvariantKind::PLACE ? &pinv : &tinv);
                        break;
                    case FlowMatrixKind::BASIS:
                        dst = (inv_spec.invknd == InvariantKind::PLACE ? &pbasis : &tbasis);
                        break;
                    case FlowMatrixKind::INTEGER_FLOWS:
                        dst = (inv_spec.invknd == InvariantKind::PLACE ? &pminflows : &tminflows);
                        break;
                    // case FlowMatrixKind::NESTED_FLOW_SPAN:
                    //     dst = (invknd == InvariantKind::PLACE ? &pnestflspan : &tnestflspan);
                    //     break;
                    default: throw program_exception("Missing case.");
                }
                *dst = psf;
                // Save the flows to the disk in GreatSPN format
                string FlowFile(*netName + GetGreatSPN_FileExt(inv_spec));
                ofstream flow_os(FlowFile.c_str());
                SaveFlows(*psf, flow_os);
                PrintFlows(*pn, *psf, cmdArg.c_str(), verboseLvl);
                flow_os.close();
                cout << "\nTOTAL TIME: " << timer << endl;
            }
            else if (cmdArg == "-bnd") {
                cout << "COMPUTING PLACE BOUNDS FROM P-SEMIFLOWS ..." << endl;
                RequirePSemiflows();
                pbounds = make_shared<place_bounds_t>();
                ComputeBoundsFromSemiflows(*pn, *pinv, *pbounds);
                string BndFile = *netName + ".bnd";
                ofstream bnd_os(BndFile.c_str());
                SaveBounds(*pbounds, bnd_os);
                PrintBounds(*pn, *pbounds, verboseLvl);
            }
            else if (cmdArg == "-load-bnd") {
                RequirePetriNet();
                cout << "LOADING BND FILE ..." << endl;
                string BndFile = *netName + ".bnd";
                ifstream bnd_is(BndFile.c_str());
                pbounds = make_shared<place_bounds_t>();
                LoadBounds(*pn, *pbounds, bnd_is);
                PrintBounds(*pn, *pbounds, verboseLvl);
            }
            else if (cmdArg == "-m0min") {
                cout << "COMPUTING MINIMAL TOKEN COUNTS FROM P-SEMIFLOWS ..." << endl;
                RequirePSemiflows();
                std::vector<int> m0min;
                // pbounds = make_shared<place_bounds_t>();
                ComputeMinimalTokensFromFlows(*pn, *pinv, m0min);
                string m0minFile = *netName + ".m0min";
                ofstream m0min_os(m0minFile.c_str());
                SaveMinimalTokens(m0min, m0min_os);
                PrintMinimalTokens(*pn, m0min, verboseLvl);
            }
            else if (cmdArg == "-timeout" && remainedArgs >= 1) {
                int seconds = atoi(argv[argNum++]);
                if (seconds > 0) {
                    signal(SIGALRM, sig_alarm_handler);
                    alarm(seconds);
                    cout << "Scheduled timeout of " << seconds << " seconds." << endl;
                }
                else {
                    alarm(seconds);
                    signal(SIGALRM, SIG_DFL);
                    cout << "Cancelling timeout." << endl;
                }
            }
#ifdef HAS_LP_SOLVE_LIB
            // Commands that use the ILP solver
            else if (cmdArg == "-ilp-bnd") {
                cout << "COMPUTING PLACE BOUNDS USING ILP ..." << endl;
                RequirePetriNet();
                pIlpBounds = make_shared<place_ilp_bounds_t>();
                ComputeILPBounds(*pn, pbounds.get(), *pIlpBounds, verboseLvl);
                string IlpBndFile = *netName + ".ilpbnd";
                ofstream ilpbnd_os(IlpBndFile.c_str());
                SaveILPBounds(*pIlpBounds, ilpbnd_os);
            }
            else if (cmdArg == "-ilp-max-bnd") {
                cout << "COMPUTING TOKEN COUNT BOUND USING ILP ..." << endl;
                RequirePetriNet();
                string MaxBndFile = *netName + ".maxbnd";
                ofstream maxbnd_os(MaxBndFile.c_str());
                ComputeILPTotalBound(*pn, maxbnd_os, verboseLvl);
            }
            else if (cmdArg == "-upper-bounds-mcc" && remainedArgs >= 1) {
                RequirePetriNet();
                ifstream UB(argv[argNum++]);
                UpperBoundsMCC(*pn, verboseLvl, UB);
            }
#endif // HAS_LP_SOLVE_LIB
            else if (cmdArg == "-omega" && remainedArgs >= 1) {
                spar.omega = atof(argv[argNum++]);
                cout << "USING OVER-RELAXATION COEFFICIENT ";
                cout << spar.omega << " FOR NUMERICAL ANALYSIS." << endl;
            }
            else if (cmdArg == "-epsilon" && remainedArgs >= 1) {
                spar.epsilon = atof(argv[argNum++]);
                cout << "MAX. RESIDUAL ERROR SET TO ";
                cout << spar.epsilon << " FOR NUMERICAL ANALYSIS." << endl;
            }
            else if (cmdArg == "-M" && remainedArgs >= 1) {
                spar.KrylovM = atoi(argv[argNum++]);
                cout << "MAX. NUMBER OF ARNOLDI ITERATIONS IN GMRES: ";
                cout << spar.KrylovM << endl;
            }
            else if (cmdArg == "-max-iters" && remainedArgs >= 1) {
                spar.maxIters = atoi(argv[argNum++]);
                cout << "MAX. NUMBER OF ITERATIONS FOR NUMERICAL ANALYSIS ";
                cout << "SET TO " << spar.maxIters << " ." << endl;
            }
            else if (cmdArg == "-jor") {
                spar.explicitAlgo = ELA_JOR;
                cout << "USING JACOBI/JOR METHOD FOR STATIONARY ANALYSIS." << endl;
            }
            else if (cmdArg == "-forward-sor") {
                spar.explicitAlgo = ELA_FORWARD_SOR;
                cout << "USING FORWARD GAUSS-SEIDEL/SOR METHOD FOR ";
                cout << "STATIONARY ANALYSIS." << endl;
            }
            else if (cmdArg == "-backward-sor") {
                spar.explicitAlgo = ELA_BACKWARD_SOR;
                cout << "USING BACKWARD GAUSS-SEIDEL/SOR METHOD FOR ";
                cout << "STATIONARY ANALYSIS." << endl;
            }
            else if (cmdArg == "-ssor") {
                spar.explicitAlgo = ELA_SYMMETRIC_SOR;
                cout << "USING SYMMETRIC SOR METHOD FOR STATIONARY ANALYSIS." << endl;
            }
            else if (cmdArg == "-i-power") {
                spar.implicitAlgo = ILA_POWER_METHOD;
                cout << "USING POWER METHOD METHOD FOR IMPLICIT STATIONARY ANALYSIS." << endl;
            }
            else if (cmdArg == "-gmres") {
                spar.implicitAlgo = ILA_GMRES_M;
                spar.explicitAlgo = ELA_GMRES_M;
                cout << "USING GMRES METHOD FOR IMPLICIT STATIONARY ANALYSIS." << endl;
            }
            else if (cmdArg == "-bicgstab") {
                spar.implicitAlgo = ILA_BICGSTAB;
                spar.explicitAlgo = ELA_BICGSTAB;
                cout << "USING Bi-CONJUGATE GRADIENTS (STABILIZED) METHOD ";
                cout << "FOR IMPLICIT STATIONARY ANALYSIS." << endl;
            }
            else if (cmdArg == "-cgs") {
                spar.implicitAlgo = ILA_CGS;
                spar.explicitAlgo = ELA_CGS;
                cout << "USING CONJUGATE GRADIENTS SQUARED METHOD ";
                cout << "FOR IMPLICIT STATIONARY ANALYSIS." << endl;
            }
            else if (cmdArg == "-no-prec") {
                spar.precAlgo = PA_NULL_PRECONDITIONER;
                cout << "USING NO PRECONDITIONER PRECONDITIONER STRATEGY FOR KRYLOV STATIONARY ANALYSIS." << endl;
            }
            else if (cmdArg == "-diag-prec") {
                spar.precAlgo = PA_DIAG;
                cout << "USING DIAGONAL(JACOBI) PRECONDITIONER STRATEGY FOR KRYLOV STATIONARY ANALYSIS." << endl;
            }
            else if (cmdArg == "-ilu0-prec") {
                spar.precAlgo = PA_ILU0;
                cout << "USING ILU(0) PRECONDITIONER STRATEGY FOR KRYLOV STATIONARY ANALYSIS." << endl;
            }
            else if (cmdArg == "-ilutk-prec" && remainedArgs >= 2) {
                spar.ilutkThreshold = atof(argv[argNum++]);
                spar.ilutkMaxNumEntries = atoi(argv[argNum++]);
                spar.precAlgo = PA_ILUTK;
                cout << "USING ILUTK(Threshold=" << spar.ilutkThreshold << ", K=";
                cout << spar.ilutkMaxNumEntries << ") PRECONDITIONER STRATEGY FOR KRYLOV STATIONARY ANALYSIS." << endl;
            }
            else if (cmdArg == "-i-no-inout") {
                spar.implicitPrecStrategy = IPS_NO_PRECONDITIONER;
                cout << "USING NO INNER/OUTER PRECONDITIONER STRATEGY FOR IMPLICIT MRP STATIONARY ANALYSIS." << endl;
            }
            else if (cmdArg == "-i-prec-emc") {
                spar.implicitPrecStrategy = IPS_PREC_EMC;
                cout << "PRECONDITIONER WILL BE BUILT FROM THE EXACT EMC MATRIX (SLOW)." << endl;
            }
            else if (cmdArg == "-i-prec-sim") {
                spar.implicitPrecStrategy = IPS_PREC_SIM;
                cout << "PRECONDITIONER WILL BE BUILT FROM A SIMULATED EMC MATRIX." << endl;
            }
            else if (cmdArg == "-i-inout-emc") {
                spar.implicitPrecStrategy = IPS_INNEROUTER_EMC;
                cout << "INNER/OUTER PRECONDITIONER WILL BE BUILT FROM THE EXACT EMC MATRIX SLOW)." << endl;
            }
            else if (cmdArg == "-i-inout-sim") {
                spar.implicitPrecStrategy = IPS_INNEROUTER_SIM;
                cout << "INNER/OUTER PRECONDITIONER WILL BE BUILT FROM A SIMULATED EMC." << endl;
            }
            else if (cmdArg == "-i-inout-epsilon" && remainedArgs >= 1) {
                spar.inoutEpsilon = atof(argv[argNum++]);
                cout << "INNER/OUTER PRECONDITIONER EPSILON SET TO ";
                cout << spar.inoutEpsilon << endl;
            }
            else if (cmdArg == "-i-inout-max-iters" && remainedArgs >= 1) {
                spar.inoutMaxIters = atoi(argv[argNum++]);
                cout << "INNER/OUTER PRECONDITIONER MAXIMUM NUMBER OF ITERATIONS SET TO ";
                cout << spar.inoutMaxIters << endl;
            }
            else if (cmdArg == "-i-inout-M" && remainedArgs >= 1) {
                spar.inoutKrylovM = atoi(argv[argNum++]);
                cout << "INNER/OUTER PRECONDITIONER ARNOLDI BASE DIMENSION SET TO ";
                cout << spar.inoutKrylovM << endl;
            }
            else if (cmdArg == "-i-num-sim" && remainedArgs >= 1) {
                spar.ipaNumSimulations = atoi(argv[argNum++]);
                cout << "NUMBER OF SIMULATIONS PER MARKING: ";
                cout << spar.ipaNumSimulations << endl;
            }
            else if (cmdArg == "-i-sim-samples" && remainedArgs >= 1) {
                spar.ipaSamplesPerRow = atoi(argv[argNum++]);
                cout << "MAX. NUMBER OF SAMPLES IN EACH PRECONDITIONER ROW: ";
                cout << spar.ipaSamplesPerRow << endl;
            }
            else if (cmdArg == "-i-alpha" && remainedArgs >= 1) {
                spar.ipaAlpha = atof(argv[argNum++]);
                cout << "PRECONDITIONER MIXING COEFFICIENT: ";
                cout << spar.ipaAlpha << endl;
            }
            else if (cmdArg == "-i-prec-sum" && remainedArgs >= 1) {
                spar.ipaPrecSum = atof(argv[argNum++]);
                cout << "PRECONDITIONER TOTAL SUM: " << spar.ipaPrecSum << endl;
            }
            else if (cmdArg == "-i-old-prec") {
                spar.ipaOldPrec = true;
                cout << "USING OLD PRECONDITIONER." << endl;
            }
            else if (cmdArg == "-i-self-loops") {
                spar.ipaAddSelfLoop = true;
                cout << "APPROXIMATED EMC MATRIX IS BUILT WITH SELF LOOPS." << endl;
            }
            else if (cmdArg == "-i-no-self-loops") {
                spar.ipaAddSelfLoop = false;
                cout << "APPROXIMATED EMC MATRIX IS BUILT WITH DEFECTIVE ROWS (NO SELF LOOPS)." << endl;
            }
            else if (cmdArg == "-i-seed" && remainedArgs >= 1) {
                spar.rndSeed = atoi(argv[argNum++]);
                cout << "RANDOM SEED: " << spar.rndSeed << endl;
            }
            else if (cmdArg == "-gui-stat") {
                enable_print_stat_for_gui();
            }
            /*else if (cmdArg == "-isomorphism") {
            	detectIsomorphism = true;
            	cout << "ISOMORPHISM DETECTION ACTIVATED." << endl;
            }
            else if (cmdArg == "-old-mrp") {
            	reduceInitialTransient = false;
            	cout << "INITIAL MRP TRANSIENT ANALYSIS DISABLED." << endl;
            }*/
            else if (cmdArg == "-i") {
                spar.solMethod = LSS_IMPLICIT;
                cout << "USING IMPLICIT STEADY STATE MRP SOLUTION METHOD." << endl;
            }
            else if (cmdArg == "-e") {
                spar.solMethod = LSS_EXPLICIT;
                cout << "USING EXPLICIT STEADY STATE MRP SOLUTION METHOD." << endl;
            }
            else if (cmdArg == "-scc") {
                spar.useSCCDecomp = true;
                cout << "MRP STEADY STATE SOLUTION USES SCC DECOMPOSITION (IMPLICIT)." << endl;
            }
            // else if (cmdArg == "-sccrep") {
            // 	componentProcMethod = CPM_REPEAT_PRODUCTS;
            // 	cout << "MRP STEADY STATE SOLUTION USES SCC DECOMPOSITION (REPEAT)." << endl;
            // }
            else if (cmdArg == "-shuffle-mrp") {
                RequireTRG_1or2();
                if (GetRGClass() == RGC_MarkovRenewalProcess) {
                    RequireOrBuildMRP();
                    shared_ptr<MRP> n = make_shared<MRP>();
                    srand(spar.rndSeed);

                    std::vector<marking_t> order(mrp->N());
                    for (marking_t m(0); m<mrp->N(); m++)
                        order[m] = m;
                    for (marking_t m(0); m<mrp->N(); m++)
                        std::swap(order[marking_t(rand()) % mrp->N()], 
                                  order[marking_t(rand()) % mrp->N()]);
                    // for (marking_t m(0); m<mrp->N(); m++)
                    //     order[m] = mrp->N() - m - marking_t(1);
                    cout << "SHUFFLING MRP ORDER." << endl;
                    // cout << "ORDER: " << print_vec(order) << endl;
                    ReorderMRP(*mrp, *n, order, verboseLvl);
                    mrp = n;
                }
                else {
                    cout << "NOT AN MRP. CANNOT SHUFFLE." << endl;
                }
            }
            else if (cmdArg == "-noscc") {
                spar.useSCCDecomp = false;
                cout << "MRP STEADY STATE SOLUTION USES NO SCC DECOMPOSITION." << endl;
            }
            else if (cmdArg == "-no-c-aggreg") {
                spar.aggregateComplexComp = false;
                cout << "COMPLEX COMPONENT AGGREGATION IS DISABLED." << endl;
            }
            else if (cmdArg == "-greedy-scc") {
                spar.compMethod = PCM_GREEDY;
                cout << "USE OLDER GREEDY METHOD FOR COMPONENT CONSTRUCTION." << endl;
            }
            else if (cmdArg == "-optimal-scc") {
                spar.compMethod = PCM_POLYNOMIAL_ILP;
                cout << "USE POLYNOMIAL ILP-BASED METHOD FOR COMPONENT CONSTRUCTION." << endl;
            }
            else if (cmdArg == "-ilp-scc") {
                spar.compMethod = PCM_ILP;
                cout << "USE ILP-BASED METHOD FOR COMPONENT CONSTRUCTION (requires lp_solve)." << endl;
            }
            else if (cmdArg == "-randomize-scc") {
                spar.randomizeSccSelection = true;
                cout << "RANDOMIZE SCC SELECTION FOR GREEDY SCC METHOD." << endl;
            }
            else if (cmdArg == "-s") {
                performance_timer timer;
                RequireTRG_1or2();
                if (GetRGClass() == RGC_CTMC) {
                    cout << console::beg_title() << "\nCOMPUTING STEADY STATE SOLUTION ";
                    cout << "OF THE CTMC..." << console::end_title() << endl;
                    RequireOrBuildCTMC();
                    sol = make_shared<PetriNetSolution>();
                    SolveSteadyStateCTMC(*ctmc, *sol, KED_FORWARD, nullptr, spar, verboseLvl);
                    if (rg != nullptr) ComputeMeasures(*rg, *sol, verboseLvl);
                    else 		  ComputeMeasures(*rg2, *sol, verboseLvl);
                    cout << console::beg_success() << "\nSTEADY STATE SOLUTION COMPUTED.\n";
                    cout << console::end_success() << endl;
                }
                else if (GetRGClass() == RGC_MarkovRenewalProcess) {
                    cout << console::beg_title() << "\nCOMPUTING STEADY STATE SOLUTION ";
                    cout << "OF THE MARKOV RENEWAL PROCESS..." << console::end_title() << endl;
                    RequireOrBuildMRP();
                    sol = make_shared<PetriNetSolution>();
                    MRPStateKinds stKinds;
                    ClassifyMRPStates(*mrp, stKinds, verboseLvl);
                    if (spar.useSCCDecomp)
                        SteadyStateMrpBySccDecomp(*mrp, stKinds, *sol, spar,
                                                  verboseLvl);
                    else if (spar.solMethod == LSS_EXPLICIT)
                        SteadyStateMRPExplicit(*mrp, stKinds, *sol, spar, verboseLvl);
                    /*SolveSteadyStateMRP(*rg, *sol, detectIsomorphism,
                    					reduceInitialTransient,
                    					spar, verboseLvl);*/
                    else  // spar.solMethod == SM_IMPLICIT
                        SteadyStateMRPImplicit(*mrp, stKinds, *sol, spar, verboseLvl);

                    if (rg != nullptr) ComputeMeasures(*rg, *sol, verboseLvl);
                    else 		  ComputeMeasures(*rg2, *sol, verboseLvl);

                    cout << console::beg_success() << "\nSTEADY STATE SOLUTION COMPUTED.\n";
                    cout << console::end_success() << endl;
                }
                else throw program_exception("Steady State solution is not "
                                                 "supported for this RG class.");
                cout << "SOLUTION TIME: " << timer << endl << endl;
                if (print_stat_for_gui()) 
                    cout << "#{GUI}# RESULT STAT steady_state_solution_time " << timer << endl;
            }
            else if (cmdArg == "-t" && remainedArgs >= 1) {
                double t = atof(argv[argNum++]);
                performance_timer timer;
                RequireTRG_1or2();
                if (GetRGClass() == RGC_CTMC) {
                    cout << "\nCOMPUTING TRANSIENT SOLUTION OF THE CTMC ";
                    cout << "AT TIME INSTANT " << t << " ..." << endl;
                    RequireOrBuildCTMC();
                    sol = make_shared<PetriNetSolution>();
                    SolveTransientCTMC(*ctmc, *sol, t, KED_FORWARD, spar, verboseLvl);
                    if (rg != nullptr) ComputeMeasures(*rg, *sol, verboseLvl);
                    else 		  ComputeMeasures(*rg2, *sol, verboseLvl);
                    cout << console::beg_success() << "\nTRANSIENT SOLUTION COMPUTED.\n";
                    cout << console::end_success() << endl;
                }
                else throw program_exception("Transient solution is not "
                                                 "supported for this RG class.");
                cout << "SOLUTION TIME: " << timer << endl << endl;
                if (print_stat_for_gui()) 
                    cout << "#{GUI}# RESULT STAT transient_solution_time " << timer << endl;
            }
            else if (cmdArg == "-b" && remainedArgs >= 1) {
                const char *measureName = argv[argNum++];
                performance_timer timer;
                RequireTRG_1or2();
                RewardVectors rv;
                if (rg != nullptr) GetMeasureRewardVectors(*rg, measureName, rv);
                else 		  GetMeasureRewardVectors(*rg2, measureName, rv);
                double totImpulseRew = vec_sum(rv.impulseRewards);
                cout << console::beg_title() << "\nCOMPUTING BACKWARD STEADY STATE SOLUTION ";
                cout << "OF THE " << RGClass_Names[GetRGClass()] << "..." << console::end_title() << endl;
                PetriNetSolution backwardSol; // TODO: cannot be a local variable
                if (GetRGClass() == RGC_MarkovRenewalProcess) {
                    if (totImpulseRew != 0.0)
                        throw program_exception("Non-zero impulse rewards are not supported.");
                    RequireOrBuildMRP();
                    MRPStateKinds stKinds;
                    ClassifyMRPStates(*mrp, stKinds, verboseLvl);
                    if (spar.useSCCDecomp)
                        SteadyStateMrpBySccDecompBackward(*mrp, stKinds, rv.rateRewards,
                                                          backwardSol.stateProbs, spar,
                                                          verboseLvl);
                    else if (spar.solMethod == LSS_EXPLICIT)
                        SteadyStateMRPExplicitBackward(*mrp, stKinds, rv.rateRewards,
                                                       backwardSol.stateProbs, spar, verboseLvl);
                    else
                        SteadyStateMRPImplicitBackward(*mrp, stKinds, rv.rateRewards,
                                                       backwardSol.stateProbs, spar, verboseLvl);
                }
                else if (GetRGClass() == RGC_CTMC) {
                    RequireOrBuildCTMC();
                    SolveSteadyStateCTMC(*ctmc, backwardSol, KED_BACKWARD,
                                         &rv.rateRewards, spar, verboseLvl);
                }
                else throw program_exception("Backward solution is not "
                                                 "supported for this RG class.");
                cout << console::beg_success() << "\nBACKWARD STEADY STATE SOLUTION COMPUTED.\n";
                cout << console::end_success() << endl;
                cout << "SOLUTION TIME: " << timer << endl << endl;
                if (print_stat_for_gui()) 
                    cout << "#{GUI}# RESULT STAT backward_steady_state_solution_time " << timer << endl;
            }
            //----------------------------------------------------------------
            //   Internal commands
            //----------------------------------------------------------------
            else if (cmdArg == "-sleep" && remainedArgs >= 1) {
                int numSec = atoi(argv[argNum++]);
                cout << "SLEEPING FOR " << numSec << " SECONDS...\n" << endl;
                sleep(numSec);
            }
            else if (cmdArg == "-wr" || (cmdArg == "-wr-f" && remainedArgs >= 1)) {
                RequireSolution();
                string ResName;
                if (cmdArg == "-wr-f")
                    ResName = argv[argNum++];
                else ResName = *netName + ".results";

                ofstream res(ResName.c_str());
                SaveMeasureValuesOnFile(*rg, res, verboseLvl);
                res.close();
                cout << "COMPUTED MEASURE VALUES WRITTEN IN " << ResName << ".\n" << endl;
            }
            else if (cmdArg == "-wrsat" && remainedArgs >= 1) {
                RequireSAT();
                const char *satFName = argv[argNum++];
                ofstream ofs(satFName);
                sat->WriteToFile(ofs);
                ofs.close();
                cout << "MODEL CHECKING SOLUTION WRITTEN TO " << satFName << ".\n" << endl;
            }
            else if (cmdArg == "-wr-sol-f" && remainedArgs >= 1) {
                RequireSolution();
                const char *solfn = argv[argNum++];
                ofstream ofs(solfn);
                SaveStateProbsOnFile(*sol, ofs, verboseLvl);
                ofs.close();
                cout << "COMPUTED SOLUTION VALUES WRITTEN IN " << solfn << ".\n" << endl;
            }
            else if (cmdArg == "-bench-wr" && remainedArgs >= 1) { // Internal command
                string WrName(argv[argNum++]);
                ifstream wrf(WrName.c_str());
                cout << "\nCOMPARING COMPUTED PROBABILITIES..." << endl;
                CompareMeasuresAgainstFile(*sol, wrf, spar.epsilon);
                cout << "PROBABILITIES MATCH THOSE WRITTEN IN " << WrName << ".\n" << endl;
            }
            else if (cmdArg == "-bench-wrsat" && remainedArgs >= 1) { // Internal command
                RequireSAT();
                string WrName(argv[argNum++]);
                ifstream wrf(WrName.c_str());
                cout << "\nCOMPARING COMPUTED MODEL CHECKING PROBABILITIES..." << endl;
                SAT rdSat;
                if (!rdSat.ReadFromFile(wrf))
                    throw program_exception("Cannot read SAT file.");
                if (!CompareSATs(*sat, rdSat, spar.epsilon))
                    throw program_exception("Results do not match.");
                cout << "RESULTS MATCH THOSE WRITTEN IN " << WrName << ".\n" << endl;
            }
            // TODO: remove this if clause
            else if ((cmdArg == "-write-arff" || cmdArg == "-write-arff-nonerg") &&
                     remainedArgs >= 1) {
                const char *arffName = argv[argNum++];
                Require_VRGorTRG_1();
                bool onlyRecStates = (cmdArg == "-write-arff");
                if (onlyRecStates)
                    cout << "WRITING THE RECURRENT REACHABILITY GRAPH STATES ";
                else cout << "WRITING THE REACHABILITY GRAPH STATES ";
                cout << "INTO THE ARFF FILE \"" << arffName << "\"." << endl;
                WriteStatesInArffFormat(*pn, *rg, netName->c_str(),
                                        onlyRecStates, arffName);
            }
            else if (cmdArg == "-experiment") {
                Require_VRGorTRG_1();
                if (GetRGClass() != RGC_MarkovRenewalProcess)
                    throw program_exception("Only for MRP!!.");

                WriteEmcAndPreconditioner(*pn, *rg, netName->c_str(), spar);
            }
            else if (cmdArg == "-experiment1") {
                Experiment1();
            }
            else if (cmdArg == "-experiment2" && remainedArgs >= 1) {
                Experiment2(argv[argNum++]);
            }
            else if (cmdArg == "-experiment3") {
                RequireOrBuildMRP();
                Experiment3(*mrp, spar);
            }
            else if ((cmdArg == "-t-pade" || cmdArg == "-t-krylov") && remainedArgs >= 1) {
                double t = atof(argv[argNum++]);
                performance_timer timer;
                RequireTRG_1or2();
                if (GetRGClass() == RGC_CTMC) {
                    cout << "\nCOMPUTING TRANSIENT SOLUTION OF THE CTMC ";
                    cout << "AT TIME INSTANT " << t << " ..." << endl;
                    RequireOrBuildCTMC();
                    sol = make_shared<PetriNetSolution>();
                    NewSolveTransientCTMC(*ctmc, *sol, t, spar, verboseLvl);
                    if (rg != nullptr) ComputeMeasures(*rg, *sol, verboseLvl);
                    else 		  ComputeMeasures(*rg2, *sol, verboseLvl);
                    cout << console::beg_success() << "\nTRANSIENT SOLUTION COMPUTED.\n";
                    cout << console::end_success() << endl;
                }
                else throw program_exception("Transient solution is not "
                                                 "supported for this RG class.");
                cout << "SOLUTION TIME: " << timer << endl << endl;
            }
            else if (cmdArg == "-probe-automata" && remainedArgs >= 3) {
                const char *rsName = argv[argNum++];
                double time = atof(argv[argNum++]);
                size_t numSamples = atoi(argv[argNum++]);
                string lab_fn(rsName), wnrgRS_fn(rsName), wnrgRSxA_fn(rsName), weight_fn(rsName);
                lab_fn += ".outT";
                wnrgRS_fn += ".wngrR";
                wnrgRSxA_fn += ".wngr";
                weight_fn += ".WEIGHT";
                ifstream lab(lab_fn.c_str()), wnrgRS(wnrgRS_fn.c_str()), wnrgRSxA(wnrgRSxA_fn.c_str());
                if (!lab)
                    cout << "CANNOT READ LABEL FILE \"" << lab_fn << "\"." << endl;
                if (!wnrgRS)
                    cout << "CANNOT READ WNRG FILE \"" << wnrgRS_fn << "\"." << endl;
                if (!wnrgRSxA)
                    cout << "CANNOT READ WNRG FILE \"" << wnrgRSxA_fn << "\"." << endl;
                if (lab && wnrgRS && wnrgRSxA) {
                    ofstream weights(weight_fn.c_str(), ios::out | ios::binary);
                    cout << "\nCOMPUTING PASSAGE TIME DISTRIBUTION OF PROBE AUTOMATA.\n" << endl;
                    PassageTimeForProbeAutomata(lab, wnrgRS, wnrgRSxA, weights, spar,
                                                time, numSamples, verboseLvl);
                    cout << endl;
                }
            }
            /*else if (cmdArg == "-s-gmres-ilu0" || cmdArg == "-s-gmres") {
            	performance_timer timer;
            	bool useIlu0 = (cmdArg == "-s-gmres-ilu0");
            	if (!rgBuilt)
            		throw program_exception("Build the RG first.");
            	if (*rg.rgClass == RGC_CTMC) {
            		cout << console::beg_title() << "\nCOMPUTING STEADY STATE SOLUTION ";
            		cout << "OF THE CTMC..." << console::end_title() << endl;
            		NewSolveSteadyStateCTMC(*rg, *sol, useIlu0, spar, verboseLvl);
            		hasSolution = true;
            		cout << console::beg_success() <<"\nSTEADY STATE SOLUTION COMPUTED.\n";
            		cout << console::end_success() << endl;
            	}
            	else throw program_exception("Steady State solution is not "
            								 "supported for this RG class.");
            	cout << "SOLUTION TIME: " << timer << endl << endl;
            }*/
            /*else if (cmdArg == "-experiment2") { // TODO: remove this section
            	Experiment2();
            }*/
            else {
                cout << "\n" << console::beg_error() << "UNRECOGNIZED ARGUMENT: ";
                cout << console::end_error() << cmdArg << "\n" << endl;
                return -1;
            }

        }
        catch (std::exception &excp) {
            cout << console::beg_error() << "\nERROR: " << console::end_error();
            cout << excp.what() << endl << endl;
            return -1;
        }
    }

    return 0;
}

//=============================================================================


// TODO: remove this function
static
void PrintAsMathematicaMatrix(const ublas::compressed_matrix<double> &M,
                              const MRPStateKinds *pStKinds,
                              const char *name, ostream &out) {
    out << name << " = {";
    for (size_t j = 0, nj = 0; j < M.size2(); j++) {
        if (pStKinds != nullptr && 0 == (pStKinds->kinds[j] & MSK_REGENSTATE))
            continue;
        if (nj > 0)
            out << ",";
        out << "{";
        for (size_t i = 0, ni = 0; i < M.size1(); i++) {
            if (pStKinds != nullptr && 0 == (pStKinds->kinds[i] & MSK_REGENSTATE))
                continue;
            if (ni > 0)
                out << ",";
            if (M(i, j) == 0)
                out << "0";
            else
                out << M(i, j);
            ni++;
        }
        out << "}\n";
        nj++;
    }
    out << "};" << endl;
}

void BuildSimulatedEMC(const MRP &mrp, const SolverParams &spar,
                       const MRPStateKinds &stKinds,
                       ublas::compressed_matrix<double> &outM,
                       VerboseLevel verboseLvl);

void BuildEmbeddedMatrices(const MRP &mrp, const MRPStateKinds &stKinds,
                           const SolverParams &spar,
                           ublas::compressed_matrix<double> &outP,
                           ublas::compressed_matrix<double> &outC,
                           VerboseLevel verboseLvl);

void WriteEmcAndPreconditioner(const PN &pn, const RG &rg, const char *netName,
                               const SolverParams &spar) {
    VerboseLevel verboseLvl = VL_BASIC;
    MRP mrp;
    ExtractMRP(rg, mrp, verboseLvl);

    MRPStateKinds stKinds;
    ClassifyMRPStates(mrp, stKinds, verboseLvl);

    ublas::compressed_matrix<double> M, P, C;
    BuildSimulatedEMC(mrp, spar, stKinds, M, verboseLvl);
    BuildEmbeddedMatrices(mrp, stKinds, spar, P, C, verboseLvl);

    string NbName(netName);
    NbName += "-EMC-Prec.nb";
    ofstream of(NbName.c_str());
    of << fixed << setprecision(15);
    //PrintAsMathematicaMatrix(M, stKinds, "outM", of);
    cout << "MRP size is " << mrp.N() << endl;
    PrintAsMathematicaMatrix(mrp.Q, nullptr, "mQ", of);
    PrintAsMathematicaMatrix(mrp.Qbar, nullptr, "mQbar", of);
    PrintAsMathematicaMatrix(mrp.Delta, nullptr, "mDelta", of);
    PrintAsMathematicaMatrix(P, nullptr, "mP", of);
    PrintAsMathematicaMatrix(C, nullptr, "mC", of);

    cout << "FILE " << NbName << " WRITTEN.\n" << endl;
}



//=============================================================================

void WriteStatesInArffFormat(const PN &pn, const RG &rg, const char *netName,
                             bool onlyRecurrentStates, const char *arffName) {
    // Extract the SCC of the reachability graph
    BSCC bscc;
    switch (rg.rgClass) {
    case RGC_CTMC: {
        CTMC ctmc;
        ExtractCTMC(rg, ctmc, VL_NONE);
        BottomStronglyConnectedComponents(ctmc.Q, bscc);
    }
    break;

    case RGC_MarkovRenewalProcess: {
        MRP mrp;
        ExtractMRP(rg, mrp, VL_NONE);
        BottomStronglyConnectedComponents(mrp.Q + mrp.Qbar + mrp.Delta, bscc);
    }
    break;

    default:
        throw program_exception("Use only with CTMC and MRP.");
    }

    ofstream arff(arffName);

    // Write the ARFF header
    arff << "@RELATION " << netName << "\n\n";

    // Write places as attributes
    vector<Place>::const_iterator plcit;
    for (plcit = pn.plcs.begin(); plcit != pn.plcs.end(); ++plcit)
        arff << "@ATTRIBUTE " << plcit->name << "  INTEGER\n";
    arff << "@ATTRIBUTE recurrenceClass  INTEGER\n\n";

    // Write markings as states
    marking_t N(rg.markSet.size());
    for (marking_t m(0); m < N; ++m) {
        if (onlyRecurrentStates && bscc.indices[m] == TRANSIENT_COMPONENT)
            continue;
        const SparseMarking &sm = rg.markSet[m].marking;
        for (plcit = pn.plcs.begin(); plcit != pn.plcs.end(); ++plcit) {
            arff << sm[plcit->index] << ",";
        }
        arff << int(bscc.indices[m]) + 1 << "\n";
    }
    arff << endl;
    arff.close();
}


//=============================================================================



template<class MatrixIn, class MatrixOut>
void
PadeExp(const MatrixIn &A, MatrixOut &expA, const size_t p = 13) {
    // Find the appropriate scaling factor m
    typedef typename MatrixIn::value_type Real;
    Real normA = NormInf(A);
    size_t m = (size_t)std::max(int(0), int(floor(log(normA) / log(2.0))) + 1);

    // Compute coefficient array C[]
    vector<Real> C(p + 1);
    C[0] = 1;
    for (size_t j = 1; j <= p; j++)
        C[j] = C[j - 1] * (p + 1 - j) / (j * (2 * p + 1 - j));

    //identity_functor ident;
    //cout << "C = " << print_vec_functor<Real*, identity_functor>(C, C+p+1, "{, }", ident) << endl;

    // Initialize Horner matrices
    ublas::identity_matrix<Real> I(A.size1());
    ublas::matrix<Real> X1(pow(Real(2), -Real(m)) * A);
    ublas::matrix<Real> X2(prod(X1, X1));
    ublas::matrix<Real> T(C[p] * I);
    ublas::matrix<Real> S(C[p - 1] * I);

    // Application of Horner scheme
    bool odd = true;
    for (size_t j = p - 1; j >= 1; j--) {
        if (odd)
            T = prod(T, X2) + C[j - 1] * I;
        else
            S = prod(S, X2) + C[j - 1] * I;
        odd = !odd;
    }
    if (odd) {
        T = prod(T, X1);
        T -= S;
        expA = -(I + 2 * prod(Inverse(T), S));
    }
    else {
        S = prod(S, X1);
        T -= S;
        expA = I + 2 * prod(Inverse(T), S);
    }

    // Raise matrix to power 2^m by repeated squaring
    for (size_t j = 0; j < m; j++)
        expA = prod(expA, expA);
}


// Matrix Exponential operator implemented with a Krylov method.
// The implementation is based on the work in [2], with minor changes.
//
//  \param A         Input matrix.
//  \param pi0       Input vector, multiplied with exp(A*T).
//  \param piT       Output vector
//  \param T         Scalar multiplier of \a A in the exponential.
//  \param m         Size of the Krylov supspace.
//  \param H         Input Hessemberg matrix.
//  \param F         Input exponential matrix.
//  \param tol       Tolerance. On output contains the total error estimate.
//  \param hump      Estimate of the ill-conditioning of \a A.
//  \param norm_A    Infinity norm of \a A matrix.
//  \param printOut  Prints information on the screen.
//
// \note
// See also:
//  (1) "Analysis of some Krylov subspace Approximations to the matrix
//        exponential operator", Youcef Saad, SIAM J. Numer. Anal.,
//        vol 29:209-228, 1992
//  (2) "Expokit: a software package for computing matrix exponentials",
//        Roger B. Sidje, ACM Trans. Math. Softw., vol 24(1):130-156, 1998
//
template < class ResidualFunctor, class VectorIn, class VectorOut,
           class Real, class Matrix, class PrintOut >
void
KrylovExp(const ResidualFunctor &A, const VectorIn &pi0, VectorOut &piT,
          const Real T, const size_t m, Matrix &H, Matrix &F, Real &tol,
          Real &hump, const Real normA, PrintOut &printOut) {
    const size_t maxRejects(30), N(pi0.size());
    H.resize(m + 2, m + 2);
    F.resize(m + 2, m + 2);
    std::vector<VectorOut> V(m + 1);
    /*for (size_t i=0; i<m+1; i++) {
    	V[i].resize(N);
    	fill(V[i].begin(), V[i].end(), 0.0);
    }*/
    Real stepT, nextT, currT(0), xm(Real(1) / m), epsilon;
    Real norm_pi0, beta, normAV(0), totErr(0), fact, s;
    get_machine_epsilon(epsilon);
    bool breakdown = false;
    size_t stepCnt(0), mb(m), rejectCnt(0), AVproductCnt(0);
    const Real gamma(0.9), delta(1.2), breakTol(min((Real)1.0e-7, tol));
    VectorOut r;

    beta = norm_pi0 = norm_2(pi0);
    piT  = pi0;
    hump = norm_pi0;

    Real accel = 1;

    // First time step estimate
    fact  = pow((m + 1) / M_E, Real(m + 1)) * sqrt(2 * M_PI * (m + 1));
    nextT = (1 / normA) * pow((fact * tol) / (4 * beta * normA), xm);
    s     = pow(10, floor(log10(nextT / accel)) - 1);
    nextT = ceil(nextT / s) * s;

    // Time-stepping strategy cycle
    while (currT < T) {
        stepCnt++;
        // Choose the next time step
        stepT = std::min(nextT, T - currT);

#define PRINT_MSG() \
		printOut.onMessage(false, "KRYLOV-EXP(%d): %lf/%lf (%3.1lf%%), " \
						   "step=%d, #prod=%d, t=%e, rejects=%d.", \
						   m, currT, T, float(0.1*round(currT/T*1000)), \
						   stepCnt, AVproductCnt, stepT, rejectCnt)
#define LAST_MSG() \
		printOut.onMessage(true, "KRYLOV-EXP(%d): %lf (100%%), step=%d, " \
						   "#prod=%d, t=%e, rejects=%d, err=%e, hump=%lf.", \
						   m, T, stepCnt, AVproductCnt, stepT, rejectCnt, \
						   totErr, hump)

        //cout << "KrylovExp("<<stepCnt<<"): currT = " << currT << ",  stepT = " << stepT << endl;

        // ARNOLDI Method
        V[0] = piT * (1 / beta);
        for (size_t j = 0; j < m; j++) {
            r = A.product(V[j]);
            AVproductCnt++;
            PRINT_MSG();
            for (size_t i = 0; i <= j; i++) {
                H(i, j) = inner_prod(V[i], r);
                r -= H(i, j) * V[i];
            }
            Real resid = norm_2(r);
            if (resid < breakTol) {   // Happy breakdown
                cout << "  Happy breakdown!\n" << endl;
                breakdown = true;
                mb = j;
                stepT = (T - currT);
                break;
            }
            H(j + 1, j) = resid;
            V[j + 1] = r * (1 / resid);
        }
        if (!breakdown) {
            H(m + 1, m) = 1;
            normAV = norm_2(A.product(V[m]));
        }

        // Estimate the time step with a reasonable accuracy
        Real errLoc;
        size_t rejects = 0;
        while (true) {
            const size_t mx = mb + (breakdown ? 0 : 2);
            ublas::matrix_range<Matrix> F_mx(subrange(F, 0, mx, 0, mx));
            PRINT_MSG();
            PadeExp(subrange(stepT * H, 0, mx, 0, mx), F_mx);
            // FIXME: MatrixExp(F, H, mx, stepT);

            if (breakdown) {
                errLoc = tol;
                break;
            }
            else {
                // Local error estimate (Sidje method)
                Real err1 = std::abs(beta * F(m, 1));
                Real err2 = std::abs(beta * F(m + 1, 1) * normAV);
                if (err1 > err2 * 10) {  // Small time step, quick convergence
                    errLoc = err2;
                    xm = Real(1) / m;
                }
                else if (err1 > err2) {  // Slow convergence
                    errLoc = (err1 * err2) / (err1 + err2);
                    xm = Real(1) / m;
                }
                else {  // Asymptotic convergence
                    errLoc = err1;
                    xm = Real(1) / (m - 1);
                }
                /*cout << "  Guess " << rejects << ": errLoc="<<errLoc;
                cout << ", err1="<<err1<<", err2="<<err2<<", normAV="<<normAV;
                cout << ", delta*stepT*tol=" << delta * stepT * tol;
                cout << ", stepT="<<stepT;//*/
            }
            errLoc *= accel;
            if (errLoc <= delta * stepT * tol)
                break;

            rejectCnt++;
            if (++rejects >= maxRejects)
                throw program_exception("KrylovExp: requested tolerance is too high.");

            // Get the new time step estimate
            stepT = gamma * stepT * pow(stepT * tol / errLoc, xm);
            s     = pow(10, floor(log10(stepT)) - 1);
            stepT = ceil(stepT / s) * s;
            //cout << ",  stepT' = " << stepT << ", s="<<s<<endl<< endl;
            assert(stepT < T - currT && stepT > 0);
        }
        //cout << "  ok" << endl << endl;

        // Compute pi(stepT)
        const size_t mx = mb + (breakdown ? 0 : 1);
        // TODO: piT = V(:, 1:mx) * (beta * F(1:mx, 1))
        // Compute pi(currT+stepT)
        // piT = Transpose[V][[All, 1 ;; mx]].(beta * F[[1 ;; mx, 1]]);
        for (size_t i = 0; i < N; i++) {
            piT(i) = 0;
            for (size_t j = 0; j < mx; j++)
                piT(i) += beta * V[j](i) * F(j, 0);
        }
        beta = norm_2(piT);
        hump = std::max(hump, beta);

        // TODO: Normalization for Markov chain probabilities
        Real roundoff_err = 0;
        for (size_t i = 0; i < N; i++) {
            if (piT[i] < 0)
                piT[i] = 0;
        }
        Real norm_piT = vec_sum(piT);
        piT /= norm_piT;
        roundoff_err = std::abs(1 - norm_piT) / N;

        // Estimate the next time step
        nextT = gamma * stepT * pow(stepT * tol / errLoc, xm);
        s     = pow(10, floor(log10(nextT)) - 1);
        nextT = ceil(nextT / s) * s;
        assert(nextT > 0);
        //cout << "  nextT = " << nextT <<", s="<<s<< endl;

        currT  += stepT;
        errLoc  = std::max(errLoc, roundoff_err);
        errLoc  = std::max(errLoc, normA * epsilon);
        totErr += errLoc;
    }

    hump /= norm_pi0;
    tol = totErr;
    LAST_MSG();
}


void NewSolveTransientCTMC(const CTMC &ctmc, PetriNetSolution &sol, double t,
                           const SolverParams &spar, VerboseLevel verboseLvl) {
    LinearIterationPrinter printOut(verboseLvl);
    full_element_selector all_elems(ctmc.Q.size1());
    /*Uniformization2(Q, pi0, sol.stateProbs, accumProbs, t,
    				spar.epsilon, all_elems, all_elems, printOut);*/

    cout << "BUILD RESIDUAL FUNCTOR" << endl;
    xA_Residual< ublas::compressed_matrix<double> > A(ctmc.Q);
    ublas::matrix<double> H, F;
    cout << "COMPUTING NORM_INF(Q)..." << endl;
    double hump, normA = NormInf(ctmc.Q), tol = spar.epsilon;
    cout << "\nUSING KRYLOV-EXP...." << endl;
    KrylovExp(A, ctmc.pi0, sol.stateProbs, t, spar.KrylovM, H, F,
              tol, hump, normA, printOut);
    cout << "hump = " << hump << "  total error = " << tol << endl;

    /*ublas::matrix<double> expQt;
    PadeExp(Q * t, expQt);
    sol.stateProbs = prod(trans(expQt), pi0);*/
    if (verboseLvl >= VL_VERBOSE) {
        cout << "PROBABILITIES = " << sol.stateProbs << endl << endl;
    }

    //ublas::identity_matrix<double> I(Q.size1());
    //print_matrix(cout, prod(Q+I, Inverse(Q+I)), "P * P^-1");
}

//-----------------------------------------------------------------------------

/*void Experiment2()
{
	const size_t N = 9;
	const double data[N][N] = {
			{-60.4, 60, 0, 0.4, 0, 0, 0, 0, 0},
			{60, -90.4, 30, 0, 0.4, 0, 0, 0, 0},
			{0, 120, -120.4, 0, 0, 0.4, 0, 0, 0},
			{0.5, 0, 0, -60.7, 60, 0, 0.2, 0, 0},
			{0, 0.5, 0, 60, -90.7, 30, 0, 0.2, 0},
			{0, 0, 0.5, 0, 120, -120.7, 0, 0, 0.2},
			{0, 0, 0, 1, 0, 0, -61, 60, 0},
			{0, 0, 0, 0, 1, 0, 60, -91, 30},
			{0, 0, 0, 0, 0, 1, 0, 120, -121}
	};
	//ublas::matrix<double> Q(N,N);//, M(N,N);
	ublas::compressed_matrix<double> Q(N,N); //, M2(N,N), M3(N,N);
	for (size_t i=0; i<N; i++)
		for (size_t j=0; j<N; j++)
			Q(i,j) = data[i][j];
	double lambda = 121.5;
	//Q /= lambda;
	cout << endl;
	print_matrix(cout, Q, " Q");

	do {
		ublas::compressed_matrix<double> A(Q);
		cout << "size1: " << A.size1() << endl;
		cout << "size2: " << A.size2() << endl;
		cout << "index1_data: " << print_vec(A.index1_data()) << endl;
		cout << "index2_data: " << print_vec(A.index2_data()) << endl;
		cout << "value_data: " << print_vec(A.value_data()) << endl;
		cout << "index_base: " << A.index_base() << endl;
		cout << "filled1: " << A.filled1() << endl;
		cout << "filled2: " << A.filled2() << endl;
		cout << "\n\n";
	} while (0);

	PrintAsMathematicaMatrix(Q, "Q");
	const double vec_data[N] = { 0.0771828, 0.168329, 0.0619015, 0.161979,
			0.106717, 0.100662, 0.0203886, 0.152187, 0.150653 };
	ublas::vector<double> vy, vx(N);
	for (size_t i=0; i<N; i++)
		vx(i) = vec_data[i];
	full_element_selector all(N);

	cout << vx << endl;

	prod_vecmat_unif(vy, vx, Q, lambda, all, all);
	cout << vy << endl;

	prod_matvec_unif(vy, Q, vx, lambda, all, all);
	cout << vy << endl;
	*/

/*const size_t K = 1000000, FILL=300000, REP=20;
ublas::compressed_matrix<double> M(K,K);
cout << "CREO MATRICE...." << endl;
performance_timer timer1;
do {
	numerical::matrix_map<double> M_ins(K,K);
	for (size_t i=0; i<FILL; i++) {
		M_ins(rand()%K, rand()%K) = (rand()%10) + 1;
	}
	compress_matrix(M, M_ins);
} while (0);
cout << "TOTALE: " << timer1 << "\n\n" << flush;


cout << "ITERAZIONE CON ITERATORI 1 e 2...." << endl;
performance_timer timer2;
double v = 0;
size_t nrows = 0, nelems = 0;
for (size_t r=0; r<REP; r++) {
	ublas::compressed_matrix<double>::const_iterator1 it1, it1E;
	ublas::compressed_matrix<double>::const_iterator2 it2, it2E;
	for (it1 = M.begin1(), it1E = M.end1(); it1 != it1E; ++it1) {
		++nrows;
		for (it2 = it1.begin(), it2E = it1.end(); it2 != it2E; ++it2) {
			v += *it2;
			++nelems;
		}
	}
}
cout << "     " << v << "  #rows=" << nrows << "  #elems=" << nelems << endl;
cout << "TOTALE: " << timer2 << "\n\n" << flush;

cout << "ITERAZIONE CON ROW PROXY..." << endl;
performance_timer timer3;
v = 0; nrows = 0; nelems = 0;
for (size_t r=0; r<REP; r++) {
	typedef ublas::matrix_row< ublas::compressed_matrix<double> > row_proxy_t;
	for (size_t i=0; i<M.size1(); i++) {
		++nrows;
		row_proxy_t ith_row(M, i);
		row_proxy_t::const_iterator it(ith_row.begin()), itE(ith_row.end());
		for (; it != itE; ++it) {
			v += *it;
			++nelems;
		}
	}
}
cout << "     " << v << "  #rows=" << nrows << "  #elems=" << nelems << endl;
cout << "TOTALE: " << timer3 << "\n\n" << flush;*/

/*M = Q;
double det = ILU0(M);
cout << endl << "FAST ILU(0):" << endl;
print_matrix(cout, M, " M");
cout << "Det[M] = " << det << endl;

M = Q;
det = ILU0_slow(M);
cout << endl << "SLOW ILU(0):" << endl;
print_matrix(cout, M, " M");
cout << "Det[M] = " << det << endl;

Qs = Q;
ILUTK(Qs, M2, 1.0e-5, 4);
cout << endl << "ILUT:" << endl;
print_matrix(cout, M2, "M2");*/

/*M3 = Q;
gmm_ILUT_d(M3, 1.0e-5, 4);
cout << endl << "GMM ILUT:" << endl;
print_matrix(cout, M3, "M3");*/
//}

//-----------------------------------------------------------------------------

void BuildEmbeddedMatrices(const MRP &mrp, const MRPStateKinds &stKinds,
                           const SolverParams &spar,
                           ublas::compressed_matrix<double> &outP,
                           ublas::compressed_matrix<double> &outC,
                           VerboseLevel verboseLvl);

void Experiment3(const MRP &mrp, const SolverParams &spar) {
    /*typedef	EmcVectorMatrixProduct<full_element_selector,
    / ** /						   full_element_selector> EmcProd;
    const size_t N_MRP(mrp.N());
    full_element_selector allElem(N_MRP);

    MRPStateKinds stKinds;
    ClassifyMRPStates(mrp, stKinds, VL_NONE);
    ublas::compressed_matrix<double> mP, mC;
    BuildEmbeddedMatrices(mrp, stKinds, spar, mP, mC, VL_VERBOSE);

    EmcProd VxP(mrp, spar, mrp.Subsets, KED_FORWARD, allElem, allElem);
    EmcProd PxV(mrp, spar, mrp.Subsets, KED_BACKWARD, allElem, allElem);

    ublas::vector<double> vIn(N_MRP), vOut, vRho(N_MRP), vXi;
    for (size_t i=0; i<N_MRP; i++)
    	vIn(i) = double(i+1);
    vIn /= vec_sum(vIn);

    PrintAsMathematicaMatrix(mP, "mP");
    cout << "vIn = " << vIn << endl << endl;

    prod_vecmat(vOut, vIn, mP, allElem, allElem);
    cout << "v*P: " << vOut << endl;

    vOut = VxP.product(vIn);
    cout << "VxP: " << vOut << endl << endl;

    prod_matvec(vOut, mP, vIn, allElem, allElem);
    cout << "P*V: " << vOut << endl;

    vOut = PxV.product(vIn);
    cout << "PxV: " << vOut << endl;

    set_vec(vRho, 0.0, allElem);
    vRho(N_MRP-1) = 1.0;
    vXi = vRho;
    for (size_t i=0; i<100; i++) {
    	vXi = PxV.product(vXi);
    }
    cout << "\nXI: " << vXi << endl;*/
}

//-----------------------------------------------------------------------------

void LoadWnrgCtmc(ifstream &wnrg_ifs, const size_t N,
                  ublas::compressed_matrix<double> &ctmc) {
    simple_tokenizer wnrg(&wnrg_ifs);
    wnrg.readtok("1");
    numerical::matrix_map<double> Mat(N, N);
    // read the N WNRG rows
    for (size_t i = 0; i < N; i++) {
        size_t rowElems, j;
        double value, sum = 0.0;
        wnrg.read(rowElems);
        // read every nonzero entry in row i
        while (rowElems-- > 0) {
            wnrg.read(j).read(value);
            if (j >= N) {
                cout << "N=" << N << " i=" << i << " j=" << j << endl;
            }
            verify(j < N);
            sum += value;
            Mat(i, j) = value;
        }
        // sum up the row entries in the diagonal
        Mat(i, i) = -sum;
    }
    compress_matrix(ctmc, Mat);

    size_t n;
    wnrg.read(n);
    verify(n == N);
}

//-----------------------------------------------------------------------------

void ReadStateLabels(ifstream &lab_ifs, size_t &N_RS, size_t &N_RSxA,
                     ublas::vector<double> &SE,
                     ublas::vector<double> &SF) {
    simple_tokenizer lab(&lab_ifs);

    // Read RS and RSxA sizes
    lab.readtok("RS size:").read(N_RS).readtok("RS X A size:").read(N_RSxA);
    SE.resize(N_RSxA, false);
    SF.resize(N_RSxA, false);
    std::fill(SE.begin(), SE.end(), 0.0);
    std::fill(SF.begin(), SF.end(), 0.0);

    // Read state labels
    string line;
    lab.getline(line); // read empty line
    for (size_t i = 0; i < N_RSxA; i++) {
        lab.getline(line);
        istringstream iss(line);
        size_t i2;
        char l, colon;
        do {
            iss >> l >> colon >> i2;
            if (!iss)
                break;
            switch (l) {
            case 'S':	SE[i2] = 1.0;   break;	// Start state
            case 'E': 	SF[i2] = 1.0;   break;	// End state
            case 'F':   break;  // Forbidden state, ignore
            default:
                throw program_exception("Unknown label.");
            }
        }
        while (false);
        //iss >> readtok(":") >> i2;
        //verify(i == i2);
    }
}

//-----------------------------------------------------------------------------

void WriteHydraVector(ofstream &hy, const ublas::vector<double> &vec) {
    long sz = (long)vec.size();

    // Write vector length as a long
    hy.write((const char *)(&sz), sizeof(long));

    // Write vector entries as doubles
    for (size_t i = 0; i < vec.size(); i++) {
        double v = vec[i];
        hy.write((const char *)(&v), sizeof(double));
    }
    hy.flush();
}

//-----------------------------------------------------------------------------

template<class RowSelector, class ColSelector>
void AbsorptionCTMC(const CTMC &ctmc, const ublas::vector<double> &reward,
                    ublas::vector<double> &absorptProb, const SolverParams &spar,
                    const RowSelector &rows, const ColSelector &cols,
                    VerboseLevel verboseLvl) {
    const size_t N = ctmc.Q.size1();
    assert(ctmc.Q.size2() == N && reward.size() == N);
    absorptProb = reward;

    if (verboseLvl >= VL_BASIC)
        cout << "CTMC STEADY STATE ABSORPTION PROBABILITIES." << endl;

    LinearIterationPrinter printOut(verboseLvl);
    //full_element_selector entire_matrix(N);
    ublas::zero_vector<double> zeroDiag(N);
    if (!LinearSolve(ctmc.Q, absorptProb, reward, zeroDiag,
                     spar, printOut, rows, cols, &reward))
        throw program_exception("Solution method does not converge.");
}

//-----------------------------------------------------------------------------

struct is_zero : public unary_function<double, bool> {
    inline bool operator()(const double val) const  {  return (val == 0);  }
};
struct is_nonzero : public unary_function<double, bool> {
    inline bool operator()(const double val) const  {  return (val != 0);  }
};

//-----------------------------------------------------------------------------

void PassageTimeForProbeAutomata(ifstream &lab, ifstream &ifrs,
                                 ifstream &ifrsXa, ofstream &ofweights,
                                 const SolverParams &spar, double time,
                                 size_t numSamples, VerboseLevel verboseLvl) {
    CTMC ctmcRS, ctmcRSxA;
    size_t N_RS, N_RSxA;
    ublas::vector<double> SE, SF;

    // Read RS and RSxA sizes, and the state labels SE and SF
    ReadStateLabels(lab, N_RS, N_RSxA, SE, SF);
    lab.close();

    // Read RS and RSxA CTMC matrices (written concatenated in *wnrg)
    LoadWnrgCtmc(ifrs, N_RS, ctmcRS.Q);
    LoadWnrgCtmc(ifrsXa, N_RSxA, ctmcRSxA.Q);
    ifrs.close();
    ifrsXa.close();

    // Prepare the initial distribution vector
    ctmcRS.pi0.resize(N_RS);
    ctmcRS.pi0[0] = 1.0;

    if (verboseLvl >= VL_VERBOSE) {
        cout << "STATE LABEL VECTORS:" << endl;
        cout << "  START = " << print_vec(SE) << endl;
        cout << "  FINAL = " << print_vec(SF) << endl;
        if (ctmcRS.Q.size1() < 50) {
            cout << "\nRS MATRIX:" << endl;
            print_matrix(cout, ctmcRS.Q, "    RS");
            cout << "\n\nRSxA MATRIX:" << endl;
            print_matrix(cout, ctmcRSxA.Q, "  RSxA");
            cout << endl << endl;
        }
    }

    // 1) Compute the steady state solution of the first ergodic CTMC
    PetriNetSolution solRS;
    SolveSteadyStateCTMC(ctmcRS, solRS, KED_FORWARD, nullptr, spar, verboseLvl);

    // Prepare the initial distribution vector of RSxA
    ctmcRSxA.pi0.resize(N_RSxA);
    std::fill(std::copy(solRS.stateProbs.begin(), solRS.stateProbs.end(),
                        ctmcRSxA.pi0.begin()), ctmcRSxA.pi0.end(), 0.0);


    // 2) Compute the absorption probability from pi0 to the enter frontier SE
    //    States labels as SE and SF are already absorbing states
    ublas::vector<double> SNotE(N_RSxA);
    std::fill(SNotE.begin(), SNotE.end(), 1.0);
    SNotE -= SE;
    subset_selector<ublas::vector<double>, is_nonzero> sel_NotE(SNotE);
    full_element_selector entire_RGxA(SE.size());
    cout << "COMPUTING ABSORPTION PROBABILITY..." << endl;
    CTMC ctmcRSxA_F;
    filter_matrix(ctmcRSxA.Q, ctmcRSxA_F.Q, sel_NotE, entire_RGxA);
    if (verboseLvl >= VL_VERBOSE) {
        if (ctmcRSxA_F.Q.size1() < 50) {
            cout << "STEADY = " << print_vec(ctmcRSxA.pi0) << endl;
            cout << "\nRSxA FILTERED MATRIX:" << endl;
            print_matrix(cout, ctmcRSxA_F.Q, "  RSxA.F");
            cout << endl << endl;
            PrintAsMathematicaMatrix(ctmcRSxA_F.Q, "RSxAF");
        }
    }

    PetriNetSolution initPassage;
    ctmcRSxA_F.pi0 = ctmcRSxA.pi0;
    SolveSteadyStateCTMC(ctmcRSxA_F, initPassage, KED_FORWARD, nullptr, spar, verboseLvl);

    // Write the entering distribution in hydra format on the disk
    WriteHydraVector(ofweights, initPassage.stateProbs);
    if (verboseLvl >= VL_BASIC) {
        cout << "HYDRA INITIAL PROBABILITY VECTOR WRITTEN." << endl;
    }
    if (numSamples == 0)
        return;


    /*long_interval_timer timer;
    LinearIterationPrinter printOut(verboseLvl, spar, LSM_EXPLICIT, &timer);
    //full_element_selector entire_matrix(N);
    ublas::zero_vector<double> zeroDiag(N_RSxA);
    const ublas::vector<double>* init_x0 = &ctmcRSxA.pi0;
    if (!LinearSolveTransposed(absorptProbSE, ctmcRSxA.Q, zeroDiag, zeroDiag,
    						   spar, printOut, sel_NotE, entire_RGxA, init_x0))
    	throw program_exception("Solution method does not converge.");*/
    /*AbsorptionCTMC(ctmcRSxA, SE, absorptProbSE, spar,
    			   entire_RGxA, entire_RGxA, verboseLvl);*/
    //cout << "SUM(pt) = " << norm_1(absorptProbSE) << endl;
    //}
    //else absorptProbSE = SE;
    /*{
    	ublas::vector<double> SE_RS(SE);
    	SE_RS.resize(ctmcRS.Q.size1());
    	absorptProbSE = SE_RS;
    	CTMC ctmcRS_T;
    	FastTranspose(ctmcRS.Q, ctmcRS_T.Q);
    	full_element_selector entire_RG(SE_RS.size());
    	AbsorptionCTMC(ctmcRS_T, SE_RS, absorptProbSE, spar, entire_RG, entire_RG, verboseLvl);
    }*/
    if (verboseLvl >= VL_VERBOSE) {
        cout << "ABSORPTION(1) = " << print_vec(initPassage.stateProbs) << endl;
        //PrintAsMathematicaMatrix(ctmcRSxA.Q, "RSxA");
    }


    // 3) Compute the passage time distribution from the entering states to
    //    the absorption states.
    ublas::vector<double> absorptProbFinal, startProb, *no_vec = nullptr;
    ublas::vector<double> passageProbs(numSamples);
    CTMC ctmcRSxA_T;
    FastTranspose(ctmcRSxA.Q, ctmcRSxA_T.Q);
    if (verboseLvl >= VL_VERBOSE && ctmcRSxA_T.Q.size1() < 50) {
        cout << "\nRSxA TRANSPOSED MATRIX:" << endl;
        print_matrix(cout, ctmcRSxA_T.Q, "  RSxA^T");
        cout << endl << endl;
        cout << "    SF = " << print_vec(SF) << endl;
        ublas::vector<double> SFxA;
        prod_vecmat_unif(SFxA, SF, ctmcRSxA_T.Q, 30, entire_RGxA, entire_RGxA);
        cout << "  SF.A = " << print_vec(SFxA) << endl;
        cout << endl;
    }

    subset_selector<ublas::vector<double>, is_zero> sel_F(SF);
    startProb = SF;
    cout << "COMPUTING TRANSIENT PASSAGE TIMES..." << endl;
    for (size_t s = 0; s < numSamples; s++) {
        LinearIterationPrinter printOut(VL_NONE);
        UniformizationDet(ctmcRSxA_T.Q, startProb, absorptProbFinal, no_vec, time,
                          spar.epsilon, KED_FORWARD, entire_RGxA, entire_RGxA, printOut);
        if (verboseLvl >= VL_VERBOSE) {
            cout << "ABSORPTION(2) = " << print_vec(absorptProbFinal) << endl;
        }
        startProb = absorptProbFinal;

        // 4) Passage time distribution is conditioned to the initial distribution
        passageProbs[s] = ublas::inner_prod(absorptProbFinal, initPassage.stateProbs);
    }

    if (verboseLvl >= VL_BASIC) {
        cout << "\nPASSAGE PROBABILITY DISTRIBUTION:" << endl;
        for (size_t s = 0; s < numSamples; s++) {
            cout << "   TIME " << fmtdbl(time * (s + 1), "%1.4lf") << " = " << passageProbs[s] << endl;
        }
    }
}

//-----------------------------------------------------------------------------






//-----------------------------------------------------------------------------

void Experiment1() 
{
    // sparsevector<size_t, int> s1(10), s2(10);
    // s1.insert_element(0, 1);
    // s2.insert_element(3, 1);
    // // s1.insert_element(5, 1);
    // // s2.insert_element(0, 2);
    // // s2.insert_element(4, 2);
    // // s2.insert_element(5, 2);
    // // s1.insert_element(6, 1);
    // int j, v1, v2;
    // auto i1 = s1.begin(), i2 = s2.begin();
    // while (-1 != (j = traverse_both(i1, s1, v1, i2, s2, v2))) {
    //     cout << j << " " << v1 << " " << v2 << endl;
    // }


    // size_t NP = 14, MT = 10;
    // flows_generator_t msa(NP, MT, VL_VERY_VERBOSE);
    // vector<vector<size_t>> Table = {
    //    {5, 4}, {1, 3}, // t1
    //    {14, 7}, {6, 8}, // t2
    //    {9}, {10, 11}, // t3
    //    {2, 13}, {4}, // t4
    //    {1}, {2}, // t5
    //    {3}, {14}, // t6
    //    {6}, {5}, // t7
    //    {8}, {9}, // t8
    //    {10, 12}, {7}, // t9
    //    {11}, {12, 13} // t10
    // };
    // for (size_t t = 0; t < MT; t++) {
    //     const vector<size_t>& in = Table[t*2], &out = Table[t*2+1];
    //    for (size_t p = 0; p < in.size(); p++) {
    //        msa.addFlow(in[p] - 1, t, -1);
    //    }
    //    for (size_t p = 0; p < out.size(); p++) {
    //        msa.addFlow(out[p] - 1, t, +1);
    //    }
    // }


    // size_t MT = 16, NP= 12;
    // flow_matrix_t psfm(NP, NP, MT, InvariantKind::PLACE, SystemMatrixType::REGULAR, 0, false, true, true);
    // incidence_matrix_generator_t inc_gen(psfm);
    // inc_gen.add_flow_entry(0, 0, 1);
    // inc_gen.add_flow_entry(0, 2, 1);
    // inc_gen.add_flow_entry(0, 7, -1);
    // inc_gen.add_flow_entry(0, 8, 1);
    // inc_gen.add_flow_entry(1, 0, 1);
    // inc_gen.add_flow_entry(1, 1, 1);
    // inc_gen.add_flow_entry(1, 4, -1);
    // inc_gen.add_flow_entry(1, 5, 1);
    // inc_gen.add_flow_entry(10, 13, -1);
    // inc_gen.add_flow_entry(10, 14, 1);
    // inc_gen.add_flow_entry(10, 3, -1);
    // inc_gen.add_flow_entry(11, 12, 1);
    // inc_gen.add_flow_entry(11, 15, -1);
    // inc_gen.add_flow_entry(11, 3, -1);
    // inc_gen.add_flow_entry(2, 1, 1);
    // inc_gen.add_flow_entry(2, 11, 1);
    // inc_gen.add_flow_entry(2, 12, -1);
    // inc_gen.add_flow_entry(2, 3, 1);
    // inc_gen.add_flow_entry(3, 10, -1);
    // inc_gen.add_flow_entry(3, 13, 1);
    // inc_gen.add_flow_entry(3, 2, 1);
    // inc_gen.add_flow_entry(3, 3, 1);
    // inc_gen.add_flow_entry(4, 1, -1);
    // inc_gen.add_flow_entry(4, 4, 1);
    // inc_gen.add_flow_entry(4, 6, -1);
    // inc_gen.add_flow_entry(5, 10, 1);
    // inc_gen.add_flow_entry(5, 14, -1);
    // inc_gen.add_flow_entry(5, 2, -1);
    // inc_gen.add_flow_entry(6, 2, -1);
    // inc_gen.add_flow_entry(6, 8, -1);
    // inc_gen.add_flow_entry(6, 9, 1);
    // inc_gen.add_flow_entry(7, 1, -1);
    // inc_gen.add_flow_entry(7, 11, -1);
    // inc_gen.add_flow_entry(7, 15, 1);
    // inc_gen.add_flow_entry(8, 0, -1);
    // inc_gen.add_flow_entry(8, 5, -1);
    // inc_gen.add_flow_entry(8, 6, 1);
    // inc_gen.add_flow_entry(9, 0, -1);
    // inc_gen.add_flow_entry(9, 7, 1);
    // inc_gen.add_flow_entry(9, 9, -1);
    // inc_gen.generate_matrix();

    // class Printer : public flow_algorithm_printer_t {
    // public:
    //     virtual void advance(const char*, size_t step, size_t totalSteps, size_t, ssize_t) {
    //         cout << "Step " << (step + 1) << "/" << totalSteps << endl;
    //     }

    // } printer;
    // flows_generator_t sf_gen(psfm, printer, VL_VERBOSE);
    // sf_gen.compute_semiflows();

    // cout << "# semiflows : " << psfm.num_flows() << endl;
    // for (auto sf = psfm.begin(); sf != psfm.end(); ++sf) {
    //     for (size_t n=0; n<sf->nonzeros(); n++)
    //         if (sf->ith_nonzero(n).value != 0)
    //             cout << sf->ith_nonzero(n).value << "*P" << (sf->ith_nonzero(n).index+1) << " ";
    //     cout << endl;
    // }
}

//-----------------------------------------------------------------------------























