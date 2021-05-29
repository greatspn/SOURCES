#include "LTL.h"
#include "CTL.h"
#include <typeinfo>


#include <fstream>
// LIBSPOT (formula -> HOA)
#include <spot/tl/parse.hh>
#include <spot/twaalgos/hoa.hh>
#include <spot/twaalgos/translate.hh>
#include <spot/twaalgos/dot.hh>

// CPPHOAFPARSER https://automata.tools/hoa/cpphoafparser/docs/index.html
#include "./ltl_include/cpphoafparser/consumer/hoa_intermediate.hh"
#include "./ltl_include/cpphoafparser/consumer/hoa_consumer_null.hh"
#include "./ltl_include/cpphoafparser/consumer/hoa_consumer_print.hh"
#include "./ltl_include/cpphoafparser/parser/hoa_parser.hh"


using namespace MEDDLY;


// convert a SPOT formula into a ctlmddref_ptr<::Formula> object
ref_ptr<ctlmdd::Formula> 
parse_spot_formula(const std::string& formula, 
                   const std::vector<ref_ptr<ctlmdd::Formula>>& atomic_propositions,
                   const std::vector<size_t>& spot_ap_to_greatspn_ap_index);

//-----------------------------------------------------------------------------


namespace ctlmdd {

//-----------------------------------------------------------------------------

const char* g_str_BAType[4] = { "STRONG_BA", "WEAK_BA", "VERY_WEAK_BA", "TERMINAL_BA" };

//-----------------------------------------------------------------------------

ba_edge_t::ba_edge_t(ba_location_t _src_loc, ba_location_t _dst_loc, 
                     ref_ptr<Formula> _state_formula, bool _is_initial) 
{
    src_loc       = _src_loc;
    dst_loc       = _dst_loc;
    state_formula = _state_formula;
    is_initial    = _is_initial;
}

// ba_edge_t::~ba_edge_t() {
//     // cout << "deleting ba_edge_t..." << state_formula << endl;
//     // safe_removeOwner(state_formula);
// }

ostream& operator<<(ostream& os, const ba_edge_t& e) {
    os << "  q"<<e.src_loc << " -> q" << e.dst_loc 
       << (e.is_initial ? " init" : "     ")
       << " label: ";
    if (e.state_formula) 
        os << *e.state_formula;
    else
        os << "<nullptr>";
    return os;
}

//-----------------------------------------------------------------------------

BuchiAutomaton::BuchiAutomaton() { }

BuchiAutomaton::~BuchiAutomaton() { }

void BuchiAutomaton::pre_compute_subformula_MDDs(Context& ctx) {
    for (auto&& edge : edges)
        edge.state_formula->getMDD(ctx);
}

ostream& operator<<(ostream& os, const BuchiAutomaton& ba) {
    os << "|Q| = " << ba.num_locs << endl;
    os << "Q0 = { ";
    for (size_t i=0; i<ba.init_locs.size(); i++)
        os << "q" << ba.init_locs[i] << (i==ba.init_locs.size()-1 ? " }": ", ");
    os << endl;

    os << "S = { ";
    for (size_t i=0; i<ba.accept_loc_sets.size(); i++) {
        os << "{ ";
        for (size_t i=0; i<ba.init_locs.size(); i++)
            os << "q" << ba.init_locs[i] << (i==ba.init_locs.size()-1 ? " }": ", ");
        os << (i==ba.accept_loc_sets.size()-1 ? " }": ", ");
    }
    os << endl;

    for (const ba_edge_t& e : ba.edges)
        os << e << endl;
    os << g_str_BAType[ba.type] << endl;
    return os;
}

//-----------------------------------------------------------------------------

void BuchiAutomaton::reindex_locations(const int *reindex_tbl) {
    // reindex initial locations
    for (size_t i=0; i<init_locs.size(); i++)
        init_locs[i] = reindex_tbl[init_locs[i]];
    // reindex the set of sets of accepting locations
    for (auto& as : accept_loc_sets)
        for (auto& q : as)
            q = reindex_tbl[q];
    // reindex edges
    for (ba_edge_t& e : edges) {
        e.src_loc = reindex_tbl[e.src_loc];
        e.dst_loc = reindex_tbl[e.dst_loc];
    }
}

//-----------------------------------------------------------------------------

/**
 * Load an automa from a stream
 * --------------------------------------------
 * The loader maps the transition relations
 * of the parsed automa in HOA format
 * into various sets of edges, accepting states
 * and keeps track of initial state and
 * acceptance expression.
 * --------------------------------------------
*/
class hoa_to_buchi_loader : public cpphoafparser::HOAIntermediate {
public:
    // Atomic propositions of the LTL subformulas
    const std::vector<ref_ptr<Formula>>& atomic_propositions;
    // Output Buchi Automata
    BuchiAutomaton *p_ba;
    // SPOT AP indices -> atomic_propositions[] indices
    std::vector<size_t> spot_ap_to_greatspn_ap_index;

    hoa_to_buchi_loader(HOAConsumer::ptr next, 
                        const std::vector<ref_ptr<Formula>>& AP, 
                        BuchiAutomaton *p) 
    : cpphoafparser::HOAIntermediate(next), atomic_propositions(AP), p_ba(p) {}

    virtual ~hoa_to_buchi_loader() { }

    /// called as the start states are set
    virtual void 
    addStartStates(const int_list& states) override {
        // cout << "addStartStates " << states.size() << endl;
        p_ba->init_locs = states;
    }

    // properties: header
    virtual void 
    addProperties(const std::vector<std::string>& properties) override
    {
        for(const auto& prop: properties) {
            // cout << "addProperties " << prop << endl;
            if(prop == "weak" && p_ba->type != TERMINAL_BA && p_ba->type != VERY_WEAK_BA) {
                p_ba->type = WEAK_BA; // terminal implies weak
            }
            if(prop == "very-weak" && p_ba->type != TERMINAL_BA) {
                p_ba->type = VERY_WEAK_BA;
            }
            if(prop == "terminal") {
                p_ba->type = TERMINAL_BA;
                break;
            }
        }
    }

    /// called only once when AC is set
    virtual void 
    setAcceptanceCondition(unsigned int nsets, // number of accepting sets
                           acceptance_expr::ptr accExpr) override 
    {
        // cout << "setAcceptanceCondition " << nsets << " " << accExpr << endl;
        p_ba->accept_loc_sets.resize(nsets);
        next->setAcceptanceCondition(nsets, accExpr);
    }

    // order of insertion of aps = index of aps
    virtual void 
    setAPs(const std::vector<std::string>& aps) override {
        for(std::string ap: aps) {
            size_t index;
            if (1 != sscanf(ap.c_str(), "a%zu", &index))
                throw "Unexpected AP label.";

            // cout << "setAP " << ap << " " << index << endl;
            spot_ap_to_greatspn_ap_index.push_back(index);
        }
    }

    virtual void 
    setNumberOfStates(unsigned int numberOfStates) override{
        p_ba->num_locs = numberOfStates;
    }

    /// called whenever a state gets added
    virtual void 
    addState(ba_location_t id,  // state identifier
             std::shared_ptr<std::string> info, // info (empty pointer if none)
             label_expr::ptr labelExpr, // boolean expression over labels
             std::shared_ptr<int_list> accSignature // list of accept. set indices (SGBA)
             ) override 
    {
        // add a state to the list of accepting states
        // if it represents one (or more) accepting sets
        if(accSignature != nullptr) {
            for(unsigned int set: *accSignature.get()) {
                p_ba->accept_loc_sets[set].push_back(id);
            }
        }

        next->addState(id, info, labelExpr, accSignature);
    }

    /// called whenever an explicit edge definition is read
    virtual void 
    addEdgeWithLabel(ba_location_t stateId,                  // state identifier
                     label_expr::ptr labelExpr,              // boolean expression over labels
                     const int_list& conjSuccessors,         // list of successor states (conjunction)
                     std::shared_ptr<int_list> accSignature  // list of acceptance set indices (TGBA)
                     ) override 
    {
        bool initial = std::find(p_ba->init_locs.begin(),
                                 p_ba->init_locs.end(),
                                 stateId) != p_ba->init_locs.end();

        for(ba_location_t s: conjSuccessors) {
            // parse the edge label, extract the boolean state formula
            ref_ptr<Formula> stateFormula = parse_spot_formula(labelExpr->toString(), 
                                                       atomic_propositions,
                                                       spot_ap_to_greatspn_ap_index);

            p_ba->edges.emplace_back(std::move(ba_edge_t(stateId, s, ref_ptr<Formula>(nullptr), initial)));
            p_ba->edges.back().state_formula = stateFormula;
        }

        next->addEdgeWithLabel(stateId, labelExpr, conjSuccessors, accSignature);
    }
};

//-----------------------------------------------------------------------------

/**
 * Convert LTL formula to TGBA using SPOT
 * ---------------------------------------------------------
 * Dump the HOA representation of the automa
 * to a stream object which can be used by the parser.
 * The output stream can be any std::ostream implementation,
 * notably `stringstream` (used here) or any file stream
 * ---------------------------------------------------------
 * Returns: vector of APs, useful to map NUMap -> IDap
 * ---------------------------------------------------------
*/
void translate_SPOT_formula(std::stringstream& out, const char* formula)
{
    // parse formula
    spot::formula pf = spot::parse_formula(formula);

    // use a translator to perform conversion formula -> tgba
    spot::translator trans;
    trans.set_type(spot::postprocessor::BA);
    // trans.set_type(spot::postprocessor::TGBA);
    // trans.set_pref(spot::postprocessor::SBAcc);
    // trans.set_level(spot::postprocessor::optimization_level::High);

    // trans.set_pref(spot::postprocessor::Unambiguous);
    spot::twa_ptr aut = trans.run(pf);

    // print automaton in HOA format to output stream
    // opts = 's' means state-based acceptance conditions
    // opts = 'v' means verbose properties
    const char opts[3] = {'s','v','\0'};
    spot::print_hoa(out, aut, opts);

#ifdef DEBUG_BUILD
    spot::print_hoa(std::cout, aut, opts);
    std::cout << "\n=== Writing automa .svg to: automa.dot ===" << std::endl;
    std::ofstream dotFile("automa.dot");
    spot::print_dot(dotFile, aut, opts);
    system("dot -Tsvg automa.dot -o automa.svg");
#endif
}

//-----------------------------------------------------------------------------

BuchiAutomaton
spot_LTL_to_Buchi_automaton(const char* ltl_formula,
                            const std::vector<ref_ptr<Formula>>& atomic_propositions) 
{
    // Get the Hanoi-Omega Automata format (HOA) using SPOT
    std::stringstream hoa;
    translate_SPOT_formula(hoa, ltl_formula);

    // HOA parser
    std::shared_ptr<cpphoafparser::HOAConsumer> hoaNull;
    hoaNull = make_shared<cpphoafparser::HOAConsumerNull>();

    // HOA -> Buchi Automata conversion
    BuchiAutomaton ba;
    shared_ptr<hoa_to_buchi_loader> loader;
    loader = make_shared<hoa_to_buchi_loader>(hoaNull, atomic_propositions, &ba);
    cpphoafparser::HOAParser::parse(hoa, loader);

    // cout << endl;
    // cout << hoa.str() << endl;
    // cout << ba << endl;

    return ba;
}

//-----------------------------------------------------------------------------
/**
 * Add a [loc:i -> loc':j] relation to the location levels of the NSF MxD
 * ----------------------------------------------------------------------
 * - NSF: the MxA next state function
 * - i: the starting location of the relation (edge)
 * - j: the ending location of the relation (edge)
 * ----------------------------------------------------------------------
*/
dd_edge mxd_location_change(dd_edge& NSF, const size_t i, const size_t j)
{
    expert_forest* forestMxD = static_cast<expert_forest*>(NSF.getForest());
    const int locLvl = forestMxD->getDomain()->getNumVariables();
    const int locLvlBnd = forestMxD->getDomain()->getVariableBound(locLvl);


    // The NSF should not have nodes in the location level (by construction)
    m_assert(std::abs(NSF.getLevel()) < locLvl, "Extra levels are not reduced.");
    m_assert(g_rsrg->getExtraLvls() == 1, "# of extra levels in the RS should be 1");
    CTL_ASSERT(g_rsrg->isIndexOfExtraLvl(locLvl-1));

    // Create a node nh1=[j:NSF] in the primed loc level
    unpacked_node* un;
    un = unpacked_node::newFull(forestMxD, -locLvl, locLvlBnd);
    // un->set_d(j, NSF);
    un->d_ref(j) = forestMxD->linkNode(NSF.getNode());
    node_handle nh1 = forestMxD->createReducedNode(-1, un);
    dd_edge j_to_nsf(forestMxD);
    j_to_nsf.set(nh1);

    // Create a node nh2=[i:nh1] in the loc level
    un = unpacked_node::newFull(forestMxD, +locLvl, locLvlBnd);
    // un->set_d(i, j_to_nsf);
    un->d_ref(i) = forestMxD->linkNode(j_to_nsf.getNode());
    node_handle nh2 = forestMxD->createReducedNode(-1, un);
    dd_edge ij_to_nsf(forestMxD);
    ij_to_nsf.set(nh2);

    return ij_to_nsf;
}

// //-----------------------------------------------------------------------------

/**
 * Create a self loop between a set of markings and itself 
 * ----------------------------------------------------------------------
 * Generate two equal vectors and build a MxD having equal unprimed and
 * primed variables.
 * ----------------------------------------------------------------------
*/
// dd_edge self_loop_mxd(forest* forestMxD,
//                       dd_edge& dead,
//                       const size_t i,
//                       const size_t j)
// {
//     dd_edge selfLoopMxD(forestMxD);
//     unsigned int nvars = forestMxD->getDomain()->getNumVariables();
//     std::vector<int> states(nvars+1, DONT_CARE), primedStates(nvars+1, DONT_CHANGE);
//     const int *st, *pst;

//     auto it = enumerator(dead);
//     const int* asn = it.getAssignments(); // marking is always the same
//     for(int j=0; j<nvars; j++) { // exclude extra lvl
//         // cout << *(asn+j);
//         states[j] = *(asn+j);
//         primedStates[j] = *(asn+j);
//     }
//     states[nvars] = i;
//     primedStates[nvars] = j;

//     // cout <<  "v ";
//     // for(auto s: states) cout << s;
//     // cout << endl;
//     // cout <<  "vp ";
//     // for(auto s: primedStates) cout << s;
//     // cout << endl;
    
//     st = states.data();
//     pst = primedStates.data();

//     forestMxD->createEdge(&st, &pst, 1, selfLoopMxD);
    
//     return selfLoopMxD;
// }

// //-----------------------------------------------------------------------------


// Generate a Transition MxD so that it can be used to relabel a
// location in in the extra level used as a placeholder
dd_edge mxd_relabel_loc(int loc, int loc_primed, forest* forestMxD)
{
    dd_edge labelingMxD = dd_edge(forestMxD);
    unsigned int nvars = forestMxD->getDomain()->getNumVariables();
    std::vector<int> states(nvars+1, DONT_CARE), primedStates(nvars+1, DONT_CHANGE);
    const int *st, *pst;

    // npl is a global var (!)
    states[npl+1] = loc;
    primedStates[npl+1] = loc_primed;
    st = states.data();
    pst = primedStates.data();

    forestMxD->createEdge(&st, &pst, 1, labelingMxD);

    return labelingMxD;
}

//-----------------------------------------------------------------------------

// relabel the location level of a MDD, changing from loc to loc_primed
dd_edge mdd_relabel(const dd_edge& mdd, int loc, int loc_primed, forest* forestMxD) {
    // create a relabeling MxD
    dd_edge relabelMxD = mxd_relabel_loc(loc, loc_primed, forestMxD);

    // apply the relabeling operation
    dd_edge result(mdd.getForest());
    apply(POST_IMAGE, mdd, relabelMxD, result);
    return result;
}

//-----------------------------------------------------------------------------

// RS_times_BA::RS_times_BA(RSRG* rsrg) 
// : S0(ctx.get_MDD_forest()), NSF(ctx.get_MxD_forest()), RS(ctx.get_MDD_forest())
// {}

// //-----------------------------------------------------------------------------

// RS_times_BA 
// build_RS_times_BA(Context& ctx, RSRG* rsrg, BuchiAutomaton& ba, dd_edge deadlock)
// {
//     forest* forestMDD = ctx.get_MDD_forest();
//     forest* forestMxD = ctx.get_MxD_forest();
//     expert_domain* dom = (expert_domain*)rsrg->getDomain();
//     const int nvars = dom->getNumVariables();

//     // Resize the variable bound of the extra level to accomodate BA location indicess
//     const unsigned int currentLocLvlBound = dom->getVariableBound(nvars);
//     const unsigned int locLvlBound = std::max(ba.num_locs + 1, currentLocLvlBound);
//     if (currentLocLvlBound < locLvlBound) // resize needed
//         dom->enlargeVariableBound(nvars, false, locLvlBound);

//     // full potential RS (any state in the domain)
//     dd_edge pot_RS = dd_edge(forestMDD);
//     std::vector<int> src(nvars+1, DONT_CARE);
//     const int* sa = src.data();
//     forestMDD->createEdge(&sa, 1, pot_RS);

//     // location-agnostic deadlock states
//     dd_edge deadlock_anyloc = mdd_relabel(deadlock, DONT_CARE, DONT_CARE, forestMxD);


//     RS_times_BA TS(rsrg);

//     //===============================================================
//     // Encode the NSF of the Transition System
//     for (ba_edge_t& edge : ba.edges) {
//         // create the MxD for the edge: src --(s)--> dst
//         // relabel(Sat(s), )
//         dd_edge sat_s = edge.state_formula->getMDD(ctx);
//         sat_s = mdd_relabel(sat_s, DONT_CARE, DONT_CARE, forestMxD);

//         // edgeMxD = NSF intersect (RS x Sat(s))
//         dd_edge edgeMxD(forestMxD);
//         apply(CROSS, pot_RS, sat_s, edgeMxD);
//         apply(INTERSECTION, rsrg->getNSF(), edgeMxD, edgeMxD);

//         // Finally, encode the location change from src to dst
//         edgeMxD = mxd_location_change(edgeMxD, edge.src_loc, edge.dst_loc);

//         TS.NSF += edgeMxD;

//         // Encode the self loop around deadlock states
//         // Get the deadlock states satisfying Sat(s)
//         dd_edge dead_sat_s(forestMDD);
//         apply(INTERSECTION, deadlock_anyloc, sat_s, dead_sat_s);

//         if (!ctlmdd::isEmptySet(dead_sat_s)) {
//             // Encode the MxD that remains in the same (deadlock) state
// // #warning ELVIO: self_loop_mxd: potrebbe non essere corretto
// // =======
//             dd_edge selfLoopMxD(forestMxD);
//             apply(CROSS, dead_sat_s, dead_sat_s, selfLoopMxD);

//             // Clear any location change
//             dd_edge rMxD = mxd_relabel_loc(DONT_CARE, DONT_CHANGE, forestMxD);
//             apply(INTERSECTION, rMxD, selfLoopMxD, selfLoopMxD);

//             // Encode the location change
//             dd_edge slMxD = mxd_location_change(selfLoopMxD, edge.src_loc, edge.dst_loc);

//             // dd_edge sllMxD = self_loop_mxd(forestMxD, dead_sat_s, edge.src_loc, edge.dst_loc);
//             // dumpDD("slmxd low", sllMxD);
//             // if(slMxD == sllMxD) cout << "Sono uguali" << endl;
//             TS.NSF += slMxD;
//         }
//     }

//     //===============================================================
//     // Encode the Initial states of the Transition System
//     for (ba_edge_t& edge : ba.edges) {
//         if(edge.is_initial) { // edge:  0 --(s)--> dst 
//             // Move all Sat(s) states in the dst location, and add them to S0
//             dd_edge sat_s = edge.state_formula->getMDD(ctx);
//             dd_edge init_dst = mdd_relabel(sat_s, 0, edge.dst_loc, forestMxD);

//             TS.S0 += init_dst;
//         }
//     }

//     //===============================================================
//     // Encode the accepting sets as MDDs
//     bool terms[locLvlBound];
//     for (auto&& F : ba.accept_loc_sets) {
//         std::fill(terms, terms+locLvlBound, false);
//         // mark all locations in the acceptance set F
//         for (int loc : F)
//             terms[loc] = true;

//         dd_edge mdd_F(forestMDD);
//         forestMDD->createEdgeForVar(nvars, false, terms, mdd_F);

//         TS.accept_loc_sets.push_back(mdd_F);
//     }

//     return TS;
// }

//-----------------------------------------------------------------------------
// Initial states in the RS*BA synchronized product
//-----------------------------------------------------------------------------

dd_edge
RSxBA_init_states(Context& ctx, BuchiAutomaton& ba) {
    expert_domain* dom = (expert_domain*)ctx.get_domain();
    const int nvars = dom->getNumVariables();
    // Resize the variable bound of the extra level to accomodate BA location indicess
    const unsigned int currentLocLvlBound = dom->getVariableBound(nvars);
    const unsigned int locLvlBound = std::max(ba.num_locs + 1, currentLocLvlBound);
    if (currentLocLvlBound < locLvlBound) // resize needed
        dom->enlargeVariableBound(nvars, false, locLvlBound);

    // Encode the initial states of the Transition System
    dd_edge S0(ctx.get_MDD_forest());
    for (ba_edge_t& edge : ba.edges) {
        if(edge.is_initial) { // edge:  0 --(s)--> dst 
            // Move all Sat(s) states in the dst location, and add them to S0
            dd_edge sat_s = ctx.SELECT_REAL(edge.state_formula->getMDD(ctx));
            dd_edge init_dst = mdd_relabel(sat_s, 0, edge.dst_loc, ctx.get_MxD_forest());

            S0 += init_dst;
        }
    }

    return S0;
}

//-----------------------------------------------------------------------------
// Encoding of the final acceptance sets in the RS*BA synchronized product
//-----------------------------------------------------------------------------

std::list<dd_edge>
RSxBA_final_sets(Context& ctx, BuchiAutomaton& ba) {
    forest* forestMDD = ctx.get_MDD_forest();
    forest* forestMxD = ctx.get_MxD_forest();
    expert_domain* dom = (expert_domain*)ctx.get_domain();
    const int nvars = dom->getNumVariables();

    const unsigned int locLvlBound = dom->getVariableBound(nvars);

    // Enocde all the accepting sets (1 for BA, >1 for GBA)
    std::list<dd_edge> AS;
    bool terms[locLvlBound];
    for (auto&& F : ba.accept_loc_sets) {
        std::fill(terms, terms+locLvlBound, false);
        // mark all locations in the acceptance set F
        for (int loc : F) {
            assert(loc < locLvlBound);
            terms[loc] = true;
        }

        dd_edge mdd_F(forestMDD);
        forestMDD->createEdgeForVar(nvars, false, terms, mdd_F);

        AS.push_back(mdd_F);
    }
    return AS;
}

//-----------------------------------------------------------------------------
// Explicit NSF of RS*BA
//-----------------------------------------------------------------------------

dd_edge 
RSxBA_makeNSF(Context& ctx, BuchiAutomaton& ba, dd_edge& deadlock) {
    forest* forestMDD = ctx.get_MDD_forest();
    forest* forestMxD = ctx.get_MxD_forest();
    expert_domain* dom = (expert_domain*)ctx.get_domain();
    const int nvars = dom->getNumVariables();

    // full potential RS (any state in the domain)
    dd_edge pot_RS = dd_edge(forestMDD);
    std::vector<int> src(nvars+1, DONT_CARE);
    const int* sa = src.data();
    forestMDD->createEdge(&sa, 1, pot_RS);

    // location-agnostic deadlock states
    dd_edge deadlock_anyloc = mdd_relabel(deadlock, DONT_CARE, DONT_CARE, forestMxD);

    dd_edge NSF(forestMxD);

    // Encode the NSF of the Transition System
    for (ba_edge_t& edge : ba.edges) { // edge: src --(s)--> dst
        dd_edge sat_ap = edge.state_formula->getMDD(ctx);
        sat_ap = mdd_relabel(sat_ap, DONT_CARE, DONT_CARE, forestMxD);

        // edgeMxD = NSF intersect (RS x Sat(s))
        dd_edge edgeMxD(forestMxD);
        apply(CROSS, pot_RS, sat_ap, edgeMxD);
        apply(INTERSECTION, ctx.rsrg->getNSF(), edgeMxD, edgeMxD);

        // Encode the location change from src to dst
        edgeMxD = mxd_location_change(edgeMxD, edge.src_loc, edge.dst_loc);

        NSF += edgeMxD;

        // Encode the self loop around deadlock states
        // Get the deadlock states satisfying Sat(s)
        dd_edge dead_sat_ap(forestMDD);
        apply(INTERSECTION, deadlock_anyloc, sat_ap, dead_sat_ap);

        if (!ctx.is_false(dead_sat_ap)) {
            // Encode the MxD that remains in the same (deadlock) state
            dd_edge selfLoopMxD(forestMxD);
            apply(CROSS, dead_sat_ap, dead_sat_ap, selfLoopMxD);

            // Clear any location change
            dd_edge rMxD = mxd_relabel_loc(DONT_CARE, DONT_CHANGE, forestMxD);
            apply(INTERSECTION, rMxD, selfLoopMxD, selfLoopMxD);

            // Encode the location change
            dd_edge slMxD = mxd_location_change(selfLoopMxD, edge.src_loc, edge.dst_loc);

            // dd_edge sllMxD = self_loop_mxd(forestMxD, dead_sat_ap, edge.src_loc, edge.dst_loc);
            // dumpDD("slmxd low", sllMxD);
            // if(slMxD == sllMxD) cout << "Sono uguali" << endl;
            NSF += slMxD;
        }
    }

    return NSF;
}

//-----------------------------------------------------------------------------
// Implicit LTL postimage operator
//-----------------------------------------------------------------------------

dd_edge
RSxBA_postimage(Context& ctx, BuchiAutomaton& ba, const dd_edge& deadlock, const dd_edge& setMxA) {
    forest* forestMDD = ctx.get_MDD_forest();
    forest* forestMxD = ctx.get_MxD_forest();
    dd_edge succMxA(forestMDD);

    // location-agnostic deadlock states
    dd_edge deadlock_anyloc = mdd_relabel(deadlock, DONT_CARE, DONT_CARE, forestMxD);

    // get all deadlock states in setMxA
    dd_edge deadMxA(forestMDD);
    apply(INTERSECTION, deadlock_anyloc, setMxA, deadMxA);

    // compute model successors of setMxA, using transition firings
    dd_edge succM = ctx.vNSF->post_image(setMxA);

    // compute Buchi automaton actions
    for (ba_edge_t& edge : ba.edges) {
        // firing from: src --(ap)--> dst
        dd_edge sat_src_ap = edge.state_formula->getMDD(ctx);
        sat_src_ap = mdd_relabel(sat_src_ap, DONT_CARE, edge.src_loc, forestMxD);

        dd_edge succMxEdges(forestMDD); 
        // take states in src satisfying the atomic proposition
        apply(INTERSECTION, succM, sat_src_ap, succMxEdges);
        // move to dst location
        succMxEdges = mdd_relabel(succMxEdges, edge.src_loc, edge.dst_loc, forestMxD);

        // select dead states satisfying Sat(ap) and in src location
        dd_edge stutteredMxEdge(forestMDD);
        apply(INTERSECTION, deadMxA, sat_src_ap, stutteredMxEdge);
        // move to dst location
        stutteredMxEdge = mdd_relabel(stutteredMxEdge, edge.src_loc, edge.dst_loc, forestMxD);

        // add new states to the final result
        succMxA += succMxEdges;
        succMxA += stutteredMxEdge;
    }

    return succMxA;
}

//-----------------------------------------------------------------------------
// Implicit LTL preimage operator
//-----------------------------------------------------------------------------

dd_edge
RSxBA_preimage(Context& ctx, BuchiAutomaton& ba, const dd_edge& deadlock, const dd_edge& setMxA) {
    forest* forestMDD = ctx.get_MDD_forest();
    forest* forestMxD = ctx.get_MxD_forest();

    // location-agnostic deadlock states
    dd_edge deadlock_anyloc = mdd_relabel(deadlock, DONT_CARE, DONT_CARE, forestMxD);

    // get all deadlock states in setMxA
    dd_edge deadMxA(forestMDD);
    apply(INTERSECTION, deadlock_anyloc, setMxA, deadMxA);

    // automata predecessors of setMxA
    dd_edge predA(forestMDD);
    dd_edge deadA(forestMDD);

    // compute Buchi automaton reverse actions
    for (ba_edge_t& edge : ba.edges) { // edge: src --(ap)--> dst
        dd_edge sat_dst_ap = edge.state_formula->getMDD(ctx);
        sat_dst_ap = mdd_relabel(sat_dst_ap, DONT_CARE, edge.dst_loc, forestMxD);

        dd_edge predA_edges(forestMDD); 
        // take states in dst satisfying the atomic proposition
        apply(INTERSECTION, setMxA, sat_dst_ap, predA_edges);
        // move back to src location
        predA_edges = mdd_relabel(predA_edges, edge.dst_loc, edge.src_loc, forestMxD);

        // select dead states satisfying Sat(ap) in dst location
        dd_edge stutteredA_edge(forestMDD);
        apply(INTERSECTION, deadMxA, sat_dst_ap, stutteredA_edge);
        // move back to src location
        stutteredA_edge = mdd_relabel(stutteredA_edge, edge.dst_loc, edge.src_loc, forestMxD);

        // add new states to the final result
        predA += predA_edges;
        //predA += stutteredA_edge;
        deadA += stutteredA_edge;
    }

    // compute model predecessors
    dd_edge predMxA = ctx.vNSF->pre_image(predA);
    // add back dead states filtered by the stuttered automaton
    predMxA += deadA;

    return predMxA;
}

//-----------------------------------------------------------------------------

forest* RSxBA_VirtualNSF::getForestMxD() const { 
    return ctx->get_MxD_forest();
}

dd_edge RSxBA_VirtualNSF::pre_image(const dd_edge& set) const {
    if (implicit)
        return RSxBA_preimage(*ctx, *ba, deadlock, set);
    else {
        dd_edge pre_img(set.getForest());
        MEDDLY::apply(PRE_IMAGE, set, NSF, pre_img);
        return pre_img;
    }
}

dd_edge RSxBA_VirtualNSF::post_image(const dd_edge& set) const {
    if (implicit)
        return RSxBA_postimage(*ctx, *ba, deadlock, set);
    else {
        dd_edge post_img(set.getForest());
        MEDDLY::apply(POST_IMAGE, set, NSF, post_img);
        return post_img;
    }
}

dd_edge RSxBA_VirtualNSF::forward_reachable(const dd_edge& s0) const {
    if (implicit)
        return reachable_BFS(s0, this);
    else {
        dd_edge reachab = s0;
        apply(REACHABLE_STATES_DFS, reachab, NSF, reachab);
        return reachab;
    }
}

//-----------------------------------------------------------------------------
} // namespace ctlmdd




















