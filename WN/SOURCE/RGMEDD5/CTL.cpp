#include "CTL.h"
#include "LTL.h"

#undef GREATER_THAN
#undef DEAD

using namespace std;

namespace std {
    std::size_t 
    hash<ctlmdd::BaseFormula*>::operator()(const ctlmdd::BaseFormula* pf) const {
        return pf->hash();        
    }
}

namespace ctlmdd {

//-----------------------------------------------------------------------------

const char* s_languageName[3] = { "CTL", "LTL", "CTLSTAR" };

//-----------------------------------------------------------------------------

const char* g_quant_type_str[2] = { "E", "A" };
const char* g_path_op_type[4] = { "X", "G", "F", "U" };

//-----------------------------------------------------------------------------

inline size_t hash_combine(size_t seed) { return seed; }

template <typename T, typename... Rest>
inline size_t hash_combine(size_t seed, T v, Rest&&... rest)
{
    seed ^= std::hash<T>{}(v) + 0xd2911b3ffc2dd383 + (seed << 6) + (seed >> 2);
    return hash_combine(seed, std::forward<Rest>(rest)...);
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CTLMDD
//-----------------------------------------------------------------------------
CTLMDD *CTLMDD::instance = NULL;

void CTLMDD::CTLinit() {
    int nvar = g_rsrg->getDomain()->getNumVariables();
    ins = new int *;
    *ins = new int[nvar + 1];
    for (int j = 1; j <= nvar; j++) {
        ins[0][j] = DOUBLELEVEL ? DONT_CHANGE : DONT_CARE;
    }
    forest::policies fp(DOUBLELEVEL); // false: not a relation

    if (DOUBLELEVEL)
        fp.setIdentityReduced();
    else
        fp.setFullyReduced();
    //fp.setCompactStorage();
    fp.setOptimistic();
    forestMTMDD = g_rsrg->getDomain()->createForest(DOUBLELEVEL, forest::REAL, 
                                                    forest::MULTI_TERMINAL, fp);
    //if (DOUBLELEVEL)
    //  forestMTMDD->setReductionRule(forest::IDENTITY_REDUCED);
    //forestMTMDD->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);
    //forestMTMDD->setNodeDeletion(forest::OPTIMISTIC_DELETION);
}

CTLMDD* CTLMDD::getInstance() {
    if (instance == NULL) {
        static CTLMDD tmp;
        instance = &tmp;
    }
    return instance;
}

forest *CTLMDD::getMTMDDForest() const {
    return forestMTMDD;
}

int **CTLMDD::getIns() const {
#ifndef NDEBUG
    int nvar = g_rsrg->getDomain()->getNumVariables();
    bool ins_damaged = false;
    for (int j = 1; j <= nvar; j++) {
        if (ins[0][j] != DONT_CARE) {
            ins_damaged = true;
            ins[0][j] = DONT_CARE;
        }
    }
    if (ins_damaged) {
        cerr << "WARN: getIns(): the support vector has not been cleared correctly!!." << endl;
    }
#endif
    return ins;
}

void CTLMDD::print_state(const int *marking) const {
    bool first = true;
    for (size_t i = 0; i < npl; i++) {
        if (marking[i + 1] > 0) {
            // if (marking[i] > 1)
            //     cout << marking[i] << "*";
            cout << (first ? "" : ", ") << g_rsrg->nameOfMddVar(i);
            cout << "(" << marking[i + 1] << ")";
            first = false;
        }
    }
}

CTLMDD::CTLMDD() { }

CTLMDD::~CTLMDD() {
    if (ins != NULL) {
        delete[] ins[0];
        delete ins;
    }
}

bool CTLMDD::cache_contains(BaseFormula* f) const {
    return formula_cache.count(f) != 0;
}

BaseFormula* CTLMDD::cache_insert(BaseFormula* f) {
    CTL_ASSERT(!f->is_cached());
    // First, verify if an equivalent object already exists in the cache
    auto iter = formula_cache.find(f);
    if (iter != formula_cache.end()) {
#ifndef NDEBUG
        {
            // Verify consistency of the equal() methods of the BaseFormula* objects
            // To be equivalent, f and *iter must also have the same textual representation.
            std::stringstream repr_f1, repr_f2;
            repr_f1 << *f;
            repr_f2 << *(*iter);
            CTL_ASSERT(repr_f1.str() == repr_f2.str()); // must have the same representation.
        }
#endif
        // return the cached object instead of f (and delete f)
        // cout << "CACHE HIT: " << (*f) << "   f="<<f<<" cache="<<*iter<<endl;
        safe_remove_ref(f);
        f = *iter; // cached object
        f->add_ref();
        return f;
    }
    // f does not exists in the cache yet. Add it.
    // cout << "CACHE ADD: " << (*f) << "   f="<<f<<endl;
    formula_cache.insert(f);
    f->set_cached(true);
    return f;
}

void CTLMDD::cache_remove(BaseFormula* f) {
    CTL_ASSERT(f->is_cached());
    // cout << "CACHE REMOVE: " << (*f) << "   f="<<f<<endl;
    auto iter = formula_cache.find(f);
    CTL_ASSERT(iter != formula_cache.end());
    CTL_ASSERT(*iter == f);
    (*iter)->set_cached(false);
    formula_cache.erase(iter);
}

size_t CTLMDD::cache_size() const {
    return formula_cache.size();
}


void CTLMDD::printStatistics() {
    // rs.getForest()->garbageCollect();
    // NSF.getForest()->garbageCollect();
    // forestMTMDD->garbageCollect();
    const dd_edge& rs = g_rsrg->getRS();

    print_banner(" CTL MEMORY ");
    cout << " RS nodes:                " << rs.getNodeCount() << endl;
    cout << " Forest(RS) nodes:        " << rs.getForest()->getCurrentNumNodes() << " actives, "
         << rs.getForest()->getPeakNumNodes() << " peak, "
         << rs.getForest()->getStats().num_compactions << " compactions." << endl;
    cout << " Forest(RS) size:         " << rs.getForest()->getCurrentMemoryUsed() << " Bytes now, "
         << rs.getForest()->getPeakMemoryUsed() << " Bytes peak." << endl;

    // cout << " Potential RG nodes:      " << NSF.getNodeCount() << endl;
    // cout << " Forest(RG) nodes:        " << NSF.getForest()->getCurrentNumNodes() << " actives, "
    //      << NSF.getForest()->getPeakNumNodes() << " peak, "
    //      << NSF.getForest()->getStats().num_compactions << " compactions." << endl;
    // cout << " Forest(RG) size:         " << NSF.getForest()->getCurrentMemoryUsed() << " Bytes now, "
    //      << NSF.getForest()->getPeakMemoryUsed() << " Bytes peak." << endl;

    // cout << " Forest(RG+real) nodes:   " << forestMTMDD->getCurrentNumNodes() << " actives, "
    //      << forestMTMDD->getPeakNumNodes() << " peak, "
    //      << forestMTMDD->getStats().num_compactions << " compactions." << endl;
    // cout << " Forest(RG+real) size:    " << forestMTMDD->getCurrentMemoryUsed() << " Bytes now, "
    //      << forestMTMDD->getPeakMemoryUsed() << " Bytes peak." << endl;

    // cout << "\n\n================== MEMORY CTL==================" << endl;
    // cout << "Peak Used Memory (RS): " << rs.getForest()->getPeakMemoryUsed() << "B" << endl;
    // cout << "Current Used  Memory (RS): " << rs.getForest()->getCurrentMemoryUsed() << "B" << endl;
    // cout << "Peak Used Memory (potential RG): " << NSF.getForest()->getPeakMemoryUsed() << "B" << endl;
    // cout << "Current Used  Memory (potential RG): " << NSF.getForest()->getCurrentMemoryUsed() << "B" << endl;
    // cout << "Peak Used Memory (potential RG + Real): " << forestMTMDD->getPeakMemoryUsed() << "B" << endl;
    // cout << "Current Used  Memory (potential RG +Real): " << forestMTMDD->getCurrentMemoryUsed() << "B" << endl;
    // cout << "===============================================\n" << endl;

    // cout << "==================== NODE ======================" << endl;
    // cout << "Peak Node Number (RS): " << rs.getForest()->getPeakNumNodes() << endl;
    // cout << "Used Node Number (RS): " << rs.getForest()->getCurrentNumNodes() << endl;
    // cout << "Peak Node Number (potential RG): " << NSF.getForest()->getPeakNumNodes() << endl;
    // cout << "Used Node Number (potential RG): " << NSF.getForest()->getCurrentNumNodes() << endl;
    // cout << "Peak Node Number (potential RG + Real): " << forestMTMDD->getPeakNumNodes() << endl;
    // cout << "Used Node Number (potential RG + Real): " << forestMTMDD->getCurrentNumNodes() << endl;
    // cout << "================================================\n" << endl;
}

//-----------------------------------------------------------------------------

template<typename Printable>
struct FormulaPrinter {
    Printable *e;
    clock_t start;
    const char *optMsg;
    FormulaPrinter(Printable *_e) : e(_e), optMsg(NULL) {
        if (running_for_MCC() || CTL_quiet)
            return;
        start = clock();
        cout << "Eval: " << (*e);
        cout << (e->is_top_level() ? "  TOP" : "");
        if constexpr (std::is_base_of<Formula, Printable>::value) {
            cout << (e->isStateFormula() ? "  STATE" : "");
            cout << (e->isPathFormula() ? "  PATH" : "");
            cout << (e->isAtomicPropos() ? "  AP" : "");
        }
        cout << endl << flush;
    }
    ~FormulaPrinter() {
        if (running_for_MCC() || CTL_quiet)
            return;
        if (optMsg)
            cout << "      " << optMsg;
        else {
            long msec = long(1000000.0 * (double(clock() - start) / CLOCKS_PER_SEC));
            cout << "      " << setprecision(6) << fixed << (msec / 1000000.0) << " sec.";
        }
        cout.unsetf(ios_base::floatfield);
        stat(e);
        cout << endl;
        cout.unsetf(ios_base::floatfield);
    }
    void stat(Formula *f) {
        if (running_for_MCC() || CTL_quiet)
            return;
        cout << "  potential card = " << f->getStoredMDD().getCardinality();
        if (f->getStoredMDD().getNode() == g_rsrg->getRS().getNode())
            cout << " (RS)";

        if (CTL_print_intermediate_sat_sets) {            
            cout << endl;
            // const dd_edge& dd = f->getMDD(ctx);
            dd_edge dd(f->getStoredMDD());
            apply(INTERSECTION, g_rsrg->getRS(), dd, dd);
            enumerator i(dd);
            int nvar = dd.getForest()->getDomain()->getNumVariables();
            cout << "     real card = " << dd.getCardinality() << endl;
            for (enumerator i(dd); i != 0; ++i) {
                cout << "     ";
                for(int j=1; j <= nvar; j++) { // for each variable
                    int val = *(i.getAssignments() + j);
                    const char* s = dd.getForest()->getDomain()->getVar(j)->getName();
                    if (g_rsrg->isIndexOfPlace(j - 1)) {
                        if(val == 1)
                            cout << s << " ";
                        else if(val != 0) 
                            cout << s << "(" << val << ") ";
                    }
                    else if(val != 0)
                        cout << "loc(" << val << ") ";
                }
                cout << endl;
            }
            // cout << endl;
            // // ostream_output meddout(cout);
            // // dd.show(meddout, 2);
            // // cout << endl;
        }
    }
    void stat(IntFormula *e) {
        if (running_for_MCC() || CTL_quiet)
            return;
        //cout << "  card = " << e->getMTMDD(ctx)->getCardinality();
    }
    void optimized(const char *m) { optMsg = m; }
};


//-----------------------------------------------------------------------------
// TreeTraceNode
//-----------------------------------------------------------------------------
void TreeTraceNode::print_trace(const char *prefix, int nest_level) const {
    CTLMDD *ctl = CTLMDD::getInstance();

    size_t cnt = 1;
    const TreeTraceNode *ttn = this;
    for (int i = 0; i < nest_level; i++)
        cout << "  ";
    if (nest_level == 0)
        cout << "Initial state";
    else
        cout << "State " << prefix;
    cout << (traceTy == TT_COUNTEREXAMPLE ? " does not satisfy: " : " satisfies: ");
    cout << *(ttn->formula) << ". ";
    if (ttn->isECTL_cntexample) {
        cout << "Could not show a counter-example for this ECTL formula.";
    }
    if (ttn->isCircular) {
        cout << "Start of loop.";
    }
    cout << endl << endl;
    if (nest_level > 0 && ttn->next == NULL && ttn->sub_trace1 == NULL && ttn->sub_trace2 == NULL)
        return; // Do not show this AP state
    while (ttn) {
        for (int i = 0; i < nest_level; i++)
            cout << "  ";
        cout << prefix << cnt << ": ";
        ctl->print_state(ttn->marking.data());
        cout << endl;
        if (ttn->sub_trace1) {
            ostringstream oss;
            oss << prefix << cnt << "." << (ttn->sub_trace2 ? "L." : "");
            ttn->sub_trace1->print_trace(oss.str().c_str(), nest_level + 1);
        }
        if (ttn->sub_trace2) {
            ostringstream oss;
            oss << prefix << cnt << "." << (ttn->sub_trace1 ? "R." : "");
            ttn->sub_trace2->print_trace(oss.str().c_str(), nest_level + 1);
        }
        ttn = ttn->next;
        cnt++;
    }
    if (isCircular) {
        for (int i = 0; i < nest_level; i++)
            cout << "  ";
        cout << prefix << cnt << ": loop back to state " << prefix << "1." << endl;
    }
}

TreeTraceNode::TreeTraceNode()
    : next(NULL), sub_trace1(NULL), sub_trace2(NULL),
      isECTL_cntexample(false), isCircular(false), traceTy(TT_COUNTEREXAMPLE) { }

TreeTraceNode::TreeTraceNode(const vector<int> &mark, ref_ptr<Formula> f, TraceType tt)
    : marking(mark), next(NULL), sub_trace1(NULL), sub_trace2(NULL),
      formula(f), isECTL_cntexample(false), isCircular(false), traceTy(tt)
{ /*f->addOwner();*/ }

TreeTraceNode::~TreeTraceNode() {
    if (next != NULL)
        delete next;
    if (sub_trace1 != NULL)
        delete sub_trace1;
    if (sub_trace2 != NULL)
        delete sub_trace2;
    // safe_removeOwner(formula);
}

//----------------------------------------------------------------------------





//-----------------------------------------------------------------------------
// IntLiteral
//-----------------------------------------------------------------------------
void IntLiteral::createMTMDD(Context& ctx) {
    CTLMDD *ctl = CTLMDD::getInstance();
    forest *mtmdd_forest = ctl->getMTMDDForest();
    dd_edge complete(ctl->getMTMDDForest());
    int **m = ctl->getIns();
    FormulaPrinter<IntLiteral> fp(this);
    float constant = getConstant();
    if (DOUBLELEVEL)
        mtmdd_forest->createEdge(m, m, &constant, 1, complete);
    else
        mtmdd_forest->createEdge(m, &constant, 1, complete);
    setMTMDD(complete);
}

IntLiteral::IntLiteral(float constant) {
    this->_constant = constant;
}

float IntLiteral::getConstant() const {
    return this->_constant;
}

void IntLiteral::print(std::ostream &os) const {
    os << getConstant();
}

IntLiteral::~IntLiteral() {
}

bool IntLiteral::equals(const BaseFormula* pf) const {
    if (typeid(*pf) == typeid(*this)) {
        const IntLiteral* p = dynamic_cast<const IntLiteral*>(pf);
        if (p->getConstant() == getConstant())
            return true;
    }
    return false;
}

size_t IntLiteral::compute_hash() const {
    return hash_combine(0x1e705ad9e70dd872, getConstant());
}


//-----------------------------------------------------------------------------
// BoundOfPlaces
//-----------------------------------------------------------------------------
BoundOfPlaces::BoundOfPlaces(const std::vector<int>* p) 
: IntLiteral(-1), places(*p) {
    _computedBound = make_pair(-1000, -1000); // cannot be negative
}

BoundOfPlaces::~BoundOfPlaces() {}

float BoundOfPlaces::getConstant() const {
    compute_bounds();
    return (float)getUpperBound();
}

void BoundOfPlaces::compute_bounds() const {
    if (_computedBound.first >= 0) // Already computed
        return; // nothing to do

    // Just one place in the list:
    if (places.size() == 1) {
        _computedBound = make_pair(g_rsrg->getMinValueOfPlaceRS(places[0]),
                                   g_rsrg->getMaxValueOfPlaceRS(places[0]));
        return;
    }
    
    // Compute the bound of the set of places (the maximum number of tokens that these
    // places may have in the TRG at the same time). -> MAX( p1 + p2 + .. + pn)
    std::vector<bool> selected_vars(npl + g_rsrg->getExtraLvls());
    std::fill(selected_vars.begin(), selected_vars.end(), false);
    for (int i=0; i<places.size(); i++) {
        //cout << "adding " << g_rsrg->getPL(places[i] - 1) << " to the list of places for the bound." << endl;
        //cout << "selected_vars.size()="<<selected_vars.size()<<"  places[i] - 1 = " << (places[i] - 1) << endl;
        int var_of_pl = g_rsrg->convertPlaceToMDDLevel(places[i]);

        assert(selected_vars.size() > var_of_pl);
        if (selected_vars[var_of_pl]) {
            cerr << "The same place cannot appear multiple times in the bounds() list of places." << endl;
            exit(-1);
        }
        selected_vars[var_of_pl] = true;
    }

    _computedBound = g_rsrg->computeRealBoundOfVariables(selected_vars);
    return;
}

void BoundOfPlaces::print(std::ostream &os) const {
    os << "bounds(";
    for (int p=0; p<places.size(); p++) {
        os << (p==0 ? "" : ", ") << tabp[places[p]].place_name;
    }
    os << ")";
}

bool BoundOfPlaces::equals(const BaseFormula* pf) const {
    if (typeid(*pf) == typeid(*this)) {
        const BoundOfPlaces* p = dynamic_cast<const BoundOfPlaces*>(pf);
        if (p->places == places)
            return true;
    }
    return false;
}

size_t BoundOfPlaces::compute_hash() const {
    size_t seed = 0x5b82a5e5a3cf2942;
    for (auto p : places)
        seed = hash_combine(seed, p);
    return seed;
}

//-----------------------------------------------------------------------------
// PlaceTerm
//-----------------------------------------------------------------------------
PlaceTerm::PlaceTerm(float _coeff, int _place, IntFormula::op_type _op)
: coeff(_coeff), place(_place), op(_op)
{ }

PlaceTerm::~PlaceTerm() { }

int PlaceTerm::getMeddlyLevel1based() const {
    return g_rsrg->convertPlaceToMDDLevel(place) + 1;
}

bool PlaceTerm::equals(const BaseFormula* pf) const {
    if (typeid(*pf) == typeid(*this)) {
        const PlaceTerm* p = dynamic_cast<const PlaceTerm*>(pf);
        if (p->coeff == coeff && p->place == place && p->op == op)
            return true;
    }
    return false;
}

size_t PlaceTerm::compute_hash() const {
    return hash_combine(0x1b509f09cbfe005b, coeff, place, int(op));
}

void PlaceTerm::print(std::ostream &os) const {
    if (coeff != 1 || op != EOP_TIMES)
        os << coeff << " " << IntFormula::OP_Names[op] << " ";
    os << tabp[place].place_name;
}

void PlaceTerm::createMTMDD(Context& ctx) {
    CTLMDD *ctl = CTLMDD::getInstance();
    forest *mtmdd_forest = ctl->getMTMDDForest();
    int level = getMeddlyLevel1based();
    int variable_bound = g_rsrg->getMaxValueOfPlaceRS(place);//g_rsrg->getRealBound(variable);
    int dom_bound = g_rsrg->getDomain()->getVariableBound(level);
    assert(variable_bound <= dom_bound);
    FormulaPrinter<PlaceTerm> fp(this);

    // Old method: compose the MTMDD one value at a time
    // dd_edge tmp_mdd(ctl->getMTMDDForest());
    // int val;
    // int **m = ctl->getIns();
    // float terminale;
    // for (val = 0; val <= variable_bound; val++) {
    //     m[0][level] = val;
    //     switch (op) {
    //         case IntFormula::EOP_PLUS:
    //             terminale = coeff + (float) val;
    //             break;
    //         case IntFormula::EOP_TIMES:
    //             terminale = coeff * (float) val;
    //             break;
    //         case IntFormula::EOP_MINUS:
    //             terminale = coeff - (float) val;
    //             break;
    //         case IntFormula::EOP_DIV:
    //             terminale = coeff / (float) val;
    //             break;
    //     }
    //     // dd_edge tmp_new_ap(mtmdd_forest);
    //     dd_edge new_ap(ctl->getMTMDDForest());// = tmp_new_ap;
    //     if (DOUBLELEVEL)
    //         mtmdd_forest->createEdge(m, m, &terminale, 1, new_ap);
    //     else
    //         mtmdd_forest->createEdge(m, &terminale, 1, new_ap);
    //     tmp_mdd += new_ap;
    // }
    // m[0][level] = DOUBLELEVEL ? DONT_CHANGE : DONT_CARE;
    // setMTMDD(tmp_mdd);

    // New method: use a single createEdgeForVar
    float term_vec[dom_bound + 1];
    for (int mark = 0; mark <= dom_bound; mark++) {
        if (mark <= variable_bound) {
            switch (op) {
                case IntFormula::EOP_PLUS:
                    term_vec[mark] = coeff + (float)mark;
                    break;
                case IntFormula::EOP_TIMES:
                    term_vec[mark] = coeff * (float)mark;
                    break;
                case IntFormula::EOP_MINUS:
                    term_vec[mark] = coeff - (float)mark;
                    break;
                case IntFormula::EOP_DIV:
                    term_vec[mark] = coeff / (float)mark;
                    break;
            }
        }
        else {
            term_vec[mark] = 0;
        }
    }
    dd_edge mark_of_place(ctl->getMTMDDForest());
    mtmdd_forest->createEdgeForVar(level, DOUBLELEVEL, term_vec, mark_of_place);
    setMTMDD(mark_of_place);


    // cout << "LVL="<<level<<" bnd="<<variable_bound<<" dombnd="<<dom_bound
    //      <<"   "<<mark_of_place.getNode()<<"="<<tmp_mdd.getNode()<<endl;
    // // MEDDLY::ostream_output stdout_wrap(cout);
    // // cout << "\n\nMETODO CORRENTE:" << endl;
    // // tmp_mdd.show(stdout_wrap, 2);
    // // cout << "\n\nMETODO NUOVO:" << endl;
    // // mark_of_place.show(stdout_wrap, 2);
    // // cout << endl << endl;
    // assert(mark_of_place == tmp_mdd);
}

//-----------------------------------------------------------------------------
// IntExpression
//-----------------------------------------------------------------------------
IntExpression::IntExpression(ref_ptr<IntFormula> _expr1, ref_ptr<IntFormula> _expr2, IntFormula::op_type _op) 
: expr1(_expr1), expr2(_expr2), op(_op)
{ }

IntExpression::~IntExpression() { }

bool IntExpression::equals(const BaseFormula* pf) const {
    if (typeid(*pf) == typeid(*this)) {
        const IntExpression* p = dynamic_cast<const IntExpression*>(pf);
        if (p->op == op && ::ctlmdd::equals(p->getExpr1(), getExpr1()) 
            && ::ctlmdd::equals(p->getExpr2(), getExpr2()))
            return true;
    }
    return false;
}

size_t IntExpression::compute_hash() const {
    return hash_combine(0xfbe236e8b5140335, op, expr1->hash(), expr2->hash());
}

void IntExpression::print(std::ostream &os) const {
    os << "(" << *expr1 << " " << IntFormula::OP_Names[op] << " " << *expr2 << ")";
}

void IntExpression::createMTMDD(Context& ctx) {
    CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge e1 = expr1->getMTMDD(ctx);
    dd_edge e2 = expr2->getMTMDD(ctx);
    dd_edge r(ctl->getMTMDDForest());
    FormulaPrinter<IntExpression> fp(this);
    switch (op) {
        case IntFormula::EOP_PLUS:
            r = e1 + e2;
            break;
        case IntFormula::EOP_TIMES:
            r = e1 * e2;
            break;
        case IntFormula::EOP_MINUS:
            r = e1 - e2;
            break;
        case IntFormula::EOP_DIV:
            apply(DIVIDE, e1, e2, r);
            break;
    }

#if DEBUG
    r->show(stdout, 2);
#endif
    setMTMDD(r);
}

//-----------------------------------------------------------------------------
// IntFormula
//-----------------------------------------------------------------------------
IntFormula::IntFormula() { }

IntFormula::~IntFormula() { }

const dd_edge& IntFormula::getMTMDD(Context& ctx) {
    if (!hasStoredMTMDD()) {
        // Setting computedMTMDD here avoids infinite recursions when Meddly ends the memory
        computedMTMDD = true; 
        createMTMDD(ctx);
    }
    CTL_ASSERT(hasStoredMTMDD());
    return MTMDD;
}

void IntFormula::clearMTMDD() {
    MTMDD.clear();
    computedMTMDD = false;
}

void IntFormula::setMTMDD(dd_edge newMTMDD) {
    MTMDD = newMTMDD;
    computedMTMDD = true; 
}
bool IntFormula::hasStoredMTMDD() const {
    return computedMTMDD;
}
bool IntFormula::isBoolFormula() const {
    return false;
}
bool IntFormula::isIntFormula() const {
    return true;
}


const char *IntFormula::OP_Names[4] = {
    "+", "-", "*", "/"
};

//-----------------------------------------------------------------------------
// Inequality
//-----------------------------------------------------------------------------
const char *Inequality::OP_Names[8] = {
    "<", ">", "<=", ">=", "=", "!=", "==", "<>"
};

Inequality::Inequality(op_type _op, ref_ptr<IntFormula> _expr1, float _constant) 
: expr1(_expr1), op(_op), constant(_constant)
{ }

Inequality::Inequality(op_type _op, ref_ptr<IntFormula> _expr1, ref_ptr<IntFormula> _expr2) 
: expr1(_expr1), expr2(_expr2), op(_op), constant(-1)
{ }

Inequality::~Inequality() { }

bool Inequality::equals(const BaseFormula* pf) const {
    if (typeid(*pf) == typeid(*this)) {
        const Inequality* p = dynamic_cast<const Inequality*>(pf);
		if (p->op == op && getConstant() == p->getConstant() 
            && ::ctlmdd::equals(p->getExpr1(), getExpr1()) 
			&& ::ctlmdd::equals(p->getExpr2(), getExpr2()))
            return true;
    }
    return false;
}

size_t Inequality::compute_hash() const {
    return hash_combine(0xec77d08af3db1eb8, op, expr1->hash(), 
                        expr2 ? expr2->hash() : 0, 
                        expr2 ? .0f : constant);
}

void Inequality::print(std::ostream &os) const {
    os << "(" << *expr1 << " " << OP_Names[op] << " ";
    if (expr2 != nullptr)
        os << *expr2;
    else
        os << constant;
    os << ")";
}

void Inequality::createMDD(Context& ctx) {
    //se minore di const positiva devo fare complemento
    //se maggiore di const negativa devo fare complemento
    //se <= >= o = 0
    CTLMDD *ctl = CTLMDD::getInstance();

    clearMDD();
    //caso confronto tra posti "==" o "<>"
    if ((op == IOP_DIF) || (op == IOP_SIM)) {
        FormulaPrinter<Inequality> fp(this);
        dd_edge boole(ctx.get_MDD_forest());
        dd_edge tmp_complete(ctx.get_MDD_forest());
        int **m = ctl->getIns();
        std::fill(m[0], m[0] + ctx.get_domain()->getNumVariables() + 1, DONT_CARE);
        // memset(m[0], -1, (g_rsrg->getDomain()->getNumVariables())*sizeof(int));
        ref_ptr<PlaceTerm> plterm1 = dynamic_ptr_cast<PlaceTerm>(expr1);
        ref_ptr<PlaceTerm> plterm2 = dynamic_ptr_cast<PlaceTerm>(expr2);
        int variable_bound1 = g_rsrg->getMaxValueOfPlaceRS(plterm1->getPlace());
        int variable_bound2 = g_rsrg->getMaxValueOfPlaceRS(plterm2->getPlace());
        // n*expr1 == m*expr2  ->  expr1 == (m/n)*expr2
        for (int i = 0; i <= variable_bound1  ; i++) {
            m[0][plterm1->getMeddlyLevel1based()] = i;
            int div = 1, mult = 1;
            if (plterm1->getOp() == IntFormula::EOP_DIV)
                mult *= (int)plterm1->getCoeff();
            else
                div *= (int)plterm1->getCoeff();
            if (plterm2->getOp() == IntFormula::EOP_DIV)
                div *= (int)plterm2->getCoeff();
            else
                mult *= (int)plterm2->getCoeff();
            if (((i % div) == 0) && ((i / div * mult) <= variable_bound2)) {
                m[0][plterm2->getMeddlyLevel1based()] = int(i / div * mult);
                ctx.get_MDD_forest()->createEdge(m, 1, tmp_complete);
                apply(UNION, tmp_complete, boole, boole);
                m[0][plterm2->getMeddlyLevel1based()] = DONT_CARE;
            }
            m[0][plterm1->getMeddlyLevel1based()] = DONT_CARE;
        }

        switch (op) {
            case IOP_SIM:
                apply(INTERSECTION, ctx.get_state_set(), boole, boole);
                break;
            case IOP_DIF:
                apply(DIFFERENCE, ctx.get_state_set(), boole, boole);
                break;
            default:
                throw rgmedd_exception("Internal error.");
        }
        setMDD(boole);
        return;
    }

    // only for case  exp <op> term
    if ((expr2 == nullptr) &&
            ((op == IOP_MIN && constant > 0) ||
             (op == IOP_MAJ && constant < 0) ||
             (op == IOP_MAJEQ && constant <= 0) ||
             (op == IOP_MINEQ && constant >= 0) ||
             (op == IOP_EQ && constant == 0))) {
        createMDDByComplement(ctx);
        return;
    }

    forest *mtmdd_forest = ctl->getMTMDDForest();
    dd_edge exp2MDD;
    dd_edge exp1MDD = expr1->getMTMDD(ctx);
    dd_edge q(mtmdd_forest);
    if (expr2 != nullptr) { //case exp <op> exp
        exp2MDD = expr2->getMTMDD(ctx);
    }
    else { //case exp <op> term
        int **m = ctl->getIns();
        exp2MDD = dd_edge(ctl->getMTMDDForest());
        if (DOUBLELEVEL)
            mtmdd_forest->createEdge(m, m, &(constant), 1, exp2MDD);
        else
            mtmdd_forest->createEdge(m, &(constant), 1, exp2MDD);
        // exp2MDD->show(stdout,1);
    }
    FormulaPrinter<Inequality> fp(this);
    const MEDDLY::binary_opname *opname;
    switch (op) {
        case IOP_MIN:    opname = LESS_THAN;            break;
        case IOP_MAJ:    opname = GREATER_THAN;         break;
        case IOP_MINEQ:  opname = LESS_THAN_EQUAL;      break;
        case IOP_MAJEQ:  opname = GREATER_THAN_EQUAL;   break;
        case IOP_EQ:     opname = EQUAL;                break;
        case IOP_NEQ:    opname = NOT_EQUAL;            break;
        default: throw rgmedd_exception("Internal error.");
    }

    apply(opname, exp1MDD, exp2MDD, q);
    dd_edge boole(ctx.get_MDD_forest());
    apply(COPY, q, boole);
    apply(INTERSECTION, ctx.get_state_set(), boole, boole);
    setMDD(boole);
}

void Inequality::createMDDByComplement(Context& ctx) {
    CTLMDD *ctl = CTLMDD::getInstance();
    forest *mtmdd_forest = ctl->getMTMDDForest();
    dd_edge complete(mtmdd_forest);
    int **m = ctl->getIns();
    float t = this->constant;
    if (DOUBLELEVEL)
        mtmdd_forest->createEdge(m, m, &t, 1, complete);
    else
        mtmdd_forest->createEdge(m, &t, 1, complete);

    dd_edge r(expr1->getMTMDD(ctx));
    dd_edge complement(mtmdd_forest);
    FormulaPrinter<Inequality> fp(this);

    // Select the complement operator
    const MEDDLY::binary_opname *complement_opname;
    switch (op) {
        case IOP_MIN:    complement_opname = GREATER_THAN_EQUAL;   break;
        case IOP_MAJ:    complement_opname = LESS_THAN_EQUAL;      break;
        case IOP_MINEQ:  complement_opname = GREATER_THAN;         break;
        case IOP_MAJEQ:  complement_opname = LESS_THAN;            break;
        case IOP_EQ:     complement_opname = NOT_EQUAL;            break;
        case IOP_NEQ:    complement_opname = EQUAL;                break;
        default: throw rgmedd_exception("Internal error.");
    }

    apply(complement_opname, r, complete, complement);
    dd_edge boole(ctx.get_MDD_forest());
    apply(COPY, complement, boole);
    apply(DIFFERENCE, ctx.get_state_set(), boole, boole);
    setMDD(boole);
}

TreeTraceNode *Inequality::generateTrace(const vector<int> &state, TraceType traceTy) {
    // CTLMDD *ctl = CTLMDD::getInstance();
    // bool isSat = ctl->SatSetContains(getMDD(ctx), state.data());

    // // cout << "isSat="<<isSat<<" traceTy == TT_WITNESS is " << (traceTy == TT_WITNESS) << endl;
    // CTL_ASSERT(isSat == (traceTy == TT_WITNESS));

    // return new TreeTraceNode(state, this, traceTy);
    return nullptr;
}

//-----------------------------------------------------------------------------
// Fireability
//-----------------------------------------------------------------------------
Fireability::~Fireability() { }

bool Fireability::equals(const BaseFormula* pf) const {
    if (typeid(*pf) == typeid(*this)) {
        const Fireability* p = dynamic_cast<const Fireability*>(pf);
        if (p->transitions == transitions)
            return true;
    }
    return false;
}

size_t Fireability::compute_hash() const {
    size_t seed = 0x9cae04e20f61868f;
    for (auto tr : transitions)
        seed = hash_combine(seed, tr);
    return seed;
}

void Fireability::createMDD(Context& ctx) {
    dd_edge sat(ctx.get_MDD_forest());
    // Generate the MDD corresponding to the union of the enabling conditions of the transitions
    for (int tr : transitions) {
        dd_edge enab_tr(ctx.get_MDD_forest());
        apply(PRE_IMAGE, ctx.get_state_set(), g_rsrg->getEventMxD(tr), enab_tr);
        sat += enab_tr;
    }
    setMDD(sat);
}

void Fireability::print(std::ostream &os) const {
    os << "en(";
    for (int i=0; i<transitions.size(); i++)
        os << (i==0 ? "" : ", ") << tabt[transitions[i]].trans_name;
    os << ")";
}

TreeTraceNode* Fireability::generateTrace(const vector<int> &state, TraceType traceTy) {
    // CTL_ASSERT(value == (traceTy == TT_WITNESS));
    return new TreeTraceNode(state, make_ref_ptr<Formula>(this), traceTy);
}

//-----------------------------------------------------------------------------
// AtomicProposition
//-----------------------------------------------------------------------------
AtomicProposition::~AtomicProposition() { }

bool AtomicProposition::isStateFormula() const {
    return false;
}
bool AtomicProposition::isPathFormula() const {
    return false;
}
bool AtomicProposition::isAtomicPropos() const {
    return true;
}
void AtomicProposition::maximal_path_subformula(Context& ctx, std::ostream& os, quant_type quantifier,
                                                std::vector<ref_ptr<Formula>>& subformulas) {
    // add the AP as a new subformula and output a new AP identifier
    add_this_as_subformula(os, subformulas);
}

//-----------------------------------------------------------------------------
// BoolLiteral
//-----------------------------------------------------------------------------
BoolLiteral::BoolLiteral(bool _value) : value(_value) { }

BoolLiteral::~BoolLiteral() { }

void BoolLiteral::maximal_path_subformula(Context& ctx, std::ostream& os, quant_type quantifier,
                                          std::vector<ref_ptr<Formula>>& subformulas) {
    os << (value ? "true" : "false");
}

// void BoolLiteral::setValue(bool value) {
//     this->value = value;
//     clearMDD();
// }
bool BoolLiteral::getValue() const {
    return value;
}

bool BoolLiteral::equals(const BaseFormula* pf) const {
    if (typeid(*pf) == typeid(*this)) {
        const BoolLiteral* p = dynamic_cast<const BoolLiteral*>(pf);
        if (p->getValue() == getValue())
            return true;
    }
    return false;
}

size_t BoolLiteral::compute_hash() const {
    return hash_combine(0x1d162faa0926f9ae, getValue());
}

void BoolLiteral::print(std::ostream &os) const {
    os << (value ? "true" : "false");
}

void BoolLiteral::createMDD(Context& ctx) {
    FormulaPrinter<BoolLiteral> fp(this);
    CTLMDD *ctl = CTLMDD::getInstance();
    if (value) { // TRUE
        setMDD(ctx.get_state_set());
    }
    else { // FALSE
        setMDD(ctx.empty_set());
    }
}

TreeTraceNode *BoolLiteral::generateTrace(const vector<int> &state, TraceType traceTy) {
    CTL_ASSERT(value == (traceTy == TT_WITNESS));
    return new TreeTraceNode(state, make_ref_ptr<Formula>(this), traceTy);
}


//-----------------------------------------------------------------------------
// Deadlock
//-----------------------------------------------------------------------------
Deadlock::Deadlock(bool _value) : value(_value) { }

Deadlock::~Deadlock() { }

bool Deadlock::equals(const BaseFormula* pf) const {
    if (typeid(*pf) == typeid(*this)) {
        const Deadlock* p = dynamic_cast<const Deadlock*>(pf);
        if (p->getValue() == getValue())
            return true;
    }
    return false;
}

size_t Deadlock::compute_hash() const {
    return hash_combine(0x4868947feed9fc17, getValue());
}

void Deadlock::print(std::ostream &os) const {
    os << (value ? "deadlock" : "ndeadlock");
}

void Deadlock::createMDD(Context& ctx) {
    FormulaPrinter<Deadlock> fp(this);

    if (value) // DEADLOCK
        setMDD(ctx.DEAD());
    else // NDEADLOCK
        setMDD(ctx.NON_DEAD());
}

TreeTraceNode *Deadlock::generateTrace(const vector<int> &state, TraceType traceTy) {
    // CTLMDD *ctl = CTLMDD::getInstance();
    // bool isSat = ctl->SatSetContains(getMDD(), state.data());

    // if (value) { // deadlock
    //     CTL_ASSERT(isSat == (traceTy == TT_WITNESS));
    // }
    // else { // not deadlock
    //     CTL_ASSERT(isSat == (traceTy == TT_WITNESS));
    // }

    // return new TreeTraceNode(state, this, traceTy);
    return nullptr;
}

//-----------------------------------------------------------------------------
// InitState
//-----------------------------------------------------------------------------
InitState::InitState() { }

InitState::~InitState() { }

bool InitState::equals(const BaseFormula* pf) const {
    if (typeid(*pf) == typeid(*this)) {
        // const InitState* p = dynamic_cast<const InitState*>(pf);
        // if (p->getValue() == getValue())
        return true;
    }
    return false;
}

size_t InitState::compute_hash() const {
    return hash_combine(0x29093712493f5446/*, getValue()*/);
    
}

void InitState::print(std::ostream &os) const {
    os << "initial";
}

void InitState::createMDD(Context& ctx) {
    FormulaPrinter<InitState> fp(this);
    setMDD(ctx.rsrg->getInitMark());
}

TreeTraceNode *InitState::generateTrace(const vector<int> &state, TraceType traceTy) {
    // TBD
    return nullptr;
}

//-----------------------------------------------------------------------------
// Reachability
//-----------------------------------------------------------------------------
Reachability::Reachability(ref_ptr<Formula> _subf, prop_type _type) 
: subf(_subf), type(_type)
{ }

Reachability::~Reachability() { }

bool Reachability::equals(const BaseFormula* pf) const {
    if (typeid(*pf) == typeid(*this)) {
        const Reachability* p = dynamic_cast<const Reachability*>(pf);
        if (p->type == type && ::ctlmdd::equals(p->subf.get(), subf.get()))
            return true;
    }
    return false;
}

size_t Reachability::compute_hash() const {
    return hash_combine(0xa86df651fe8d5b6b, type, subf->hash());
}

void Reachability::createMDD(Context& ctx) {
    CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge f1 = subf->getMDD(ctx);
    bool result;
    switch (type) {
        case RPT_INVARIANTLY:       result = ctx.is_true(f1);       break;
        case RPT_POSSIBILITY:       result = !ctx.is_false(f1);     break;
        case RPT_IMPOSSIBILITY:     result = ctx.is_false(f1);      break;
    }

    FormulaPrinter<Reachability> fp(this);
    setMDD(result ? ctx.get_state_set() : ctx.empty_set());
}
TreeTraceNode *Reachability::generateTrace(const vector<int> &state, TraceType traceTy) {
    throw rgmedd_exception("Unimplemented");
    // Should show a state in *subf that satisfyes/does not satisfy the invariant.
}
void Reachability::print(std::ostream &os) const {
    os << (type == RPT_POSSIBILITY ? "P" : (type == RPT_IMPOSSIBILITY ? "N" : "I")) 
       << " " << *subf;
}

//-----------------------------------------------------------------------------
// LogicalFormula
//-----------------------------------------------------------------------------
LogicalFormula::LogicalFormula(ref_ptr<Formula> _formula1, ref_ptr<Formula> _formula2, op_type _op) 
: formula1(_formula1), formula2(_formula2), op(_op)
{ }

LogicalFormula::LogicalFormula(ref_ptr<Formula> _formula1) 
: formula1(_formula1), op(CBF_NOT)
{ }

LogicalFormula::~LogicalFormula() { }

bool LogicalFormula::equals(const BaseFormula* pf) const {
    if (typeid(*pf) == typeid(*this)) {
        const LogicalFormula* p = dynamic_cast<const LogicalFormula*>(pf);
        if (p->op == op && ::ctlmdd::equals(p->getFormula1(), getFormula1()) 
            && ::ctlmdd::equals(p->getFormula2(), getFormula2()))
            return true;
    }
    return false;
}

size_t LogicalFormula::compute_hash() const {
    return hash_combine(0xb0a022609433e4cd, op, formula1->hash(),
                        formula2 ? formula2->hash() : 0);
}

void LogicalFormula::print(std::ostream &os) const {
    if (op == CBF_NOT) {
        os << "(not " << *formula1 << ")";
    }
    else {
        const char*opname = "and";
        if (op == CBF_OR)
            opname = "or";
        else if (op == CBF_IMPLY)
            opname = "imply";
        os << "(" << *formula1 << " " << opname << " " << *formula2 << ")";
    }
}

void LogicalFormula::maximal_path_subformula(Context& ctx, std::ostream& os, quant_type quantifier,
                                             std::vector<ref_ptr<Formula>>& subformulas) {
    if (isPathFormula()) {
        if (op == CBF_NOT) {
            os << "(!";
            formula1->maximal_path_subformula(ctx, os, quantifier, subformulas);
            os << ")";
        }
        else {
            if (op == CBF_IMPLY) {
                os << "(!";
                formula1->maximal_path_subformula(ctx, os, quantifier, subformulas);
                os << " | ";
                formula2->maximal_path_subformula(ctx, os, quantifier, subformulas);
                os << ")";
            }
            else {
                os << "(";
                formula1->maximal_path_subformula(ctx, os, quantifier, subformulas);
                os << ((op == CBF_AND) ? " & " : " | ");
                formula2->maximal_path_subformula(ctx, os, quantifier, subformulas);
                os << ")";
            }
        }
    }
    else {
        // add this state formula as an atomic proposition
        add_this_as_subformula(os, subformulas);
    }
}

void LogicalFormula::createMDD(Context& ctx) {
    // This implementation is only used when the logical formula is
    // a state formula. For logical formulas that are inside path formulas,
    // they should be treated as a single maximal path formula.
    if (!isStateFormula()) {
        cout << "The term: " << *this << " is not a logical state-formula, "
             << "and should be evaluated with a quantifier." << endl;
        throw rgmedd_exception("evaluating non-state LogicalFormula");
    }
    CTL_ASSERT(isStateFormula());

    // CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge result(ctx.get_MDD_forest());
    dd_edge f1 = formula1->getMDD(ctx);
    dd_edge f2 = formula2 ? formula2->getMDD(ctx) : dd_edge();

    FormulaPrinter<LogicalFormula> fp(this);
    switch (op) {
        case CBF_NOT:
            result = ctx.NOT(f1);
            break;

        case CBF_AND: {
            Formula *pf2 = formula2.get();
            if (typeid(*pf2) == typeid(LogicalFormula)) {
                LogicalFormula *logic_f2 = dynamic_cast<LogicalFormula *>(pf2);
                if (logic_f2->getOp() == LogicalFormula::CBF_NOT) {
                    // f1 AND NOT f2 can be computed more efficiently using AND_NOT
                    dd_edge not_f2 = logic_f2->getFormula1()->getMDD(ctx);
                    result = ctx.AND_NOT(f1, not_f2);
                    break;
                }
            }
            result = ctx.AND(f1, f2);
            break;
        }

        case CBF_OR: {
            result = ctx.OR(f1, f2);
            break;
        }

        case CBF_IMPLY: {
            result = ctx.OR(ctx.NOT(f1), f2);
            break;
        }
    }
    setMDD(result);
}

TreeTraceNode *LogicalFormula::generateTrace(const vector<int> &state0, TraceType traceTy) {
    assert(0);
    // CTLMDD *ctl = CTLMDD::getInstance();
    // CTL_ASSERT(ctl->SatSetContains(getMDD(), state0.data()) == (traceTy == TT_WITNESS));

    // if (op == CBF_NOT) {
    //     // Counter-example for:  NOT Phi  is a witness for Phi
    //     // Witness for:  NOT Phi  is a counter-example for Phi
    //     return getFormula1()->generateTrace(state0, oppositeTraceType(traceTy));
    //     // TreeTraceNode *not_ttn = new TreeTraceNode(state0, this, traceTy);
    //     // not_ttn->set_sub_trace1(getFormula1()->generateTrace(state0, oppositeTraceType(traceTy)));
    //     // return not_ttn;
    // }

    // // Counter-example for:  Phi_1 AND Phi_2 -> Either !Phi_1 or !Phi_2 (if false & valid)
    // // Witness for:  Phi_1 AND Phi_2 -> Both Phi_1 and Phi_2 must be true

    // // Counter-example for:  Phi_1 OR Phi_2 -> Both Phi_1 and Phi_2 must be false
    // // Witness for:  Phi_1 OR Phi_2 -> Either Phi_1 or Phi_2

    // TreeTraceNode *ttn = new TreeTraceNode(state0, this, traceTy);
    // if (traceTy == TT_WITNESS) {
    //     if (op == CBF_AND) {
    //         // cout << "ctl->SatSetContains(getFormula1()->getMDD(ctx), state0.data()) = " << ctl->SatSetContains(getFormula1()->getMDD(ctx), state0.data()) << endl;
    //         // cout << "ctl->SatSetContains(getFormula2()->getMDD(ctx), state0.data()) = " << ctl->SatSetContains(getFormula2()->getMDD(ctx), state0.data()) << endl;
    //         CTL_ASSERT(ctl->SatSetContains(getFormula1()->getMDD(ctx), state0.data()));
    //         CTL_ASSERT(ctl->SatSetContains(getFormula2()->getMDD(ctx), state0.data()));
    //         ttn->set_sub_trace1(getFormula1()->generateTrace(state0, traceTy));
    //         ttn->set_sub_trace2(getFormula2()->generateTrace(state0, traceTy));
    //     }
    //     else { // (op == CBF_OR)
    //         bool isSat1 = ctl->SatSetContains(getFormula1()->getMDD(ctx), state0.data());
    //         if (isSat1)
    //             ttn->set_sub_trace1(getFormula1()->generateTrace(state0, traceTy));
    //         else {
    //             CTL_ASSERT(ctl->SatSetContains(getFormula2()->getMDD(ctx), state0.data()));
    //             ttn->set_sub_trace1(getFormula2()->generateTrace(state0, traceTy));
    //         }
    //     }
    // }
    // else { // traceTy == TT_COUNTEREXAMPLE
    //     if (op == CBF_AND) {
    //         bool isSat1 = ctl->SatSetContains(getFormula1()->getMDD(ctx), state0.data());
    //         if (!isSat1)
    //             ttn->set_sub_trace1(getFormula1()->generateTrace(state0, traceTy));
    //         else {
    //             CTL_ASSERT(!ctl->SatSetContains(getFormula2()->getMDD(ctx), state0.data()));
    //             ttn->set_sub_trace1(getFormula2()->generateTrace(state0, traceTy));
    //         }
    //     }
    //     else { // (op == CBF_OR)
    //         CTL_ASSERT(!ctl->SatSetContains(getFormula1()->getMDD(ctx), state0.data()));
    //         CTL_ASSERT(!ctl->SatSetContains(getFormula2()->getMDD(ctx), state0.data()));
    //         ttn->set_sub_trace1(getFormula1()->generateTrace(state0, traceTy));
    //         ttn->set_sub_trace2(getFormula2()->generateTrace(state0, traceTy));
    //     }
    // }
    // return ttn;
}

bool LogicalFormula::isStateFormula() const {
    if (op == CBF_NOT)
        return formula1->isStateFormula() || formula1->isAtomicPropos();

    // A binary logical formula is a state formula iff both operands are state formulas
    return (formula1->isStateFormula() || formula1->isAtomicPropos())
        && (formula2->isStateFormula() || formula2->isAtomicPropos());
}

bool LogicalFormula::isPathFormula() const {
    // NOTE: this method is crucial in deciding which subformulas are converted in atomic
    // propositions during CTL* model checking. The implementation favors the most extensive
    // path formula, which means that AP are not maximal proper state subformulas.
    if (op == CBF_NOT)
        return formula1->isPathFormula() || formula1->isAtomicPropos();

    // A binary logical formula is a path formula iff both operands are path formulas
    return (formula1->isPathFormula() || formula1->isAtomicPropos())
        || (formula2->isPathFormula() || formula2->isAtomicPropos());
    // return !isStateFormula();
}

bool LogicalFormula::isAtomicPropos() const {
    return false;
}



//-----------------------------------------------------------------------------
// QuantifiedFormula
//-----------------------------------------------------------------------------
QuantifiedFormula::QuantifiedFormula(ref_ptr<Formula> _formula, quant_type _quantifier) 
: formula(_formula), quantifier(_quantifier)
{ }

QuantifiedFormula::~QuantifiedFormula() { }

bool QuantifiedFormula::equals(const BaseFormula* pf) const {
    if (typeid(*pf) == typeid(*this)) {
        const QuantifiedFormula* p = dynamic_cast<const QuantifiedFormula*>(pf);
        if (p->quantifier == quantifier && ::ctlmdd::equals(p->getPathFormula(), getPathFormula()))
            return true;
    }
    return false;
}

size_t QuantifiedFormula::compute_hash() const {
    return hash_combine(0x10321fa763621142, quantifier, formula->hash());
}

void QuantifiedFormula::print(std::ostream &os) const {
	os << "(" << g_quant_type_str[quantifier] << " " << *formula << ")";
}

void QuantifiedFormula::maximal_path_subformula(Context& ctx, std::ostream& os, quant_type quantifier,
                                                std::vector<ref_ptr<Formula>>& subformulas) {
    // add the Quantified formula as a new atomic proposition.
    add_this_as_subformula(os, subformulas);
}

// verify if a quantified path formula is aCTL formula
bool QuantifiedFormula::is_CTL() const {
    Formula* ppathfrm = getPathFormula();
    if (typeid(*ppathfrm) == typeid(TemporalFormula)) {
        TemporalFormula* ptf = dynamic_cast<TemporalFormula*>(ppathfrm);
        Formula* f1 = ptf->getFormula1();
        Formula* f2 = ptf->getFormula2();

        switch (ptf->getOp()) {
            case POT_GLOBALLY: // EG/AG
            case POT_FUTURE:   // EF/AF
            case POT_NEXT:     // EX/AX
                return (f1->isStateFormula() || f1->isAtomicPropos());

            case POT_UNTIL:
                return ((f1->isStateFormula() || f1->isAtomicPropos()) &&
                        (f2->isStateFormula() || f2->isAtomicPropos())); // EU/AU
        }
    }
    return false;
}

// Perform CTL model checking if the sub-formula is a valid CTL formula
bool QuantifiedFormula::do_CTL_model_checking(Context& ctx) 
{
    dd_edge result(ctx.get_MDD_forest());

    Formula* ppathfrm = getPathFormula();
    if (typeid(*ppathfrm) == typeid(TemporalFormula)) {
        TemporalFormula* ptf = dynamic_cast<TemporalFormula*>(ppathfrm);
        Formula* f1 = ptf->getFormula1();
        Formula* f2 = ptf->getFormula2();

        switch (ptf->getOp()) {
            case POT_NEXT:
                if (f1->isStateFormula() || f1->isAtomicPropos()) { // EX/AX
                    FormulaPrinter<QuantifiedFormula> fp(this);
                    if (quantifier == QOP_EXISTS)
                        result = ctx.EXfair(f1->getMDD(ctx));
                    else // QOP_ALWAYS
                        result = ctx.AXfair(f1->getMDD(ctx));
                    setMDD(result);
                    return true;
                }
                break;
            case POT_FUTURE:
                if (f1->isStateFormula() || f1->isAtomicPropos()) { // EF/AF
                    FormulaPrinter<QuantifiedFormula> fp(this);
                    if (quantifier == QOP_EXISTS)
                        result = ctx.EFfair(f1->getMDD(ctx));
                    else // QOP_ALWAYS
                        result = ctx.AFfair(f1->getMDD(ctx));
                    setMDD(result);
                    return true;
                }
                break;
            case POT_GLOBALLY:
                if (f1->isStateFormula() || f1->isAtomicPropos()) { // EG/AG
                    FormulaPrinter<QuantifiedFormula> fp(this);
                    if (quantifier == QOP_EXISTS)
                        result = ctx.EGfair(f1->getMDD(ctx));
                    else // QOP_ALWAYS
                        result = ctx.AGfair(f1->getMDD(ctx));
                    setMDD(result);
                    return true;
                }
                break;
            case POT_UNTIL:
                if ((f1->isStateFormula() || f1->isAtomicPropos()) && 
                    (f2->isStateFormula() || f2->isAtomicPropos())) { // EU/AU
                    FormulaPrinter<QuantifiedFormula> fp(this);
                    if (quantifier == QOP_EXISTS)
                        result = ctx.EUfair(f1->getMDD(ctx), f2->getMDD(ctx));
                    else // QOP_ALWAYS
                        result = ctx.AUfair(f1->getMDD(ctx), f2->getMDD(ctx));
                    setMDD(result);
                    return true;
                }
                break;
            default: 
                throw rgmedd_exception("Internal error.");
        }
    }

    return false; // not a CTL operator
}

// Core CTL* model-checking algorithm
void QuantifiedFormula::createMDD(Context& ctx) {
    if(!formula->isPathFormula()) { // accept quantified non-path formulas
        setMDD(formula->getMDD(ctx));
        return;
    }

    // Validate language constraints
    if (ctx.language==Language::CTL && !is_CTL()) {
        cerr << *this;
        throw rgmedd_exception("Not a CTL formula");
    }
    if (ctx.language==Language::LTL && !is_top_level()) {
        cerr << *this;
        // cout << (this->is_top_level() ? "  TOP" : "");
        // cout << (this->isStateFormula() ? "  STATE" : "");
        // cout << (this->isPathFormula() ? "  PATH" : "");
        // cout << (this->isAtomicPropos() ? "  AP" : "");
        // cout << endl;
        throw rgmedd_exception("Not a LTL formula");
    }

    // If the formula is a pure CTL formula, and rgmedd allows the CTL mode,
    // perform standard CTL model checking instead of CTL* model checking
    if (!ctx.always_use_SatELTL) {
        if (do_CTL_model_checking(ctx))
            return; // model checked as CTL instead of CTL*

        if (ctx.language==Language::CTL) {
            // since the user asked to use only pure CTL and not SatELTL, 
            // it is not possible to continue.
            throw rgmedd_exception("Not a CTL formula");
        }
    }

    // cout << "Using SatELTL." << endl;

    // Get the maximal path subformula and the list of subformula pointers
    std::stringstream path_formula;
    std::vector<ref_ptr<Formula>> subformulas;
    formula->maximal_path_subformula(ctx, path_formula, quantifier, subformulas);

    // Negate LTL formula for universally quantified LTL
    std::string ltl_formula = path_formula.str();
    if (quantifier == QOP_ALWAYS)
        ltl_formula = "!(" + ltl_formula + ")";
    if (print_intermediate_expr()) {
        cout << "SPOT FORMULA: " << ltl_formula << endl;
        for (size_t i=0; i<subformulas.size(); i++)
            cout << "    a"<<i<<": " << *subformulas[i] << endl;
        cout << endl;
    }

    // Generate the Buchi Automaton of the maximal path formula with SPOT
    BuchiAutomaton ba = spot_LTL_to_Buchi_automaton(ltl_formula.c_str(), subformulas);
    // int q_subst[] = {1, 0}; ba.reindex_locations(q_subst);
    // cout << ba << endl;

    dd_edge result(ctx.get_MDD_forest());
    if (!ba.accept_loc_sets.empty()) {
        // Evaluate all subformulas and atomic propositions
        ba.pre_compute_subformula_MDDs(ctx);

        // Get all deadlock states of the RS from the deadlock atomic proposition
        ref_ptr<Deadlock> dead_ap = ctlnew<Deadlock>(true);
        dd_edge deadlock = dead_ap->getMDD(ctx);

        // Initial & final states of RS*BA
        dd_edge Z0 = RSxBA_init_states(ctx, ba);
        std::list<dd_edge> AS = RSxBA_final_sets(ctx, ba);

        shared_ptr<RSxBA_VirtualNSF> RSxBA_NSF = make_shared<RSxBA_VirtualNSF>(&ctx, &ba, deadlock);
        if (!implicitNextForCTLstar) {
            // Build monolithic NSF of the RS*BA product
            dd_edge NSF = RSxBA_makeNSF(ctx, ba, deadlock);
            RSxBA_NSF->set_explicit(NSF);
        }

        // build the reachable set of RS*BA
        dd_edge reachabRSxBA = RSxBA_NSF->forward_reachable(Z0);

        // Create the evaluation context for the synchronized product
        Context ctxRSxBA(ctx.rsrg, reachabRSxBA, 
                         mdd_potential_state_set(ctx.rsrg, ctx.get_MDD_forest(), false),
                         std::move(RSxBA_NSF), 
                         false, // important! LTL semantics requires no stuttering for EG!
                         false, // no stuttering in EX (not used)
                         false, // will not use SatELTL recusively to model check the TS (not used)
                         Language::LTL, // language is irrelevant since it used only for TS evaluation.
                         ctx.statePol,
                         ctx.verbose);

        // Find the states of RS*BA that visit infinitely often at least 
        // a location of each acceptance sets of the Buchi automaton
        dd_edge fair_states;
        switch (ba.type) {
            case STRONG_BA: // Emerson-Lei algorithm
                // define the fair set for Fair CTL evaluation
                for (dd_edge& Fi : AS)
                    ctxRSxBA.add_fairness_constraint(Fi);
                // ctxRSxBA.fair_sets = AS; 
                fair_states = ctxRSxBA.EGfair(reachabRSxBA);
                break;

            case WEAK_BA:
            case VERY_WEAK_BA:
                CTL_ASSERT(AS.size() == 1);
                fair_states = ctxRSxBA.EF(ctxRSxBA.EG(AS.front()));
                break;

            case TERMINAL_BA:
                CTL_ASSERT(AS.size() == 1);
                fair_states = ctxRSxBA.EF(AS.front());
                break;

            default: throw rgmedd_exception("Unsupported Buchi automaton category.");
        }

        // Get the initial states of RS*BA that are also fair states
        dd_edge fair_Z0 = MDD_INTERSECT(fair_states, Z0); // NOTE: do not use AND!!!
        // Clear the location from RS*BA states, to get back to Petri net markings
        result = mdd_relabel(fair_Z0, DONT_CARE, 0, ctxRSxBA.vNSF->getForestMxD());

        /*std::string root("/home/elvio/amparore-svn/CTLstar/TOOL/model_");
        for (int tr=0; tr<ntr; tr++)
            write_dd_as_pdf(nullptr, ctx.rsrg->getEventMxD(tr), (root + "trn_" + tabt[tr].trans_name).c_str(), true, true, true);
        write_dd_as_pdf(nullptr, ctx.rsrg->getInitMark(), (root + "m0").c_str(), true, true, true);
        write_dd_as_pdf(nullptr, ctx.RS, (root + "RS").c_str(), true, true, true);
        write_dd_as_pdf(nullptr, ctx.rsrg->getNSF(), (root + "NSF").c_str(), true, true, true);
        write_dd_as_pdf(nullptr, S0, (root + "S0_SyncProd").c_str(), true, true, false);
        write_dd_as_pdf(nullptr, reachabRSxBA, (root + "RS_SyncProd").c_str(), true, true, false);
        const RSxBA_VirtualNSF *pNSF_RSxBA = dynamic_cast<RSxBA_VirtualNSF*>(ctxRSxBA.vNSF.get());
        write_dd_as_pdf(nullptr, pNSF_RSxBA->get_explicit(), (root + "NSF_SyncProd").c_str(), true, true, false);
        int as_cnt = 0;
        for (auto&& as : AS)
            write_dd_as_pdf(nullptr, as, (root + "AS_"+std::to_string(as_cnt++)+"_SyncProd").c_str(), true, true, false);
        write_dd_as_pdf(nullptr, fair_states, (root + "fair_states").c_str(), true, true, false);
        dd_edge result_real(result);
        apply(INTERSECTION, ctx.RS, result, result_real);
        write_dd_as_pdf(nullptr, result_real, (root + "result").c_str(), true, true, true);
        cout << ba << endl;//*/
    }

    // Invert the satisfiability set for universally quantified formulas
    if (quantifier == QOP_ALWAYS) 
        result = ctx.NOT(result); // A phi = not E (not phi)

    setMDD(result);
}

TreeTraceNode *QuantifiedFormula::generateTrace(const vector<int> &state, TraceType traceTy) {
    assert(false);
}

bool QuantifiedFormula::isStateFormula() const {
    return true;
}

bool QuantifiedFormula::isPathFormula() const {
    return false;
}

bool QuantifiedFormula::isAtomicPropos() const {
    return false;
}


//-----------------------------------------------------------------------------
// TemporalFormula
//-----------------------------------------------------------------------------
TemporalFormula::TemporalFormula(ref_ptr<Formula> _formula, path_op_type _op) 
: formula1(_formula), op(_op)
{ }

TemporalFormula::TemporalFormula(ref_ptr<Formula> _formula1, ref_ptr<Formula> _formula2/*, path_op_type _op*/) 
: formula1(_formula1), formula2(_formula2), op(POT_UNTIL)
{ }
    
TemporalFormula::~TemporalFormula() { }

bool TemporalFormula::equals(const BaseFormula* pf) const {
    if (typeid(*pf) == typeid(*this)) {
        const TemporalFormula* p = dynamic_cast<const TemporalFormula*>(pf);
        if (p->op == op && ::ctlmdd::equals(p->getFormula1(), getFormula1()) 
            && ::ctlmdd::equals(p->getFormula2(), getFormula2()))
            return true;
    }
    return false;
}

size_t TemporalFormula::compute_hash() const {
    return hash_combine(0x66504e2162c582ec, op, formula1->hash(),
                        formula2 ? formula2->hash() : 0);
}

// #ifdef DEBUG_BUILD
// void dumpDD(const std::string msg1, dd_edge& dd)
// {
//     cout << "\n" << msg1 << "\n" << std::endl;
//     ostream_output meddout(cout);
//     dd.show(meddout, 2);

//     cout << "\n=== State-location pairs reachable ===" << std::endl;
//     enumerator i(dd);
//     int nvar = dd.getForest()->getDomain()->getNumVariables();
//     while(i != 0) { // for each marking in the sat set
//         int j;
//         for(j=1; j <= nvar; j++) { // for each place
//             int val = *(i.getAssignments() + j);
//             const char* s = dd.getForest()->getDomain()->getVar(j)->getName();
//             if(val!=0 && j < nvar) 
//                 cout << s << "(" << val << ") ";
//             else if(j == nvar)
//                 cout << "loc(" << val << ") ";
//         }
//         ++i;
//         cout << endl;
//     }
//     cout << endl;
// }
// #endif

void TemporalFormula::createMDD(Context& ctx) {
    throw rgmedd_exception("TemporalFormula::createMDD cannot be called.");
}

void TemporalFormula::print(std::ostream &os) const {
    os << "(";
    if (op == POT_UNTIL) {
        os << *formula1 << " U " << *formula2;
    }
    else {
        os << g_path_op_type[op] << " " << *formula1;
    }
    os << ")";
}

void TemporalFormula::maximal_path_subformula(Context& ctx, std::ostream& os, quant_type quantifier,
                                              std::vector<ref_ptr<Formula>>& subformulas) {
    os << "(";
    if (op == POT_UNTIL) {
        formula1->maximal_path_subformula(ctx, os, quantifier, subformulas);
        os << " U ";
        formula2->maximal_path_subformula(ctx, os, quantifier, subformulas);
    }
    else {
        if (!ctx.stutter_EX && op == POT_NEXT){
            // cout << "####  !ctx.stutter_EX" << endl;
            if (quantifier == QOP_ALWAYS) {
                // X a  ==>  deadlock | X a
                ref_ptr<Deadlock> fdead = ctlnew<Deadlock>(true);
                os << "(";
                fdead->maximal_path_subformula(ctx, os, quantifier, subformulas);
                os << " | X ";
                formula1->maximal_path_subformula(ctx, os, quantifier, subformulas);
                os << ")";
                // safe_removeOwner(fdead); // clear the newly created expression
            }
            else { // quantifier == QOP_EXISTS
                // X a  ==>  !deadlock & X a
                ref_ptr<Deadlock> fndead = ctlnew<Deadlock>(false);
                os << "(";
                fndead->maximal_path_subformula(ctx, os, quantifier, subformulas);
                os << " & X ";
                formula1->maximal_path_subformula(ctx, os, quantifier, subformulas);
                os << ")";
                // safe_removeOwner(fndead); // clear the newly created expression
            }

            // // X a   ->   ((dead & a) | X a)
            // formula1->addOwner(); // new ownership in *f1
            // // Formula* f1 = ctlnew<LogicalFormula>(ctlnew<Deadlock>(true), formula1, 
            // //                                      LogicalFormula::op_type::CBF_AND);
            // Formula* fd = ctlnew<Deadlock>(true);
            // Formula* fnd = ctlnew<Deadlock>(false);
            // // fnd->maximal_path_subformula(ctx, os, quantifier, subformulas);
            // // os << "(";
            // // fnd->maximal_path_subformula(ctx, os, quantifier, subformulas);
            // os << "X (";
            // formula1->maximal_path_subformula(ctx, os, quantifier, subformulas);
            // os << " & ";
            // fnd->maximal_path_subformula(ctx, os, quantifier, subformulas);
            // os << ")";

            // safe_removeOwner(fd); // clear the newly created expression
            // safe_removeOwner(fnd);
        }
        else {
            os << g_path_op_type[op] << " ";
            formula1->maximal_path_subformula(ctx, os, quantifier, subformulas);
        }
    }
    os << ")";
}

TreeTraceNode *TemporalFormula::generateTrace(const vector<int> &state, TraceType traceTy) {
    assert(false);
}

bool TemporalFormula::isStateFormula() const {
    return false;
}

bool TemporalFormula::isPathFormula() const {
    return true;
}

bool TemporalFormula::isAtomicPropos() const {
    return false;
}

//-----------------------------------------------------------------------------
// GlobalProperty
//-----------------------------------------------------------------------------
const char *g_global_property_type_name[] = {
    "HAS_DEADLOCK", "QUASI_LIVENESS", "STABLE_MARKING", "LIVENESS", "ONESAFE"
};

GlobalProperty::GlobalProperty(global_property_type type) {
    this->type = type;
}

GlobalProperty::~GlobalProperty() { }

bool GlobalProperty::equals(const BaseFormula* pf) const {
    if (typeid(*pf) == typeid(*this)) {
        const GlobalProperty* p = dynamic_cast<const GlobalProperty*>(pf);
        if (p->type == type)
            return true;
    }
    return false;
}

size_t GlobalProperty::compute_hash() const {
    return hash_combine(0x5a4f1e3110433b15, type);
}

void GlobalProperty::createMDD(Context& ctx) {
    const bool is_unfolded = ctx.rsrg->has_unfolding_map();

    // CTLMDD *ctl = CTLMDD::getInstance();
    bool result;
    switch (type) {
        case GPT_HAS_DEADLOCK: { 
            // EF deadlock
            // computing non-deadlock states is faster
            ref_ptr<Deadlock> fnd = ctlnew<Deadlock>(false); // non-dead states
            dd_edge dd = fnd->getMDD(ctx);
            apply(INTERSECTION, ctx.RS, dd, dd); // remove potential states
            // result = (dd.getNode() != ctx.RS.getNode());
            result = !ctx.is_true(dd);
        }
        break;

        case GPT_QUASI_LIVENESS: {
            // forall t : EF fireable(t)
            // equivalent to testing if all transitions are enabled
            std::vector<int> v1(1);
            result = true;
            size_t num_transitions = ntr;

            if (is_unfolded) {
                // This PN is the unfolding of a colored model: the transitions
                // considered in the quasi-liveness should be the original colored 
                // transitions, not the unfolded ones
                num_transitions = ctx.rsrg->get_unfolding_map().tr_unf.size();
            }

            // evaluate E F firable(t) for each transition of the *ORIGINAL* model
            for (size_t tr = 0; tr<num_transitions; tr++) {
                ref_ptr<Fireability> f;
                if (is_unfolded) {
                    const auto& unfolded_trns = (ctx.rsrg->get_unfolding_map()
                                                  .tr_unf[tr].second);
                    f = ctlnew<Fireability>(&unfolded_trns);
                }
                else {
                    v1[0] = tr;
                    f = ctlnew<Fireability>(&v1);
                }
                // f = ctlnew<TemporalFormula>(f, POT_FUTURE);
                // f = ctlnew<QuantifiedFormula>(f, QOP_EXISTS);
                dd_edge enab_tr = f->getMDD(ctx);

                // apply(INTERSECTION, ctx.rsrg->getInitMark(), dd, dd);
                apply(INTERSECTION, ctx.RS, enab_tr, enab_tr); // remove potential states
                if (ctx.is_false(enab_tr)) {
                    result = false;
                    break;
                }                    
            }
        }
        break;

        case GPT_LIVENESS: {
            // forall t : AG EF fireable(t)
            std::vector<int> v1(1);
            result = true;
            size_t num_transitions = (is_unfolded ? ctx.rsrg->get_unfolding_map()
                                                            .tr_unf.size() : ntr);
            // evaluate AG EF firable(t) for each transition of the *ORIGINAL* model
            for (size_t tr = 0; tr<num_transitions; tr++) {
                ref_ptr<Fireability> f;
                if (is_unfolded) {
                    const auto& unfolded_trns = (ctx.rsrg->get_unfolding_map()
                                                  .tr_unf[tr].second);
                    f = ctlnew<Fireability>(&unfolded_trns);
                }
                else {
                    v1[0] = tr;
                    f = ctlnew<Fireability>(&v1);
                }
                dd_edge enab_tr = f->getMDD(ctx);

                dd_edge dd = ctx.AGfair(ctx.EF(enab_tr));
                // m0 |= AG EF firable(tr)
                apply(INTERSECTION, ctx.rsrg->getInitMark(), dd, dd);
                if (ctx.is_false(dd)) {
                    result = false;
                    break;
                }                    
            }
        }
        break;

        case GPT_ONESAFE: {
            // forall p : AG token_count(p) <= 1
            // equivalent to a bound computation
            std::vector<int> v1(1);
            result = true;
            int bound;
            size_t num_places = (is_unfolded ? ctx.rsrg->get_unfolding_map()
                                                .pl_unf.size() : npl);
            for (size_t pl = 0; pl<num_places; pl++) {
                ref_ptr<BoundOfPlaces> bof;
                if (is_unfolded) {
                    bof = ctlnew<BoundOfPlaces>(&ctx.rsrg->get_unfolding_map()
                                                .pl_unf[pl].second);
                }
                else {
                    v1[0] = pl;
                    bof = ctlnew<BoundOfPlaces>(&v1);
                }
                int bound = bof->getUpperBound();
                if (bound > 1) {
                    result = false;
                    break;
                }
            }
        }
        break;

        case GPT_STABLE_MARKING: {
            // E p : E x : AG token_count(p) == x
            std::vector<int> v1(1);
            result = false;
            int bound;
            size_t num_places = (is_unfolded ? ctx.rsrg->get_unfolding_map()
                                                .pl_unf.size() : npl);
            for (size_t pl = 0; pl<num_places; pl++) {
                ref_ptr<BoundOfPlaces> bof;
                if (is_unfolded) {
                    bof = ctlnew<BoundOfPlaces>(&ctx.rsrg->get_unfolding_map()
                                                .pl_unf[pl].second);
                }
                else {
                    v1[0] = pl;
                    bof = ctlnew<BoundOfPlaces>(&v1);
                }
                bool stable = bof->getLowerBound() == bof->getUpperBound();

                if (stable) {
                    result = true;
                    break;
                }
            }
        }
        break;
    }

    FormulaPrinter<GlobalProperty> fp(this);
    setMDD(result ? ctx.RS : ctx.empty_set());
}

TreeTraceNode *GlobalProperty::generateTrace(const vector<int> &state, TraceType traceTy) {
    throw rgmedd_exception("Unimplemented");
}

void GlobalProperty::print(std::ostream &os) const {
    os << g_global_property_type_name[type];
}

global_property_type GlobalProperty::getType() const {
    return type;
}



//-----------------------------------------------------------------------------
// Formula
//-----------------------------------------------------------------------------
Formula::Formula() : SatMDD(g_rsrg->getForestMDD()) { }

Formula::~Formula() { }

const dd_edge &Formula::getMDD(Context& ctx) {
    if (!hasStoredMDD()) {
        // Setting computedMDD here avoids infinite recursions when Meddly ends the memory
        computedMDD = true;
        // cout << "##>> " << *this << "  ";
        // cout << (this->is_top_level() ? "  TOP" : "");
        // cout << (this->isStateFormula() ? "  STATE" : "");
        // cout << (this->isPathFormula() ? "  PATH" : "");
        // cout << (this->isAtomicPropos() ? "  AP" : "");
        // cout << endl;
        createMDD(ctx);
        // ctx.rsrg->show_markings(cout, SatMDD);
    }
    CTL_ASSERT(hasStoredMDD());
    return SatMDD;
}

bool Formula::hasStoredMDD() const {
    return computedMDD;
}

const dd_edge& Formula::getStoredMDD() const {
    CTL_ASSERT(hasStoredMDD());
    return SatMDD;
}

void Formula::setMDD(dd_edge newMDD) {
    SatMDD = newMDD;
    computedMDD = true; 
}

void Formula::clearMDD() {
    SatMDD.clear();
    computedMDD = false;
}

bool Formula::isBoolFormula() const {
    return true;
}

bool Formula::isIntFormula() const {
    return false;
}

void Formula::add_this_as_subformula(std::ostream &os, std::vector<ref_ptr<Formula>>& subformulas) {
    for (size_t i=0; i<subformulas.size(); i++) {
        if (subformulas[i].get() == this) {
            // already present as an atomic proposition!
            os << "a" << i;
            return;
        }
    }
    // add the formula as a new Atomic proposition in the subformulas[] array,
    // and output a new AP identifier
    os << "a" << subformulas.size();
    subformulas.emplace_back(make_ref_ptr<Formula>(this));
}



//-----------------------------------------------------------------------------
// BaseFormula
//-----------------------------------------------------------------------------
BaseFormula::BaseFormula() : computed_hash(0), _is_cached(false) {}

BaseFormula::~BaseFormula() {
    assert(!is_cached()) ;
}

void BaseFormula::before_delete() { 
    if (is_cached())
        CTLMDD::getInstance()->cache_remove(this);
}

size_t BaseFormula::hash() const {
    if (computed_hash == 0) { // compute and store
        computed_hash = compute_hash();
        computed_hash = ((computed_hash==0) ? 1 : computed_hash);
    }
    return computed_hash;
}

void BaseFormula::set_cached(bool b) {
    _is_cached = b;
}

//-----------------------------------------------------------------------------

bool equals(const BaseFormula* pf1, const BaseFormula* pf2) {
    if (pf1 == pf2) // same pointer
        return true;
    if (pf1 != nullptr && pf2 != nullptr && pf1->hash() == pf2->hash() && pf1->equals(pf2))
        return true;
    return false;
}

//-----------------------------------------------------------------------------

} // end namespace ctlmdd








//-----------------------------------------------------------------------------
// reference counter base
//-----------------------------------------------------------------------------

refcounted_base::refcounted_base() : ref_count(1) {}

refcounted_base::~refcounted_base() {
    // cout << "delete ("<<(this)<<") "<<ref_count<<endl;
    assert(ref_count == 0); 
}

refcounted_base::refcounted_base(const refcounted_base& rcb) {
    ref_count = 1; // start with a new ref count
}

refcounted_base& refcounted_base::operator=(const refcounted_base& rcb) {
    ref_count = 1; // start with a new ref count
    return *this;
}

void refcounted_base::add_ref() {
    // cout << "add_ref("<<(this)<<"): "<<ref_count<<"->"<<(ref_count + 1)<<endl;
    ref_count++;
}
void refcounted_base::remove_ref() {
    // cout << "remove_ref("<<(this)<<"): "<<ref_count<<"->"<<(ref_count - 1)<<endl;
    ref_count--;
    if (ref_count < 0)
        throw rgmedd_exception("ownership count cannot become negative.");
    if (ref_count < 1) {
        before_delete();
        delete this;
    }
}

void refcounted_base::before_delete() { }

//-----------------------------------------------------------------------------
