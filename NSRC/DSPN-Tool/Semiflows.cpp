/*
 *  Semiflows.cpp
 *
 *  Implementation of the P/T-semiflows computation methods
 *
 *  Created by Elvio Amparore
 *
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cfloat>
#include <vector>
#include <map>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <exception>
#include <algorithm>
#include <ctime>
#include <climits>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <functional>
using namespace std;

#include "../numeric/numeric.h"
using namespace numerical;

#include "utils.h"
#include "sparsevector.h"
#include "PetriNet.h"
#include "Semiflows.h"

//-----------------------------------------------------------------------------

template<typename Tag>
std::function<ostream& (ostream &)>
print_spvec_dense_1base(const sparsevector<Tag>& v) {
    return [&](ostream & os) -> ostream& {
        os << '[' ;//<< v.size() << "][";
        for (size_t i = 0; i < v.nonzeros(); i++)
            os << (i > 0 ? ", " : "") << (v.ith_nonzero(i).index+1) << ":" << v.ith_nonzero(i).value;
        return os << "]";
    }; 
}

//-----------------------------------------------------------------------------

inline int gcd(int a, int b) {
    assert(a >= 0 && b >= 0);
    if (a == 0)
        return b;

    while (b != 0) {
        if (a > b)
            a = a - b;
        else
            b = b - a;
    }
    return a;
}

//-----------------------------------------------------------------------------

inline int sign(int num) {
    if (num > 0)
        return +1;
    else if (num < 0)
        return -1;
    return 0;
}

//-----------------------------------------------------------------------------

template<typename Tag>
inline size_t count_nonzeros(const sparsevector<Tag>& sv) {
    size_t cnt = 0;
    for (size_t k=0; k<sv.nonzeros(); k++)
        if (sv.ith_nonzero(k).value != 0)
            cnt++;
    // for (auto elem : sv)
    //     if (elem.value != typename sparsevector<I,V,Z>::value_type(0))
    //         cnt++;
    return cnt;
}

//-----------------------------------------------------------------------------

template<typename Container1, typename Container2>
inline ssize_t traverse_both(typename Container1::const_iterator& it1,
                             const Container1& cont1,
                             typename Container1::value_type& value1,
                             typename Container2::const_iterator& it2,
                             const Container2& cont2,
                             typename Container1::value_type& value2) 
{
    bool end1 = (it1 == cont1.end());
    bool end2 = (it2 == cont2.end());

    if (end1 && end2)
        return -1;

    if (end2 || (!end1 && it1->index < it2->index)) { // take from it1
        ssize_t index = ssize_t(it1->index);
        value1 = it1->value;
        value2 = 0;
        ++it1;
        return index;
    }
    if (end1 || it1->index > it2->index) { // take from it2
        ssize_t index = ssize_t(it2->index);
        value1 = 0;
        value2 = it2->value;
        ++it2;
        return index;
    }

    // take from both iterators
    assert(it1->index == it2->index);
    ssize_t index = ssize_t(it1->index);
    value1 = it1->value;
    value2 = it2->value;
    ++it1;
    ++it2;
    return index;
}

//-----------------------------------------------------------------------------

const char* GetGreatSPN_FileExt(InvariantKind ik, FlowMatrixKind matk, int inc_dec) {
    typedef const char*  T[3];
    size_t i = (inc_dec == 0 ? 0 : (inc_dec > 0 ? 1 : 2));
    const char *ext;
    switch (matk) {
        case FlowMatrixKind::SEMIFLOWS:
            ext = (ik==InvariantKind::PLACE) ? T{ ".pin", ".pin+", ".pin-" }[i] : T{ ".tin", ".tin+", ".tin-" }[i];
            break;
        case FlowMatrixKind::BASIS:
            ext = (ik==InvariantKind::PLACE) ? T{ ".pba", ".pba+", ".pba-" }[i] : T{ ".tba", ".tba+", ".tba-" }[i];
            break;
        case FlowMatrixKind::INTEGER_FLOWS:
            ext = (ik==InvariantKind::PLACE) ? T{ ".pfl", ".pfl+", ".pfl-" }[i] : T{ ".tfl", ".tfl+", ".tfl-" }[i];
            break;
        // case FlowMatrixKind::NESTED_FLOW_SPAN:
        //     return (ik==InvariantKind::PLACE) ? ".pspan" : ".tspan";
        default:
            throw program_exception("Internal error in GetGreatSPN_FileExt");
    }
    return ext;
}
//-----------------------------------------------------------------------------

const char* GetFlowName(InvariantKind ik, FlowMatrixKind matk) {
    bool p = (ik==InvariantKind::PLACE);
    switch (matk) {
        case FlowMatrixKind::EMPTY:         
            return "EMPTY";
        case FlowMatrixKind::INCIDENCE:     
            return "INCIDENCE MATRIX";
        case FlowMatrixKind::SEMIFLOWS:     
            return p ? "PLACE SEMIFLOWS" : "TRANSITION SEMIFLOWS";
        case FlowMatrixKind::BASIS:         
            return p ? "PLACE FLOW BASIS" : "TRANSITION FLOW BASIS";
        case FlowMatrixKind::INTEGER_FLOWS: 
            return p ? "PLACE FLOWS" : "TRANSITION FLOWS";
        // case FlowMatrixKind::NESTED_FLOW_SPAN: 
        //     return p ? "NESTED PLACE FLOW SPAN" : "NESTED TRANSITION FLOW SPAN";
        default:
            throw program_exception("Internal error in GetFlowMatrixName");
    }    
}

//-----------------------------------------------------------------------------

inline bool flow_entry_t::operator < (const flow_entry_t& f) const {
    if (i < f.i) return true;
    if (i > f.i) return false;
    if (j < f.j) return true;
    if (j > f.j) return false;
    if (card < f.card) return true;
    if (card > f.card) return false;
    return false;
}

//-----------------------------------------------------------------------------

ostream& operator<<(ostream& os, const flow_entry_t& f) {
    return os << "flow(" << f.i << " -> " << f.j << " : " << f.card << ")";
}

//-----------------------------------------------------------------------------

ostream& operator<<(ostream& os, const flow_matrix_t::row_t& mrow) {
    return mrow.print(os, false);
}

//-----------------------------------------------------------------------------
static const size_t MAX_DENSE_REPR = 35;

ostream& flow_matrix_t::row_t::print(ostream& os, bool highlight_annulled) const {
    if (D.size() > MAX_DENSE_REPR) { // dense representation
        for (size_t i = 0, cnt = 0; i < D.nonzeros(); i++)
            if (D.ith_nonzero(i).value)
                os << (cnt++ > 0 ? ", " : "") << (D.ith_nonzero(i).index+1) 
                   << ":" << D.ith_nonzero(i).value;
        os << " | ";
        for (size_t i = 0, cnt = 0; i < A.nonzeros(); i++)
            if (A.ith_nonzero(i).value)
                os << (cnt++ > 0 ? ", " : "") << (A.ith_nonzero(i).index+1) 
                   << ":" << A.ith_nonzero(i).value;
    }
    else { // sparse representation
        for (size_t n=0; n<D.size(); n++)
            os << setw(3) << D[n];
        os << " |";
        for (size_t m=0; m<A.size(); m++)
            os << setw(3) << A[m];
    }
    if (highlight_annulled && A.nonzeros() == 0)
        os << console::green_fgnd() << "  inv" << console::default_disp();
    return os;
}

//-----------------------------------------------------------------------------

ostream& operator<<(ostream& os, const flow_matrix_t& msa) {
    return msa.print(os, false);
}

//-----------------------------------------------------------------------------

ostream& flow_matrix_t::print(ostream& os, bool highlight_annulled) const {
    size_t row = 0;
    for (const auto& rr : mK) {
        os << setw(2) << row++ << ": ";
        rr.print(os, highlight_annulled) << endl;
        assert(rr.neg_D == rr.count_negatives_D());
    }
    return os;
}

//-----------------------------------------------------------------------------

flow_matrix_t::flow_matrix_t(size_t _N, size_t _N0, size_t _M, InvariantKind _ik, int _inc_dec, bool _add_extra_vars) 
: N(_N), N0(_N0), M(_M), inv_kind(_ik), inc_dec(_inc_dec), add_extra_vars(_add_extra_vars), mat_kind(FlowMatrixKind::EMPTY) 
{ /*cout << "M="<<M<<", N="<<N<<endl;*/ }

//-----------------------------------------------------------------------------

inline int flow_matrix_t::row_t::gcd_nnz_DA() const {
    int g = -1;
    for (size_t id=0; id<D.nonzeros(); id++) {
        if (D.ith_nonzero(id).value != 0) {
            g = (g == -1) ? abs(D.ith_nonzero(id).value) : gcd(g, abs(D.ith_nonzero(id).value));
            if (g == 1)
                return 1;
        }
    }
    for (size_t ia=0; ia<A.nonzeros(); ia++) {
        if (A.ith_nonzero(ia).value != 0) {
            g = (g == -1) ? abs(A.ith_nonzero(ia).value) : gcd(g, abs(A.ith_nonzero(ia).value));
            if (g == 1)
                return 1;
        }
    }
    assert(g != -1);
    return (g == -1) ? 1 : g;
}

//-----------------------------------------------------------------------------

inline bool flow_matrix_t::row_t::test_minimal_support_D(const spintvector& D2) const
{
    // Check if the support of D2 is included in D
    //   support(D2) subseteq sopport(D)    
    // where support(.) is the set of columns with non-zero entries
    if (D2.nonzeros() > D.nonzeros())
        return false;

    for (size_t i2=0, i=0; i2<D2.nonzeros(); i2++) {
        while (i < D.nonzeros() && (D.ith_nonzero(i).index < D2.ith_nonzero(i2).index)) {
            i++;
        }
        if (i >= D.nonzeros() || D.ith_nonzero(i).index != D2.ith_nonzero(i2).index) {
            return false;
        }
    }
    return true;
}

//-----------------------------------------------------------------------------

inline bool flow_matrix_t::row_t::test_minimal_positive_support_D(const spintvector& D2, int mult) const
{
    // pos_support(D2) subseteq sopport(D)    
    // where pos_support(.) is the set of columns with entries > 0
    for (size_t i2=0, i=0; i2<D2.nonzeros(); i2++) {
        while (i2<D2.nonzeros() && D2.ith_nonzero(i2).value * mult > 0)
            i2++;
        if (i2 == D2.nonzeros())
            break;
        while (i < D.nonzeros() && (D.ith_nonzero(i).index < D2.ith_nonzero(i2).index)) {
            i++;
        }
        if (i >= D.nonzeros() || D.ith_nonzero(i).index != D2.ith_nonzero(i2).index) {
            return false;
        }
    }
    return true;
}

//-----------------------------------------------------------------------------

bool flow_matrix_t::row_t::test_subst(const spintvector& R, int& multR, int& multD) const {
    // Step 1: check that R complements D, i.e.:
    //  * everytime R has a negative entry, D has a positive entry to be annulled
    //  * everytime R has a positive entry, D is has zero in that column
    multR = multD = -1;
    int valR, valD, i;
    auto itD = D.begin(), itR = R.begin();
    while (-1 != (i = traverse_both(itD, D, valD, itR, R, valR))) {
        if (valR == 0)
            continue; // ok any value in D when R is zero
        // if (sign(valR * multR) == sign(valD))
        //     break; // Cannot have both -1 or both +1
        if ((valR < 0 && valD > 0) || (valR > 0 && valD < 0)) {
            if (multR == -1) {
                int gcdRD = gcd(abs(valD), abs(valR));
                multR = abs(valD) / gcdRD;
                multD = abs(valR) / gcdRD;
            }
            else {
                // TODO: test
                int gcdRD = gcd(abs(valD), abs(valR));
                if (multR != abs(valD) / gcdRD || multD != abs(valR) / gcdRD) {
                    cout << "multR=" << multR << " multD="<<multD
                         <<"  abs(valD)/gcdRD="<<(abs(valD) / gcdRD)<<"  abs(valR)/gcdRD="<<(abs(valR) / gcdRD)<<endl;
                    return false; // Cannot substitute, the sum would not zero all the entries.
                }
                assert(multR == abs(valD) / gcdRD);
                assert(multD == abs(valR) / gcdRD);
            }
            continue; // R will annull the value in D
        }
        if (valR > 0 && valD == 0)
            continue; // R will set a value in the column, or D will annul R
        break;
    }
    return (i == -1);
}

//-----------------------------------------------------------------------------

// Check support(D2) subseteq support(D +/- R)
bool 
flow_matrix_t::row_t::test_minimal_support_linear_comb_D(const spintvector& D2, 
                                                         const spintvector& R) const 
{
    // cout << "\nsupport(D2) subseteq support(D +/- R)\n";
    // cout << "D2 = ";
    // for (size_t n=0; n<D2.size(); n++)
    //     cout << setw(3) << D2[n];
    // cout << endl;

    // cout << "D  = ";
    // for (size_t n=0; n<D.size(); n++)
    //     cout << setw(3) << D[n];
    // cout << endl;

    // cout << "R  = ";
    // for (size_t n=0; n<R.size(); n++)
    //     cout << setw(3) << R[n];
    // cout << endl;

    // Step 1: check that R complements D, i.e.:
    //  * everytime R has a negative entry, D has a positive entry to be annulled
    //  * everytime R has a positive entry, D is has zero in that column
    // Repeat twice, once for +R and another for -R.
    int multR, valR, valD, i;
    bool R_complements_D = false;
    for (size_t phase = 0; phase < 2 && !R_complements_D; phase ++) {
        multR = (phase == 0) ? +1 : -1;
        auto itD = D.begin(), itR = R.begin();
        while (-1 != (i = traverse_both(itD, D, valD, itR, R, valR))) {
            if (valR == 0)
                continue; // ok any value in D when R is zero
            // if (sign(valR * multR) == sign(valD))
            //     break; // Cannot have both -1 or both +1
            if (sign(valR * multR) < 0 && valD > 0)
                continue; // R will annull the value in D
            if (sign(valR * multR) > 0 && valD == 0)
                continue; // R will set a value in the column, or D will annul R
            break;
        }
        if (i == -1)
            R_complements_D = true;
    }
    // cout << "R complements D : " << R_complements_D << "   multR=" << multR << endl;
    if (!R_complements_D)
        return false;

    // int multR = +1;
    // valR = D[ R.ith_nonzero(0).index ], valD;
    // if (valR == 0) { // try the first negative index in R
    //     for (size_t iR=0; iR<R.nonzeros(); iR++) {
    //         // if (R.ith_nonzero(iR).value < 0) {
    //         if (sign(R.ith_nonzero(iR).value) != sign(R.ith_nonzero(0).value)) {
    //             valR = D[ R.ith_nonzero(iR).index ];
    //             if (valR == 0)
    //                 return false;
    //         }
    //     }
    //     multR = -1;
    // }

    auto itD = D.begin(), itR = R.begin();
    for (size_t i2=0; i2<D2.nonzeros(); i2++) {
        while(-1 != (i = traverse_both(itD, D, valD, itR, R, valR)) &&
              ( multR * sign(valR) + sign(valD) == 0 || /* entry annulled by R or by D */
                i < (int)D2.ith_nonzero(i2).index) )
        {
            // cout << "  skip i="<<i<<"  m*R+D="<<(multR * sign(valR) + sign(valD)) 
            // << "  next(D2)="<<D2.ith_nonzero(i2).index<<endl;
            continue;
        }
        // cout << "  test i="<<i<<"  m*R+D="<<(multR * sign(valR) + sign(valD))
        // <<" next(D2)"<<D2.ith_nonzero(i2).index<<endl;
        if (i == -1 || i != (int)D2.ith_nonzero(i2).index) {
            return false;
        }
    }
    return true;
}

//-----------------------------------------------------------------------------

inline bool flow_matrix_t::row_t::test_common_nonzeros(const spintvector& D2) const {
    int valD, valD2, i;
    auto itD = D.begin(), itD2 = D2.begin();
    while(-1 != (i = traverse_both(itD, D, valD, itD2, D2, valD2))) {
        if (valD != 0 && valD2 != 0)
            return true; // There is at least a common non-zero
    }
    return false;
}

//-----------------------------------------------------------------------------

// Test that the positive/negative supports of the two rows are either
// disjoint or complementary (when one is positive, the other is negative).
inline bool flow_matrix_t::row_t::test_disjoint_supports(const row_t& row2, int mult) const {
    int valD, valD2, i;
    auto itD = D.begin(), itD2 = row2.D.begin();
    while(-1 != (i = traverse_both(itD, D, valD, itD2, row2.D, valD2))) {
        if ((valD * mult) < 0 && valD2 < 0)
            return true; // There is at least a common negative entry in D
        if ((valD * mult) > 0 && valD2 > 0)
            return true; // There is at least a common positive entry in D
    }
    int valA, valA2;
    auto itA = A.begin(), itA2 = row2.A.begin();
    while(-1 != (i = traverse_both(itA, A, valA, itA2, row2.A, valA2))) {
        if ((valA * mult) < 0 && valA2 < 0)
            return true; // There is at least a common negative entry in A
        if ((valA * mult) > 0 && valA2 > 0)
            return true; // There is at least a common positive entry in A
    }
    return false;
}

//-----------------------------------------------------------------------------

inline bool flow_matrix_t::row_t::linear_comb_nnD(const row_t& row1, int mult1, 
                                                  const row_t& row2, int mult2) 
{
    int val1, val2;
    ssize_t j;
    // Add this->D = row1.D + row2.D and return immediately if this->D is empty.
    // Two phases: k=0 reserves the allocated space, k=1 inserts the values
    for (int k=0, reserved=0; k<2; k++) { 
        if (k == 1) {
            if (reserved == 0) {
                return false; // D is empty
            }
            this->D.reserve(reserved);
        }
        auto itD1 = row1.D.begin(), itD2 = row2.D.begin();
        while(-1 != (j = traverse_both(itD1, row1.D, val1, itD2, row2.D, val2))) {
            ssize_t value = ssize_t(val1) * mult1 + ssize_t(val2) * mult2;
            if (value > INT_MAX || value < INT_MIN) 
                throw program_exception("Integer overflow when combining flows.");
            // verify(value >= 0);
            if (value != 0) {
                if (k == 0) {
                    reserved++;
                }
                else {
                    this->D.insert_element_0(j, value);
                }
            }
        }
    }

    // Add this->A = row1.A + row2.A.
    // k=0 reserves the allocated space, k=1 inserts the values
    for (int k=0, reserved=0; k<2; k++) {
        if (k == 1) {
            if (reserved == 0) {
                break;
            }
            this->A.reserve(reserved);
        }
        auto itA1 = row1.A.begin(), itA2 = row2.A.begin();
        while(-1 != (j = traverse_both(itA1, row1.A, val1, itA2, row2.A, val2))) {
            ssize_t value = ssize_t(val1) * mult1 + ssize_t(val2) * mult2;
            if (value > INT_MAX || value < INT_MIN) 
                throw program_exception("Integer overflow when combining flows.");
            if (value != 0) {
                if (k == 0) {
                    reserved++;
                }
                else {
                    this->A.insert_element_0(j, value);
                }
            }
        }
    }
    return true;
}

//-----------------------------------------------------------------------------

inline void flow_matrix_t::row_t::canonicalize() {
    // The gcd of its non-zero elements must be 1
    int gcdAD = gcd_nnz_DA();
    size_t num_neg_nnz_D = 0;
    assert(gcdAD >= 1);
    if (gcdAD != 1) {
        for (size_t id=0; id<D.nonzeros(); id++) {
            D.set_nnz_value(id, D.ith_nonzero(id).value / gcdAD);
            if (D.ith_nonzero(id).value < 0)
                num_neg_nnz_D++;
        }
        for (size_t ia=0; ia<A.nonzeros(); ia++)
            A.set_nnz_value(ia, A.ith_nonzero(ia).value / gcdAD);
    }

    // if (num_neg_nnz_D > 0 && A.nonzeros() == 0) {
    //     if (num_neg_nnz_D * 2 < D.nonzeros() ||
    //         (num_neg_nnz_D * 2 == D.nonzeros() && D.ith_nonzero(0).value < 0))
    //     {
    //         // Invert all signs
    //         for (size_t id=0; id<D.nonzeros(); id++)
    //             D.set_nnz_value(id, -D.ith_nonzero(id).value);
    //     }
    // }

    // // If the vector has negative entries in D, the first entry must be positive
    // if (D.ith_nonzero(0).value < 0) {
    //     for (size_t id=0; id<D.nonzeros(); id++)
    //         D.set_nnz_value(id, -D.ith_nonzero(id).value);
    // }
}

//-----------------------------------------------------------------------------

inline size_t flow_matrix_t::row_t::count_negatives_D() const {
    size_t num = 0;
    for (size_t id=0; id<D.nonzeros(); id++)
        if (D.ith_nonzero(id).value < 0)
            num++;
    return num;
}

//-----------------------------------------------------------------------------

void flow_matrix_t::clear_A_vectors() {
    for (auto& row : mK)
        row.A.clear();
}

//-----------------------------------------------------------------------------

void incidence_matrix_generator_t::add_flow_entry(size_t i, size_t j, int cardinality) {
    assert(i < f.N && j < f.M);
    auto elem = initEntries.lower_bound(flow_entry_t(i, j, numeric_limits<int>::min()));
    if (elem != initEntries.end() && elem->i == i && elem->j == j) {
        cardinality += elem->card;
        initEntries.erase(elem);
    }
        // initEntries.insert(Flow(i, j, elem->card + cardinality));
    // else
    initEntries.insert(flow_entry_t(i, j, cardinality));
    // cout << "msa.add_flow_entry("<<i<<", "<<j<<", "<<cardinality<<");"<<endl;
}

//-----------------------------------------------------------------------------

// Insert flows from Petri net
void incidence_matrix_generator_t::add_flows_from(const PN& pn, bool print_warns) {
    // Load the incidence matrix into the flows_generator_t class
    bool warnForInhibitor = print_warns;
    bool warnForMarkingDep = print_warns;
    for (const Transition& trn : pn.trns) {
        if (warnForInhibitor && !trn.arcs[HA].empty()) {
            cerr << console::beg_error() << "WARNING: " << console::end_error() 
                 << "PETRI NET HAS INHIBITOR ARCS THAT WILL BE IGNORED." << endl;
            warnForInhibitor = false;
        }
        for (int k=0; k<2; k++) {
            ArcKind ak = (k==0 ? IA : OA);
            int sign = (ak==IA ? -1 : +1);
            for (const Arc& arc : trn.arcs[ak]) {
                if (arc.isMultMarkingDep()) {
                    if (warnForMarkingDep) {
                        cerr << console::beg_error() << "WARNING: " << console::end_error() 
                             << "PETRI NET HAS MARKING-DEPENDENT ARCS THAT WILL BE IGNORED." << endl;
                         warnForMarkingDep = false;
                    }
                }
                else {
                    int card = get_value(arc.getConstantMult()) * sign;
                    if (f.inv_kind == InvariantKind::PLACE)
                        add_flow_entry(arc.plc, trn.index, card);
                    else
                        add_flow_entry(trn.index, arc.plc, card);
                }
            }
        }
    }

}

//-----------------------------------------------------------------------------

void incidence_matrix_generator_t::generate_matrix() {
    // Initialize matrix K with the initEntries
    for (size_t i = 0; i < f.N; i++) {
        flow_matrix_t::row_t row(f);
        // Diagonal entry in D
        row.D.insert_element(i, 1);

        // Insert the flows in the A matrix, which starts as the incidence matrix
        auto it1 = initEntries.lower_bound(flow_entry_t(i, 0, numeric_limits<int>::min()));
        auto it2 = initEntries.lower_bound(flow_entry_t(i, numeric_limits<size_t>::max(), 0));
        if (it1 == it2 && i >= f.N0 && f.add_extra_vars)
            continue;
        for (; it1 != it2; ++it1) {
            assert(it1->i == i && it1->j < f.M);
            row.A.add_element(it1->j, it1->card);
        }

        f.mK.emplace_back(std::move(row));
    }
    initEntries.clear();
    f.mat_kind = FlowMatrixKind::INCIDENCE;
}

//-----------------------------------------------------------------------------

void incidence_matrix_generator_t::add_increase_decrease_flows() {
    assert(f.M == f.N - f.N0);
    assert(f.inc_dec == 1 || f.inc_dec == -1);

    // add an arc for each place N0+i from transition i
    for (size_t i=0; f.N0 + i < f.N; i++)
        add_flow_entry(f.N0 + i, i, f.inc_dec);
}

//-----------------------------------------------------------------------------

flows_generator_t::flows_generator_t(flow_matrix_t& _f, flow_algorithm_printer_t& _p, 
                                             VerboseLevel _verboseLvl) 
: verboseLvl(_verboseLvl), printer(_p), f(_f) { }

//-----------------------------------------------------------------------------

void flows_generator_t::init_A_columns_sum() {
    // Sum all the columns of A in matrix K
    A_cols_count.resize(f.M);
    std::fill(A_cols_count.begin(), A_cols_count.end(), make_pair(0, 0));
    for (const flow_matrix_t::row_t& row : f.mK) {
        for (auto elem : row.A) {
            if (elem.value > 0)
                ++A_cols_count[elem.index].first;
            else if (elem.value < 0)
                ++A_cols_count[elem.index].second;
        }
    }
}

//-----------------------------------------------------------------------------

inline void flows_generator_t::update_A_columns_sum(const spintvector& A, int mult) {
    for (auto elem : A) {
        if (elem.value > 0)
            A_cols_count[elem.index].first += mult; // +1 or -1
        else if (elem.value < 0)
            A_cols_count[elem.index].second += mult; // +1 or -1
    }
}

//-----------------------------------------------------------------------------

ostream& operator<<(ostream& os, const flows_generator_t& fg) {
    os << "\n\n=========================\nStep: " << fg.step << "\n";
    return fg.f.print(os, true);
}

//-----------------------------------------------------------------------------

void flows_generator_t::compute_semiflows() 
{
    const char* ALGO = "Generation of Semiflows";
    verify(f.mat_kind == FlowMatrixKind::INCIDENCE);
    init_A_columns_sum();
    bool update_neg_D_count = false;

    // Fourier-Motzkin Elimination procedure.
    // Matrix A starts with the incidence matrix, D starts as the identity.
    // Invariants: rows in [D|A] are linearly independent, and gcd(D,A)=1.
    for (step = 0; step < f.M; step++) {
        if (verboseLvl >= VL_VERY_VERBOSE)
            cout << *this << endl;

        // Pivoting: determine the column i which will generate less candidate rows
        ssize_t i = -1, num_prod = std::numeric_limits<ssize_t>::max();
        for (size_t k=0; k<f.M; k++) {
            int npos = A_cols_count[k].first, nneg = A_cols_count[k].second;
            if ((npos + nneg) == 0)
                continue;
            ssize_t new_num_prod = npos * nneg - npos - nneg; // number of generated entries
            if (num_prod > new_num_prod) { // pivot k is better than pivot i
                i = k;
                num_prod = new_num_prod;
            }
        }
        if (i == -1)
            break; // Nothing more to do.

        // Extract from the K=[D|A] matrix all the rows with A[i] != 0
        // These rows are removed from K, and are combined to generate the new rows
        std::list<flow_matrix_t::row_t> AiPositiveRows, AiNegativeRows;
        for (auto row = f.mK.begin(); row != f.mK.end(); ) {
            if (row->A[i] != 0) {
                update_A_columns_sum(row->A, -1);
                if (row->A[i] > 0) 
                    AiPositiveRows.splice(AiPositiveRows.begin(), f.mK, row++);
                else
                    AiNegativeRows.splice(AiNegativeRows.begin(), f.mK, row++);
            }
            else 
                ++row;
        }
        if (verboseLvl >= VL_VERY_VERBOSE)
            cout << "pivot="<<i<<": |K|=" << f.mK.size() 
                 << " |Ai+|="<<AiPositiveRows.size() << " |Ai-|="<<AiNegativeRows.size() << endl;

        printer.advance(ALGO, step, f.M, f.mK.size(), num_prod);

        if (AiPositiveRows.size() == 0 || AiNegativeRows.size() == 0) {
            if (verboseLvl >= VL_VERY_VERBOSE) { // Can we detect syphons/traps from this?
                cout << console::red_fgnd() << "NON-ANNULLABLE COLUMN i=" << i << console::default_disp() << endl; 
                // cout << "DEL ";
                // AiNonnullRows.begin()->print(cout, true) << endl << endl;
            }
            if (f.add_extra_vars) {
                // Add a new row that will annull column i, using an extra variable T_i
                int sgn = (AiPositiveRows.size() == 0) ? -1 : +1;
                flow_matrix_t::row_t newRow(f);
                newRow.D.insert_element(f.N0 + i, -sgn);
                newRow.A.insert_element(i, -sgn);
                newRow.neg_D = newRow.count_negatives_D();
                update_neg_D_count = true;
                newRow.gen_step = step;
                if (verboseLvl >= VL_VERY_VERBOSE) {
                    cout << "+++ ";
                    newRow.print(cout, true) << endl << endl;
                }
                if (AiPositiveRows.size() == 0)
                    AiPositiveRows.emplace_back(std::move(newRow));
                else
                    AiNegativeRows.emplace_back(std::move(newRow));
            }
            else
                continue; // Nothing to combine, simply throw away all the selected rows.
        }

        // Append to the matrix [D|A] every rows resulting as a non-negative
        // linear combination of row pairs from the rows with A[i] != 0
        for (auto row1 = AiPositiveRows.begin(); row1 != AiPositiveRows.end(); ++row1) {
            for (auto row2 = AiNegativeRows.begin(); row2 != AiNegativeRows.end(); ++row2) {
                // Find two rows r1 and r2 such that A[r1][i] and A[r2][i] have opposite sign.
                int abs1 = std::abs(row2->A[i]), abs2 = std::abs(row1->A[i]);
                int gcd12 = gcd(abs1, abs2);
                abs1 /= gcd12;
                abs2 /= gcd12;

                // Create newRow = row1 * mult1 + row2 * mult2
                flow_matrix_t::row_t newRow(f);
                if (!newRow.linear_comb_nnD(*row1, abs1, *row2, abs2))
                    continue; // dropped because newRow.D is empty
                assert(newRow.A[i] == 0);

                // Make newRow canonical, i.e. gcd of its non-zero elements is 1
                newRow.canonicalize();
                if (update_neg_D_count)
                    newRow.neg_D = newRow.count_negatives_D();

                // Test if newRow is not minimal.
                // A row is not minimal if another row in K shares the same support,
                // i.e. has non-zero values in at least the same places as newRow.
                // Test all the existing rows in K exhaustively.
                bool dropNewRow = false;
                for (auto row = f.mK.begin(); row != f.mK.end() && !dropNewRow;) {
                    if (newRow.test_minimal_support_D(row->D)) {
                        dropNewRow = true;
                        break;
                    }
                    else ++row;
                }
                if (dropNewRow) {
                    if (verboseLvl >= VL_VERY_VERBOSE)
                        cout << "DROP" << console::red_fgnd() << newRow 
                             << console::default_disp() << endl;
                    continue;
                }
                if (verboseLvl >= VL_VERY_VERBOSE) {
                    cout << "ADD ";
                    newRow.print(cout, true) << endl;
                }

                // Add newRow to K, and update the pre-computed column sums of A
                update_A_columns_sum(newRow.A, +1);
                // Beware: newRow must be inserted at the front!!!
                f.mK.emplace_front(std::move(newRow));
                if (f.mK.size() > max_peak_rows)
                    throw program_exception("Exceeded peak row count in semiflow generation.");
            }
        }
    }

    printer.advance(ALGO, f.M, f.M, f.mK.size(), step);
    f.mat_kind = FlowMatrixKind::SEMIFLOWS;
    if (verboseLvl >= VL_VERY_VERBOSE)    
        cout << f << endl;
    A_cols_count.clear();
    f.clear_A_vectors();
}

//-----------------------------------------------------------------------------

void flows_generator_t::compute_integer_flows() 
{
    const char* ALGO = "Generation of Minimal Flows";
    verify(f.mat_kind == FlowMatrixKind::INCIDENCE);
    init_A_columns_sum();

    // Fourier-Motzkin Elimination procedure for general flows (both positive and negative).
    // Matrix A starts with the incidence matrix, D starts as the identity.
    // Invariants: rows in [D|A] are linearly independent, and gcd(D,A)=1.
    for (step = 0; step < f.M; step++) {
        if (verboseLvl >= VL_VERY_VERBOSE)
            cout << *this << endl;

        // Pivoting: determine the column i which will generate less candidate rows
        ssize_t i = -1, num_prod = std::numeric_limits<ssize_t>::max();
        for (size_t k=0; k<f.M; k++) {
            int nrows = A_cols_count[k].first + A_cols_count[k].second;
            if (nrows == 0)
                continue;
            ssize_t new_num_prod = nrows * (nrows - 1); // number of generated entries
            if (num_prod > new_num_prod) { // pivot k is better than pivot i
                i = k;
                num_prod = new_num_prod;
            }
        }
        if (i == -1)
            break; // Nothing more to do.

        // Extract from the K=[D|A] matrix all the rows with A[i] != 0
        // These rows are removed from K, and are combined to generate the new rows
        std::list<flow_matrix_t::row_t> AiNonnullRows;
        size_t num_posi = 0;
        for (auto row = f.mK.begin(); row != f.mK.end(); ) {
            auto Ai = row->A[i];
            if (Ai != 0) {
                update_A_columns_sum(row->A, -1);
                AiNonnullRows.splice(AiNonnullRows.begin(), f.mK, row++);
                if (Ai > 0)
                    num_posi++;
            }
            else 
                ++row;
        }
        if (verboseLvl >= VL_VERY_VERBOSE)
            cout << "pivot="<<i<<": |K|=" << f.mK.size() 
                 << " |Ai_nn|="<<AiNonnullRows.size() << endl;

        printer.advance(ALGO, step, f.M, f.mK.size(), num_prod);

        if (num_posi == 0 || num_posi == AiNonnullRows.size()) {
            if (verboseLvl >= VL_VERY_VERBOSE) {
                cout << console::red_fgnd() << "ALL ROWS HAVE UNIFORM SIGN AT COLUMN i=" << i << console::default_disp() << endl; 
            }
            if (f.add_extra_vars) {
                // Add a new row that will annull column i, using an extra variable T_i
                int sgn = (num_posi == 0) ? -1 : +1;
                flow_matrix_t::row_t newRow(f);
                newRow.D.insert_element(f.N0 + i, -sgn);
                newRow.A.insert_element(i, -sgn);
                newRow.neg_D = newRow.count_negatives_D();
                newRow.gen_step = step;
                if (verboseLvl >= VL_VERY_VERBOSE) {
                    cout << "+++ ";
                    newRow.print(cout, true) << endl << endl;
                }
                AiNonnullRows.emplace_back(std::move(newRow));
            }
            else
                continue; // Nothing to combine, simply throw away all the selected rows.
        }
        /*if (AiNonnullRows.size() <= 1) { // Nothing to combine, simply throw away all the selected rows.
            if (verboseLvl >= VL_VERY_VERBOSE) { // Can we detect syphons/traps from this?
                cout << console::red_fgnd() << "NON-ANNULLABLE COLUMN i=" << i << console::default_disp() << endl; 
                cout << "DEL ";
                AiNonnullRows.begin()->print(cout, true) << endl << endl;
            }
            if (f.add_extra_vars) {
                // Add a new row that will annull column i, using an extra variable T_i
                flow_matrix_t::row_t newRow(f);
                int sgn = sign(AiNonnullRows.begin()->A[i]);
                newRow.D.insert_element(f.N0 + i, -sgn);
                newRow.A.insert_element(i, -sgn);
                if (verboseLvl >= VL_VERY_VERBOSE) {
                    cout << "+++ ";
                    newRow.print(cout, true) << endl << endl;
                }
                AiNonnullRows.emplace_back(std::move(newRow));
            }
            else
                continue; // Nothing to do.
        }*/

        // Append to the matrix [D|A] every rows resulting as a
        // linear combination of row pairs from the rows with A[i] != 0
        size_t index1 = 0;
        for (auto row1 = AiNonnullRows.begin(); row1 != AiNonnullRows.end(); ++row1, ++index1) {
            size_t index2 = 0;
            for (auto row2 = AiNonnullRows.begin(); row2 != AiNonnullRows.end(); ++row2, ++index2) {
                if (index1 >= index2)
                    continue; // skip same rows and avoid making both A+B and B+A.

                // Get the multiplier coefficients such that row1*mult1 + row2*mult2 annuls A[i]
                int mult1 = std::abs(row2->A[i]), mult2 = std::abs(row1->A[i]);
                int gcd12 = gcd(mult1, mult2);
                mult1 /= gcd12;
                mult2 /= gcd12;
                if (sign(row1->A[i]) == sign(row2->A[i]))
                    mult1 *= -1;

                // Create newRow = row1 * mult1 + row2 * mult2
                flow_matrix_t::row_t newRow(f);
                if (!newRow.linear_comb_nnD(*row1, mult1, *row2, mult2))
                    continue; // dropped because newRow.D is empty
                newRow.neg_D = newRow.count_negatives_D();

                assert(newRow.A[i] == 0);

                // Make newRow canonical, i.e. gcd of its non-zero elements is 1
                newRow.canonicalize();

                // Test if newRow is not minimal.
                // A row is not minimal if another row in K shares the same support,
                // i.e. has non-zero values in at least the same places as newRow.
                // Test all the existing rows in K exhaustively.
                bool dropNewRow = false;
                for (auto row = f.mK.begin(); row != f.mK.end() && !dropNewRow;) {
                    if (newRow.test_minimal_support_D(row->D)) {
                        dropNewRow = true;
                        break;
                    }
                    else ++row;
                }
                if (dropNewRow) {
                    if (verboseLvl >= VL_VERY_VERBOSE)
                        cout << "DROP" << console::red_fgnd() << newRow 
                             << console::default_disp() << endl;
                    continue;
                }
                if (verboseLvl >= VL_VERY_VERBOSE) {
                    cout << "ADD ";
                    newRow.print(cout, true) << endl;
                }

                // // Unlike P-semiflows, whose support cannot decrease (monotonic property),
                // // for integer P-flow the support may be smaller. Therefore, entries in mK
                // // could now have a support smaller than newRow
                // for (auto row = f.mK.begin(); row != f.mK.end(); /**/) {
                //     if (row->test_minimal_support_D(newRow.D)) {
                //         cout << "DROP" << endl;
                //         row = f.mK.erase(row); // drop and continue;
                //     }
                //     else 
                //         ++row; // continue
                // }

                // Add newRow to K, and update the pre-computed column sums of A
                update_A_columns_sum(newRow.A, +1);
                // Beware: newRow must be inserted at the front!!!
                f.mK.emplace_front(std::move(newRow));
                if (f.mK.size() > max_peak_rows)
                    throw program_exception("Exceeded peak row count in flow generation.");
            }
        }
    }

    printer.advance(ALGO, f.M, f.M, f.mK.size(), step);
    f.mat_kind = FlowMatrixKind::INTEGER_FLOWS;
    if (verboseLvl >= VL_VERY_VERBOSE)    
        cout << f << endl;
    A_cols_count.clear();
    f.clear_A_vectors();
}

//-----------------------------------------------------------------------------

/*void flows_generator_t::compute_nested_flow_span() 
{
    const char* ALGO = "Generation of Nested Flows Span";
    verify(f.mat_kind == FlowMatrixKind::INCIDENCE);
    init_A_columns_sum();

    // Fourier-Motzkin Elimination procedure for general flows (both positive and negative).
    // Matrix A starts with the incidence matrix, D starts as the identity.
    // Invariants: rows in [D|A] are linearly independent, and gcd(D,A)=1.
    for (step = 0; step < f.M; step++) {
        if (verboseLvl >= VL_VERY_VERBOSE)
            cout << *this << endl;

        // TODO: fix
        // Pivoting: determine the column i which will generate less candidate rows
        ssize_t i = -1, num_prod = std::numeric_limits<ssize_t>::max();
        for (size_t k=0; k<f.M; k++) {
            int nrows = A_cols_count[k].first + A_cols_count[k].second;
            if (nrows == 0)
                continue;
            ssize_t new_num_prod = nrows * (nrows - 1); // number of generated entries
            if (num_prod > new_num_prod) { // pivot k is better than pivot i
                i = k;
                num_prod = new_num_prod;
            }
        }
        if (i == -1)
            break; // Nothing more to do.

        // Extract from the K=[D|A] matrix all the rows with A[i] != 0
        // These rows are removed from K, and are combined to generate the new rows
        std::list<flow_matrix_t::row_t> AiNonnullRows;
        for (auto row = f.mK.begin(); row != f.mK.end(); ) {
            if (row->A[i] != 0) {
                update_A_columns_sum(row->A, -1);
                AiNonnullRows.splice(AiNonnullRows.begin(), f.mK, row++);
            }
            else 
                ++row;
        }
        if (verboseLvl >= VL_VERY_VERBOSE)
            cout << "pivot="<<i<<": |K|=" << f.mK.size() 
                 << " |Ai_nn|="<<AiNonnullRows.size() << endl;

        printer.advance(ALGO, step, f.M, f.mK.size(), num_prod);

        if (AiNonnullRows.size() <= 1)
            continue; // Nothing to combine, simply throw away all the selected rows.

        // Append to the matrix [D|A] every rows resulting as a
        // linear combination of row pairs from the rows with A[i] != 0
        for (size_t phase = 0; phase < 2; phase++) {
            size_t index1 = 0;
            for (auto row1 = AiNonnullRows.begin(); row1 != AiNonnullRows.end(); ++row1, ++index1) {
                size_t index2 = 0;
                for (auto row2 = AiNonnullRows.begin(); row2 != AiNonnullRows.end(); ++row2, ++index2) {
                    if (index1 >= index2)
                        continue; // skip same rows and avoid making both A+B and B+A.
                    // Start first with the integer flows, then pass to the semiflows.
                    // In this way, when we build the semiflows we already have the integer flows
                    // built, to simplify them.
                    bool combine_subtract = (sign(row1->A[i]) == sign(row2->A[i]));
                    if ((phase == 1) == combine_subtract)
                        continue; // phase 0: subtract.  phase 1: add
                    // Subtract only if we have at least a common place between row1 and row2
                    // if (combine_subtract && row1->test_common_nonzeros(row2->D))
                    //     continue;
                    if ((row1->is_negative() || row2->is_negative()) && 
                        row1->test_disjoint_supports(*row2, combine_subtract ? -1 : +1))
                        continue;
                    // // Do not combine integer flows togheter
                    // if (row1->is_negative() && row2->is_negative())
                    //     continue;
                    // if (combine_subtract && row1->is_negative() &&
                    //     !row1->test_minimal_positive_support_D(row2->D, +1))
                    //     continue;
                    // if (combine_subtract && row2->is_negative() &&
                    //     !row2->test_minimal_positive_support_D(row1->D, +1))
                    //     continue;

                    // Get the multiplier coefficients such that row1*mult1 + row2*mult2 annuls A[i]
                    int mult1 = std::abs(row2->A[i]), mult2 = std::abs(row1->A[i]);
                    int gcd12 = gcd(mult1, mult2);
                    mult1 /= gcd12;
                    mult2 /= gcd12;
                    if (combine_subtract)
                        mult1 *= -1;

                    // Create newRow = row1 * mult1 + row2 * mult2
                    flow_matrix_t::row_t newRow(f);
                    if (!newRow.linear_comb_nnD(*row1, mult1, *row2, mult2))
                        continue; // dropped because newRow.D is empty
                    newRow.neg_D = newRow.count_negatives_D();
                    newRow.gen_step = step;

                    if (newRow.neg_D == newRow.D.nonzeros())
                        continue;

                    assert(newRow.A[i] == 0);

                    // Make newRow canonical, i.e. gcd of its non-zero elements is 1
                    newRow.canonicalize();


                    // TODO: remove these checks.
                    bool stop = false;
                    for (auto e : newRow.D) {
                        if (abs(e.value) > 1) 
                            stop = true;
                    }
                    for (auto e : newRow.A) {
                        if (abs(e.value) > 1) 
                            stop = true;
                    }
                    if (newRow.neg_D == newRow.D.nonzeros())
                        stop = true;
                        
                    if (stop) {
                        cout << endl;
                        cout << "row1   " << *row1 << endl;
                        cout << "row2   " << *row2 << endl;
                        cout << "newRow " << newRow << endl;
                        cout << "mult1 " << mult1 << endl;
                        cout << "mult2 " << mult2 << endl;
                        cout << "combine_subtract " << combine_subtract << endl;
                        cout << "row1->is_negative() " << row1->is_negative() << endl;
                        cout << "row2->is_negative() " << row2->is_negative() << endl;
                        exit(0);
                    }


                    bool dropNewRow = false;
                    // Apply all reductions to newRow
                    for (auto row = f.mK.begin(); row != f.mK.end() && !dropNewRow; ) {
                        if (row->is_negative() 
                            //&& row->A.nonzeros() == 0 
                            / *&& row->test_minimal_positive_support_D(newRow.D, +1)* /)
                        {
                            int m1, m2;
                            if (newRow.test_subst(row->D, m1, m2)) {
                                cout << "SBST" << console::green_fgnd() << newRow 
                                               << console::default_disp() << endl;
                                cout << "  + " << console::green_fgnd() << *row 
                                               << console::default_disp() << endl;
                                flow_matrix_t::row_t tmpRow(f);
                                if (!tmpRow.linear_comb_nnD(newRow, m1, *row, m2)) {
                                    if (verboseLvl >= VL_VERY_VERBOSE)
                                        cout << "  NO" << console::red_fgnd() << tmpRow 
                                             << console::default_disp() << endl;
                                    dropNewRow = true;
                                    continue; // dropped because newRow.D is empty
                                }
                                tmpRow.canonicalize();
                                tmpRow.neg_D = tmpRow.count_negatives_D();
                                tmpRow.gen_step = step;
                                newRow = std::move(tmpRow);
                                cout << "  = " << console::green_fgnd() << newRow 
                                               << console::default_disp() << endl;
                                // TODO: Restart from the beginning
                                // row = f.mK.begin();
                            }
                        }
                        ++row;
                    }
                    if (dropNewRow)
                        continue;

                    // Test if newRow is not minimal.
                    // A row is not minimal if another row in K shares the same support,
                    // i.e. has non-zero values in at least the same places as newRow.
                    // Test all the existing rows in K exhaustively.
                    for (auto row = f.mK.begin(); row != f.mK.end() && !dropNewRow;) {
                        if (newRow.test_minimal_support_D(row->D)) {
                        // if (newRow.test_minimal_positive_support_D(row->D, -1)) {
                            if (verboseLvl >= VL_VERY_VERBOSE)
                                cout << "DROP" << console::red_fgnd() << newRow 
                                     << console::default_disp() << endl;
                            dropNewRow = true;
                            break;
                        }
                        // else if (row->is_negative() / *&& row->A.nonzeros() == 0* /) {
                        //     // Test if newRow +/- row already exists in K
                        //     for (auto&& row2 : f.mK) {
                        //         if (newRow.test_minimal_support_linear_comb_D(row2.D, row->D)) {
                        //             if (verboseLvl >= VL_VERY_VERBOSE) {
                        //                 cout << "DROP" << console::magenta_fgnd() << newRow 
                        //                      << console::default_disp() << endl;
                        //                 cout << "  D2" << console::cyan_fgnd() << row2 
                        //                      << console::default_disp() << endl;
                        //                 cout << "   R" << console::cyan_fgnd() << *row 
                        //                      << console::default_disp() << endl;
                        //             }
                        //             dropNewRow = true;
                        //             break; 
                        //         }
                        //     }
                        //     if (dropNewRow)
                        //         break;
                        // }
                        // else if (row->is_negative() && //row->A.nonzeros() == 0 &&
                        //          // (newRow.test_minimal_positive_support_D(row->D, +1) ||
                        //           newRow.test_minimal_positive_support_D(row->D, -1)) 
                        // {
                        //     // Throw iff K already contains newRow - row
                        //     if (verboseLvl >= VL_VERY_VERBOSE)
                        //         cout << "DROP" << console::magenta_fgnd() << newRow 
                        //              << console::default_disp() << endl;
                        //     dropNewRow = true;
                        //     break;                        
                        // }
                        ++row;
                    }
                    if (dropNewRow)
                        continue;

                    if (verboseLvl >= VL_VERY_VERBOSE) {
                        cout << "ADD ";
                        newRow.print(cout, true) << endl;
                    }

                    if (newRow.is_negative() / *&& newRow.A.nonzeros() == 0* /) {
                        // We discovered a new integer flow invariant!
                        for (auto ex_row = f.mK.begin(); ex_row != f.mK.end(); ) {
                            int m1, m2;
                            if (ex_row->gen_step != step)
                                break;
                            if (ex_row->test_subst(newRow.D, m1, m2)) {
                                update_A_columns_sum(ex_row->A, -1);
                                cout << "SBST" << console::cyan_fgnd() << *ex_row 
                                               << console::default_disp() << endl;
                                cout << "  + " << console::cyan_fgnd() << newRow 
                                               << console::default_disp() << endl;
                                flow_matrix_t::row_t tmpRow(f);
                                if (!tmpRow.linear_comb_nnD(*ex_row, m1, newRow, m2)) {
                                    if (verboseLvl >= VL_VERY_VERBOSE)
                                        cout << "  NO" << console::red_fgnd() << tmpRow 
                                             << console::default_disp() << endl;
                                    // dropped because newRow.D is empty
                                    ex_row = f.mK.erase(ex_row);
                                    continue;
                                }
                                tmpRow.canonicalize();
                                tmpRow.neg_D = tmpRow.count_negatives_D();
                                tmpRow.gen_step = ex_row->gen_step;

                                // Test unique support of the modified row
                                bool dropExRow = false;
                                for (auto row = f.mK.begin(); row != f.mK.end() && !dropExRow;) {
                                    if (tmpRow.test_minimal_support_D(row->D)) {
                                        if (verboseLvl >= VL_VERY_VERBOSE)
                                            cout << "DROP" << console::cyan_fgnd() << tmpRow 
                                                 << console::default_disp() << endl;
                                        dropExRow = true;
                                        break;
                                    }
                                    ++row;
                                }
                                if (dropExRow) {
                                    ex_row = f.mK.erase(ex_row);
                                    continue;
                                }

                                *ex_row = std::move(tmpRow);
                                update_A_columns_sum(ex_row->A, +1);
                                cout << "  = " << console::cyan_fgnd() << *ex_row 
                                               << console::default_disp() << endl;
                            }
                            ++ex_row;
                        }
                    }

                    // Add newRow to K, and update the pre-computed column sums of A
                    update_A_columns_sum(newRow.A, +1);
                    // Beware: newRow must be inserted at the front!!!
                    f.mK.emplace_front(std::move(newRow));
                    if (f.mK.size() > max_peak_rows)
                        throw program_exception("Exceeded peak row count in nested flow generation.");
                }
            }
        }
    }

    printer.advance(ALGO, f.M, f.M, f.mK.size(), step);
    f.mat_kind = FlowMatrixKind::NESTED_FLOW_SPAN;
    if (verboseLvl >= VL_VERY_VERBOSE)    
        cout << f << endl;
    f.clear_A_vectors();
    A_cols_count.clear();
}*/

//-----------------------------------------------------------------------------

void flows_generator_t::compute_basis() 
{
    const char* ALGO = "Computation of Flow basis";
    verify(f.mat_kind == FlowMatrixKind::INCIDENCE);
    init_A_columns_sum();

    // Triangularization of the incidence matrix procedure.
    // Matrix A starts with the incidence matrix, D starts as the identity.
    // Rows in matrix D become a basis of flows for the PN
    for (step = 0; step < f.M; step++) {
        if (verboseLvl >= VL_VERY_VERBOSE)
            cout << *this << endl;

        // Pivoting: determine the column i of A for which either npos=nneg=1, or npos*nneg is minimum
        ssize_t i = -1, num_sums = std::numeric_limits<ssize_t>::max();
        for (size_t phase=0; phase<2 && i==-1; phase++) {
            for (size_t k=0; k<f.M; k++) {
                int npos = A_cols_count[k].first, nneg = A_cols_count[k].second;
                if ((npos + nneg) == 0)
                    continue;
                if (phase == 0 && npos==1 && nneg==1) { // select k when npos==nneg
                    i = k;
                    num_sums = 1;
                    break;
                } 
                else if (phase == 1 && num_sums > npos * nneg) { // pivot k is better than pivot i
                    i = k;
                    num_sums = (npos + nneg - 1);
                }
            }
        }
        if (i == -1)
            break; // Nothing more to do.

        // Extract from the K=[D|A] matrix all the rows with A[i] != 0
        std::list<flow_matrix_t::row_t> AiNonnullRows;
        size_t num_posi = 0;
        for (auto row = f.mK.begin(); row != f.mK.end(); ) {
            auto Ai = row->A[i];
            if (Ai != 0) {
                update_A_columns_sum(row->A, -1);
                AiNonnullRows.splice(AiNonnullRows.begin(), f.mK, row++);
                if (Ai > 0)
                    num_posi++;
            }
            else 
                ++row;
        }
        if (verboseLvl >= VL_VERY_VERBOSE)
            cout << "pivot="<<i<<": |K|=" << f.mK.size() 
                 << " |Ai_nn|="<<AiNonnullRows.size() << endl;

        printer.advance(ALGO, step, f.M, f.mK.size(), num_sums);

        if (num_posi == 0 || num_posi == AiNonnullRows.size()) {
            if (verboseLvl >= VL_VERY_VERBOSE) {
                cout << console::red_fgnd() << "ALL ROWS HAVE UNIFORM SIGN AT COLUMN i=" << i << console::default_disp() << endl; 
            }
            if (f.add_extra_vars) {
                // Add a new row that will annull column i, using an extra variable T_i
                int sgn = (num_posi == 0) ? -1 : +1;
                flow_matrix_t::row_t newRow(f);
                newRow.D.insert_element(f.N0 + i, -sgn);
                newRow.A.insert_element(i, -sgn);
                newRow.neg_D = newRow.count_negatives_D();
                newRow.gen_step = step;
                if (verboseLvl >= VL_VERY_VERBOSE) {
                    cout << "+++ ";
                    newRow.print(cout, true) << endl << endl;
                }
                AiNonnullRows.emplace_back(std::move(newRow));
            }
            else
                continue; // Nothing to combine, simply throw away all the selected rows.
        }
        /*if (AiNonnullRows.size() <= 1) { // This row cannot be annulled, will never form a flow
            if (verboseLvl >= VL_VERY_VERBOSE) { // Can we detect syphons/traps from this?
                cout << console::red_fgnd() << "NON-ANNULLABLE COLUMN i=" << i << console::default_disp() << endl; 
                cout << "DEL ";
                AiNonnullRows.begin()->print(cout, true) << endl << endl;
            }
            if (f.add_extra_vars) {
                // Add a new row that will annull column i, using an extra variable T_i
                flow_matrix_t::row_t newRow(f);
                int sgn = sign(AiNonnullRows.begin()->A[i]);
                newRow.D.insert_element(f.N0 + i, -sgn);
                newRow.A.insert_element(i, -sgn);
                if (verboseLvl >= VL_VERY_VERBOSE) {
                    cout << "+++ ";
                    newRow.print(cout, true) << endl << endl;
                }
                AiNonnullRows.emplace_back(std::move(newRow));
            }
            else
                continue; // Nothing to do.
        }*/


        // Select the row that will be added to all the others
        std::list<flow_matrix_t::row_t> sel_row;
        for (auto row = AiNonnullRows.begin(); row != AiNonnullRows.end(); row++) {
            if (!row->is_negative()) {
                sel_row.splice(sel_row.begin(), AiNonnullRows, row);
                break;
            }
        }
        if (sel_row.empty()) {
            sel_row.splice(sel_row.begin(), AiNonnullRows, AiNonnullRows.begin());
        }

        // Append to the matrix [D|A] all the rows resulting from linear combination
        // of row AiNonnullRows[0] with all other rows AiNonnullRows[>0], such that
        // the i-th column of each summed row is zeroed.
        auto row1 = sel_row.begin();
        for (auto row2 = AiNonnullRows.begin(); row2 != AiNonnullRows.end(); ++row2) {
            // Get the multiplicative factors
            int mult1 = std::abs(row2->A[i]), mult2 = std::abs(row1->A[i]);
            int gcd12 = gcd(mult1, mult2);
            mult1 /= gcd12;
            mult2 /= gcd12;
            // If both rows have the same sign on the A[i] entry, change the sign of the first multiplier
            if (sign(row1->A[i]) == sign(row2->A[i]))
                mult1 *= -1;

            // Create newRow = row1 * mult1 + row2 * mult2
            flow_matrix_t::row_t newRow(f);
            if (!newRow.linear_comb_nnD(*row1, mult1, *row2, mult2))
                continue; // dropped because newRow.D is empty
            newRow.neg_D = newRow.count_negatives_D();
            newRow.gen_step = step;
            assert(newRow.A[i] == 0);

            // Make newRow canonical, i.e. gcd of its non-zero elements is 1
            newRow.canonicalize();
            
            if (verboseLvl >= VL_VERY_VERBOSE) {
                cout << "ADD ";
                newRow.print(cout, true) << endl;
            }

            // Add newRow to K, and update the pre-computed column sums of A
            update_A_columns_sum(newRow.A, +1);
            f.mK.emplace_back(std::move(newRow));
            if (f.mK.size() > max_peak_rows) // can this really happen? 
                throw program_exception("Exceeded peak row count in basis generation.");
        }
    }

    printer.advance(ALGO, f.M, f.M, f.mK.size(), step);
    f.mat_kind = FlowMatrixKind::BASIS;
    if (verboseLvl >= VL_VERY_VERBOSE)    
        cout << f << endl;
    A_cols_count.clear();
    f.clear_A_vectors();
}

//-----------------------------------------------------------------------------

shared_ptr<flow_matrix_t>
ComputeFlows(const PN& pn, InvariantKind inv_kind, FlowMatrixKind mat_kind, 
             bool detect_exp_growth, int inc_dec, VerboseLevel verboseLvl) 
{
    if (verboseLvl >= VL_BASIC) {
        cout << "COMPUTING " << GetFlowName(inv_kind, mat_kind) << "..." << endl;
    }
    bool is_id = (inc_dec != 0);
    bool dynamic_extra_var_gen = (inc_dec == 2);

    shared_ptr<flow_matrix_t> pfm;
    size_t N, M, N0;
    if (inv_kind == InvariantKind::PLACE) { // P-invariants
        N0 = pn.plcs.size();
        N  = N0 + (is_id ? pn.trns.size() : 0);
        M  = pn.trns.size();
    }
    else { // T-invariants
        N0 = pn.trns.size();
        N  = N0 + (is_id ? pn.plcs.size() : 0);
        M  = pn.plcs.size();
    }
    pfm = make_shared<flow_matrix_t>(N, N0, M, inv_kind, inc_dec, dynamic_extra_var_gen);

    // Initialize the flow matrix with the incidence matrix
    incidence_matrix_generator_t inc_gen(*pfm);
    inc_gen.add_flows_from(pn, verboseLvl >= VL_BASIC);
    if (is_id && !dynamic_extra_var_gen)
        inc_gen.add_increase_decrease_flows();
    inc_gen.generate_matrix();

    // Message printer
    class fa_printer_t : public flow_algorithm_printer_t {
    public:
        fa_printer_t(VerboseLevel vl) : verboseLvl(vl) { }
        long_interval_timer msgTimer;
        VerboseLevel verboseLvl;

        virtual void advance(const char* algo, size_t step, size_t totalSteps, 
                             size_t size_K, ssize_t num_prods) {
            bool veryVerb = (verboseLvl >= VL_VERY_VERBOSE);
            if (verboseLvl == VL_NONE)
                return; // silent
            if (step == totalSteps) { // final message
                if (!veryVerb) 
                    msgTimer.clear_any();
                cout << algo << ": completed in " << num_prods
                     <<" steps, |K|="<<size_K<<".                      " << endl;

            } else { // intermediate message
                if (msgTimer || num_prods > 4000 || veryVerb) {
                    if (!veryVerb) 
                        msgTimer.prepare_print();
                    cout << algo << ": step " << (step+1) << "/" << totalSteps 
                         << ", |K|="<<size_K;
                    if (num_prods>0)
                        cout << ", products="<<num_prods;
                    cout << endl;
                }
            }
        }

    } printer(verboseLvl);

    // Initialize generator
    flows_generator_t sf_gen(*pfm, printer, verboseLvl);
    if (detect_exp_growth)
        sf_gen.max_peak_rows = 5 * pn.plcs.size();

    // Start the computation of the P/T semiflows/basis/flows
    switch (mat_kind) {
        case FlowMatrixKind::SEMIFLOWS:
            sf_gen.compute_semiflows();
            break;
        case FlowMatrixKind::BASIS:
            sf_gen.compute_basis();
            break;
        case FlowMatrixKind::INTEGER_FLOWS:
            sf_gen.compute_integer_flows();
            break;
         // case FlowMatrixKind::NESTED_FLOW_SPAN:
         //    sf_gen.compute_nested_flow_span();
         //    break;
        default:
            throw program_exception("Unknown kind of flows!");
    }

    if (verboseLvl >= VL_BASIC) {
        if (mat_kind == FlowMatrixKind::BASIS) {
            cout << "FOUND " << pfm->num_flows()
                 << " VECTORS IN THE " << GetFlowName(inv_kind, pfm->mat_kind);
        }
        else {
            cout << "FOUND " << pfm->num_flows()
                 << " " << GetFlowName(inv_kind, pfm->mat_kind);
        }
        size_t num_neg = 0;
        for (auto&& row : pfm->mK)
            num_neg += (row.is_negative() ? 1 : 0);
        if (num_neg > 0) 
            cout << " (" << (pfm->num_flows() - num_neg) << " semiflows, " << num_neg << " flows)";
        cout << ".\n" << endl;
    }

    return pfm;
}

//-----------------------------------------------------------------------------

// Save semiflows in the GreatSPN format
void SaveFlows(const flow_matrix_t& psfm, ofstream& file) {
    file << psfm.num_flows() << "\n";

    for (flow_matrix_t::const_iterator flow = psfm.begin(); flow != psfm.end(); ++flow) {
        file << count_nonzeros(*flow) << " ";
        for (const auto& elem : *flow) {
            file << elem.value << " " << (elem.index+1) << " ";
        }
        file << "\n";
    }
    file << "0" << endl;
}

//-----------------------------------------------------------------------------

void PrintFlows(const PN& pn, const flow_matrix_t& psfm, 
                const char* cmd, VerboseLevel verboseLvl) 
{
    size_t num = 1;
    bool pinv = (psfm.inv_kind == InvariantKind::PLACE);
    bool semi = (psfm.mat_kind == FlowMatrixKind::SEMIFLOWS);

    if (verboseLvl > VL_BASIC) {
        const int space = int(std::log10(psfm.num_flows())) + 1;
        for (flow_matrix_t::const_iterator flow = psfm.begin(); flow != psfm.end(); ++flow) {
            cout << (semi ? "INV " : "FLOW ") << setw(space) << num++ << ": ";

            for (int i=0; i<2; i++) { // first positive, then negative entries
                for (const auto& elem : *flow) {
                    if ((i == 0 && elem.value < 0) || (i == 1 && elem.value > 0))
                        continue;

                    if (i == 1 && elem.index < psfm.N0)
                        cout << console::yellow_fgnd();
                    if (elem.index >= psfm.N0)
                        cout << console::cyan_fgnd();

                    if (elem.value == -1)
                        cout << "-";
                    else if (elem.value != 1)
                        cout << elem.value << "*";
                    if (pinv) {
                        if (elem.index < pn.plcs.size())
                            cout << pn.plcs[elem.index].name;
                        else
                            cout << "[" << pn.trns[elem.index - pn.plcs.size()].name << "]";
                            // cout << "[T" << (elem.index - pn.plcs.size()) << "]";
                    }
                    else {
                        if (elem.index < pn.trns.size())
                            cout << pn.trns[elem.index].name;
                        else
                            cout << "[P" << (elem.index - pn.trns.size()) << "]";
                    }
                    cout << " ";
                    if (i == 1 || elem.index >= psfm.N0)
                        cout << console::default_disp();
                }
            }

            // for (const auto& elem : *flow) { // positive entries
            //     if (elem.value < 0)
            //         continue;
            //     if (elem.value != 1)
            //         cout << elem.value << "*";
            //     cout << (pinv ? pn.plcs[elem.index].name : pn.trns[elem.index].name) << " ";
            // }
            // for (const auto& elem : *flow) { // negative entries
            //     if (elem.value > 0)
            //         continue;
            //     cout << console::yellow_fgnd();
            //     if (elem.value == -1)
            //         cout << "-";
            //     else if (elem.value != 1)
            //         cout << elem.value << "*";
            //     cout << (pinv ? pn.plcs[elem.index].name : pn.trns[elem.index].name) << " ";
            //     cout << console::default_disp();
            // }
            cout << endl;
        }
    }
    // else if (verboseLvl == VL_BASIC) {
    //     cout << "  Use option -v (verbose) before the "<<cmd<<" command "
    //             "to print the flows on stdout.\n";
    // }

    if (verboseLvl >= VL_BASIC) {
        // Compute coverage
        std::vector<bool> covered(pinv ? pn.plcs.size() : pn.trns.size(), false);
        for (auto&& flow : psfm)
            for (auto&& elem : flow)
                if (elem.index < psfm.N0)
                    covered[elem.index] = true;

        if (exists(covered.begin(), covered.end(), false)) {
            size_t num_uncovered = 0;
            for (bool c : covered)
                num_uncovered += (c ? 0 : 1);
            const char* notcov_p = " places not covered by any P-";
            const char* notcov_t = " transitions not covered by any T-";
            cout << "\nThere are " << num_uncovered << (pinv ? notcov_p : notcov_t) 
                   << (semi ? "semiflow:" : "flow:") << endl;
            if (verboseLvl >= VL_VERBOSE) {
                for (size_t k=0; k<covered.size(); k++) {
                    if (!covered[k])
                        cout << (pinv ? pn.plcs[k].name : pn.trns[k].name) << " ";
                }
                cout << endl;
            }
        }
        else {
            const char* cov_p = "\nAll places are covered by some P-";
            const char* cov_t = "\nAll transitions are covered by some T-";
            cout << (pinv ? cov_p : cov_t) << (semi ? "semiflow." : "flow.") << endl;
        }
    }
}

//-----------------------------------------------------------------------------

// Compute place bounds using the P-semiflows
void ComputeBoundsFromSemiflows(const PN& pn, const flow_matrix_t& semiflows, 
                                place_bounds_t& bounds) 
{
    assert(semiflows.mat_kind == FlowMatrixKind::SEMIFLOWS);
    assert(semiflows.inv_kind == InvariantKind::PLACE);
    bounds.resize(pn.plcs.size());
    std::fill(bounds.begin(), bounds.end(), PlaceBounds{ 0, numeric_limits<int>::max() });

    for (const auto& sf : semiflows) {
        // Get the amount of tokens circulating in semiflow @sf
        int tokenCnt = 0;
        for (auto& elem : sf)
            tokenCnt += elem.value * int(pn.plcs[elem.index].getInitTokenCount());

        int kk = -1;
        // Set upper bounds for all the places in semiflow @sf
        for (auto& elem : sf) {
            kk = tokenCnt / elem.value;
            bounds[elem.index].upper = std::min(bounds[elem.index].upper, kk);
        }
        // Set lower bounds for all the places in @sf
        if (sf.nonzeros() == 1 && kk > bounds[sf.front_nonzero().index].lower)
            bounds[sf.front_nonzero().index].lower = kk;
    }
}

//-----------------------------------------------------------------------------

// Save place bounds in GreatSPN format
void SaveBounds(const place_bounds_t& bounds, ofstream& file) {
    for (auto& b : bounds) {
        file << b.lower << " " << (b.is_upper_bounded() ? b.upper : -1) << "\n";
    }
    file << flush;
}

//-----------------------------------------------------------------------------

void LoadBounds(const PN& pn, place_bounds_t& bounds, ifstream& file)
{
    bounds.resize(pn.plcs.size(), PlaceBounds{0, numeric_limits<int>::max()});

    size_t i = 0;
    while (file && i < bounds.size()) {
        int lowerBoundP, upperBoundP;
        file >> lowerBoundP >> upperBoundP;
        // The bnd file may have both <=0 or 2147483647 as "unknown bounds".
        if (upperBoundP >= 2147483647 || upperBoundP <= 0)
            bounds[i].upper = numeric_limits<int>::max();
        else
            bounds[i].upper = upperBoundP;

        bounds[i].lower = lowerBoundP;
        i++;
    }
}

//-----------------------------------------------------------------------------

void PrintBounds(const PN& pn, const place_bounds_t& bounds, VerboseLevel verboseLvl) {
    if (verboseLvl > VL_BASIC) {
        size_t max_plc_len = 0;
        for (auto& plc : pn.plcs)
            max_plc_len = std::max(max_plc_len, plc.name.size());

        for (size_t p = 0; p<pn.plcs.size(); p++) {
            cout << setw(max_plc_len) << pn.plcs[p].name << ": ["
                 << bounds[p].lower << ", ";
            if (bounds[p].upper != numeric_limits<int>::max())
                cout << bounds[p].upper;
            else
                cout << "inf";
            cout << "]" << endl;
        }
    }
}

//-----------------------------------------------------------------------------















