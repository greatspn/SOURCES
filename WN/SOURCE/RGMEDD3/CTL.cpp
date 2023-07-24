/*
 * CTL.cpp
 *
 *  Created on: 11/gen/2011
 *      Author: Erica Turin
 */

#include "CTL.h"
#undef GREATER_THAN

using namespace std;

inline bool print_intermediate_expr() {
    return !running_for_MCC() && !CTL_quiet;
}

namespace ctlmdd {

//-----------------------------------------------------------------------------

inline bool isEmptySet(const dd_edge& e) {
    expert_forest *forest = static_cast<expert_forest *>(e.getForest());
    return e.getNode() == forest->handleForValue(false);
}

//-----------------------------------------------------------------------------

/*-----------------
 ---	CTLMDD	---
 ------------------*/
CTLMDD *CTLMDD::instance = NULL;

void CTLMDD::CTLinit() {
    int nvar = rsrg->getDomain()->getNumVariables();
    ins = new int *;
    *ins = new int[nvar + 1];
    for (int j = 1; j <= nvar; j++) {
        ins[0][j] = DOUBLELEVEL ? DONT_CHANGE : DONT_CARE;
    }
    MEDDLY::policies fp(DOUBLELEVEL); // false: not a relation


    if (DOUBLELEVEL)
        fp.setIdentityReduced();
    else
        fp.setFullyReduced();
    //fp.setCompactStorage();
    fp.setOptimistic();
    forestMTMDD = rsrg->getDomain()->createForest(DOUBLELEVEL, range_type::REAL, edge_labeling::MULTI_TERMINAL, fp);
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
    return ins;
}

void CTLMDD::print_state(const int *marking) const {
    bool first = true;
    for (size_t i = 0; i < npl; i++) {
        if (marking[i + 1] > 0) {
            // if (marking[i] > 1)
            //     cout << marking[i] << "*";
            cout << (first ? "" : ", ") << rsrg->placeNameOfMDDVar(i);
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

void CTLMDD::printStatistics() {
    // rs.getForest()->garbageCollect();
    // NSF.getForest()->garbageCollect();
    // forestMTMDD->garbageCollect();
    const dd_edge& rs = rsrg->getRS();
    const dd_edge& NSF = rsrg->getNSF();

    print_banner(" CTL MEMORY ");
    cout << " RS nodes:                " << rs.getNodeCount() << endl;
    cout << " Forest(RS) nodes:        " << rs.getForest()->getCurrentNumNodes() << " actives, "
         << rs.getForest()->getPeakNumNodes() << " peak, "
         << rs.getForest()->getStats().num_compactions << " compactions." << endl;
    cout << " Forest(RS) size:         " << rs.getForest()->getCurrentMemoryUsed() << " Bytes now, "
         << rs.getForest()->getPeakMemoryUsed() << " Bytes peak." << endl;

    cout << " Potential RG nodes:      " << NSF.getNodeCount() << endl;
    cout << " Forest(RG) nodes:        " << NSF.getForest()->getCurrentNumNodes() << " actives, "
         << NSF.getForest()->getPeakNumNodes() << " peak, "
         << NSF.getForest()->getStats().num_compactions << " compactions." << endl;
    cout << " Forest(RG) size:         " << NSF.getForest()->getCurrentMemoryUsed() << " Bytes now, "
         << NSF.getForest()->getPeakMemoryUsed() << " Bytes peak." << endl;

    cout << " Forest(RG+real) nodes:   " << forestMTMDD->getCurrentNumNodes() << " actives, "
         << forestMTMDD->getPeakNumNodes() << " peak, "
         << forestMTMDD->getStats().num_compactions << " compactions." << endl;
    cout << " Forest(RG+real) size:    " << forestMTMDD->getCurrentMemoryUsed() << " Bytes now, "
         << forestMTMDD->getPeakMemoryUsed() << " Bytes peak." << endl;

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


template<typename Printable>
struct FormulaPrinter {
    Printable *e;
    clock_t start;
    const char *optMsg;
    FormulaPrinter(Printable *_e) : e(_e), optMsg(NULL) {
        if (running_for_MCC() || CTL_quiet)
            return;
        start = clock();
        cout << "Eval: " << (*e) << endl << flush;
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
    void stat(StateFormula *f) {
        if (running_for_MCC() || CTL_quiet)
            return;
        cout << "  card = " << f->getMDD().getCardinality();
        if (f->getMDD().getNode() == rsrg->getRS().getNode())
            cout << " (RS)";

        if (CTL_print_intermediate_sat_sets) {
            cout << endl;
            // const dd_edge& dd = f->getMDD();
            dd_edge dd(f->getMDD());
            apply(INTERSECTION, rsrg->getRS(), dd, dd);
            enumerator i(dd);
            int nvar = dd.getForest()->getDomain()->getNumVariables();
            cout << "     " << dd.getCardinality() << endl;
            for (enumerator i(dd); i != 0; ++i) {
                cout << "     ";
                for(int j=1; j <= nvar; j++) { // for each variable
                    int val = *(i.getAssignments() + j);
                    const char* s = dd.getForest()->getDomain()->getVar(j)->getName();
                    if(val==1) 
                        cout << s << " ";
                    else if(val!=0) 
                        cout << s << "(" << val << ") ";
                }
                cout << endl;
            }
            cout << endl;

            // // ostream_output meddout(cout);
            // // dd.show(meddout, 2);
            // // cout << endl;
        }
    }
    void stat(IntFormula *e) {
        if (running_for_MCC() || CTL_quiet)
            return;
        //cout << "  card = " << e->getMTMDD()->getCardinality();
    }
    void optimized(const char *m) { optMsg = m; }
};


/*--------------------------
 ---    TreeTraceNode    ---
 ---------------------------*/

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
    : next(NULL), sub_trace1(NULL), sub_trace2(NULL), formula(NULL),
      isECTL_cntexample(false), isCircular(false), traceTy(TT_COUNTEREXAMPLE) { }

TreeTraceNode::TreeTraceNode(const vector<int> &mark, StateFormula *f, TraceType tt)
    : marking(mark), next(NULL), sub_trace1(NULL), sub_trace2(NULL),
      formula(f), isECTL_cntexample(false), isCircular(false), traceTy(tt)
{ f->addOwner(); }

TreeTraceNode::~TreeTraceNode() {
    if (next != NULL)
        delete next;
    if (sub_trace1 != NULL)
        delete sub_trace1;
    if (sub_trace2 != NULL)
        delete sub_trace2;
    if (formula != NULL)
        formula->removeOwner();
}

/*-------------------------
 ---	IntLiteral	---
 ---------------------------*/
void IntLiteral::createMTMDD() {
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

void IntLiteral::setConstant(float constant) const {
    assert(!hasStoredMTMDD());
    this->_constant = constant;
    // clearMTMDD();
}

float IntLiteral::getConstant() const {
    return this->_constant;
}

void IntLiteral::print(std::ostream &os) const {
    os << getConstant();
}

IntLiteral::~IntLiteral() {
}

/*------------------------
 ---  Bound of Places  ---
 -------------------------*/

BoundOfPlaces::BoundOfPlaces(const std::vector<int>* p) 
: IntLiteral(-1), places(*p) {
    setConstant(-1);
}

float BoundOfPlaces::getConstant() const {
    if (IntLiteral::getConstant() >= 0) // Already computed
        return IntLiteral::getConstant();

    // Just one place in the list:
    if (places.size() == 1) {
        // int var_of_pl = rsrg->convertPlaceToMDDLevel(places[0]) + 1;
        setConstant(rsrg->getMaxValueOfPlaceRS(places[0]));
        return IntLiteral::getConstant();
    }
    
    // Compute the bound of the set of places (the maximum number of tokens that these
    // places may have in the TRG at the same time). -> MAX( p1 + p2 + .. + pn)
    std::vector<bool> selected_vars(npl);
    std::fill(selected_vars.begin(), selected_vars.end(), false);
    for (int i=0; i<places.size(); i++) {
        //cout << "adding " << rsrg->getPL(places[i] - 1) << " to the list of places for the bound." << endl;
        //cout << "selected_vars.size()="<<selected_vars.size()<<"  places[i] - 1 = " << (places[i] - 1) << endl;
        int var_of_pl = rsrg->convertPlaceToMDDLevel(places[i]);

        assert(selected_vars.size() > var_of_pl);
        if (selected_vars[var_of_pl]) {
            cerr << "The same place cannot appear multiple times in the bounds() list of places." << endl;
            exit(-1);
        }
        selected_vars[var_of_pl] = true;
    }

    setConstant(rsrg->computeRealBoundOfVariables(selected_vars));
    return IntLiteral::getConstant();
}

void BoundOfPlaces::print(std::ostream &os) const {
    os << "bounds(";
    for (int p=0; p<places.size(); p++) {
        os << (p==0 ? "" : ", ") << tabp[places[p]].place_name;
    }
    os << ")";
}


/*-----------------
 ---    PlaceTerm    ---
 ------------------*/

PlaceTerm::PlaceTerm(float coeff, int place, IntFormula::op_type op) {
    this->coeff = coeff;
    this->place = place;
    this->op = op;
}

PlaceTerm::~PlaceTerm() {
}

float PlaceTerm::getCoeff() const {
    return coeff;
}

int PlaceTerm::getVariable() const {
    return rsrg->convertPlaceToMDDLevel(place) + 1;
}

int PlaceTerm::getPlace() const {
    return place;
}

void PlaceTerm::setCoeff(float coeff) {
    this->coeff = coeff;
    clearMTMDD();
}

IntFormula::op_type PlaceTerm::getOp() const {
    return op;
}

void PlaceTerm::setOp(IntFormula::op_type op) {
    this->op = op;
    clearMTMDD();
}

void PlaceTerm::print(std::ostream &os) const {
    if (coeff != 1 || op != EOP_TIMES)
        os << coeff << " " << IntFormula::OP_Names[op] << " ";
    os << tabp[place].place_name;
}

void PlaceTerm::createMTMDD() {
    CTLMDD *ctl = CTLMDD::getInstance();
    forest *mtmdd_forest = ctl->getMTMDDForest();
    dd_edge tmp_mdd(ctl->getMTMDDForest());
    int val;
    int **m = ctl->getIns();
    float terminale;
    int level = rsrg->convertPlaceToMDDLevel(place) + 1;
    int variable_bound = rsrg->getMaxValueOfPlaceRS(place);//rsrg->getRealBound(variable);
#warning "Use createEdgeForVar"
#warning "non dovrebbero mai comparire PLUS e MINUS, controllare"
    FormulaPrinter<PlaceTerm> fp(this);
    for (val = 0; val <= variable_bound; val++) {
        m[0][level] = val;
        switch (op) {
            case IntFormula::EOP_PLUS:
                terminale = coeff + (float) val;
                break;
            case IntFormula::EOP_TIMES:
                terminale = coeff * (float) val;
                break;
            case IntFormula::EOP_MINUS:
                terminale = coeff - (float) val;
                break;
            case IntFormula::EOP_DIV:
                terminale = coeff / (float) val;
                break;
        }
        // dd_edge tmp_new_ap(mtmdd_forest);
        dd_edge new_ap(ctl->getMTMDDForest());// = tmp_new_ap;
        if (DOUBLELEVEL)
            mtmdd_forest->createEdge(m, m, &terminale, 1, new_ap);
        else
            mtmdd_forest->createEdge(m, &terminale, 1, new_ap);
        tmp_mdd += new_ap;
    }
    m[0][level] = DOUBLELEVEL ? DONT_CHANGE : DONT_CARE;
    setMTMDD(tmp_mdd);

#warning "Try and test this implementation below."
    // float term_vec[variable_bound + 1];
    // for (int mark = 0; mark <= variable_bound; mark++)
    //     term_vec[mark] = mark;
    // Controllare se gli indici si term_vec partono da 0 o 1, e se serve l'elemento -1
    // dd_edge* mark_of_place(ctl->getMTMDDForest());
    // mtmdd_forest->createEdgeForVar(level, DOUBLELEVEL, term_vec, *mark_of_place);
    // setMTMDD(mark_of_place);
}

/*---------------------
 ---	IntExpression	---
 ----------------------*/
IntExpression::IntExpression(IntFormula *expr1, IntFormula *expr2, IntFormula::op_type op) {
    this->expr1 = expr1;
    this->expr2 = expr2;
    this->op = op;
    expr1->addOwner();
    expr2->addOwner();
}

IntExpression::~IntExpression() {
    expr1->removeOwner();
    expr2->removeOwner();
}

IntFormula *IntExpression::getExpr1() const {
    return expr1;
}

void IntExpression::setExpr1(IntFormula *expr1) {
    this->expr1 = expr1;
    clearMTMDD();
}

IntFormula *IntExpression::getExpr2() const {
    return expr2;
}

void IntExpression::setExpr2(IntFormula *expr2) {
    this->expr2 = expr2;
    clearMTMDD();
}

IntFormula::op_type IntExpression::getOp() const {
    return op;
}

void IntExpression::setOp(IntFormula::op_type op) {
    this->op = op;
    clearMTMDD();
}

void IntExpression::print(std::ostream &os) const {
    os << "(" << (*expr1) << " " << IntFormula::OP_Names[op] << " " << (*expr2) << ")";
}

void IntExpression::createMTMDD() {
    CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge e1 = expr1->getMTMDD();
    dd_edge e2 = expr2->getMTMDD();
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

/*---------------------
 ---	IntFormula	---
 ----------------------*/
IntFormula::IntFormula() {
    countOwner = 0;
}

IntFormula::~IntFormula() {
    // clearMTMDD();
}

void IntFormula::addOwner() {
    countOwner++;
}

void IntFormula::removeOwner() {
    countOwner--;
    if (countOwner < 1)
        delete this;
}

const dd_edge& IntFormula::getMTMDD() {
    if (!hasStoredMTMDD()) {
        // Setting computedMTMDD here avoids infinite recursions when Meddly ends the memory
        computedMTMDD = true; 
        createMTMDD();
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

/*---------------------
 ---	Inequality	---
 ----------------------*/
const char *Inequality::OP_Names[8] = {
    "<", ">", "<=", ">=", "=", "!=", "==", "<>"
};

float Inequality::getConstant() const {
    return constant;
}

// IntFormula *Inequality::getExpr() const {
//     return expr;
// }
Inequality::op_type Inequality::getOp() const {
    return op;
}

void Inequality::setConstant(float constant) {
    this->constant = constant;
    clearMDD();
}

// void Inequality::setExpr(IntFormula *expr) {
//     this->expr = expr;
//     clearMDD();
// }

void Inequality::setOp(op_type op) {
    this->op = op;
    clearMDD();
}

Inequality::Inequality(op_type op, IntFormula *expr1, float constant) {
    this->op = op;
    this->expr1 = expr1;
    this->expr2 = NULL;
    this->constant = constant;
    expr1->addOwner();
}

Inequality::Inequality(op_type op, IntFormula *expr1, IntFormula *expr2) {
    this->op = op;
    this->expr1 = expr1;
    this->expr2 = expr2;
    this->constant = -1;
    expr1->addOwner();
    expr2->addOwner();
}

Inequality::~Inequality() {
    expr1->removeOwner();
    if (expr2 != NULL)
        expr2->removeOwner();
}

void Inequality::print(std::ostream &os) const {
    os << "(" << (*expr1) << " " << OP_Names[op] << " ";
    if (expr2 != NULL)
        os << (*expr2) << ")";
    else
        os << constant << ")";
}

void Inequality::createMDD() {
    //se minore di const positiva devo fare complemento
    //se maggiore di const negativa devo fare complemento
    //se <= >= o = 0
    CTLMDD *ctl = CTLMDD::getInstance();

    clearMDD();
    //caso confronto tra posti "==" o "<>"
    if ((op == IOP_DIF) || (op == IOP_SIM)) {
        FormulaPrinter<Inequality> fp(this);
        dd_edge boole(rsrg->getForestMDD());
        dd_edge tmp_complete(rsrg->getForestMDD());
        int **m = ctl->getIns();
        std::fill(m[0], m[0] + rsrg->getDomain()->getNumVariables() + 1, DONT_CARE);
        // memset(m[0], -1, (rsrg->getDomain()->getNumVariables())*sizeof(int));
        int variable_bound1 = rsrg->getMaxValueOfPlaceRS(((PlaceTerm *)expr1)->getPlace());
        int variable_bound2 = rsrg->getMaxValueOfPlaceRS(((PlaceTerm *)expr2)->getPlace());
        // n*expr1 == m*expr2  ->  expr1 == (m/n)*expr2
        for (int i = 0; i <= variable_bound1  ; i++) {
            m[0][((PlaceTerm *)expr1)->getVariable()] = i;
            int div = 1, mult = 1;
            if (((PlaceTerm *)expr1)->getOp() == IntFormula::EOP_DIV)
                mult *= (int)((PlaceTerm *)expr1)->getCoeff();
            else
                div *= (int)((PlaceTerm *)expr1)->getCoeff();
            if (((PlaceTerm *)expr2)->getOp() == IntFormula::EOP_DIV)
                div *= (int)((PlaceTerm *)expr2)->getCoeff();
            else
                mult *= (int)((PlaceTerm *)expr2)->getCoeff();
            if (((i % div) == 0) && ((i / div * mult) <= variable_bound2)) {
                m[0][((PlaceTerm *)expr2)->getVariable()] = int(i / div * mult);
                rsrg->getForestMDD()->createEdge(m, 1, tmp_complete);
                apply(UNION, tmp_complete, boole, boole);
                m[0][((PlaceTerm *)expr2)->getVariable()] = DONT_CARE;
            }
            m[0][((PlaceTerm *)expr1)->getVariable()] = DONT_CARE;
        }

        switch (op) {
            case IOP_SIM:
                apply(INTERSECTION, rsrg->getRS(), boole, boole);
                break;
            case IOP_DIF:
                apply(DIFFERENCE, rsrg->getRS(), boole, boole);
                break;
            default:
                throw;
        }
        setMDD(boole);
        return;
    }

    // only for case  exp <op> term
    if ((expr2 == NULL) &&
            ((op == IOP_MIN && constant > 0) ||
             (op == IOP_MAJ && constant < 0) ||
             (op == IOP_MAJEQ && constant <= 0) ||
             (op == IOP_MINEQ && constant >= 0) ||
             (op == IOP_EQ && constant == 0))) {
        createMDDByComplement();
        return;
    }

    forest *mtmdd_forest = ctl->getMTMDDForest();
    dd_edge exp2MDD;
    dd_edge exp1MDD = expr1->getMTMDD();
    dd_edge q(mtmdd_forest);
    if (expr2 != NULL) { //case exp <op> exp
        exp2MDD = expr2->getMTMDD();
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
    MEDDLY::binary_handle opname;
    switch (op) {
        case IOP_MIN:    opname = LESS_THAN;            break;
        case IOP_MAJ:    opname = GREATER_THAN;         break;
        case IOP_MINEQ:  opname = LESS_THAN_EQUAL;      break;
        case IOP_MAJEQ:  opname = GREATER_THAN_EQUAL;   break;
        case IOP_EQ:     opname = EQUAL;                break;
        case IOP_NEQ:    opname = NOT_EQUAL;            break;
        default: throw;
    }
    apply(opname, exp1MDD, exp2MDD, q);
    dd_edge boole(rsrg->getForestMDD());
    apply(COPY, q, boole);
    apply(INTERSECTION, rsrg->getRS(), boole, boole);//estraggo da rs quelli che
    setMDD(boole);
}

void Inequality::createMDDByComplement() {
    CTLMDD *ctl = CTLMDD::getInstance();
    forest *mtmdd_forest = ctl->getMTMDDForest();
    dd_edge complete(mtmdd_forest);
    int **m = ctl->getIns();
    float t = this->constant;
    if (DOUBLELEVEL)
        mtmdd_forest->createEdge(m, m, &t, 1, complete);
    else
        mtmdd_forest->createEdge(m, &t, 1, complete);

    dd_edge r(expr1->getMTMDD());
    dd_edge complement(mtmdd_forest);
    FormulaPrinter<Inequality> fp(this);

    // Select the complement operator
    MEDDLY::binary_handle complement_opname;
    switch (op) {
        case IOP_MIN:    complement_opname = GREATER_THAN_EQUAL;   break;
        case IOP_MAJ:    complement_opname = LESS_THAN_EQUAL;      break;
        case IOP_MINEQ:  complement_opname = GREATER_THAN;         break;
        case IOP_MAJEQ:  complement_opname = LESS_THAN;            break;
        case IOP_EQ:     complement_opname = NOT_EQUAL;            break;
        case IOP_NEQ:    complement_opname = EQUAL;                break;
        default: throw;
    }

    apply(complement_opname, r, complete, complement);
    dd_edge boole(rsrg->getForestMDD());
    apply(COPY, complement, boole);
    apply(DIFFERENCE, rsrg->getRS(), boole, boole);
    setMDD(boole);
}

TreeTraceNode *Inequality::generateTrace(const vector<int> &state, TraceType traceTy) {
    CTLMDD *ctl = CTLMDD::getInstance();
    bool isSat = ctl->SatSetContains(getMDD(), state.data());

    // cout << "isSat="<<isSat<<" traceTy == TT_WITNESS is " << (traceTy == TT_WITNESS) << endl;
    CTL_ASSERT(isSat == (traceTy == TT_WITNESS));

    return new TreeTraceNode(state, this, traceTy);
}

/*---------------------
 ---  Fireability   ---
 ----------------------*/

void Fireability::createMDD() {
    dd_edge sat(rsrg->getForestMDD());
    // Generate the MDD corresponding to the union of the enabling conditions of the transitions
    for (int tr : transitions) {
        dd_edge enab_tr(rsrg->getForestMDD());
        apply(PRE_IMAGE, rsrg->getRS(), rsrg->getEventMxD(tr), enab_tr);
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
    return new TreeTraceNode(state, this, traceTy);
}

/*---------------------
 ---	BoolLiteral	---
 ----------------------*/

BoolLiteral::BoolLiteral(bool value) {
    this->value = value;
}

BoolLiteral::~BoolLiteral() {
}

void BoolLiteral::setValue(bool value) {
    this->value = value;
    clearMDD();
}
bool BoolLiteral::getValue() {
    return value;
}
void BoolLiteral::print(std::ostream &os) const {
    os << (value ? "true" : "false");
}
void BoolLiteral::createMDD() {
    FormulaPrinter<BoolLiteral> fp(this);
    CTLMDD *ctl = CTLMDD::getInstance();
    if (value) { // TRUE
        setMDD(rsrg->getRS());
    }
    else { // FALSE
        setMDD(dd_edge(rsrg->getForestMDD()));
    }
}

TreeTraceNode *BoolLiteral::generateTrace(const vector<int> &state, TraceType traceTy) {
    CTL_ASSERT(value == (traceTy == TT_WITNESS));
    return new TreeTraceNode(state, this, traceTy);
}


/*---------------------
 ---	Deadlock	---
 ----------------------*/

Deadlock::Deadlock(bool value) {
    this->value = value;
}

Deadlock::~Deadlock() {
}

void Deadlock::setValue(bool value) {
    this->value = value;
    clearMDD();
}
bool Deadlock::getValue() {
    return value;
}
void Deadlock::print(std::ostream &os) const {
    os << (value ? "deadlock" : "ndeadlock");
}

void Deadlock::createMDD() {
    CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge r(rsrg->getForestMDD());
    FormulaPrinter<Deadlock> fp(this);

    apply(PRE_IMAGE, rsrg->getRS(), rsrg->getNSF(), r);
    if (value) { // DEADLOCK
        // Compute RS \ r
        bool r_empty = isEmptySet(r);
        bool r_allRS = (r.getNode() == rsrg->getRS().getNode());
        if (r_empty) { // No state in r is a deadlock state, return RS
            setMDD(rsrg->getRS());
        }
        else if (r_allRS) { // Every state in r is a deadlock state, return 0
            setMDD(dd_edge(rsrg->getForestMDD()));
        }
        else {   // Compute RS \ r as a set difference
            apply(DIFFERENCE, rsrg->getRS(), r, r);
            setMDD(r);
        }
    }
    else { // NDEADLOCK
        setMDD(r);
    }
}

TreeTraceNode *Deadlock::generateTrace(const vector<int> &state, TraceType traceTy) {
    CTLMDD *ctl = CTLMDD::getInstance();
    bool isSat = ctl->SatSetContains(getMDD(), state.data());

    if (value) { // deadlock
        CTL_ASSERT(isSat == (traceTy == TT_WITNESS));
    }
    else { // not deadlock
        CTL_ASSERT(isSat == (traceTy == TT_WITNESS));
    }

    return new TreeTraceNode(state, this, traceTy);
}

/*-------------------------
 ---    Reachability    ---
 --------------------------*/

Reachability::Reachability(StateFormula *subf, prop_type type) {
    this->subf = subf;
    this->type = type;
    subf->addOwner();
}
Reachability::~Reachability() {
    subf->removeOwner();
}
void Reachability::createMDD() {
    CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge f1 = subf->getMDD();
    bool f1_empty = isEmptySet(f1);
    bool f1_allRS = (f1.getNode() == rsrg->getRS().getNode());
    bool result;
    switch (type) {
        case RPT_INVARIANTLY:       result = f1_allRS;      break;
        case RPT_POSSIBILITY:       result = !f1_empty;     break;
        case RPT_IMPOSSIBILITY:     result = f1_empty;      break;
    }

    FormulaPrinter<Reachability> fp(this);
    setMDD(result ? rsrg->getRS() : dd_edge(rsrg->getForestMDD()));
}
TreeTraceNode *Reachability::generateTrace(const vector<int> &state, TraceType traceTy) {
    throw "Unimplemented";
    // Should show a state in *subf that satisfyes/does not satisfy the invariant.
}
void Reachability::print(std::ostream &os) const {
    os << (type == RPT_POSSIBILITY ? "P" : (type == RPT_IMPOSSIBILITY ? "N" : "I")) << " ";
    subf->print(os);
}


/*-------------------------
 ---	CTLStateFormula	---
 --------------------------*/
CTLStateFormula::CTLStateFormula(StateFormula *formula1, StateFormula *formula2) {
    this->formula1 = formula1;
    this->formula2 = formula2;
    this->op = CTLOP_EU;
    formula1->addOwner();
    formula2->addOwner();
}
CTLStateFormula::CTLStateFormula(StateFormula *formula1, CTLStateFormula::op_type op) {
    this->formula1 = formula1;
    this->formula2 = NULL;
    this->op = op;
    formula1->addOwner();
}
CTLStateFormula::~CTLStateFormula() {
    if (formula1 != NULL) formula1->removeOwner();
    if (formula2 != NULL) formula2->removeOwner();

}
StateFormula *CTLStateFormula::getFormula1() const {
    return formula1;
}
void CTLStateFormula::setFormula1(StateFormula *formula1) {
    this->formula1 = formula1;
    clearMDD();
}
StateFormula *CTLStateFormula::getFormula2() const {
    return formula2;
}
void CTLStateFormula::setFormula2(StateFormula *formula2) {
    this->formula2 = formula2;
    clearMDD();
}

CTLStateFormula::op_type CTLStateFormula::getOp() const {
    return op;
}

void CTLStateFormula::setOp(CTLStateFormula::op_type op) {
    this->op = op;
    clearMDD();
}

void CTLStateFormula::print(std::ostream &os) const {
    if (op == CTLOP_EU)
        os << "E [" << (*formula1) << " U " << (*formula2) << "]";
    else {
        os << (op == CTLOP_EG ? "E G " : (op == CTLOP_EF ? "E F " : "E X "));
        os << (*formula1);
    }
}

void CTLStateFormula::createMDD() {
    switch (op) {
        case CTLOP_EU:
            createEUMDD();
            break;
        case CTLOP_EG:
            createEGMDD();
            break;
        case CTLOP_EF:
            createEFMDD(formula1);
            break;
        case CTLOP_EX:
            createEXMDD();
            break;
    }
}

void CTLStateFormula::createEXMDD() {
    CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge f1 = formula1->getMDD();
    bool f1_empty = isEmptySet(f1);
    // bool f1_allRS = (f1.getNode() == rsrg->getRS().getNode());

    FormulaPrinter<CTLStateFormula> fp(this);
    if (f1_empty) { // E X False
        fp.optimized("[E X FALSE]");
        setMDD(dd_edge(rsrg->getForestMDD()));
        return;
    }

    // Note: for non-ergodic RS, the E X true is only valid
    // for non-dead states. Therefore, the E X true case
    // is not different from the general case.
    dd_edge r(rsrg->getForestMDD());

//     else if (f1_allRS) { // E X True
// #warning Ricontrollare x il deadlock.
//     	setMDD(rsrg->getRS());
//     }
//     else { // E X f1
    // E X f1  or  E X true
    apply(PRE_IMAGE, f1, rsrg->getNSF(), r);
    setMDD(r);
    // }
}

void CTLStateFormula::createEGMDD() {
    CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge f1 = formula1->getMDD();
    bool f1_empty = isEmptySet(f1);
    // bool f1_allRS = (f1.getNode() == rsrg->getRS().getNode());

    FormulaPrinter<CTLStateFormula> fp(this);
    if (f1_empty) { // E G false
        fp.optimized("[E G FALSE]");
        setMDD(dd_edge(rsrg->getForestMDD()));
        return;
    }
    // NOTE: the case E G true is not trivial for non-ergodic RS.
    // Therefore, we leave it with the general case and do not
    // terat it separately.
    dd_edge r(rsrg->getForestMDD());
    dd_edge pre_r(rsrg->getForestMDD());
    dd_edge pred_of_r(rsrg->getForestMDD());
    r = f1;

    // int it_cnt1 = 0;
    int it_cnt2 = 0;
    {
        // dd_edge PtoP(rsrg->getNSF()->getForest());
        // int allMinus1[npl+1], *pAll = allMinus1;
        // std::fill_n(allMinus1, npl+1, -1);
        // dd_edge all(f1->getForest());
        // f1->getForest()->createEdge(&pAll, 1, all);
        // apply(CROSS, *f1, all, PtoP);
        // dd_edge Psf(rsrg->getNSF()->getForest());
        // cout << "adesso eseguo l'intersezione...  " << endl;
        // apply(INTERSECTION, PtoP, rsrg->getNSF(), Psf);

        // dd_edge *result = new dd_edge(*f1);
        // cout << "inizio iterazione  " << endl;
        // clock_t beginEG = clock();
        // while(true) {
        //     dd_edge prev = *result;
        //     apply(PRE_IMAGE, *result, Psf, *result);
        //     // apply(REVERSE_REACHABLE_DFS, *result, Psf, *result);
        //     // apply(PRE_IMAGE, *result, Psf, *result);

        //     it_cnt1++;
        //     if (*result == prev)
        //         break;
        // }
        // cout << "  Total: " << double(clock() - beginEG)/CLOCKS_PER_SEC << endl;
        // cout << "R1 = " << result->getNode() << " in " << it_cnt1 << " iterations." << endl;
        // // rsrg->show_markings(cout, *result);
        // setMDD(result);
    }

    dd_edge non_deadlock_states(rsrg->getForestMDD());
    apply(PRE_IMAGE, rsrg->getRS(), rsrg->getNSF(), non_deadlock_states);
    dd_edge deadlock_f1 = f1 - non_deadlock_states;

    do {
        pre_r = r; // risultati ottenuti all'iterazione precedente
        apply(PRE_IMAGE, r, rsrg->getNSF(), pred_of_r); // 		}
        apply(INTERSECTION, pred_of_r, r, r);
        r = r + deadlock_f1;
        it_cnt2++;
        if (print_intermediate_expr()) {
            cout << "EG: step=" << it_cnt2 << ",  SAT size=" << fixed << r.getCardinality() << endl;

            /*enumerator i(*r);
            int nvar = rsrg->getDomain()->getNumVariables();
            while(i != 0) { // for each marking in the sat set
                int j;
                for(j=1; j <= nvar; j++) { // for each place
                    int val = *(i.getAssignments() + j);
                    const std::string& s = rsrg->getPL(j - 1);
                    if(val!=0) 
                        cout << s << "(" << val << ")";
                }
                ++i;
                cout << endl;
            }
            cout << endl;*/
        }
        cout.unsetf(ios_base::floatfield);
    }
    while (r != pre_r);
    if (print_intermediate_expr()) {
        cout << "R2 = " << r.getNode() << " in " << it_cnt2 << " iterations." << endl;
    }
    // rsrg->show_markings(cout, *r);
    setMDD(r);
}


void CTLStateFormula::createEFMDD(StateFormula *formula) {
    CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge f1 = formula->getMDD();

    FormulaPrinter<CTLStateFormula> fp(this);
    bool f1_empty = isEmptySet(f1);
    bool f1_allRS = (f1.getNode() == rsrg->getRS().getNode());

    if (f1_allRS) { // E F true  ->  true
        fp.optimized("[E F TRUE]");
        setMDD(rsrg->getRS());
        return;
    }
    if (f1_empty) { // E F false  ->  false
        fp.optimized("[E F FALSE]");
        setMDD(dd_edge(rsrg->getForestMDD()));
        return;
    }

    // int it_cnt1 = 0, it_cnt2 = 0;
    {
        // // dd_edge RStoP(rsrg->getNSF()->getForest());
        // // apply(CROSS, *f1, *(rsrg->getRS()), RStoP);
        // // dd_edge Psf(rsrg->getNSF()->getForest());
        // // apply(INTERSECTION, RStoP, rsrg->getNSF(), Psf);

        // dd_edge *result = new dd_edge(*f1);
        // // while(true) {
        //     // dd_edge prev = *result;
        //     // apply(REVERSE_REACHABLE_DFS, *result, Psf, *result);
        // apply(REVERSE_REACHABLE_DFS, *result, rsrg->getNSF(), *result);
        //     // apply(PRE_IMAGE, *result, Psf, *result);
        //     // apply(UNION, prev, *result, *result);

        // //     it_cnt1++;
        // //     if (*result == prev)
        // //         break;
        // // }
        // cout << "R1 = " << result->getNode() << " in " << it_cnt1 << " iterations." << endl;
        // // rsrg->show_markings(cout, *result);
        // setMDD(result);
    }

    // dd_edge *r(rsrg->getForestMDD());
    // dd_edge pre_r(rsrg->getForestMDD());
    // dd_edge pred_of_r(rsrg->getForestMDD());
    // (*r) = (*f1);
    // do {
    //     pre_r = (*r); // risultati ottenuti all'iterazione precedente
    //     apply(PRE_IMAGE, (*r), rsrg->getNSF(), pred_of_r);
    //     apply(UNION, pred_of_r, (*r), (*r));
    //     // it_cnt2++;
    // }
    // while ((*r) != pre_r);
    // // cout << "R2 = " << r->getNode() << " in " << it_cnt2 << " iterations." << endl;
    // // rsrg->show_markings(cout, *r);
    // setMDD(r);

    const int NUM_PRESTEPS = 0; // Number of steps of pre-image before appling saturation
    dd_edge result = f1;
    for (int i = 0; /*i < NUM_PRESTEPS*/true; i++) {
        dd_edge current(result);
        // Y' = Y union (Y * N^-1)
        apply(PRE_IMAGE, result, rsrg->getNSF(), result);
        apply(UNION, current, result, result);
        if (result == current) {
            if (print_intermediate_expr()) {
                cout << "      " << i << " steps: ";
            }
            setMDD(result);
            return;
        }
    }
    // Solve using backward reachability with saturation
    // (slightly more costly than PRE_IMAGE + UNION)
    apply(REVERSE_REACHABLE_DFS, result, rsrg->getNSF(), result);
    if (print_intermediate_expr()) {
        cout << "      saturation: ";
    }
    setMDD(result);
}


void CTLStateFormula::createEUMDD() {
    // E formula1 U formula2
    CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge f2 = formula2->getMDD();
    bool f2_empty = isEmptySet(f2);
    bool f2_allRS = (f2.getNode() == rsrg->getRS().getNode());

    if (f2_empty) { // E f1 U false  ->  false
        FormulaPrinter<CTLStateFormula> fp(this);
        fp.optimized("[E f1 U false]");
        setMDD(dd_edge(rsrg->getForestMDD()));
        return;
    }
    else if (f2_allRS) { // E f1 U true  ->  true
        FormulaPrinter<CTLStateFormula> fp(this);
        fp.optimized("[E f1 U true]");
        setMDD(rsrg->getRS());
        return;
    }

    dd_edge f1 = formula1->getMDD();
    bool f1_empty = isEmptySet(f1);
    bool f1_allRS = (f1.getNode() == rsrg->getRS().getNode());

    if (f1_allRS) { // E true U f2  ->  E F f2
        createEFMDD(formula2);
        return;
    }
    else if (f1_empty) { // E false U f2  ->  f2
        FormulaPrinter<CTLStateFormula> fp(this);
        fp.optimized("[E false U f2]");
        setMDD(f2);
        return;
    }

//     dd_edge *r(rsrg->getForestMDD());
//     dd_edge pre_r(rsrg->getForestMDD());
//     dd_edge pred_of_r(rsrg->getForestMDD());
//     dd_edge pred_of_r_f1(rsrg->getForestMDD());
//     (*r) = (*f2);
// #warning Aggiungere casi limite per EU
//     do {
//         pre_r = (*r); // risultati ottenuti all'iterazione precedente
//         apply(PRE_IMAGE, (*r), rsrg->getNSF(), pred_of_r);
//         apply(INTERSECTION, pred_of_r, (*f1), pred_of_r_f1);
//         apply(UNION, pred_of_r_f1, (*r), (*r));
//     }
//     while ((*r) != pre_r);
//     setMDD(r);

    FormulaPrinter<CTLStateFormula> fp(this);
    dd_edge result = f2;
    while (true) {
        /*cout << "Markings that now satisfy the EU: \n";
        enumerator i(*result);
        int nvar = rsrg->getDomain()->getNumVariables();
        while(i != 0) { // for each marking in the sat set
            int j;
            for(j=1; j <= nvar; j++) { // for each place
                int val = *(i.getAssignments() + j);
                const std::string& s = rsrg->getPL(j - 1);
                if(val!=0) 
                    cout << s << "(" << val << ")";
            }
            ++i;
            cout << endl;
        }
        cout << endl;*/

        dd_edge current(result), pred(result);
        // R' = R union (F1 intersect (R * N^-1))
        apply(PRE_IMAGE, result, rsrg->getNSF(), pred);
        apply(INTERSECTION, pred, f1, pred);
        apply(UNION, pred, result, result);
        if (result == current)
            break;
    }
    setMDD(result);
}

TreeTraceNode *CTLStateFormula::generateTrace(const vector<int> &state0, TraceType traceTy) {
    CTLMDD *ctl = CTLMDD::getInstance();
    forest *mdd_forest = rsrg->getForestMDD();

    TreeTraceNode *ttn = new TreeTraceNode(state0, this, traceTy), *current_ttn = ttn;
    if (traceTy == TT_COUNTEREXAMPLE) {
        // We cannot generate a counterexample of an existential CTL formula
        ttn->set_ECTL_cntexample();
        return ttn; // no counter-example generated
    }

    // Generate the witness of an existential formula
    switch (op) {
        case CTLOP_EF: {
            // E F Phi -> generate a trace of form:
            //   not Phi -> not Phi -> ... -> not Phi -> Phi
            dd_edge Phi = getFormula1()->getMDD();

            vector<dd_edge> intermDD;
            intermDD.push_back(Phi);
            while (!ctl->SatSetContains(intermDD.back(), state0.data())) {
                // Step back until we find the marking in the sat set
                dd_edge pre_image(mdd_forest);

                apply(PRE_IMAGE, intermDD.back(), rsrg->getNSF(), pre_image);
                intermDD.push_back(pre_image);
                // TODO: add a bound check
            }
            // Pick the trace from marking to any state in intermDD[0]
            TraceType tt0 = (intermDD.size() > 1) ? oppositeTraceType(traceTy) : traceTy;
            ttn->set_sub_trace1(getFormula1()->generateTrace(state0, tt0));
            vector<int> state = state0;
            for (ssize_t step = intermDD.size() - 2; step >= 0; step--) {
                dd_edge dd_of_state(mdd_forest);
                const int *vlist = state.data();
                mdd_forest->createEdge(&vlist, 1, dd_of_state);
                apply(POST_IMAGE, dd_of_state, rsrg->getNSF(), dd_of_state);
                apply(INTERSECTION, intermDD[step], dd_of_state, dd_of_state);
                // Now take a sample state from dd_of_state
                enumerator it(dd_of_state);
                CTL_ASSERT(it);
                const int *tmp = it.getAssignments();
                std::copy(tmp, tmp + npl + 1, state.begin());

                // Add this intermediate state to the trace.
                TreeTraceNode *next_ttn = new TreeTraceNode(state, this, traceTy);
                TraceType tt = (step > 0) ? oppositeTraceType(traceTy) : traceTy;
                next_ttn->set_sub_trace1(getFormula1()->generateTrace(state, tt));
                current_ttn->set_next(next_ttn);
                current_ttn = next_ttn;
            }
        }
        break;

        case CTLOP_EG: {
            // E G Phi -> generate a circular trace of form:
            //  /-> Phi -> Phi -> ... -> Phi -> Phi -\
            //  \------------------------------------/
            dd_edge Phi = getFormula1()->getMDD();
            CTL_ASSERT(ctl->SatSetContains(Phi, state0.data()));
            dd_edge dd_s0(mdd_forest);
            const int *vlist = state0.data();
            mdd_forest->createEdge(&vlist, 1, dd_s0);

            vector<dd_edge> intermDD;
            intermDD.push_back(dd_s0);
            do {
                dd_edge pre_image(mdd_forest);
                apply(PRE_IMAGE, intermDD.back(), rsrg->getNSF(), pre_image);
                apply(INTERSECTION, pre_image, Phi, pre_image);
                intermDD.push_back(pre_image);
            }
            while (!ctl->SatSetContains(intermDD.back(), state0.data()));

            // Sample a circular trace from s0 to s0, skipping the last s0
            // All trace node should evaluate Phi
            ttn->set_sub_trace1(getFormula1()->generateTrace(state0, traceTy));
            ttn->set_circular();
            vector<int> state = state0;
            for (ssize_t step = intermDD.size() - 2; step > 0; step--) {
                // Get the set of next states departing from *state
                // and remaining in the Phi set of intermDD
                dd_edge dd_of_state(mdd_forest);
                const int *vlist = state.data();
                mdd_forest->createEdge(&vlist, 1, dd_of_state);
                apply(POST_IMAGE, dd_of_state, rsrg->getNSF(), dd_of_state);
                apply(INTERSECTION, intermDD[step], dd_of_state, dd_of_state);
                // Pick the first state
                enumerator it(dd_of_state);
                CTL_ASSERT(it);
                const int *tmp = it.getAssignments();
                std::copy(tmp, tmp + npl + 1, state.begin());
                // Add this intermediate state to the trace.
                TreeTraceNode *next_ttn = new TreeTraceNode(state, this, traceTy);
                next_ttn->set_sub_trace1(getFormula1()->generateTrace(state, traceTy));
                current_ttn->set_next(next_ttn);
                current_ttn = next_ttn;
            }
        }
        break;

        case CTLOP_EU: {
            // E Phi1 U Phi2 generates a trace
            // Phi1 -> Phi1 -> ... -> Phi1 -> Phi2
            //  s0                             sn
            dd_edge Phi1 = getFormula1()->getMDD();
            dd_edge Phi2 = getFormula2()->getMDD();

            vector<dd_edge> intermDD;
            intermDD.push_back(Phi2);
            while (!ctl->SatSetContains(intermDD.back(), state0.data())) {
                // Step back until we find the marking in the sat set
                dd_edge pre_image(mdd_forest);
                apply(PRE_IMAGE, intermDD.back(), rsrg->getNSF(), pre_image);
                apply(INTERSECTION, intermDD.back(), Phi1, intermDD.back());
                intermDD.push_back(pre_image);
            }
            // Pick the trace from s0 to any state in intermDD[i]
            if (intermDD.size() == 1) {
                // s0 |= Phi2 and ends immediately the until
                ttn->set_sub_trace1(getFormula2()->generateTrace(state0, traceTy));
            }
            else {
                // TODO: is not formula1, but formula1 AND NOT formula2
                ttn->set_sub_trace1(getFormula1()->generateTrace(state0, traceTy));
                vector<int> state = state0;
                for (ssize_t step = intermDD.size() - 2; step >= 0; step--) {
                    dd_edge dd_of_state(mdd_forest);
                    const int *vlist = state.data();
                    mdd_forest->createEdge(&vlist, 1, dd_of_state);
                    apply(POST_IMAGE, dd_of_state, rsrg->getNSF(), dd_of_state);
                    // if (step > 0)
                    apply(INTERSECTION, intermDD[step], dd_of_state, dd_of_state);
                    // else
                        // apply(DIFFERENCE, dd_of_state, intermDD[step], dd_of_state); // Modif. 5 may 2014
                    // Now take a sample state from dd_of_state
                    enumerator it(dd_of_state);
                    CTL_ASSERT(it);
                    const int *tmp = it.getAssignments();
                    std::copy(tmp, tmp + npl + 1, state.begin());
                    // Add this intermediate state to the trace.
                    TreeTraceNode *next_ttn = new TreeTraceNode(state, this, traceTy);
                    if (step > 0)
                        // TODO: is not formula1, but formula1 AND NOT formula2
                        next_ttn->set_sub_trace1(getFormula1()->generateTrace(state, traceTy));
                    else
                        next_ttn->set_sub_trace1(getFormula2()->generateTrace(state, traceTy));
                    current_ttn->set_next(next_ttn);
                    current_ttn = next_ttn;
                }
            }
        }
        break;

        case CTLOP_EX: {
            // Generates a simple trace:
            //   (ANY) -> s1 |= Phi
            dd_edge Phi = getFormula1()->getMDD();
            vector<int> state1(npl + 1);
            dd_edge dd_s0(mdd_forest);
            const int *vlist = state0.data();
            mdd_forest->createEdge(&vlist, 1, dd_s0);
            apply(POST_IMAGE, dd_s0, rsrg->getNSF(), dd_s0);
            apply(INTERSECTION, Phi, dd_s0, dd_s0);
            enumerator it(dd_s0);
            CTL_ASSERT(it);
            const int *tmp = it.getAssignments();
            std::copy(tmp, tmp + npl + 1, state1.begin());

            TreeTraceNode *ttn1 = new TreeTraceNode(state1, this, traceTy);
            ttn1->set_sub_trace1(getFormula1()->generateTrace(state1, traceTy));
            ttn->set_next(ttn1);
        }
        break;
    }
    return ttn;
}


/*-----------------------------
 ---	LogicalFormula	---
 ------------------------------*/
LogicalFormula::LogicalFormula(StateFormula *formula1, StateFormula *formula2, op_type op) {
    this->formula1 = formula1;
    this->formula2 = formula2;
    this->op = op;
    formula1->addOwner();
    formula2->addOwner();
}
LogicalFormula::LogicalFormula(StateFormula *formula1) {
    this->formula1 = formula1;
    this->formula2 = NULL;
    this->op = CBF_NOT;
    formula1->addOwner();
}
LogicalFormula::~LogicalFormula() {
    if (formula1 != NULL) formula1->removeOwner();
    if (formula2 != NULL) formula2->removeOwner();

}
StateFormula *LogicalFormula::getFormula1() const {
    return formula1;
}
void LogicalFormula::setFormula1(StateFormula *formula1) {
    this->formula1 = formula1;
    clearMDD();
}
StateFormula *LogicalFormula::getFormula2() const {
    return formula2;
}
void LogicalFormula::setFormula2(StateFormula *formula2) {
    this->formula2 = formula2;
    clearMDD();
}

LogicalFormula::op_type LogicalFormula::getOp() const {
    return op;
}

void LogicalFormula::setOp(LogicalFormula::op_type op) {
    this->op = op;
    clearMDD();
}

void LogicalFormula::print(std::ostream &os) const {
    if (op == CBF_NOT)
        os << "(not " << (*formula1) << ")";
    else
        os << "(" << (*formula1) << ((op == CBF_AND) ? " and " : " or ")
           << (*formula2) << ")";
}

void LogicalFormula::createMDD() {
    CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge f1 = formula1->getMDD();
    bool f1_empty = isEmptySet(f1);
    bool f1_allRS = (f1.getNode() == rsrg->getRS().getNode());

    switch (op) {
        case CBF_NOT: {
            FormulaPrinter<LogicalFormula> fp(this);
            if (f1_allRS) { // NOT TRUE -> FALSE
                fp.optimized("[NOT TRUE]");
                setMDD(dd_edge(rsrg->getForestMDD()));
            }
            else if (f1_empty) { // NOT FALSE -> TRUE
                fp.optimized("[NOT FALSE]");
                setMDD(rsrg->getRS());
            }
            else { // NOT f1
                dd_edge rs = rsrg->getRS();
                apply(DIFFERENCE, rs, f1, rs);
                setMDD(rs);
            }
            break;
        }

        case CBF_AND: {
            if (f1_allRS) { // TRUE and f2 -> f2
                dd_edge f2 = formula2->getMDD();
                FormulaPrinter<LogicalFormula> fp(this);
                fp.optimized("[TRUE and f2]");
                setMDD(f2);
            }
            else if (f1_empty) { // FALSE AND f2 -> FALSE
                FormulaPrinter<LogicalFormula> fp(this);
                fp.optimized("[FALSE AND f2]");
                setMDD(dd_edge(rsrg->getForestMDD()));
            }
            else { // f1 AND f2
                CTL_ASSERT(formula2 != NULL);
                // TODO: check again why this optimization fails.
                // ./bin/RGMEDD ~/Desktop/old_home/sp116921/BenchKit/INPUTS/FMS-PT-005/model -B 6 -C -f CTL.txt
                // query: A G (P12 = 1 and A G P12 = 1)
                // when getting the counter-example

                if (typeid(*formula2) == typeid(LogicalFormula)) {
                    LogicalFormula *cbf2 = dynamic_cast<LogicalFormula *>(formula2);
                    if (cbf2->getOp() == LogicalFormula::CBF_NOT) {
                        // f1 AND NOT f2 can be computed as a SETDIFF: s1 \ s2
                        dd_edge fdiff = cbf2->getFormula1()->getMDD();
                        dd_edge r(rsrg->getForestMDD());
                        FormulaPrinter<LogicalFormula> fp(this);
                        fp.optimized("[f1 AND NOT f2]");
                        apply(DIFFERENCE, f1, fdiff, r);
                        setMDD(r);
                        break;
                    }
                }
                dd_edge f2 = formula2->getMDD();
                dd_edge r(rsrg->getForestMDD());
                FormulaPrinter<LogicalFormula> fp(this);
                apply(INTERSECTION, f1, f2, r);
                setMDD(r);
            }
            break;
        }

        case CBF_OR: {
            if (f1_allRS) { // TRUE OR f2 -> TRUE
                FormulaPrinter<LogicalFormula> fp(this);
                fp.optimized("[TRUE OR f2]");
                setMDD(rsrg->getRS());
            }
            else if (f1_empty) { // FALSE OR f2 -> f2
                dd_edge f2 = formula2->getMDD();
                FormulaPrinter<LogicalFormula> fp(this);
                fp.optimized("[FALSE OR f2]");
                setMDD(f2);
            }
            else { // f1 OR f2
                dd_edge f2 = formula2->getMDD();
                FormulaPrinter<LogicalFormula> fp(this);
                dd_edge r(rsrg->getForestMDD());
                apply(UNION, f1, f2, r);
                setMDD(r);
            }
            break;
        }
    }
}

TreeTraceNode *LogicalFormula::generateTrace(const vector<int> &state0, TraceType traceTy) {
    CTLMDD *ctl = CTLMDD::getInstance();
    CTL_ASSERT(ctl->SatSetContains(getMDD(), state0.data()) == (traceTy == TT_WITNESS));

    if (op == CBF_NOT) {
        // Counter-example for:  NOT Phi  is a witness for Phi
        // Witness for:  NOT Phi  is a counter-example for Phi
        return getFormula1()->generateTrace(state0, oppositeTraceType(traceTy));
        // TreeTraceNode *not_ttn = new TreeTraceNode(state0, this, traceTy);
        // not_ttn->set_sub_trace1(getFormula1()->generateTrace(state0, oppositeTraceType(traceTy)));
        // return not_ttn;
    }

    // Counter-example for:  Phi_1 AND Phi_2 -> Either !Phi_1 or !Phi_2 (if false & valid)
    // Witness for:  Phi_1 AND Phi_2 -> Both Phi_1 and Phi_2 must be true

    // Counter-example for:  Phi_1 OR Phi_2 -> Both Phi_1 and Phi_2 must be false
    // Witness for:  Phi_1 OR Phi_2 -> Either Phi_1 or Phi_2

    TreeTraceNode *ttn = new TreeTraceNode(state0, this, traceTy);
    if (traceTy == TT_WITNESS) {
        if (op == CBF_AND) {
            // cout << "ctl->SatSetContains(getFormula1()->getMDD(), state0.data()) = " << ctl->SatSetContains(getFormula1()->getMDD(), state0.data()) << endl;
            // cout << "ctl->SatSetContains(getFormula2()->getMDD(), state0.data()) = " << ctl->SatSetContains(getFormula2()->getMDD(), state0.data()) << endl;
            CTL_ASSERT(ctl->SatSetContains(getFormula1()->getMDD(), state0.data()));
            CTL_ASSERT(ctl->SatSetContains(getFormula2()->getMDD(), state0.data()));
            ttn->set_sub_trace1(getFormula1()->generateTrace(state0, traceTy));
            ttn->set_sub_trace2(getFormula2()->generateTrace(state0, traceTy));
        }
        else { // (op == CBF_OR)
            bool isSat1 = ctl->SatSetContains(getFormula1()->getMDD(), state0.data());
            if (isSat1)
                ttn->set_sub_trace1(getFormula1()->generateTrace(state0, traceTy));
            else {
                CTL_ASSERT(ctl->SatSetContains(getFormula2()->getMDD(), state0.data()));
                ttn->set_sub_trace1(getFormula2()->generateTrace(state0, traceTy));
            }
        }
    }
    else { // traceTy == TT_COUNTEREXAMPLE
        if (op == CBF_AND) {
            bool isSat1 = ctl->SatSetContains(getFormula1()->getMDD(), state0.data());
            if (!isSat1)
                ttn->set_sub_trace1(getFormula1()->generateTrace(state0, traceTy));
            else {
                CTL_ASSERT(!ctl->SatSetContains(getFormula2()->getMDD(), state0.data()));
                ttn->set_sub_trace1(getFormula2()->generateTrace(state0, traceTy));
            }
        }
        else { // (op == CBF_OR)
            CTL_ASSERT(!ctl->SatSetContains(getFormula1()->getMDD(), state0.data()));
            CTL_ASSERT(!ctl->SatSetContains(getFormula2()->getMDD(), state0.data()));
            ttn->set_sub_trace1(getFormula1()->generateTrace(state0, traceTy));
            ttn->set_sub_trace2(getFormula2()->generateTrace(state0, traceTy));
        }
    }
    return ttn;
}

/*---------------------
 ---	StateFormula	---
 -----------------------*/
StateFormula::StateFormula() {
    countOwner = 0;
}
StateFormula::~StateFormula() {
}
const dd_edge &StateFormula::getMDD() {
    if (!hasStoredMDD()) {
        // Setting computedMDD here avoids infinite recursions when Meddly ends the memory
        computedMDD = true;
        createMDD();
        // rsrg->show_markings(cout, SatMDD);
    }
    CTL_ASSERT(hasStoredMDD());
    return SatMDD;
}
bool StateFormula::hasStoredMDD() const {
    return computedMDD;
}
void StateFormula::setMDD(dd_edge newMDD) {
    SatMDD = newMDD;
    computedMDD = true; 
}
void StateFormula::clearMDD() {
    SatMDD.clear();
    computedMDD = false;
}
void StateFormula::addOwner() {
    countOwner++;
    // cout << "addOwner("<<*(this)<<" "<<countOwner<<endl;
}
void StateFormula::removeOwner() {
    countOwner--;
    // cout << "removeOwner("<<*(this)<<" "<<countOwner<<endl;
    if (countOwner < 1)
        delete this;
}
bool StateFormula::isBoolFormula() const {
    return true;
}
bool StateFormula::isIntFormula() const {
    return false;
}


} // end namespace ctlmdd

