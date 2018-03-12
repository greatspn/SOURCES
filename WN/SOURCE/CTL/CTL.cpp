/*
 * CTL.cpp
 *
 *  Created on: 11/gen/2011
 *      Author: Erica Turin
 */

#include "CTL.h"
#undef GREATER_THAN

using namespace std;

extern int Max_Token_Bound;
extern int npl;

inline bool print_intermediate_expr() {
    return !running_for_MCC();
}

namespace ctlmdd {
/*-----------------
 ---	CTLMDD	---
 ------------------*/
CTLMDD *CTLMDD::instance = NULL;

void CTLMDD::CTLinit(dd_edge *rs, dd_edge *initMark,
                     dd_edge *nsf, domain *d, const int *map_vec) {
    my_mdd_forest = rs->getForest();
    my_domain = d;
    this->rs = rs;
    this->nsf = nsf;
    this->initMark = initMark;
    this->pl2mdd = map_vec;
    this->mdd2pl = new int [npl];
    for (int i = 0; i < npl; i++) {
        mdd2pl[pl2mdd[i]] = i;
    }

    int nvar = my_domain->getNumVariables();
    ins = new int *;
    *ins = new int[nvar + 1];
    for (int j = 1; j <= nvar; j++) {
        ins[0][j] = DOUBLELEVEL ? -2 : -1;
    }
    forest::policies fp(DOUBLELEVEL); // false: not a relation


    if (DOUBLELEVEL)
        fp.setIdentityReduced();
    else
        fp.setFullyReduced();
    //fp.setCompactStorage();
    fp.setOptimistic();
    my_mtmdd_forest = my_domain->createForest(DOUBLELEVEL, forest::REAL, forest::MULTI_TERMINAL, fp);
    //if (DOUBLELEVEL)
    //  my_mtmdd_forest->setReductionRule(forest::IDENTITY_REDUCED);
    //my_mtmdd_forest->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);
    //my_mtmdd_forest->setNodeDeletion(forest::OPTIMISTIC_DELETION);
}

domain *CTLMDD::getDomain() const {
    return my_domain;
}

forest *CTLMDD::getMTMDDForest() const {
    return my_mtmdd_forest;
}
dd_edge *CTLMDD::getNsf() const {
    return nsf;
}
dd_edge *CTLMDD::getInitMark() const {
    return initMark;
}
forest *CTLMDD::getMDDForest() const {
    return my_mdd_forest;
}
dd_edge *CTLMDD::getRS() const {
    return rs;
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
            cout << (first ? "" : ", ") << tabp[mdd2pl[i]].place_name;
            cout << "(" << marking[i + 1] << ")";
            first = false;
        }
    }
}

CTLMDD::~CTLMDD() {
    if (ins != NULL) {
        delete[] ins[0];
        delete ins;
    }
    if (mdd2pl != NULL) {
        delete[] mdd2pl;
    }
}

void CTLMDD::getStatistic() {

    // rs->getForest()->garbageCollect();
    // nsf->getForest()->garbageCollect();
    // my_mtmdd_forest->garbageCollect();

    cout << "========================== CTL MEMORY ===========================" << endl;

    cout << " RS nodes:              " << rs->getNodeCount() << endl;
    cout << " Forest(RS) nodes:      " << rs->getForest()->getCurrentNumNodes() << " actives, "
         << rs->getForest()->getPeakNumNodes() << " peak, "
         << rs->getForest()->getStats().num_compactions << " compactions." << endl;
    cout << " Forest(RS) size:       " << rs->getForest()->getCurrentMemoryUsed() << " Bytes now, "
         << rs->getForest()->getPeakMemoryUsed() << " Bytes peak." << endl;

    cout << " Potential RG nodes:    " << nsf->getNodeCount() << endl;
    cout << " Forest(RG) nodes:      " << nsf->getForest()->getCurrentNumNodes() << " actives, "
         << nsf->getForest()->getPeakNumNodes() << " peak, "
         << nsf->getForest()->getStats().num_compactions << " compactions." << endl;
    cout << " Forest(RG) size:       " << nsf->getForest()->getCurrentMemoryUsed() << " Bytes now, "
         << nsf->getForest()->getPeakMemoryUsed() << " Bytes peak." << endl;

    cout << " Forest(RG+real) nodes: " << my_mtmdd_forest->getCurrentNumNodes() << " actives, "
         << my_mtmdd_forest->getPeakNumNodes() << " peak, "
         << my_mtmdd_forest->getStats().num_compactions << " compactions." << endl;
    cout << " Forest(RG+real) size:  " << my_mtmdd_forest->getCurrentMemoryUsed() << " Bytes now, "
         << my_mtmdd_forest->getPeakMemoryUsed() << " Bytes peak." << endl;

    // cout << "\n\n================== MEMORY CTL==================" << endl;
    // cout << "Peak Used Memory (RS): " << rs->getForest()->getPeakMemoryUsed() << "B" << endl;
    // cout << "Current Used  Memory (RS): " << rs->getForest()->getCurrentMemoryUsed() << "B" << endl;
    // cout << "Peak Used Memory (potential RG): " << nsf->getForest()->getPeakMemoryUsed() << "B" << endl;
    // cout << "Current Used  Memory (potential RG): " << nsf->getForest()->getCurrentMemoryUsed() << "B" << endl;
    // cout << "Peak Used Memory (potential RG + Real): " << my_mtmdd_forest->getPeakMemoryUsed() << "B" << endl;
    // cout << "Current Used  Memory (potential RG +Real): " << my_mtmdd_forest->getCurrentMemoryUsed() << "B" << endl;
    // cout << "===============================================\n" << endl;

    // cout << "==================== NODE ======================" << endl;
    // cout << "Peak Node Number (RS): " << rs->getForest()->getPeakNumNodes() << endl;
    // cout << "Used Node Number (RS): " << rs->getForest()->getCurrentNumNodes() << endl;
    // cout << "Peak Node Number (potential RG): " << nsf->getForest()->getPeakNumNodes() << endl;
    // cout << "Used Node Number (potential RG): " << nsf->getForest()->getCurrentNumNodes() << endl;
    // cout << "Peak Node Number (potential RG + Real): " << my_mtmdd_forest->getPeakNumNodes() << endl;
    // cout << "Used Node Number (potential RG + Real): " << my_mtmdd_forest->getCurrentNumNodes() << endl;
    // cout << "================================================\n" << endl;
}


template<typename Printable>
struct FormulaPrinter {
    Printable *e;
    clock_t start;
    const char *optMsg;
    FormulaPrinter(Printable *_e) : e(_e), optMsg(NULL) {
        if (running_for_MCC())
            return;
        start = clock();
        cout << "Eval: " << (*e) << endl << flush;
    }
    ~FormulaPrinter() {
        if (running_for_MCC())
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
    void stat(CTLFormula *f) {
        if (running_for_MCC())
            return;
        cout << "  card = " << f->getMDD()->getCardinality();
        if (f->getMDD()->getNode() == CTLMDD::getInstance()->getRS()->getNode())
            cout << " (RS)";
    }
    void stat(Expression *e) {
        if (running_for_MCC())
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

TreeTraceNode::TreeTraceNode(const vector<int> &mark, CTLFormula *f, TraceType tt)
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
 ---	ConstantExpr	---
 ---------------------------*/
void ConstantExpr::createMTMDD() {
    CTLMDD *ctl = CTLMDD::getInstance();
    forest *mtmdd_forest = ctl->getMTMDDForest();
    dd_edge *complete = ctl->newEmptyMTMDD();
    int **m = ctl->getIns();
    FormulaPrinter<ConstantExpr> fp(this);
    if (DOUBLELEVEL)
        mtmdd_forest->createEdge(m, m, &(this->constant), 1, *complete);
    else
        mtmdd_forest->createEdge(m, &(this->constant), 1, *complete);
    setMTMDD(complete);
}

ConstantExpr::ConstantExpr(float constant) {
    this->constant = constant;
}

void ConstantExpr::setConstant(float constant) {
    this->constant = constant;
    setMTMDD(NULL);
}

float ConstantExpr::getConstant() {
    return this->constant;
}

void ConstantExpr::print(std::ostream &os) const {
    os << constant;
}

ConstantExpr::~ConstantExpr() {
}

/*-----------------
 ---	Term	---
 ------------------*/

Term::Term(float coeff, int variable, Expression::op_type op) {
    this->coeff = coeff;
    this->variable = variable;
    this->op = op;
}

Term::~Term() {
}

float Term::getCoeff() const {
    return coeff;
}

int Term::getVariable() const {
    return variable;
}

void Term::setCoeff(float coeff) {
    this->coeff = coeff;
    setMTMDD(NULL);
}

void Term::setVariable(int variable) {
    this->variable = variable;
    setMTMDD(NULL);
}

Expression::op_type Term::getOp() const {
    return op;
}

void Term::setOp(Expression::op_type op) {
    this->op = op;
    setMTMDD(NULL);
}

void Term::print(std::ostream &os) const {
    if (coeff != 1 || op != EOP_TIMES)
        os << coeff << " " << Expression::OP_Names[op] << " ";
    CTLMDD *ctl = CTLMDD::getInstance();
    for (int pl = 0; pl < npl; pl++) {
        if (variable - 1 == ctl->getVal(pl)) {
            os << rsrg->getPL(pl);
            return;
        }
    }
    cerr << "Term::print(): Something wrong happened with place/variable indexes!" << endl;
    exit(-1);
}

void Term::createMTMDD() {
    CTLMDD *ctl = CTLMDD::getInstance();
    forest *mtmdd_forest = ctl->getMTMDDForest();
    dd_edge *tmp_mdd = ctl->newEmptyMTMDD();
    int val;
    int **m = ctl->getIns();
    float terminale;
    int variable_bound = rsrg->getRealBound(variable);
    // int variable_bound = Max_Token_Bound; // TODO: remove
#warning "Use createEdgeForVar"
    FormulaPrinter<Term> fp(this);
    for (val = 0; val <= variable_bound; val++) {
        // for (val = 0; val < Max_Token_Bound; val++) {
        m[0][variable] = val;
        switch (op) {
        case Expression::EOP_PLUS:
            terminale = coeff + (float) val;
            break;
        case Expression::EOP_TIMES:
            terminale = coeff * (float) val;
            break;
        case Expression::EOP_MINUS:
            terminale = coeff - (float) val;
            break;
        case Expression::EOP_DIV:
            terminale = coeff / (float) val;
            break;
        }
        dd_edge tmp_new_ap(mtmdd_forest);
        dd_edge *new_ap = &tmp_new_ap;
        if (DOUBLELEVEL)
            mtmdd_forest->createEdge(m, m, &terminale, 1, *new_ap);
        else
            mtmdd_forest->createEdge(m, &terminale, 1, *new_ap);
        (*tmp_mdd) += (*new_ap);
    }
    m[0][variable] = DOUBLELEVEL ? -2 : -1;
    setMTMDD(tmp_mdd);

#warning "Try and test this implementation below."
    // float term_vec[variable_bound + 1];
    // for (int mark = 0; mark <= variable_bound; mark++)
    //     term_vec[mark] = mark;
    // dd_edge* mark_of_place = ctl->newEmptyMTMDD();
    // mtmdd_forest->createEdgeForVar(variable, DOUBLELEVEL, term_vec, *mark_of_place);
    // setMTMDD(mark_of_place);
}

/*---------------------
 ---	ComplexExpr	---
 ----------------------*/
ComplexExpr::ComplexExpr(Expression *expr1, Expression *expr2, Expression::op_type op) {
    this->expr1 = expr1;
    this->expr2 = expr2;
    this->op = op;
    expr1->addOwner();
    expr2->addOwner();
}

ComplexExpr::~ComplexExpr() {
    expr1->removeOwner();
    expr2->removeOwner();
}

Expression *ComplexExpr::getExpr1() const {
    return expr1;
}

void ComplexExpr::setExpr1(Expression *expr1) {
    this->expr1 = expr1;
    setMTMDD(NULL);
}

Expression *ComplexExpr::getExpr2() const {
    return expr2;
}

void ComplexExpr::setExpr2(Expression *expr2) {
    this->expr2 = expr2;
    setMTMDD(NULL);
}

Expression::op_type ComplexExpr::getOp() const {
    return op;
}

void ComplexExpr::setOp(Expression::op_type op) {
    this->op = op;
    setMTMDD(NULL);
}

void ComplexExpr::print(std::ostream &os) const {
    os << "(" << (*expr1) << " " << Expression::OP_Names[op] << " " << (*expr2) << ")";
}

void ComplexExpr::createMTMDD() {
    CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge *e1 = expr1->getMTMDD();
    dd_edge *e2 = expr2->getMTMDD();
    dd_edge *r = ctl->newEmptyMTMDD();
    FormulaPrinter<ComplexExpr> fp(this);
    switch (op) {
    case Expression::EOP_PLUS:
        (*r) = (*e1) + (*e2);
        break;
    case Expression::EOP_TIMES:
        (*r) = (*e1) * (*e2);
        break;
    case Expression::EOP_MINUS:
        (*r) = (*e1) - (*e2);
        break;
    case Expression::EOP_DIV:
        apply(DIVIDE, *e1, *e2, *r);
        break;
    }

#if DEBUG
    r->show(stdout, 2);
#endif
    setMTMDD(r);
}

/*---------------------
 ---	Expression	---
 ----------------------*/
Expression::Expression() {
    pMTMDD = NULL;
    countOwner = 0;
}

Expression::~Expression() {
    setMTMDD(NULL);
}

void Expression::addOwner() {
    countOwner++;
}

void Expression::removeOwner() {
    countOwner--;
    if (countOwner < 1)
        delete this;
}

dd_edge *Expression::getMTMDD() {
    if (pMTMDD == NULL) {
        createMTMDD();
    }
    return pMTMDD;
}

void Expression::setMTMDD(dd_edge *newMTMDD) {
    if (pMTMDD != NULL)
        delete pMTMDD;
    pMTMDD = newMTMDD;
}

const char *Expression::OP_Names[4] = {
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

// Expression *Inequality::getExpr() const {
//     return expr;
// }
Inequality::op_type Inequality::getOp() const {
    return op;
}

void Inequality::setConstant(float constant) {
    this->constant = constant;
    setMDD(NULL);
}

// void Inequality::setExpr(Expression *expr) {
//     this->expr = expr;
//     setMDD(NULL);
// }

void Inequality::setOp(op_type op) {
    this->op = op;
    setMDD(NULL);
}

Inequality::Inequality(op_type op, Expression *expr1, float constant) {
    this->op = op;
    this->expr1 = expr1;
    this->expr2 = NULL;
    this->constant = constant;
    expr1->addOwner();
}

Inequality::Inequality(op_type op, Expression *expr1, Expression *expr2) {
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

    setMDD(NULL);
    //caso confronto tra posti "==" o "<>"
    if ((op == IOP_DIF) || (op == IOP_SIM)) {
        FormulaPrinter<Inequality> fp(this);
        dd_edge *boole = ctl->newEmptyMDD();
        dd_edge  tmp_complete(ctl->getMDDForest());
        int **m = ctl->getIns();
        memset(m[0], -1, (ctl->getDomain()->getNumVariables())*sizeof(int));
        int variable_bound1 = rsrg->getRealBound(((Term *)expr1)->getVariable());
        int variable_bound2 = rsrg->getRealBound(((Term *)expr2)->getVariable());
        for (int i = 0; i <= variable_bound1  ; i++) {
            m[0][((Term *)expr1)->getVariable()] = i;
            int div = 1, mult = 1;
            if (((Term *)expr1)->getOp() == Expression::EOP_DIV)
                mult *= (int)((Term *)expr1)->getCoeff();
            else
                div *= (int)((Term *)expr1)->getCoeff();
            if (((Term *)expr2)->getOp() == Expression::EOP_DIV)
                div *= (int)((Term *)expr2)->getCoeff();
            else
                mult *= (int)((Term *)expr2)->getCoeff();
            if (((i % div) == 0) && ((i / div * mult) <= variable_bound2)) {
                m[0][((Term *)expr2)->getVariable()] = int(i / div * mult);
                ctl->getMDDForest()->createEdge(m, 1, tmp_complete);
                apply(UNION, tmp_complete, *boole, *boole);
                m[0][((Term *)expr2)->getVariable()] = -1;
            }
        }
        dd_edge *rs = ctl->getRS();
        try {
            switch (op) {
            case IOP_SIM:
                apply(INTERSECTION, (*rs), (*boole), (*boole));
                break;
            case IOP_DIF:
                apply(DIFFERENCE, (*rs), (*boole), (*boole));
                break;
            default:
                throw;
            }
            setMDD(boole);
            return;
        }
        catch (MEDDLY::error e) {
            cerr << "createMDD: MEDDLY Error: " << e.getName() << endl;
            return;
        }
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
    dd_edge *exp2MDD = NULL;
    dd_edge *exp1MDD = expr1->getMTMDD();
    dd_edge q(mtmdd_forest);
    if (expr2 != NULL) { //case exp <op> exp
        exp2MDD = expr2->getMTMDD();
    }
    else { //case exp <op> term
        int **m = ctl->getIns();
        exp2MDD = ctl->newEmptyMTMDD();
        if (DOUBLELEVEL)
            mtmdd_forest->createEdge(m, m, &(constant), 1, *exp2MDD);
        else
            mtmdd_forest->createEdge(m, &(constant), 1, *exp2MDD);
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
    default: throw;
    }
    try {
        apply(opname, *exp1MDD, *exp2MDD, q);
        dd_edge *rs = ctl->getRS();
        dd_edge *boole = ctl->newEmptyMDD();
        apply(COPY, q, (*boole));
        apply(INTERSECTION, *rs, *boole, *boole);//estraggo da rs quelli che
        setMDD(boole);
    }
    catch (MEDDLY::error e) {
        cerr << "createMDD: MEDDLY Error: " << e.getName() << endl;
        return;
    }
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

    dd_edge r(*expr1->getMTMDD());
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
    default: throw;
    }
    try {
        apply(complement_opname, r, complete, complement);
        dd_edge *rs = ctl->getRS();
        dd_edge *boole = ctl->newEmptyMDD();
        apply(COPY, complement, *boole);
        apply(DIFFERENCE, *rs, *boole, *boole);
        setMDD(boole);
    }
    catch (MEDDLY::error e) {
        cerr << "createMDDByComplement: MEDDLY Error: " << e.getName() << endl;
        return;
    }
}

TreeTraceNode *Inequality::generateTrace(const vector<int> &state, TraceType traceTy) {
    CTLMDD *ctl = CTLMDD::getInstance();
    bool isSat = ctl->SatSetContains(*getMDD(), state.data());

    // cout << "isSat="<<isSat<<" traceTy == TT_WITNESS is " << (traceTy == TT_WITNESS) << endl;
    CTL_ASSERT(isSat == (traceTy == TT_WITNESS));

    return new TreeTraceNode(state, this, traceTy);
}

/*---------------------
 ---	BoolValue	---
 ----------------------*/

BoolValue::BoolValue(bool value) {
    this->value = value;
}

BoolValue::~BoolValue() {
}

void BoolValue::setValue(bool value) {
    this->value = value;
    setMDD(NULL);
}
bool BoolValue::getValue() {
    return value;
}
void BoolValue::print(std::ostream &os) const {
    os << (value ? "true" : "false");
}
void BoolValue::createMDD() {
    FormulaPrinter<BoolValue> fp(this);
    CTLMDD *ctl = CTLMDD::getInstance();
    if (value) { // TRUE
        setMDD(ctl->newRS_MDD());
    }
    else { // FALSE
        setMDD(ctl->newEmptyMDD());
    }
}

TreeTraceNode *BoolValue::generateTrace(const vector<int> &state, TraceType traceTy) {
    CTL_ASSERT(value == (traceTy == TT_WITNESS));
    return new TreeTraceNode(state, this, traceTy);
}

/*------------------------------------
 ---    ConstantScalarCTLFormula  ---
 ------------------------------------*/

ConstantScalarCTLFormula::ConstantScalarCTLFormula(Expression *ce) {
    assert(typeid(*ce) == typeid(ConstantExpr));
    expr = (ConstantExpr *)ce;
    expr->addOwner();
}
ConstantScalarCTLFormula::~ConstantScalarCTLFormula() {
    expr->removeOwner();
}
void ConstantScalarCTLFormula::createMDD() {
    throw "Unimplemented for ConstantScalarCTLFormula.";
}
void ConstantScalarCTLFormula::print(std::ostream &os) const {
    expr->print(os);
}
TreeTraceNode *ConstantScalarCTLFormula::generateTrace(const vector<int> &state, TraceType traceTy) {
    throw "Unimplemented for ConstantScalarCTLFormula.";
}
float ConstantScalarCTLFormula::getResult() {
    return expr->getConstant();
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
    setMDD(NULL);
}
bool Deadlock::getValue() {
    return value;
}
void Deadlock::print(std::ostream &os) const {
    os << (value ? "deadlock" : "ndeadlock");
}

void Deadlock::createMDD() {
    CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge *RS = ctl->getRS();
    dd_edge *r = ctl->newEmptyMDD();
    FormulaPrinter<Deadlock> fp(this);
    try {
        apply(PRE_IMAGE, (*RS), *(ctl->getNsf()), (*r));
        if (value) { // DEADLOCK
            // Compute RS \ r
            bool r_empty = (r->getNode() == dddRS::getMeddlyTerminalNodeID(false));
            bool r_allRS = (r->getNode() == ctl->getRS()->getNode());
            if (r_empty) { // No state in r is a deadlock state, return RS
                delete r;
                setMDD(ctl->newRS_MDD());
            }
            else if (r_allRS) { // Every state in r is a deadlock state, return 0
                delete r;
                setMDD(ctl->newEmptyMDD());
            }
            else {   // Compute RS \ r as a set difference
                apply(DIFFERENCE, (*ctl->getRS()), *r, *r);
                setMDD(r);
            }
        }
        else { // NDEADLOCK
            setMDD(r);
        }

    }
    catch (MEDDLY::error e) {
        cerr << "Error createMDD for deadlock: " << e.getName() << endl;
        return;
    }
}

TreeTraceNode *Deadlock::generateTrace(const vector<int> &state, TraceType traceTy) {
    CTLMDD *ctl = CTLMDD::getInstance();
    bool isSat = ctl->SatSetContains(*getMDD(), state.data());

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

Reachability::Reachability(CTLFormula *subf, prop_type type) {
    this->subf = subf;
    this->type = type;
    subf->addOwner();
}
Reachability::~Reachability() {
    subf->removeOwner();
}
void Reachability::createMDD() {
    CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge *f1 = subf->getMDD();
    bool f1_empty = (f1->getNode() == dddRS::getMeddlyTerminalNodeID(false));
    bool f1_allRS = (f1->getNode() == ctl->getRS()->getNode());
    bool result;
    switch (type) {
    case RPT_INVARIANTLY:       result = f1_allRS;      break;
    case RPT_POSSIBILITY:       result = !f1_empty;     break;
    case RPT_IMPOSSIBILITY:     result = f1_empty;      break;
    }

    FormulaPrinter<Reachability> fp(this);
    setMDD(result ? ctl->newRS_MDD() : ctl->newEmptyMDD());
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
 ---	CTLOperation	---
 --------------------------*/
CTLOperation::CTLOperation(CTLFormula *formula1, CTLFormula *formula2) {
    this->formula1 = formula1;
    this->formula2 = formula2;
    this->op = CTLOP_EU;
    formula1->addOwner();
    formula2->addOwner();
}
CTLOperation::CTLOperation(CTLFormula *formula1, CTLOperation::op_type op) {
    this->formula1 = formula1;
    this->formula2 = NULL;
    this->op = op;
    formula1->addOwner();
}
CTLOperation::~CTLOperation() {
    if (formula1 != NULL) formula1->removeOwner();
    if (formula2 != NULL) formula2->removeOwner();

}
CTLFormula *CTLOperation::getFormula1() const {
    return formula1;
}
void CTLOperation::setFormula1(CTLFormula *formula1) {
    this->formula1 = formula1;
    setMDD(NULL);
}
CTLFormula *CTLOperation::getFormula2() const {
    return formula2;
}
void CTLOperation::setFormula2(CTLFormula *formula2) {
    this->formula2 = formula2;
    setMDD(NULL);
}

CTLOperation::op_type CTLOperation::getOp() const {
    return op;
}

void CTLOperation::setOp(CTLOperation::op_type op) {
    this->op = op;
    setMDD(NULL);
}

void CTLOperation::print(std::ostream &os) const {
    if (op == CTLOP_EU)
        os << "E [" << (*formula1) << " U " << (*formula2) << "]";
    else {
        os << (op == CTLOP_EG ? "E G " : (op == CTLOP_EF ? "E F " : "E X "));
        os << (*formula1);
    }
}

void CTLOperation::createMDD() {
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

void CTLOperation::createEXMDD() {
    CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge *f1 = formula1->getMDD();
    bool f1_empty = (f1->getNode() == dddRS::getMeddlyTerminalNodeID(false));
    // bool f1_allRS = (f1->getNode() == ctl->getRS()->getNode());

    FormulaPrinter<CTLOperation> fp(this);
    if (f1_empty) { // E X False
        fp.optimized("[E X FALSE]");
        setMDD(ctl->newEmptyMDD());
        return;
    }

    // Note: for non-ergodic RS, the E X true is only valid
    // for non-dead states. Therefore, the E X true case
    // is not different from the general case.
    dd_edge *r = ctl->newEmptyMDD();

//     else if (f1_allRS) { // E X True
// #warning Ricontrollare x il deadlock.
//     	setMDD(ctl->newRS_MDD());
//     }
//     else { // E X f1
    try {
        // E X f1  or  E X true
        apply(PRE_IMAGE, (*f1), *(ctl->getNsf()), (*r));
        setMDD(r);
    }
    catch (MEDDLY::error e) {
        cerr << "createEXMDD: MEDDLY Error: " << e.getName() << endl;
        return;
    }
    // }
}

void show_markings(const dd_edge &e) {
    enumerator it(e);
    int count = 0;
    while (it) {
        const int *mark = it.getAssignments();
        cout << "    ";
        for (int i = 0; i < npl; i++) {
            cout << mark[i + 1] << " ";
        }
        cout << endl;
        ++it;
        if (count++ > 100) {
            cout << "     ..." << endl;
            return;
        }
    }
}

void CTLOperation::createEGMDD() {
    CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge *f1 = formula1->getMDD();
    bool f1_empty = (f1->getNode() == dddRS::getMeddlyTerminalNodeID(false));
    // bool f1_allRS = (f1->getNode() == ctl->getRS()->getNode());

    FormulaPrinter<CTLOperation> fp(this);
    if (f1_empty) { // E G false
        fp.optimized("[E G FALSE]");
        setMDD(ctl->newEmptyMDD());
        return;
    }
    // NOTE: the case E G true is not trivial for non-ergodic RS.
    // Therefore, we leave it with the general case and do not
    // terat it separately.
    dd_edge *r = ctl->newEmptyMDD();
    dd_edge pre_r(ctl->getMDDForest());
    dd_edge pred_of_r(ctl->getMDDForest());
    (*r) = (*f1);

    // int it_cnt1 = 0;
    int it_cnt2 = 0;
    {
        // dd_edge PtoP(ctl->getNsf()->getForest());
        // int allMinus1[npl+1], *pAll = allMinus1;
        // std::fill_n(allMinus1, npl+1, -1);
        // dd_edge all(f1->getForest());
        // f1->getForest()->createEdge(&pAll, 1, all);
        // apply(CROSS, *f1, all, PtoP);
        // dd_edge Psf(ctl->getNsf()->getForest());
        // cout << "adesso eseguo l'intersezione...  " << endl;
        // apply(INTERSECTION, PtoP, *(ctl->getNsf()), Psf);

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
        // // show_markings(*result);
        // setMDD(result);
    }

    do {
        pre_r = (*r); // risultati ottenuti all'iterazione precedente
        try {
            apply(PRE_IMAGE, *r, *(ctl->getNsf()), pred_of_r); // 		}
            apply(INTERSECTION, pred_of_r, *r, *r);
            it_cnt2++;
            if (print_intermediate_expr()) {
                cout << "EG: step=" << it_cnt2 << ",  SAT size=" << fixed << pred_of_r.getCardinality() << endl;
            }
            cout.unsetf(ios_base::floatfield);
        }
        catch (MEDDLY::error e) {
            cerr << "createEGMDD: MEDDLY Error: " << e.getName() << endl;
            return;
        }
    }
    while ((*r) != pre_r);
    if (print_intermediate_expr()) {
        cout << "R2 = " << r->getNode() << " in " << it_cnt2 << " iterations." << endl;
    }
    // show_markings(*r);
    setMDD(r);
}


void CTLOperation::createEFMDD(CTLFormula *formula) {
    CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge *f1 = formula->getMDD();

    FormulaPrinter<CTLOperation> fp(this);
    bool f1_empty = (f1->getNode() == dddRS::getMeddlyTerminalNodeID(false));
    bool f1_allRS = (f1->getNode() == ctl->getRS()->getNode());

    if (f1_allRS) { // E F true  ->  true
        fp.optimized("[E F TRUE]");
        setMDD(ctl->newRS_MDD());
        return;
    }
    if (f1_empty) { // E F false  ->  false
        fp.optimized("[E F FALSE]");
        setMDD(ctl->newEmptyMDD());
        return;
    }

    // int it_cnt1 = 0, it_cnt2 = 0;
    {
        // // dd_edge RStoP(ctl->getNsf()->getForest());
        // // apply(CROSS, *f1, *(ctl->getRS()), RStoP);
        // // dd_edge Psf(ctl->getNsf()->getForest());
        // // apply(INTERSECTION, RStoP, *(ctl->getNsf()), Psf);

        // dd_edge *result = new dd_edge(*f1);
        // // while(true) {
        //     // dd_edge prev = *result;
        //     // apply(REVERSE_REACHABLE_DFS, *result, Psf, *result);
        // apply(REVERSE_REACHABLE_DFS, *result, *(ctl->getNsf()), *result);
        //     // apply(PRE_IMAGE, *result, Psf, *result);
        //     // apply(UNION, prev, *result, *result);

        // //     it_cnt1++;
        // //     if (*result == prev)
        // //         break;
        // // }
        // cout << "R1 = " << result->getNode() << " in " << it_cnt1 << " iterations." << endl;
        // // show_markings(*result);
        // setMDD(result);
    }

    // dd_edge *r = ctl->newEmptyMDD();
    // dd_edge pre_r(ctl->getMDDForest());
    // dd_edge pred_of_r(ctl->getMDDForest());
    // (*r) = (*f1);
    // do {
    //     pre_r = (*r); // risultati ottenuti all'iterazione precedente
    //     try {
    //         apply(PRE_IMAGE, (*r), *(ctl->getNsf()), pred_of_r);
    //         apply(UNION, pred_of_r, (*r), (*r));
    //         // it_cnt2++;
    //     }
    //     catch (...) {
    //         cerr << "Error in createEFMDD " << endl;
    //         return;
    //     }
    // }
    // while ((*r) != pre_r);
    // // cout << "R2 = " << r->getNode() << " in " << it_cnt2 << " iterations." << endl;
    // // show_markings(*r);
    // setMDD(r);

    const int NUM_PRESTEPS = 0; // Number of steps of pre-image before appling saturation
    dd_edge *result = new dd_edge(*f1);
    for (int i = 0; i < NUM_PRESTEPS; i++) {
        dd_edge current(*result);
        // Y' = Y union (Y * N^-1)
        apply(PRE_IMAGE, *result, *(ctl->getNsf()), *result);
        apply(UNION, current, *result, *result);
        if (*result == current) {
            if (print_intermediate_expr()) {
                cout << "      " << i << " steps: ";
            }
            setMDD(result);
            return;
        }
    }
    // Solve using backward reachability with saturation
    // (slightly more costly than PRE_IMAGE + UNION)
    apply(REVERSE_REACHABLE_DFS, *result, *(ctl->getNsf()), *result);
    if (print_intermediate_expr()) {
        cout << "      saturation: ";
    }
    setMDD(result);
}


void CTLOperation::createEUMDD() {
    // E formula1 U formula2
    CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge *f2 = formula2->getMDD();
    bool f2_empty = (f2->getNode() == dddRS::getMeddlyTerminalNodeID(false));
    bool f2_allRS = (f2->getNode() == ctl->getRS()->getNode());

    if (f2_empty) { // E f1 U false  ->  false
        FormulaPrinter<CTLOperation> fp(this);
        fp.optimized("[E f1 U false]");
        setMDD(ctl->newEmptyMDD());
        return;
    }
    else if (f2_allRS) { // E f1 U true  ->  true
        FormulaPrinter<CTLOperation> fp(this);
        fp.optimized("[E f1 U true]");
        setMDD(ctl->newRS_MDD());
        return;
    }

    dd_edge *f1 = formula1->getMDD();
    bool f1_empty = (f1->getNode() == dddRS::getMeddlyTerminalNodeID(false));
    bool f1_allRS = (f1->getNode() == ctl->getRS()->getNode());

    if (f1_allRS) { // E true U f2  ->  E F f2
        createEFMDD(formula2);
        return;
    }
    else if (f1_empty) { // E false U f2  ->  f2
        FormulaPrinter<CTLOperation> fp(this);
        fp.optimized("[E false U f2]");
        setMDD(new dd_edge(*f2));
        return;
    }

//     dd_edge *r = ctl->newEmptyMDD();
//     dd_edge pre_r(ctl->getMDDForest());
//     dd_edge pred_of_r(ctl->getMDDForest());
//     dd_edge pred_of_r_f1(ctl->getMDDForest());
//     (*r) = (*f2);
// #warning Aggiungere casi limite per EU
//     do {
//         pre_r = (*r); // risultati ottenuti all'iterazione precedente
//         try {
//             apply(PRE_IMAGE, (*r), *(ctl->getNsf()), pred_of_r);
//             apply(INTERSECTION, pred_of_r, (*f1), pred_of_r_f1);
//             apply(UNION, pred_of_r_f1, (*r), (*r));
//         }
//         catch (...) {
//             cerr << "Error in createEUMDD" << endl;
//             return;
//         }
//     }
//     while ((*r) != pre_r);
//     setMDD(r);

    FormulaPrinter<CTLOperation> fp(this);
    dd_edge *result = new dd_edge(*f2);
    while (true) {
        dd_edge current(*result), pred(*result);
        // R' = R union (F1 intersect (R * N^-1))
        apply(PRE_IMAGE, *result, *(ctl->getNsf()), pred);
        apply(INTERSECTION, pred, *f1, pred);
        apply(UNION, pred, *result, *result);
        if (*result == current)
            break;
    }
    setMDD(result);
}

TreeTraceNode *CTLOperation::generateTrace(const vector<int> &state0, TraceType traceTy) {
    CTLMDD *ctl = CTLMDD::getInstance();
    forest *mdd_forest = ctl->getMDDForest();

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
        dd_edge Phi = *getFormula1()->getMDD();

        vector<dd_edge> intermDD;
        intermDD.push_back(Phi);
        while (!ctl->SatSetContains(intermDD.back(), state0.data())) {
            // Step back until we find the marking in the sat set
            dd_edge pre_image(mdd_forest);

            apply(PRE_IMAGE, intermDD.back(), *ctl->getNsf(), pre_image);
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
            apply(POST_IMAGE, dd_of_state, *ctl->getNsf(), dd_of_state);
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
        dd_edge Phi = *getFormula1()->getMDD();
        CTL_ASSERT(ctl->SatSetContains(Phi, state0.data()));
        dd_edge dd_s0(mdd_forest);
        const int *vlist = state0.data();
        mdd_forest->createEdge(&vlist, 1, dd_s0);

        vector<dd_edge> intermDD;
        intermDD.push_back(dd_s0);
        do {
            dd_edge pre_image(mdd_forest);
            apply(PRE_IMAGE, intermDD.back(), *ctl->getNsf(), pre_image);
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
            apply(POST_IMAGE, dd_of_state, *ctl->getNsf(), dd_of_state);
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
        dd_edge Phi1 = *getFormula1()->getMDD();
        dd_edge Phi2 = *getFormula2()->getMDD();

        vector<dd_edge> intermDD;
        intermDD.push_back(Phi2);
        while (!ctl->SatSetContains(intermDD.back(), state0.data())) {
            // Step back until we find the marking in the sat set
            dd_edge pre_image(mdd_forest);
            apply(PRE_IMAGE, intermDD.back(), *ctl->getNsf(), pre_image);
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
                apply(POST_IMAGE, dd_of_state, *ctl->getNsf(), dd_of_state);
                if (step > 0)
                    apply(INTERSECTION, intermDD[step], dd_of_state, dd_of_state);
                else
                    apply(DIFFERENCE, dd_of_state, intermDD[step], dd_of_state); // Modif. 5 may 2014
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
        dd_edge Phi = *getFormula1()->getMDD();
        vector<int> state1(npl + 1);
        dd_edge dd_s0(mdd_forest);
        const int *vlist = state0.data();
        mdd_forest->createEdge(&vlist, 1, dd_s0);
        apply(POST_IMAGE, dd_s0, *ctl->getNsf(), dd_s0);
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
 ---	ComplexBoolFormula	---
 ------------------------------*/
ComplexBoolFormula::ComplexBoolFormula(CTLFormula *formula1, CTLFormula *formula2, op_type op) {
    this->formula1 = formula1;
    this->formula2 = formula2;
    this->op = op;
    formula1->addOwner();
    formula2->addOwner();
}
ComplexBoolFormula::ComplexBoolFormula(CTLFormula *formula1) {
    this->formula1 = formula1;
    this->formula2 = NULL;
    this->op = CBF_NOT;
    formula1->addOwner();
}
ComplexBoolFormula::~ComplexBoolFormula() {
    if (formula1 != NULL) formula1->removeOwner();
    if (formula2 != NULL) formula2->removeOwner();

}
CTLFormula *ComplexBoolFormula::getFormula1() const {
    return formula1;
}
void ComplexBoolFormula::setFormula1(CTLFormula *formula1) {
    this->formula1 = formula1;
    setMDD(NULL);
}
CTLFormula *ComplexBoolFormula::getFormula2() const {
    return formula2;
}
void ComplexBoolFormula::setFormula2(CTLFormula *formula2) {
    this->formula2 = formula2;
    setMDD(NULL);
}

ComplexBoolFormula::op_type ComplexBoolFormula::getOp() const {
    return op;
}

void ComplexBoolFormula::setOp(ComplexBoolFormula::op_type op) {
    this->op = op;
    setMDD(NULL);
}

void ComplexBoolFormula::print(std::ostream &os) const {
    if (op == CBF_NOT)
        os << "(not " << (*formula1) << ")";
    else
        os << "(" << (*formula1) << ((op == CBF_AND) ? " and " : " or ")
           << (*formula2) << ")";
}

void ComplexBoolFormula::createMDD() {
    CTLMDD *ctl = CTLMDD::getInstance();
    dd_edge *f1 = formula1->getMDD();
    bool f1_empty = (f1->getNode() == dddRS::getMeddlyTerminalNodeID(false));
    bool f1_allRS = (f1->getNode() == ctl->getRS()->getNode());
    try {
        switch (op) {
        case CBF_NOT: {
            FormulaPrinter<ComplexBoolFormula> fp(this);
            if (f1_allRS) { // NOT TRUE -> FALSE
                fp.optimized("[NOT TRUE]");
                setMDD(ctl->newEmptyMDD());
            }
            else if (f1_empty) { // NOT FALSE -> TRUE
                fp.optimized("[NOT FALSE]");
                setMDD(ctl->newRS_MDD());
            }
            else { // NOT f1
                dd_edge *rs = ctl->newRS_MDD();
                apply(DIFFERENCE, *rs, *f1, *rs);
                setMDD(rs);
            }
            break;
        }

        case CBF_AND: {
            if (f1_allRS) { // TRUE and f2 -> f2
                dd_edge *f2 = formula2->getMDD();
                FormulaPrinter<ComplexBoolFormula> fp(this);
                fp.optimized("[TRUE and f2]");
                setMDD(new dd_edge(*f2));
            }
            else if (f1_empty) { // FALSE AND f2 -> FALSE
                FormulaPrinter<ComplexBoolFormula> fp(this);
                fp.optimized("[FALSE AND f2]");
                setMDD(ctl->newEmptyMDD());
            }
            else { // f1 AND f2
                CTL_ASSERT(formula2 != NULL);
                // TODO: check again why this optimization fails.
                // ./bin/RGMEDD ~/Desktop/old_home/sp116921/BenchKit/INPUTS/FMS-PT-005/model -B 6 -C -f CTL.txt
                // query: A G (P12 = 1 and A G P12 = 1)
                // when getting the counter-example

                if (typeid(*formula2) == typeid(ComplexBoolFormula)) {
                    ComplexBoolFormula *cbf2 = dynamic_cast<ComplexBoolFormula *>(formula2);
                    if (cbf2->getOp() == ComplexBoolFormula::CBF_NOT) {
                        // f1 AND NOT f2 can be computed as a SETDIFF: s1 \ s2
                        dd_edge *fdiff = cbf2->getFormula1()->getMDD();
                        dd_edge *rs = ctl->newEmptyMDD();
                        FormulaPrinter<ComplexBoolFormula> fp(this);
                        fp.optimized("[f1 AND NOT f2]");
                        apply(DIFFERENCE, *f1, *fdiff, *rs);
                        // apply(INTERSECTION, *rs, *ctl->getRS(), *rs);
                        setMDD(rs);
                        break;
                    }
                }
                dd_edge *f2 = formula2->getMDD();
                dd_edge *rs = ctl->newEmptyMDD();
                FormulaPrinter<ComplexBoolFormula> fp(this);
                apply(INTERSECTION, *f1, *f2, *rs);
                setMDD(rs);
            }
            break;
        }

        case CBF_OR: {
            if (f1_allRS) { // TRUE OR f2 -> TRUE
                FormulaPrinter<ComplexBoolFormula> fp(this);
                fp.optimized("[TRUE OR f2]");
                setMDD(ctl->newRS_MDD());
            }
            else if (f1_empty) { // FALSE OR f2 -> f2
                dd_edge *f2 = formula2->getMDD();
                FormulaPrinter<ComplexBoolFormula> fp(this);
                fp.optimized("[FALSE OR f2]");
                setMDD(new dd_edge(*f2));
            }
            else { // f1 OR f2
                dd_edge *f2 = formula2->getMDD();
                FormulaPrinter<ComplexBoolFormula> fp(this);
                dd_edge *rs = ctl->newEmptyMDD();
                apply(UNION, *f1, *f2, *rs);
                setMDD(rs);
            }
            break;
        }
        }
    }
    catch (MEDDLY::error e) {
        cerr << "ERROR in createMDD complex Bool Expr: " << e.getName() << endl;
        return;
    }
}

TreeTraceNode *ComplexBoolFormula::generateTrace(const vector<int> &state0, TraceType traceTy) {
    CTLMDD *ctl = CTLMDD::getInstance();
    CTL_ASSERT(ctl->SatSetContains(*getMDD(), state0.data()) == (traceTy == TT_WITNESS));

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
            // cout << "ctl->SatSetContains(*getFormula1()->getMDD(), state0.data()) = " << ctl->SatSetContains(*getFormula1()->getMDD(), state0.data()) << endl;
            // cout << "ctl->SatSetContains(*getFormula2()->getMDD(), state0.data()) = " << ctl->SatSetContains(*getFormula2()->getMDD(), state0.data()) << endl;
            CTL_ASSERT(ctl->SatSetContains(*getFormula1()->getMDD(), state0.data()));
            CTL_ASSERT(ctl->SatSetContains(*getFormula2()->getMDD(), state0.data()));
            ttn->set_sub_trace1(getFormula1()->generateTrace(state0, traceTy));
            ttn->set_sub_trace2(getFormula2()->generateTrace(state0, traceTy));
        }
        else { // (op == CBF_OR)
            bool isSat1 = ctl->SatSetContains(*getFormula1()->getMDD(), state0.data());
            if (isSat1)
                ttn->set_sub_trace1(getFormula1()->generateTrace(state0, traceTy));
            else {
                CTL_ASSERT(ctl->SatSetContains(*getFormula2()->getMDD(), state0.data()));
                ttn->set_sub_trace1(getFormula2()->generateTrace(state0, traceTy));
            }
        }
    }
    else { // traceTy == TT_COUNTEREXAMPLE
        if (op == CBF_AND) {
            bool isSat1 = ctl->SatSetContains(*getFormula1()->getMDD(), state0.data());
            if (!isSat1)
                ttn->set_sub_trace1(getFormula1()->generateTrace(state0, traceTy));
            else {
                CTL_ASSERT(!ctl->SatSetContains(*getFormula2()->getMDD(), state0.data()));
                ttn->set_sub_trace1(getFormula2()->generateTrace(state0, traceTy));
            }
        }
        else { // (op == CBF_OR)
            CTL_ASSERT(!ctl->SatSetContains(*getFormula1()->getMDD(), state0.data()));
            CTL_ASSERT(!ctl->SatSetContains(*getFormula2()->getMDD(), state0.data()));
            ttn->set_sub_trace1(getFormula1()->generateTrace(state0, traceTy));
            ttn->set_sub_trace2(getFormula2()->generateTrace(state0, traceTy));
        }
    }
    return ttn;
}

/*---------------------
 ---	CTLFormula	---
 -----------------------*/
CTLFormula::CTLFormula() {
    countOwner = 0;
    pMDD = NULL;
}
CTLFormula::~CTLFormula() {
    setMDD(NULL);
}
dd_edge *CTLFormula::getMDD() {
    if (pMDD == NULL) {
        this->createMDD();
    }
    CTL_ASSERT(pMDD != NULL);
    return pMDD;
}
void CTLFormula::setMDD(dd_edge *newMDD) {
    if (pMDD != NULL)
        delete pMDD;
    pMDD = newMDD;
}
void CTLFormula::addOwner() {
    countOwner++;
    // cout << "addOwner("<<*(this)<<" "<<countOwner<<endl;
}
void CTLFormula::removeOwner() {
    countOwner--;
    // cout << "removeOwner("<<*(this)<<" "<<countOwner<<endl;
    if (countOwner < 1)
        delete this;
}


} // end namespace ctlmdd

