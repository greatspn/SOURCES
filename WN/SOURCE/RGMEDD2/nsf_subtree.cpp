// compiles with:  g++ testnewcreateedge.cpp -std=c++11 -o testnewcreateedge -lmeddly -I/usr/local/include/ -L/usr/local/lib/

#include <cstdlib>
#include <string.h>
#include <fstream>
#include <iostream>
#include <typeinfo>
#include <memory>
#include <meddly.h>
#include <assert.h>
// #include "../../../../meddly/src/forests/mtmxdbool.h"
// #include "../../../../meddly/src/forests/mtmddbool.h"

using namespace MEDDLY;
using namespace std;


// This method generates a relational tree with values at level vh and vh'
// The generated MxD has this shape:
//     1 1' ... vh     vh'         ...
//     * *   *   0 primed_vars[0]   *  T
//     * *   *   1 primed_vars[1]   *  T
//     * *   *   2 primed_vars[2]   *  T
// vh is the involved level.
// primed_vars[] contains the post-image values.
void createEdgeForRelVar2(int vh, int const* primed_vars, dd_edge& out) {
    forest* mxd = out.getForest();
    const int sz = mxd->getDomain()->getVariableBound(vh + 1, false);
    const int nvars = mxd->getDomain()->getNumVariables();
    assert(vh >= 0 && vh < nvars);
    // cout << "    vh="<<vh<<" bound="<<sz<<endl;

    int *vlist[sz], *vplist[sz];
    int N = 0;
    // Create several edges with shape:
    //    1  1' 2  2' ... vh       vh'      ...
    //    *  *  *  *   *  i  primed_vars[i]  *    T 
    for (int i=0; i<sz; i++) {
        if (primed_vars[i] < 0 || primed_vars[i] >= sz)
            continue; // out of bound

        vlist[N] = new int[nvars + 1];
        vplist[N] = new int[nvars + 1];
        std::fill(vlist[N], vlist[N] + nvars + 1, DONT_CARE);
        std::fill(vplist[N], vplist[N] + nvars + 1, DONT_CARE);
        // vlist[N][0] = vplist[N][0] = 0;

        vlist[N][vh + 1] = i;
        vplist[N][vh + 1] = primed_vars[i];
        // for (int n=0; n<nvars; n++)
        //     cout << " " << vlist[N][n+1]<<":"<<vplist[N][n+1];
        // cout << endl;

        N++;
    }

    mxd->createEdge(vlist, vplist, N, out);

    // MEDDLY::ostream_output stdout_wrap(cout);
    // out.show(stdout_wrap, 2);
    // cout << endl;

    for (int i=0; i<N; i++) {
        delete[] vlist[i];
        delete[] vplist[i];
    }
}


#if 0
// This method generates a relational tree with values at level vh and vh'
// It works exclusively on boolean MxD forests having the Identity Reduced policy.
// The generated tree has this shape:
// 1 1' ... vh     vh'         ...
// * *   *   0 primed_vars[0]   *  T/F
// * *   *   1 primed_vars[1]   *  T/F
// * *   *   2 primed_vars[2]   *  T/F
// vh is the involved level.
// primed_vars[] contains the post-relation values.
void createEdgeForRelVar(int vh, int const* primed_vars, dd_edge& out){
    forest* _mxd = out.getForest();
    if (!_mxd->isIdentityReduced())
        throw MEDDLY::error(MEDDLY::error::INVALID_OPERATION);
    if (typeid(*_mxd) != typeid(MEDDLY::mt_mxd_bool))
        throw MEDDLY::error(MEDDLY::error::INVALID_OPERATION);
    MEDDLY::mt_mxd_bool* mxd = (MEDDLY::mt_mxd_bool*) _mxd;
    if (!(vh >= 0 && vh < mxd->getNumVariables()))
        throw MEDDLY::error(MEDDLY::error::INVALID_VARIABLE);
            
    
    

    int lev = mxd->getLevelByVar(vh+1);
    int levP = -lev;
    int levSucc = lev - 1;
    int levSuccP = -levSucc;
    int sz = mxd->getLevelSize(lev);
    
    bool allEq = true;
    for (int i=0; i<sz; i++)
        allEq = allEq && primed_vars[i] == i;
        
    unpacked_node* nb;
    if(allEq){
        if(lev == mxd->getNumVariables()){
            out.set(mxd->makeNodeAtLevel(levSuccP,  expert_forest::bool_Tencoder::value2handle(true)));
            return;
        }
        int levPrecP = -(lev+1);
        nb = unpacked_node::newFull(mxd, levPrecP, sz);
        node_handle nP = mxd->makeNodeAtLevel(levSuccP,  expert_forest::bool_Tencoder::value2handle(true));
        for(int i=0; i<sz; i++){
            nb->d_ref(i) = nP;
        }
    }
    else{
        nb = unpacked_node::newFull(mxd, lev, sz);
        for (int i=0; i<sz; i++) {
            if(primed_vars[i]<0 || primed_vars[i]>=sz)
                nb->d_ref(i) = mxd->makeNodeAtLevel(levP,  expert_forest::bool_Tencoder::value2handle(false));
            else if(primed_vars[i] == i){
                nb->d_ref(i) = mxd->makeNodeAtLevel(levSuccP,  expert_forest::bool_Tencoder::value2handle(true));
            } else {
                unpacked_node* nbP = unpacked_node::newSparse(mxd, levP, 1);
                nbP->i_ref(0) = primed_vars[i];
                nbP->d_ref(0) = mxd->makeNodeAtLevel(levSucc,  expert_forest::bool_Tencoder::value2handle(true));
                nb->d_ref(i) = mxd->createReducedNode(-1, nbP);
            }
        }
    }

    
    
    node_handle node = mxd->createReducedNode(-1, nb);
    node = mxd->makeNodeAtLevel(mxd->getDomain()->getNumVariables(), node);
    out.set(node);
}

void createEnablingCondition(int vh, int* placeFunction, dd_edge &out){
    forest* _fRS = out.getForest();
    if (!_fRS->isFullyReduced())
        throw MEDDLY::error(MEDDLY::error::INVALID_OPERATION);
    if (typeid(*_fRS) != typeid(MEDDLY::mt_mdd_bool))
        throw MEDDLY::error(MEDDLY::error::INVALID_OPERATION);
    MEDDLY::mt_mdd_bool* fRS = (MEDDLY::mt_mdd_bool*) _fRS;
    if (!(vh >= 0 && vh < fRS->getNumVariables()))
        throw MEDDLY::error(MEDDLY::error::INVALID_VARIABLE);
    
    int lev = fRS->getLevelByVar(vh+1);
    int sz = fRS->getLevelSize(lev);

    bool allOk = true;
    for (int i=0; i<sz; i++)
        allOk = allOk && (placeFunction[i]>=0 && placeFunction[i]<sz);
        
    
    if(allOk){
        out.set(expert_forest::bool_Tencoder::value2handle(true));
    } else {
        unpacked_node* nb;
        nb = unpacked_node::newFull(fRS, lev, sz);
        for (int i=0; i<sz; i++) {
            nb->d_ref(i) = expert_forest::bool_Tencoder::value2handle(placeFunction[i]>=0 && placeFunction[i]<sz);
        }
        node_handle node = fRS->createReducedNode(-1, nb);
        node = fRS->makeNodeAtLevel(fRS->getDomain()->getNumVariables(), node);
        out.set(node);
    }
}


/*
double sec(clock_t t){
    return t / double(CLOCKS_PER_SEC);
}*/

// createEdgeForRelVar using createEdgeForVar, EQUAL and COPY
void createEdgeForRelVar_old(int vh, const int* primed_vars, dd_edge& out){
    forest* _mxd = out.getForest();
    if (typeid(*_mxd) != typeid(MEDDLY::mt_mxd_bool))
        throw MEDDLY::error(MEDDLY::error::INVALID_OPERATION);
    MEDDLY::mt_mxd_bool* mxd = (MEDDLY::mt_mxd_bool*) _mxd;
    assert(vh >= 0 && vh < mxd->getNumVariables());
    forest* mtmxd = mxd->useDomain()->createForest(1, forest::INTEGER, forest::MULTI_TERMINAL);
    
    dd_edge transition_with_terminals(mtmxd);
    dd_edge identityPrimed(mtmxd); 
    mtmxd->createEdgeForVar(vh+1, true, identityPrimed);
    dd_edge appliedFncUnprimed(mtmxd);
    mtmxd->createEdgeForVar(vh+1, false, primed_vars, appliedFncUnprimed);
    apply(EQUAL, identityPrimed, appliedFncUnprimed, transition_with_terminals);
    apply(COPY, transition_with_terminals, out);
}
/*
// test createEdgeForRelVar
int main(){
    const int BOUND = 5;
    const int NVARS = 5;
    const int START_VAR = NVARS-1, END_VAR=NVARS;
    MEDDLY::initialize();
    int* sizes = new int[NVARS];
    int* minterm = new int[NVARS+1];
    int* mtprime = new int[NVARS+1];
    for(int i=0; i<NVARS; i++){
        sizes[i] = BOUND+1;
        minterm[i+1] = DONT_CARE;
        mtprime[i+1] = (START_VAR<=i && i<END_VAR) ? DONT_CARE : DONT_CHANGE;
        
    }
    domain* d = createDomainBottomUp(sizes, NVARS);
   
    MEDDLY::forest::policies fpol = MEDDLY::forest::getDefaultPoliciesMXDs();
    fpol.setIdentityReduced();
//    fpol.setFullyReduced();
//    fpol.setQuasiReduced();
   
    forest* mxd = d->createForest(1, forest::BOOLEAN, forest::MULTI_TERMINAL, fpol);
    clock_t totalEqualTime = 0, totalCopyTime = 0, totalIntersectTime = 0, startOfNsfEv = clock();
    vector<int> decr(BOUND+1);
    dd_edge nsf_event(mxd);
    mxd->createEdge(&minterm, &mtprime, 1, nsf_event);

    cout << "test forest : " << (typeid(*mxd).name()) << endl;
    MEDDLY::mt_mxd_bool* mxd_internal = (MEDDLY::mt_mxd_bool*)mxd;
    assert(typeid(*mxd) == typeid(MEDDLY::mt_mxd_bool));
    
    
    dd_edge transition_with_binary_terminals(mxd);
    for (int var=START_VAR; var < END_VAR; var++) {
        for (int var0 = -1; var0 <= NVARS; var0++) { decr[0] = var0;
            for (int var1 = -1; var1 <= NVARS; var1++) { decr[1] = var1;
                for (int var2 = -1; var2 <= NVARS; var2++) { decr[2] = var2;
                    for (int var3 = -1; var3 <= NVARS; var3++) { decr[3] = var3;
                        for (int var4 = -1; var4 <= NVARS; var4++) { decr[4] = var4;

                            dd_edge old_term(mxd);
                            createEdgeForRelVar_old(var, decr.data(), old_term);            
                            createEdgeForRelVar(var, decr.data(), transition_with_binary_terminals);
                            
                            if (old_term != transition_with_binary_terminals) {
                                cout << "diverso! var="<<var << endl;
                                MEDDLY::ostream_output stdout_wrap(cout);
                                transition_with_binary_terminals.show(stdout_wrap, 2);
                                old_term.show(stdout_wrap, 2);
                                exit(0);
                            }
                            
                            nsf_event*=transition_with_binary_terminals;
                        }
                    }
                }
            }
        }
    }
    
    cout << "--------------------\n";
    //MEDDLY::ostream_output stdout_wrap(cout);
    //transition_with_binary_terminals.show(stdout_wrap, 2);
    cout << "\n\n\n";
    cout << "Total EQUAL Time = " << (totalEqualTime / double(CLOCKS_PER_SEC));
    cout << ", COPY Time = " << (totalCopyTime / double(CLOCKS_PER_SEC));
    cout << ", * Time = " << (totalIntersectTime / double(CLOCKS_PER_SEC));
    cout << ", total: " << ((clock() - startOfNsfEv) / double(CLOCKS_PER_SEC)) << endl;
    return 0;
}*/
#endif