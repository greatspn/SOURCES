/*
 *  MRP_Implicit.h
 *
 *  Steady state solution of MRP with implicit method (of R. German).
 *
 *  Created by Elvio Amparore
 *
 */
//=============================================================================
#ifndef __MRP_IMPLICIT_H__
#define __MRP_IMPLICIT_H__
//=============================================================================

bool SteadyStateMRPImplicit(const MRP &mrp, const MRPStateKinds &stKinds,
                            PetriNetSolution &sol, const SolverParams &spar,
                            VerboseLevel verboseLvl);

bool SteadyStateMRPImplicitBackward(const MRP &mrp, const MRPStateKinds &stKinds,
                                    const ublas::vector<double> &rho,
                                    ublas::vector<double> &xi,
                                    const SolverParams &spar, VerboseLevel verboseLvl);

//=============================================================================
// Implicit vector X matrix product functor.
//=============================================================================

template<class RowSelector, class ColSelector>
class EmcVectorMatrixProduct : boost::noncopyable {
public:
    EmcVectorMatrixProduct(const MRP &_mrp, const SolverParams &_spar,
                           const MRPSubsetList &_Subsets,
                           KolmogorovEquationDirection _ked,
                           const RowSelector &_rows, const ColSelector &_cols)
        : mrp(_mrp), spar(_spar), ked(_ked), rows(_rows), cols(_cols), Subsets(_Subsets) {
        productCount = 0;
    }

    // Return the square matrix size
    inline size_t size() const				{   return size_t(mrp.N());   }

    // Return the number of product()s already done
    inline size_t getProductCount() const	{   return productCount;   }

    // Compute a vector * matrix product
    ublas::vector<double> product(const ublas::vector<double> &inVec) const {
        const marking_t N_MRP(mrp.N());
        ublas::vector<double> out(N_MRP);
        fill(out.begin(), out.end(), 0.0);
        NullIterPrintOut printOut;
        MRPSubsetList::selector ExpStates(Subsets.selectorOf(EXP_SUBSET));
        vector<GenMarkingSubset>::const_iterator SgenIt;

        if (ked == KED_FORWARD) {
            //=================== Forward:  y = x.P  ======================

            // The implicit EMC x Vector product is done in two phases.
            // 1) Exponential states:
            //    input vector is multiplied with P^E = I^E - diag^{-1}(Q^E)Q^E
            prod_vec_invDiagM_M(out, inVec, mrp.Q, ExpStates, cols);
            add_vec(out, inVec, ExpStates);

            // 2) Deterministic states:
            //    input vector is multiplied with Omega.Delta + Psi.Qbar
            ublas::vector<double> ag(N_MRP), bg(N_MRP);
            for (SgenIt = mrp.Gen.begin(); SgenIt != mrp.Gen.end(); ++SgenIt) {
                if (Subsets.numStatesIn(SgenIt->index) == marking_t(0))
                    continue;
                MRPSubsetList::selector GenStates(Subsets.selectorOf(SgenIt->index));

                // Compute uniformization over the Sgen subset *SgenIt
                // TODO: note that here the spar.epsilon precision is used in the
                // uniformization. In some models, this epsilon needs to be
                // lowered (for instance: spar.epsilon * 1.0e-1) in order to
                // converge.
                UniformizationGen(mrp.Q, inVec, ag, &bg, SgenIt->fg.c_str(),
                                  spar.epsilon * 1.0e-1, KED_FORWARD,
                                  GenStates, cols, printOut);
                add_prod_vecmat(out, ag, mrp.Delta, GenStates, cols);
                add_prod_vecmat(out, bg, mrp.Qbar, GenStates, cols);
            }
        }
        else {
            //================== Backward:  y = P.x  ======================
            prod_invDiagM_M_vec(out, mrp.Q, inVec, ExpStates, cols);
            add_vec(out, inVec, ExpStates);

            ublas::vector<double> DeltaX(N_MRP), QbarX(N_MRP), outExp, outExpInt;
            ublas::vector<double> *no_vec = nullptr;
            for (SgenIt = mrp.Gen.begin(); SgenIt != mrp.Gen.end(); ++SgenIt) {
                if (Subsets.numStatesIn(SgenIt->index) == marking_t(0))
                    continue;
                MRPSubsetList::selector GenStates(Subsets.selectorOf(SgenIt->index));

                prod_matvec(DeltaX, mrp.Delta, inVec, GenStates, cols);
                UniformizationGen(mrp.Q, DeltaX, outExp, no_vec, SgenIt->fg.c_str(),
                                  spar.epsilon * 1.0e-1, KED_BACKWARD,
                                  GenStates, cols, printOut);
                add_vec(out, outExp, cols);

                prod_matvec(QbarX, mrp.Qbar, inVec, GenStates, cols);
                if (vec_sum(QbarX) != 0) {
                    UniformizationGen(mrp.Q, QbarX, outExp, &outExpInt, SgenIt->fg.c_str(),
                                      spar.epsilon * 1.0e-1, KED_BACKWARD,
                                      GenStates, cols, printOut);
                    add_vec(out, outExpInt, cols);
                }
            }
        }

        productCount++;
        return out;
    }

private:
    const MRP			 &mrp;
    const SolverParams  &spar;
    const KolmogorovEquationDirection ked;
    const RowSelector	 &rows;
    const ColSelector	 &cols;
    const MRPSubsetList &Subsets;
    mutable size_t		 productCount;
};

//=============================================================================
#endif   // __MRP_IMPLICIT_H__
