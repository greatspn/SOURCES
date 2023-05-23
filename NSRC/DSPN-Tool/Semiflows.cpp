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
#include <cassert>
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

inline int lcm(int a, int b) {
    return (a * b) / gcd(a, b);
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

std::string GetGreatSPN_FileExt(invariants_spec_t is) {
    std::string ext;

    if (is.system_kind == SystemMatrixType::SIPHONS)
        ext = ".siphons";
    else if (is.system_kind == SystemMatrixType::TRAPS)
        ext = ".traps";
    else {
        switch (is.matk) {
            case FlowMatrixKind::SEMIFLOWS:
                ext = (is.invknd == InvariantKind::PLACE) ? ".pin" : ".tin";
                break;
            case FlowMatrixKind::BASIS:
                ext = (is.invknd == InvariantKind::PLACE) ? ".pba" : ".tba";
                break;
            case FlowMatrixKind::INTEGER_FLOWS:
                ext = (is.invknd == InvariantKind::PLACE) ? ".pfl" : ".tfl";
                break;
            default:
                throw program_exception("Internal error in GetGreatSPN_FileExt");
        }
        if (is.suppl_flags & FM_POSITIVE_SUPPLEMENTARY)
            ext += "+";
        if (is.suppl_flags & FM_NEGATIVE_SUPPLEMENTARY)
            ext += "-";
        if (is.suppl_flags & FM_ON_THE_FLY_SUPPL_VARS)
            ext += "o";
        if (is.suppl_flags & FM_REDUCE_SUPPLEMENTARY_VARS)
            ext += "r";
    }
    // typedef const char*  T[4];
    // size_t i = 0;
    // if (suppl_flags & FM_POSITIVE_SUPPLEMENTARY)
    //     i += 1;
    // if (suppl_flags & FM_NEGATIVE_SUPPLEMENTARY)
    //     i += 2;
    // const char *ext;
    // switch (matk) {
    //     case FlowMatrixKind::SEMIFLOWS:
    //         if (smt == SystemMatrixType::TRAPS) {
    //             ext = ".traps";
    //             break;
    //         }
    //         else if (smt == SystemMatrixType::SIPHONS) {
    //             ext = ".siphons";
    //             break;
    //         }
    //         ext = (ik==InvariantKind::PLACE) ? T{ ".pin", ".pin+", ".pin-", ".pin+-" }[i] 
    //                                          : T{ ".tin", ".tin+", ".tin-", ".tin+-" }[i];
    //         break;
    //     case FlowMatrixKind::BASIS:
    //         ext = (ik==InvariantKind::PLACE) ? T{ ".pba", ".pba+", ".pba-", ".pba+-" }[i] 
    //                                          : T{ ".tba", ".tba+", ".tba-", ".tba+-" }[i];
    //         break;
    //     case FlowMatrixKind::INTEGER_FLOWS:
    //         ext = (ik==InvariantKind::PLACE) ? T{ ".pfl", ".pfl+", ".pfl-", ".pfl+-" }[i] 
    //                                          : T{ ".tfl", ".tfl+", ".tfl-", ".tfl+-" }[i];
    //         break;
    //     // case FlowMatrixKind::NESTED_FLOW_SPAN:
    //     //     return (ik==InvariantKind::PLACE) ? ".pspan" : ".tspan";
    //     default:
    //         throw program_exception("Internal error in GetGreatSPN_FileExt");
    // }
    return ext;
}
//-----------------------------------------------------------------------------

const char* GetFlowName(InvariantKind ik, FlowMatrixKind matk, SystemMatrixType smt) {
    bool p = (ik==InvariantKind::PLACE);
    switch (matk) {
        case FlowMatrixKind::EMPTY:         
            return "EMPTY";
        case FlowMatrixKind::INCIDENCE:     
            return "INCIDENCE MATRIX";
        case FlowMatrixKind::SEMIFLOWS:
            if (smt == SystemMatrixType::REGULAR)
                return p ? "PLACE SEMIFLOWS" : "TRANSITION SEMIFLOWS";
            else if (smt == SystemMatrixType::TRAPS)
                return p ? "TRAPS" : "????";
            else // Siphons
                return p ? "SIPHONS" : "????";
        case FlowMatrixKind::BASIS:         
            return p ? "PLACE FLOW BASIS" : "TRANSITION FLOW BASIS";
        case FlowMatrixKind::INTEGER_FLOWS: 
            return p ? "PLACE FLOWS" : "TRANSITION FLOWS";
        case FlowMatrixKind::NONE: 
            return "INCIDENCE";
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

// ostream& operator<<(ostream& os, const flow_matrix_t::row_t& mrow) {
//     return mrow.print(os, false);
// }

//-----------------------------------------------------------------------------
static const size_t MAX_DENSE_REPR = 35;

ostream& flow_matrix_t::row_t::print(ostream& os, const ssize_t M, const ssize_t N0, 
                                     bool highlight_annulled) const 
{
    if (D.size() > MAX_DENSE_REPR) { // sparse representation
        for (size_t i = 0, cnt = 0; i < D.nonzeros(); i++)
            if (D.ith_nonzero(i).value)
                os << (cnt++ > 0 ? ", " : " ") << (D.ith_nonzero(i).index+1) 
                   << ":" << D.ith_nonzero(i).value;
        os << " |";
        for (size_t i = 0, cnt = 0; i < A.nonzeros(); i++)
            if (A.ith_nonzero(i).value)
                os << (cnt++ > 0 ? ", " : " ") << (A.ith_nonzero(i).index+1) 
                   << ":" << A.ith_nonzero(i).value;
    }
    else { // dense representation
        for (size_t n=0; n<D.size(); n++)
            os << setw(3) << D[n] << (ssize_t(n)==(N0-1)||ssize_t(n)==(N0+M-1) ? " " : "");
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
        rr.print(os, M, N0, highlight_annulled) << endl;
        assert(rr.neg_D == rr.count_negatives_D());
    }
    return os;
}

//-----------------------------------------------------------------------------

void flow_matrix_t::save_matrix_A(ostream& os) const {
    os << N << " " << M << endl;
    for (const auto& rr : mK) {
        for (size_t j=0; j<M; j++) {
            os << rr.A[j] << " ";
        }
        os << endl;
    }
}

//-----------------------------------------------------------------------------

flow_matrix_t::flow_matrix_t(size_t initN, size_t initN0, size_t initM, InvariantKind _ik, 
                             SystemMatrixType smt, int _suppl_flags, bool _add_extra_vars, 
                             bool _use_Colom_pivoting, bool _extra_vars_in_support) 
: N(initN), N0(initN0), M(initM), inv_kind(_ik), system_kind(smt), suppl_flags(_suppl_flags), 
add_extra_vars(_add_extra_vars), use_Colom_pivoting(_use_Colom_pivoting), 
extra_vars_in_support(_extra_vars_in_support), mat_kind(FlowMatrixKind::EMPTY) 
{ 
    cout << "M="<<M<<", N="<<N<<", N0="<<N0<<endl;
}

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

inline bool flow_matrix_t::row_t::test_minimal_support_D(const spintvector& D2, const size_t maxN) const
{
    // Check if the support of D2 is included in D
    //   support(D2) subseteq support(D)    
    // where support(.) is the set of columns with non-zero entries
    // The test checks all the nonzero entries up to maxN
    if (D2.nonzeros() > D.nonzeros() && (D.size()==maxN))
        return false;

    if (D2.nonzeros() > 0 && D2.ith_nonzero(0).index >= maxN)
        return false; // do not test containment of empty vectors

    for (size_t i2=0, i=0; i2<D2.nonzeros(); i2++) {
        if (D2.ith_nonzero(i2).index >= maxN)
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

// inline bool flow_matrix_t::row_t::test_minimal_positive_support_D(const spintvector& D2, int mult) const
// {
//     // pos_support(D2) subseteq sopport(D)    
//     // where pos_support(.) is the set of columns with entries > 0
//     for (size_t i2=0, i=0; i2<D2.nonzeros(); i2++) {
//         while (i2<D2.nonzeros() && D2.ith_nonzero(i2).value * mult > 0)
//             i2++;
//         if (i2 == D2.nonzeros())
//             break;
//         while (i < D.nonzeros() && (D.ith_nonzero(i).index < D2.ith_nonzero(i2).index)) {
//             i++;
//         }
//         if (i >= D.nonzeros() || D.ith_nonzero(i).index != D2.ith_nonzero(i2).index) {
//             return false;
//         }
//     }
//     return true;
// }

//-----------------------------------------------------------------------------

// bool flow_matrix_t::row_t::test_subst(const spintvector& R, int& multR, int& multD) const {
//     // Step 1: check that R complements D, i.e.:
//     //  * everytime R has a negative entry, D has a positive entry to be annulled
//     //  * everytime R has a positive entry, D is has zero in that column
//     multR = multD = -1;
//     int valR, valD, i;
//     auto itD = D.begin(), itR = R.begin();
//     while (-1 != (i = traverse_both(itD, D, valD, itR, R, valR))) {
//         if (valR == 0)
//             continue; // ok any value in D when R is zero
//         // if (sign(valR * multR) == sign(valD))
//         //     break; // Cannot have both -1 or both +1
//         if ((valR < 0 && valD > 0) || (valR > 0 && valD < 0)) {
//             if (multR == -1) {
//                 int gcdRD = gcd(abs(valD), abs(valR));
//                 multR = abs(valD) / gcdRD;
//                 multD = abs(valR) / gcdRD;
//             }
//             else {
//                 // TODO: test
//                 int gcdRD = gcd(abs(valD), abs(valR));
//                 if (multR != abs(valD) / gcdRD || multD != abs(valR) / gcdRD) {
//                     cout << "multR=" << multR << " multD="<<multD
//                          <<"  abs(valD)/gcdRD="<<(abs(valD) / gcdRD)<<"  abs(valR)/gcdRD="<<(abs(valR) / gcdRD)<<endl;
//                     return false; // Cannot substitute, the sum would not zero all the entries.
//                 }
//                 assert(multR == abs(valD) / gcdRD);
//                 assert(multD == abs(valR) / gcdRD);
//             }
//             continue; // R will annull the value in D
//         }
//         if (valR > 0 && valD == 0)
//             continue; // R will set a value in the column, or D will annul R
//         break;
//     }
//     return (i == -1);
// }

//-----------------------------------------------------------------------------

// Check support(D2) subseteq support(D +/- R)
// bool 
// flow_matrix_t::row_t::test_minimal_support_linear_comb_D(const spintvector& D2, 
//                                                          const spintvector& R) const 
// {
//     // cout << "\nsupport(D2) subseteq support(D +/- R)\n";
//     // cout << "D2 = ";
//     // for (size_t n=0; n<D2.size(); n++)
//     //     cout << setw(3) << D2[n];
//     // cout << endl;

//     // cout << "D  = ";
//     // for (size_t n=0; n<D.size(); n++)
//     //     cout << setw(3) << D[n];
//     // cout << endl;

//     // cout << "R  = ";
//     // for (size_t n=0; n<R.size(); n++)
//     //     cout << setw(3) << R[n];
//     // cout << endl;

//     // Step 1: check that R complements D, i.e.:
//     //  * everytime R has a negative entry, D has a positive entry to be annulled
//     //  * everytime R has a positive entry, D is has zero in that column
//     // Repeat twice, once for +R and another for -R.
//     int multR, valR, valD, i;
//     bool R_complements_D = false;
//     for (size_t phase = 0; phase < 2 && !R_complements_D; phase ++) {
//         multR = (phase == 0) ? +1 : -1;
//         auto itD = D.begin(), itR = R.begin();
//         while (-1 != (i = traverse_both(itD, D, valD, itR, R, valR))) {
//             if (valR == 0)
//                 continue; // ok any value in D when R is zero
//             // if (sign(valR * multR) == sign(valD))
//             //     break; // Cannot have both -1 or both +1
//             if (sign(valR * multR) < 0 && valD > 0)
//                 continue; // R will annull the value in D
//             if (sign(valR * multR) > 0 && valD == 0)
//                 continue; // R will set a value in the column, or D will annul R
//             break;
//         }
//         if (i == -1)
//             R_complements_D = true;
//     }
//     // cout << "R complements D : " << R_complements_D << "   multR=" << multR << endl;
//     if (!R_complements_D)
//         return false;

//     // int multR = +1;
//     // valR = D[ R.ith_nonzero(0).index ], valD;
//     // if (valR == 0) { // try the first negative index in R
//     //     for (size_t iR=0; iR<R.nonzeros(); iR++) {
//     //         // if (R.ith_nonzero(iR).value < 0) {
//     //         if (sign(R.ith_nonzero(iR).value) != sign(R.ith_nonzero(0).value)) {
//     //             valR = D[ R.ith_nonzero(iR).index ];
//     //             if (valR == 0)
//     //                 return false;
//     //         }
//     //     }
//     //     multR = -1;
//     // }

//     auto itD = D.begin(), itR = R.begin();
//     for (size_t i2=0; i2<D2.nonzeros(); i2++) {
//         while(-1 != (i = traverse_both(itD, D, valD, itR, R, valR)) &&
//               ( multR * sign(valR) + sign(valD) == 0 || /* entry annulled by R or by D */
//                 i < (int)D2.ith_nonzero(i2).index) )
//         {
//             // cout << "  skip i="<<i<<"  m*R+D="<<(multR * sign(valR) + sign(valD)) 
//             // << "  next(D2)="<<D2.ith_nonzero(i2).index<<endl;
//             continue;
//         }
//         // cout << "  test i="<<i<<"  m*R+D="<<(multR * sign(valR) + sign(valD))
//         // <<" next(D2)"<<D2.ith_nonzero(i2).index<<endl;
//         if (i == -1 || i != (int)D2.ith_nonzero(i2).index) {
//             return false;
//         }
//     }
//     return true;
// }

//-----------------------------------------------------------------------------

// inline bool flow_matrix_t::row_t::test_common_nonzeros(const spintvector& D2) const {
//     int valD, valD2, i;
//     auto itD = D.begin(), itD2 = D2.begin();
//     while(-1 != (i = traverse_both(itD, D, valD, itD2, D2, valD2))) {
//         if (valD != 0 && valD2 != 0)
//             return true; // There is at least a common non-zero
//     }
//     return false;
// }

//-----------------------------------------------------------------------------

// Test that the positive/negative supports of the two rows are either
// disjoint or complementary (when one is positive, the other is negative).
// inline bool flow_matrix_t::row_t::test_disjoint_supports(const row_t& row2, int mult) const {
//     int valD, valD2, i;
//     auto itD = D.begin(), itD2 = row2.D.begin();
//     while(-1 != (i = traverse_both(itD, D, valD, itD2, row2.D, valD2))) {
//         if ((valD * mult) < 0 && valD2 < 0)
//             return true; // There is at least a common negative entry in D
//         if ((valD * mult) > 0 && valD2 > 0)
//             return true; // There is at least a common positive entry in D
//     }
//     int valA, valA2;
//     auto itA = A.begin(), itA2 = row2.A.begin();
//     while(-1 != (i = traverse_both(itA, A, valA, itA2, row2.A, valA2))) {
//         if ((valA * mult) < 0 && valA2 < 0)
//             return true; // There is at least a common negative entry in A
//         if ((valA * mult) > 0 && valA2 > 0)
//             return true; // There is at least a common positive entry in A
//     }
//     return false;
// }

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

// void incidence_matrix_generator_t::add_flow_entry(size_t i, size_t j, int cardinality) {
//     assert(i < f.N && j < f.M);
//     auto elem = initEntries.lower_bound(flow_entry_t(i, j, numeric_limits<int>::min()));
//     if (elem != initEntries.end() && elem->i == i && elem->j == j) {
//         cardinality += elem->card;
//         initEntries.erase(elem);
//     }
//         // initEntries.insert(Flow(i, j, elem->card + cardinality));
//     // else
//     initEntries.insert(flow_entry_t(i, j, cardinality));
//     // cout << "msa.add_flow_entry("<<i<<", "<<j<<", "<<cardinality<<");"<<endl;
// }

//-----------------------------------------------------------------------------

// // Insert flows from Petri net
// void incidence_matrix_generator_t::add_flows_from(const PN& pn, bool print_warns) {
//     // Load the incidence matrix into the flows_generator_t class
//     bool warnForInhibitor = print_warns;
//     bool warnForMarkingDep = print_warns;
//     for (const Transition& trn : pn.trns) {
//         if (warnForInhibitor && !trn.arcs[HA].empty()) {
//             cerr << console::beg_error() << "WARNING: " << console::end_error() 
//                  << "PETRI NET HAS INHIBITOR ARCS THAT WILL BE IGNORED." << endl;
//             warnForInhibitor = false;
//         }
//         for (int k=0; k<2; k++) {
//             ArcKind ak = (k==0 ? IA : OA);
//             int sign = (ak==IA ? -1 : +1);
//             for (const Arc& arc : trn.arcs[ak]) {
//                 if (arc.isMultMarkingDep()) {
//                     if (warnForMarkingDep) {
//                         cerr << console::beg_error() << "WARNING: " << console::end_error() 
//                              << "PETRI NET HAS MARKING-DEPENDENT ARCS THAT WILL BE IGNORED." << endl;
//                          warnForMarkingDep = false;
//                     }
//                 }
//                 else {
//                     int card = get_value(arc.getConstantMult()) * sign;
//                     if (f.inv_kind == InvariantKind::PLACE) 
//                         add_flow_entry(arc.plc, trn.index, card);
//                     else
//                         add_flow_entry(trn.index, arc.plc, card);
//                 }
//             }
//         }
//     }
// }

//-----------------------------------------------------------------------------

// void incidence_matrix_generator_t::generate_matrix() {
//     // Initialize matrix K with the initEntries
//     for (size_t i = 0; i < f.N; i++) {
//         flow_matrix_t::row_t row(f);
//         // Diagonal entry in D
//         row.D.insert_element(i, 1);

//         // Insert the flows in the A matrix, which starts as the incidence matrix
//         auto it1 = initEntries.lower_bound(flow_entry_t(i, 0, numeric_limits<int>::min()));
//         auto it2 = initEntries.lower_bound(flow_entry_t(i, numeric_limits<size_t>::max(), 0));
//         if (it1 == it2 && i >= f.N0 && f.add_extra_vars)
//             continue;
//         for (; it1 != it2; ++it1) {
//             assert(it1->i == i && it1->j < f.M);
//             row.A.add_element(it1->j, it1->card);
//         }

//         if (row.A.empty() && i >= f.N0) // empty supplementary variable row, can drop it
//             continue;

//         f.mK.emplace_back(std::move(row));
//     }
//     initEntries.clear();
//     f.mat_kind = FlowMatrixKind::INCIDENCE;
// }

//-----------------------------------------------------------------------------

void incidence_matrix_generator_t::generate_matrix2(const PN& pn, bool print_warns) {
    // Provide warnings to the user
    if (print_warns) {
        for (const Transition& trn : pn.trns) {
            if (!trn.arcs[HA].empty()) {
                cerr << console::beg_error() << "WARNING: " << console::end_error() 
                    << "PETRI NET HAS INHIBITOR ARCS THAT WILL BE IGNORED." << endl;
                break;
            }
        }
        for (const Transition& trn : pn.trns) {
            bool warned = false;
            for (int k=0; k<2 && !warned; k++) {
                ArcKind ak = (k==0 ? IA : OA);
                for (const Arc& arc : trn.arcs[ak]) {
                    if (arc.isMultMarkingDep()) {
                        cerr << console::beg_error() << "WARNING: " << console::end_error() 
                            << "PETRI NET HAS MARKING-DEPENDENT ARCS THAT WILL BE IGNORED." << endl;
                        warned = true;
                        break;
                    }
                }
            }
            if (warned)
                break;
        }
    }

    std::vector<flow_matrix_t::spintvector*> ptr_A(f.N);

    // Initialize the N rows of matrix K
    for (size_t i = 0; i < f.N; i++) {
       flow_matrix_t::row_t row(f);
        // Diagonal entry in D
        row.D.insert_element(i, 1);
        f.mK.emplace_back(std::move(row));
        ptr_A[i] = &(f.mK.back().A);
    }

    // Initialize the rows of the A matrix
    if (f.system_kind == SystemMatrixType::REGULAR) {
        for (const Transition& trn : pn.trns) {
            for (int k=0; k<2; k++) {
                ArcKind ak = (k==0 ? IA : OA);
                int sign = (ak==IA ? -1 : +1);
                for (const Arc& arc : trn.arcs[ak]) {
                    if (arc.isMultMarkingDep()) 
                        continue;
                    int card = get_value(arc.getConstantMult()), i, j;
                    if (f.inv_kind == InvariantKind::PLACE) {
                        i = arc.plc;
                        j = trn.index;
                    }
                    else {
                        i = trn.index;
                        j = arc.plc;
                    }
                    ptr_A[i]->add_element(j, card * sign);
                }
            }
        }

        // Add initial supplementary variables
        bool dynamic_extra_var_gen = 0 != (f.suppl_flags & FM_ON_THE_FLY_SUPPL_VARS);
        if (f.suppl_flags != 0 && !dynamic_extra_var_gen) {
            // add an arc for each place N0+i from transition i
            verify(f.M == (f.N - f.N0) / 2);
            for (size_t i=0; i < f.M; i++) {
                if (f.suppl_flags & FM_NEGATIVE_SUPPLEMENTARY)
                    ptr_A[f.N0 + i]->add_element(i, -1);
                    // add_flow_entry(f.N0 + i, i, -1);

                if (f.suppl_flags & FM_POSITIVE_SUPPLEMENTARY)
                    ptr_A[f.N0 + f.M + i]->add_element(i, +1);
                    // add_flow_entry(f.N0 + f.M + i, i, +1);
            }
        }
    }
    else { // traps, siphons
        verify(f.inv_kind == InvariantKind::PLACE);
        const ArcKind dup_type = (f.system_kind==SystemMatrixType::TRAPS ? IA : OA);
        const ArcKind secondary_type = (f.system_kind==SystemMatrixType::TRAPS ? OA : IA);
        int j_start=0, j_end;
        for (const Transition& trn : pn.trns) {
            int num_arcs = trn.arcs[dup_type].size();
            j_end = j_start + num_arcs;
            int jj = j_start;
            // cout << trn.name << " num_arcs="<<num_arcs<<endl;

            for (const Arc& arc : trn.arcs[secondary_type]) {
                if (arc.isMultMarkingDep()) 
                    continue;
                int i = arc.plc;
                for (int j=j_start; j<j_end; j++)
                    ptr_A[i]->set_element(j, -1);
            }

            for (const Arc& arc : trn.arcs[dup_type]) {
                if (arc.isMultMarkingDep()) 
                    continue;
                int i = arc.plc;
                if ((*ptr_A[i])[jj] == 0)
                    ptr_A[i]->set_element(jj, 1);
                jj++;
            }
            j_start = j_end;
        }

        // Add the identity matrix below the incidence. A = [C] over [I]
        for (size_t i=0; i<f.M; i++) {
            ptr_A[i + f.N0]->insert_element(i, 1);
        }
    }

    // Drop empty rows from matrix K
    size_t i = 0;
    for (auto it = f.mK.begin(); it != f.mK.end(); i++) {
        if (it->A.empty() && i >= f.N0) // empty supplementary variable row, can drop it
            it = f.mK.erase(it);
        else
            ++it;
    }
    f.mat_kind = FlowMatrixKind::INCIDENCE;
}

//-----------------------------------------------------------------------------

// void incidence_matrix_generator_t::add_increase_decrease_flows() {
//     assert(f.M == (f.N - f.N0) / 2);
//     // assert(f.inc_dec == 1 || f.inc_dec == -1);

//     // add an arc for each place N0+i from transition i
//     for (size_t i=0; i < f.M; i++) {
//         if (f.suppl_flags & FM_NEGATIVE_SUPPLEMENTARY)
//             add_flow_entry(f.N0 + i, i, -1);

//         if (f.suppl_flags & FM_POSITIVE_SUPPLEMENTARY)
//             add_flow_entry(f.N0 + f.M + i, i, +1);
//     }
// }

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

// size_t flows_generator_t::column_of_suppl_var(size_t var, int sign) {
//     assert(sign>0 || (suppl_flags & FM_NEGATIVE_SUPPLEMENTARY));
//     assert(sign<0 || (suppl_flags & FM_POSITIVE_SUPPLEMENTARY));

//     size_t col = N0 + var;
//     if ()
// }

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
        size_t i = f.M;
        ssize_t num_prod = std::numeric_limits<ssize_t>::max();
        int best_cat = 2;
        if (f.use_Colom_pivoting) {
            for (size_t k=0; k<f.M; k++) {
                int npos = A_cols_count[k].first, nneg = A_cols_count[k].second;
                if ((npos + nneg) == 0)
                    continue;
                ssize_t new_num_prod = npos * nneg - npos - nneg; // number of generated entries
                int new_cat = (npos>0 && nneg>0) ? 1 : 2;
                if (num_prod > new_num_prod || best_cat > new_cat) { // pivot k is better than pivot i
                    i = k;
                    num_prod = new_num_prod;
                    best_cat = new_cat;
                }
            }
        }
        else {
            i = step;
            do {
                int npos = A_cols_count[i].first, nneg = A_cols_count[i].second;
                if ((npos + nneg) != 0) {
                    num_prod = npos * nneg - npos - nneg;
                    break; // column i has non-zero entries
                }
                i++;
            } while (i < f.M);
        }
        if (i == f.M)
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
                 cout << console::red_fgnd() << "ALL ROWS HAVE UNIFORM SIGN AT COLUMN i=" << i 
                     << ", N+=" << AiPositiveRows.size() << ", N-=" << AiNegativeRows.size()
                     << console::default_disp() << endl; 
                // cout << console::red_fgnd() << "NON-ANNULLABLE COLUMN i=" << i << console::default_disp() << endl; 
                // cout << "DEL ";
                // AiNonnullRows.begin()->print(cout, true) << endl << endl;
            }
            if (f.add_extra_vars) {
                // Add a new row that will annull column i, using an extra variable T_i
                int sgn = (AiPositiveRows.size() == 0) ? +1 : -1;
                flow_matrix_t::row_t newRow(f);
                newRow.D.insert_element(f.N0 + i + (sgn>0 ? f.M : 0), +1);
                newRow.A.insert_element(i, sgn);
                newRow.neg_D = newRow.count_negatives_D();
                update_neg_D_count = true;
                newRow.gen_step = step;
                if (verboseLvl >= VL_VERY_VERBOSE) {
                    cout << "+++ ";
                    newRow.print(cout, f.M, f.N0, true) << endl << endl;
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
                    if (newRow.test_minimal_support_D(row->D, f.extra_vars_in_support ? f.N : f.N0)) {
                        // cout << " ** "; row->print(cout, f.M, f.N0, true) << endl;
                        dropNewRow = true;
                        break;
                    }
                    else ++row;
                }
                if (dropNewRow) {
                    if (verboseLvl >= VL_VERY_VERBOSE) {
                        cout << console::red_fgnd() << "DROP" << console::default_disp();
                        newRow.print(cout, f.M, f.N0, true) << endl;
                    }
                    continue;
                }
                if (verboseLvl >= VL_VERY_VERBOSE) {
                    cout << "ADD ";
                    newRow.print(cout, f.M, f.N0, true) << endl;
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
        size_t i = f.M;
        ssize_t num_prod = std::numeric_limits<ssize_t>::max();
        int best_cat = 2;
        if (f.use_Colom_pivoting) {
            for (size_t k=0; k<f.M; k++) {
                int nrows = A_cols_count[k].first + A_cols_count[k].second;
                if (nrows == 0)
                    continue;
                ssize_t new_num_prod = nrows * (nrows - 1) - nrows; // number of generated entries
                int new_cat = (A_cols_count[k].first>0 && A_cols_count[k].second>0) ? 1 : 2;
                if (num_prod > new_num_prod || best_cat > new_cat) { // pivot k is better than pivot i
                    i = k;
                    num_prod = new_num_prod;
                    best_cat = new_cat;
                }
            }
        } 
        else {
            i = step;
            do {
                int nrows = A_cols_count[i].first + A_cols_count[i].second;
                if ((nrows) != 0) {
                    num_prod = nrows * (nrows - 1) - nrows;
                    break; // column i has non-zero entries
                }
                i++;
            } while (i < f.M);
        }
        if (i == f.M)
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
                cout << console::red_fgnd() << "ALL ROWS HAVE UNIFORM SIGN AT COLUMN i=" << i 
                     << ", N+=" << num_posi << ", N-=" << (AiNonnullRows.size() - num_posi) 
                     << console::default_disp() << endl; 
            }
            if (f.add_extra_vars) {
                // Add a new row that will annull column i, using an extra variable T_i
                int sgn = (num_posi == 0) ? +1 : -1;
                flow_matrix_t::row_t newRow(f);
                newRow.D.insert_element(f.N0 + i + (sgn>0 ? f.M : 0), +1);
                newRow.A.insert_element(i, sgn);
                newRow.neg_D = newRow.count_negatives_D();
                newRow.gen_step = step;
                if (verboseLvl >= VL_VERY_VERBOSE) {
                    cout << "+++ ";
                    newRow.print(cout, f.M, f.N0, true) << endl << endl;
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
                    if (newRow.test_minimal_support_D(row->D, f.extra_vars_in_support ? f.N : f.N0)) {
                        dropNewRow = true;
                        break;
                    }
                    else ++row;
                }
                if (dropNewRow) {
                    if (verboseLvl >= VL_VERY_VERBOSE) {
                        cout << console::red_fgnd() << "DROP" << console::default_disp();
                        newRow.print(cout, f.M, f.N0, true) << endl;                    }
                    continue;
                }
                if (verboseLvl >= VL_VERY_VERBOSE) {
                    cout << "ADD ";
                    newRow.print(cout, f.M, f.N0, true) << endl;
                }

                // // Unlike P-semiflows, whose support cannot decrease (monotonic property),
                // // for integer P-flows the support may be smaller. Therefore, entries in mK
                // // could now have a support smaller than newRow
                // for (auto row = f.mK.begin(); row != f.mK.end(); /**/) {
                //     if (row->test_minimal_support_D(newRow.D, f.extra_vars_in_support ? f.N : f.N0)) {
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
        size_t i = f.M;
        ssize_t num_sums = std::numeric_limits<ssize_t>::max();
        int best_cat = 2;
        if (f.use_Colom_pivoting) {
            for (size_t phase=0; phase<2 && i==f.M; phase++) {
                for (size_t k=0; k<f.M; k++) {
                    int npos = A_cols_count[k].first, nneg = A_cols_count[k].second;
                    if ((npos + nneg) == 0)
                        continue;
                    if (phase == 0) {
                        if (npos==1 && nneg==1) { // select k when npos==nneg
                            i = k;
                            num_sums = 1;
                            break;
                        }
                    } 
                    else { // phase == 1
                        int new_cat = (npos>0 && nneg>0) ? 1 : 2;
                        if (num_sums > npos * nneg || best_cat > new_cat) { // pivot k is better than pivot i
                            i = k;
                            num_sums = (npos + nneg - 1);
                            best_cat = new_cat;
                        }
                    }
                }
            }
        }
        else {
             i = step;
            do {
                int npos = A_cols_count[i].first, nneg = A_cols_count[i].second;
                if ((npos + nneg) != 0) {
                    num_sums = (npos + nneg - 1);
                    break; // column i has non-zero entries
                }
                i++;
            } while (i < f.M);           
        }
        if (i == f.M)
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

        // if (num_posi == 0 || num_posi == AiNonnullRows.size()) {
        if (AiNonnullRows.size() == 1) {
            if (verboseLvl >= VL_VERY_VERBOSE) {
                cout << console::red_fgnd() << "SINGLE ROW AT COLUMN i=" << i 
                     << ", N+=" << num_posi << ", N-=" << (AiNonnullRows.size() - num_posi) 
                     << console::default_disp() << endl; 
            }
            if (f.add_extra_vars) {
                // Add a new row that will annull column i, using an extra variable T_i
                int sgn = (num_posi == 0) ? +1 : -1;
                flow_matrix_t::row_t newRow(f);
                newRow.D.insert_element(f.N0 + i + (sgn>0 ? f.M : 0), +1);
                newRow.A.insert_element(i, sgn);
                newRow.neg_D = newRow.count_negatives_D();
                newRow.gen_step = step;
                if (verboseLvl >= VL_VERY_VERBOSE) {
                    cout << "+++ ";
                    newRow.print(cout, f.M, f.N0, true) << endl << endl;
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
                newRow.print(cout, f.M, f.N0, true) << endl;
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

void flows_generator_t::drop_slack_vars_in_D() {
    for (auto row = f.mK.begin(); row != f.mK.end(); ) {
        size_t i;
        for (i=0; i<row->D.nonzeros(); i++)
            if (row->D.ith_nonzero(i).index >= f.N0)
                break;
        if (i > 0) {
            row->D.truncate_nnz(i);
            ++row;
        }
        else {
            if (verboseLvl >= VL_VERY_VERBOSE) {
                cout << console::red_fgnd() << "TRNC" << console::default_disp();
                row->print(cout, f.M, f.N0, true) << endl;
            }
            row = f.mK.erase(row);
        }
    }
}

//-----------------------------------------------------------------------------

void flows_generator_t::reduce_non_minimal() {
    drop_slack_vars_in_D();

    auto row = f.mK.begin();
    while (row != f.mK.end()) {
        // Test for minimal support excluding the slack variables
        bool erased = false;
        for (auto row2 = f.mK.begin(); row2 != f.mK.end(); ++row2) {
            if (row == row2)
                continue;
            if (row->test_minimal_support_D(row2->D, /*f.extra_vars_in_support?f.N:*/f.N0)) {
                if (verboseLvl >= VL_VERY_VERBOSE) {
                    cout << console::red_fgnd() << "REDX" << console::default_disp();
                    row->print(cout, f.M, f.N0, true) << endl;
                }
                row = f.mK.erase(row); // drop and continue;
                erased = true;
                break;
            }
        }
        if (!erased)
            ++row;
    }
}

//-----------------------------------------------------------------------------

shared_ptr<flow_matrix_t>
ComputeFlows(const PN& pn, InvariantKind inv_kind, FlowMatrixKind mat_kind, 
             SystemMatrixType system_kind, bool detect_exp_growth, 
             int suppl_flags, bool use_Colom_pivoting, 
             bool extra_vars_in_support, VerboseLevel verboseLvl)
{
    if (verboseLvl >= VL_BASIC) {
        cout << "COMPUTING " << GetFlowName(inv_kind, mat_kind, system_kind) << "..." << endl;
    }
    bool has_suppl_vars = (suppl_flags != 0);
    bool dynamic_extra_var_gen = 0 != (suppl_flags & FM_ON_THE_FLY_SUPPL_VARS);

    shared_ptr<flow_matrix_t> pfm;
    size_t N, M, N0;
    if (inv_kind == InvariantKind::PLACE) { // P-invariants
        N = N0 = pn.plcs.size();
        if (has_suppl_vars)
            N += 2 * pn.trns.size();
        M  = pn.trns.size();
    }
    else { // T-invariants
        N = N0 = pn.trns.size();
        if (has_suppl_vars)
            N += 2 * pn.plcs.size();
        M  = pn.plcs.size();
    }
    if (system_kind != SystemMatrixType::REGULAR) {
        // for traps/siphons, the incidence matrix is modified:
        // each IA/OA arc generates a duplicate transition with an
        // associated supplementary variable.
        const ArcKind dup_type = (system_kind==SystemMatrixType::TRAPS ? IA : OA);
        M = 0; // reset transition count
        N = N0; // start from 0 suppl. vars.
        for (const Transition& trn : pn.trns) {
            int num_arcs = trn.arcs[dup_type].size();
            M += num_arcs;
            N += num_arcs;
        }
    }
    pfm = make_shared<flow_matrix_t>(N, N0, M, inv_kind, system_kind, suppl_flags, 
                                     dynamic_extra_var_gen, use_Colom_pivoting,
                                     extra_vars_in_support);

    // Initialize the flow matrix with the incidence matrix
    incidence_matrix_generator_t inc_gen(*pfm);
    // if (reduce_non_minimal)
    inc_gen.generate_matrix2(pn, verboseLvl >= VL_BASIC);
    // else {
    //     inc_gen.add_flows_from(pn, verboseLvl >= VL_BASIC);
    //     if (has_suppl_vars && !dynamic_extra_var_gen)
    //         inc_gen.add_increase_decrease_flows();
    //     inc_gen.generate_matrix();
    // }

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

    if (mat_kind != FlowMatrixKind::NONE) {
        // Initialize generator
        flows_generator_t sf_gen(*pfm, printer, verboseLvl);
        if (detect_exp_growth)
            sf_gen.max_peak_rows = 5 * pfm->N; // N=pn.plcs.size() for psfl

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

        if (system_kind != SystemMatrixType::REGULAR || 0!=(suppl_flags & FM_REDUCE_SUPPLEMENTARY_VARS))
            sf_gen.reduce_non_minimal();

        if (verboseLvl >= VL_BASIC) {
            if (mat_kind == FlowMatrixKind::BASIS) {
                cout << "FOUND " << pfm->num_flows()
                    << " VECTORS IN THE " << GetFlowName(inv_kind, pfm->mat_kind, pfm->system_kind);
            }
            else {
                cout << "FOUND " << pfm->num_flows()
                    << " " << GetFlowName(inv_kind, pfm->mat_kind, pfm->system_kind);
            }
            size_t num_neg = 0;
            for (auto&& row : pfm->mK)
                num_neg += (row.is_negative() ? 1 : 0);
            if (num_neg > 0) 
                cout << " (" << (pfm->num_flows() - num_neg) << " semiflows, " << num_neg << " flows)";
            cout << ".\n" << endl;
        }
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
    size_t NP = pn.plcs.size(), NT = pn.trns.size();

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
                        if (elem.index < NP)
                            cout << pn.plcs[elem.index].name;
                        else if (elem.index < NP + NT)
                            cout << "[°" << pn.trns[elem.index - NP].name << "]";
                        else
                            cout << "[" << pn.trns[elem.index - NP - NT].name << "°]";
                            // cout << "[T" << (elem.index - pn.plcs.size()) << "]";
                    }
                    else {
                        if (elem.index < NT)
                            cout << pn.trns[elem.index].name;
                        else if (elem.index < NT + NP)
                            cout << "[°P" << (elem.index - NT) << "]";
                        else
                            cout << "[P" << (elem.index - NT - NP) << "°]";
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
        std::vector<bool> covered(pinv ? NP : NT, false);
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

// are all places of a net covered by at least one flow?
bool IsNetCoveredByFlows(const PN& pn, const flow_matrix_t& flows) {
    assert(flows.inv_kind == InvariantKind::PLACE);
    std::vector<bool> covered(pn.plcs.size(), false);

    for (const auto &f : flows) {
        for (auto& elem : f) {
            if (elem.index < pn.plcs.size()) {
                covered[elem.index] = true;
            }
        }
    }
    for (bool is_cov : covered)
        if (!is_cov)
        return false;
    return true;
}

//-----------------------------------------------------------------------------

// Compute place bounds using the P-semiflows
void ComputeBoundsFromSemiflows(const PN& pn, const flow_matrix_t& semiflows, 
                                place_bounds_t& bounds) 
{
    // assert(semiflows.mat_kind == FlowMatrixKind::SEMIFLOWS);
    assert(semiflows.inv_kind == InvariantKind::PLACE);
    bounds.resize(pn.plcs.size());
    std::fill(bounds.begin(), bounds.end(), PlaceBounds{ 0, numeric_limits<int>::max() });

    for (const auto& sf : semiflows) {
        // Get the amount of tokens circulating in semiflow @sf
        int tokenCnt = 0;
        for (auto& elem : sf)
            if (elem.index < pn.plcs.size()) // not a support variable
                tokenCnt += elem.value * int(pn.plcs[elem.index].getInitTokenCount());

        int kk = -1;
        // Set upper bounds for all the places in semiflow @sf
        for (auto& elem : sf) {
            if (elem.index < pn.plcs.size()) {// not a support variable
                kk = tokenCnt / elem.value;
                bounds[elem.index].upper = std::min(bounds[elem.index].upper, kk);
            }
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
// minimal marking that satisfies a set of flows
//-----------------------------------------------------------------------------

// Compute the minimal number of tokens to satisfy each semiflow
void ComputeMinimalTokensFromFlows(const PN& pn, 
                                   const flow_matrix_t& semiflows, 
                                   std::vector<int>& m0min)
{
    // assert(semiflows.mat_kind == FlowMatrixKind::SEMIFLOWS);
    assert(semiflows.inv_kind == InvariantKind::PLACE);
    m0min.resize(pn.plcs.size());

    // std::fill(m0min.begin(), m0min.end(), -1);
    // for (const auto& sf : semiflows) {
    //     // get the lcm of the semiflow
    //     int lcm_sf = 1;
    //     for (auto& elem : sf) {
    //         // if (elem.index < pn.plcs.size()) {
    //         // cout << "    " << lcm_sf << " " << elem.value << endl;
    //         lcm_sf = lcm(lcm_sf, elem.value);
    //         // }
    //     }
    //     // cout << "LCM: " << lcm_sf << endl;

    //     // Set the minimum token count of m0 that satisies the semiflow.
    //     for (auto& elem : sf) {
    //         if (elem.index < pn.plcs.size()) {
    //             int tc = lcm_sf / elem.value;
    //             int m0 = int(pn.plcs[elem.index].getInitTokenCount());
    //             tc = std::min(tc, m0);
    //             if (m0min[elem.index] < 0)
    //                 m0min[elem.index] = tc;
    //             else
    //                 m0min[elem.index] = std::min(m0min[elem.index], tc);
    //         }
    //     }
    // }

    // Extract all the m0*Y of all semiflows in the net
    std::set<int> all_m0_Y;
    for (const auto& sf : semiflows) {
        int m0_Y = 0;
        for (auto& elem : sf) {
            if (elem.index < pn.plcs.size()) { // not a support variable
                m0_Y += elem.value * int(pn.plcs[elem.index].getInitTokenCount());
            }
        }
        all_m0_Y.insert(m0_Y);
    }
    // renumber the token counts to find the optimal reduced token counts
    // the optimal reduced token count is the smallest renumbering that preserves 
    // the partial order between the token counts in each circuit.
    std::map<int, int> reducer;
    for (auto m0_Y : all_m0_Y) {
        reducer.insert(make_pair(m0_Y, reducer.size() + 1));
        // cout << "reducer "<<m0_Y<<" -> "<<reducer[m0_Y]<<endl;
    }
    
    // Assign the reduced token count to each place
    std::fill(m0min.begin(), m0min.end(), -1);
    for (const auto& sf : semiflows) {
        int m0_Y = 0;
        for (auto& elem : sf) {
            if (elem.index < pn.plcs.size()) { // not a support variable
                m0_Y += elem.value * int(pn.plcs[elem.index].getInitTokenCount());
            }
        }
        int reduced_m0_Y = reducer[m0_Y];
        // get the lcm of the semiflow
        int lcm_sf = 1;
        for (auto& elem : sf)
            lcm_sf = lcm(lcm_sf, elem.value);

        // Set the minimum token count of m0 that satisfies the semiflow.
        for (auto& elem : sf) {
            if (elem.index < pn.plcs.size()) {
                int mult = lcm_sf / elem.value;
                // int tc = m0_Y / elem.value;
                int tc = mult * reduced_m0_Y;
                int m0 = int(pn.plcs[elem.index].getInitTokenCount());
                // if (elem.index==0) {
                //     cout << "P"<<elem.index<<"  m0="<<m0<<" tc="<<tc
                //          <<"   reduced_m0_Y="<<reduced_m0_Y<<" elem.value="<<elem.value<<endl;
                // }
                tc = std::min(tc, m0);
                if (m0min[elem.index] < 0)
                    m0min[elem.index] = tc;
                else
                    m0min[elem.index] = std::min(m0min[elem.index], tc);
            }
        }
    }

    /*//////////
    std::vector<std::set<int>> bounds;
    bounds.resize(pn.plcs.size());

    for (const auto& sf : semiflows) {
        // get the lcm of the semiflow
        int lcm_sf = 1;
        for (auto& elem : sf)
            lcm_sf = lcm(lcm_sf, elem.value);

        // Get the amount of tokens circulating in semiflow @sf
        int tokenCnt = 0;
        for (auto& elem : sf) {
            if (elem.index < pn.plcs.size()) { // not a support variable
                tokenCnt += elem.value * int(pn.plcs[elem.index].getInitTokenCount());
            }
        }
        cout << "lcm_sf="<<lcm_sf<<" tokenCnt="<<tokenCnt<<endl;

        int kk = -1;
        // Set upper bounds for all the places in semiflow @sf
        for (auto& elem : sf) {
            if (elem.index < pn.plcs.size()) {// not a support variable
                kk = tokenCnt / elem.value;
                bounds[elem.index].insert(kk);// = std::min(bounds[elem.index].upper, kk);
            }
        }
        // Set lower bounds for all the places in @sf
        // if (sf.nonzeros() == 1 && kk > bounds[sf.front_nonzero().index].lower)
        //     bounds[sf.front_nonzero().index].lower = kk;
    }

    cout << "MINIMAL M0:" << endl;
    size_t max_plc_len = 0;
    for (auto& plc : pn.plcs)
        max_plc_len = std::max(max_plc_len, plc.name.size());

    for (size_t p = 0; p<pn.plcs.size(); p++) {
        cout << right << setw(max_plc_len) << pn.plcs[p].name 
                << ": " << left << setw(3) << m0min[p] 
                << " (" << pn.plcs[p].getInitTokenCount() << ")   ";

        for (auto k : bounds[p])
            cout << k <<  " ";
        cout << endl;
    }
    cout << endl;//*/
}

//-----------------------------------------------------------------------------

void SaveMinimalTokens(const std::vector<int>& m0min, ofstream& file) {
    if (m0min.empty()) {
    }
    else {
        for (auto& b : m0min) {
            file << b << " ";
        }
        file << "\n" << flush;
    }
}

//-----------------------------------------------------------------------------

void PrintMinimalTokens(const PN& pn, const std::vector<int>& m0min, VerboseLevel verboseLvl) {
    if (verboseLvl > VL_BASIC) {
        if (m0min.empty()) {
            cout << "COULD NOT COMPUTE MINIMAL M0: net does not meet all the requirements." << endl;
        }
        else {
            cout << "MINIMAL M0:" << endl;
            size_t max_plc_len = 0;
            for (auto& plc : pn.plcs)
                max_plc_len = std::max(max_plc_len, plc.name.size());

            for (size_t p = 0; p<pn.plcs.size(); p++) {
                cout << right << setw(max_plc_len) << pn.plcs[p].name 
                     << ": " << left << setw(3) << m0min[p] 
                     << " (" << pn.plcs[p].getInitTokenCount() << ")"
                     << endl;
            }
            cout << endl;
        }
    }
}

//-----------------------------------------------------------------------------















