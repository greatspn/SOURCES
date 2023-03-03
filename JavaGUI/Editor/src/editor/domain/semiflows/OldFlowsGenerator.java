///*
// * To change this template, choose Tools | Templates
// * and open the template in the editor.
// */
//package editor.domain.semiflows;
//
//import common.Util;
//import editor.domain.Edge;
//import editor.domain.Node;
//import editor.domain.elements.GspnEdge;
//import editor.domain.elements.Place;
//import editor.domain.elements.Transition;
//import editor.domain.grammar.ParserContext;
//import editor.domain.grammar.TemplateBinding;
//import java.awt.Color;
//import java.util.ArrayList;
//import java.util.Arrays;
//import java.util.Comparator;
//
///** Farkas algorithm for the computation
// *  of the minimal set of P(T)-(semi)flows in a Petri net.
// *
// * @author elvio
// */
//public class OldFlowsGenerator extends StructuralAlgorithm {
//    
//    // Iteration matrix [ D(i) | A(i) ], where D(0)=I and A(0) = Flow matrix
//    // At the beginning, K=N. After the computation, K is the number of flows.
//    public ArrayList<int[]> mD;     // KxN matrix
//    public ArrayList<int[]> mA;     // KxM matrix
//    
//    // This extra informations keep the initial place marking, when
//    // the method is used for P-invariants. It allows to derive the place bounds
//    // from the P-invariants
//    public int[] initQuantity; // array of N elements
//    public int[] lowerBnd, upperBnd; // place bounds
//    
//    // Will compute semiflows or integer flows
//    public final PTFlows.Type type;
//    
//
//    // For P-flows: N=|P|, M=|T| (for T-flows: N=|T|, M=|P|)
//    // If supplementary variables are added, N0 keeps the value of N(|P| or |T|) 
//    // while N will count the basic variables plus the supplementary ones.
//    public OldFlowsGenerator(int N, int N0, int M, PTFlows.Type type) {
//        super(N, N0, M);
//        this.type = type;
////        System.out.println("M="+M+", N="+N);
//        mD = new ArrayList<>();
//        mA = new ArrayList<>();
//        for (int i = 0; i < N; i++) {
//            mD.add(new int[N]);
//            mA.add(new int[M]);
//            mD.get(i)[i] = 1;
//        }
////        scAlgo = new SilvaColom88Algorithm(N, M);
//    }
//    
//    // Initialize from an N*M matrix
//    public OldFlowsGenerator(int[][] mat, PTFlows.Type type) {
//        this(mat.length, mat.length, mat[0].length, type);
//        for (int i=0; i<mat.length; i++) {
//            for (int j=0; j<mat[i].length; j++)
//                setIncidence(i, j, mat[i][j]);
//        }
//    }
//
//    // Add an element to the incidence matrix from i to j with the specified cardinality
//    public void addIncidence(int i, int j, int card) {
//        mA.get(i)[j] += card;
//    }
//    // Set an element to the incidence matrix from i to j with the specified cardinality
//    public void setIncidence(int i, int j, int value) {
//        mA.get(i)[j] = value;
//    }
//    
//    // Add the initial token of a place (only for P-invariant computation)
//    public void setInitQuantity(int i, int quantity) {
//        assert i < N && i >= 0;
//        if (initQuantity == null) { // Not initialized yet
//            initQuantity = new int[N];
////            computeBounds = true;
//        }
//        assert initQuantity[i] == 0;
//        initQuantity[i] = quantity;
//        
////        scAlgo.setInitQuantity(i, quantity);
//    }
//    
//    public int numFlows() {
//        return mD.size();
//    }
//
//    public int[] getFlowVector(int i) {
//        return mD.get(i);
//    }
//    
//    // only for generation of all non-minimal flows
//    public boolean isFlow(int i) {
//        for (int v : mA.get(i))
//            if (v != 0)
//                return false;
//        return true; // all zeros
//    }
//    
//    // get a compact matrix of all real flows
//    public int[][] getAnnulers() {
//        int num=0, i=0;
//        for (int f=0; f<numFlows(); f++)
//            if (isFlow(f))
//                num++;
//        int[][] annulers = new int[num][];
//        for (int f=0; f<numFlows(); f++)
//            if (isFlow(f))
//                annulers[i++] = getFlowVector(f);
//        return annulers;
//    }
//
//    public static int gcd(int a, int b) {
//        assert a >= 0 && b >= 0;
//        if (a == 0)
//            return b;
//
//        while (b != 0) {
//            if (a > b)
//                a = a - b;
//            else
//                b = b - a;
//        }
//
//        return a;
//    }
//    
//    private static int sign(int num) {
//        if (num > 0)
//            return +1;
//        else if (num < 0)
//            return -1;
//        return 0;
//    }
//
//    @Override
//    public void compute(boolean log, ProgressObserver obs) throws InterruptedException {
////        printMat(mA);
//        if (log)
//            System.out.println(this);
//        // Matrix A starts with the flow matrix, D is the identity.
//        // for every transition i=[0,M), repeat:
//        for (int i = 0; i < M; i++) {
//            if (log)
//                System.out.println("\nStep "+i+"/"+(M-1)+"\n"+this);
//            // Append to the matrix [D|A] every rows resulting as a non-negative
//            // linear combination of row pairs from [D|A] whose sum zeroes
//            // the i-th column of A.
//            int nRows = numFlows();
//            int combined_with_i = 0;
//            for (int r1 = 0; r1 < nRows; r1++) {
//                if (mA.get(r1)[i] == 0) {
//                    continue;
//                }
//                obs.advance(i, M+1, r1, nRows);
//                for (int r2 = r1 + 1; r2 < nRows; r2++) {
//                    checkInterrupted();
//                    // Find two rows r1 and r2 such that r1[i] and r2[i] have opposite signs.
//                    if (mA.get(r2)[i] == 0)
//                        continue;
//                    
//                    int mult1, mult2;
//                    if (type.isSemiflow()) { // (non-negative) semiflows
//                        if (sign(mA.get(r1)[i]) == sign(mA.get(r2)[i]))
//                            continue;
//                        mult1 = Math.abs(mA.get(r1)[i]);
//                        mult2 = Math.abs(mA.get(r2)[i]);
//                    }
//                    else { // integer flows
//                        mult1 = Math.abs(mA.get(r1)[i]);
//                        mult2 = Math.abs(mA.get(r2)[i]);
//                        int gcd12 = gcd(mult1, mult2);
//                        mult1 /= gcd12;
//                        mult2 /= gcd12;
//                        if (sign(mA.get(r1)[i]) == sign(mA.get(r2)[i]))
//                            mult1 *= -1;
//                    }
//
//                    // Create a new row nr' such that:
//                    //   nr = |r2[i]| * r1 + |ri[i]| * r2
//                    //   nr' = nr / gcd(nr)
//                    int[] nrA = new int[M];
//                    int[] nrD = new int[N];
//                    int gcdAD = -1;
//                    for (int k = 0; k < M; k++) {
//                        // Compute (with arithmetic overflow check):
//                        nrA[k] = Math.addExact(Math.multiplyExact(mult2, mA.get(r1)[k]),
//                                               Math.multiplyExact(mult1, mA.get(r2)[k]));
//                        gcdAD = (k == 0) ? Math.abs(nrA[k]) : gcd(gcdAD, Math.abs(nrA[k]));
//                    }
//                    assert nrA[i] == 0;
//                    for (int k = 0; k < N; k++) {
//                        // Compute (with arithmetic overflow check):
//                        nrD[k] = Math.addExact(Math.multiplyExact(mult2, mD.get(r1)[k]),
//                                               Math.multiplyExact(mult1, mD.get(r2)[k]));
//                        gcdAD = gcd(gcdAD, Math.abs(nrD[k]));
//                    }
//                    if (gcdAD != 1) {
////                        System.out.println("  gcdAD = " + gcdAD);
//                        for (int k = 0; k < M; k++)
//                            nrA[k] /= gcdAD;
//                        for (int k = 0; k < N; k++)
//                            nrD[k] /= gcdAD;
//                    }
//                    int nnzD = 0;
//                    for (int k = 0; k < N; k++) 
//                        if (nrD[k] != 0)
//                            nnzD++;                  
//                    if (nnzD == 0)
//                        continue; // drop empty row
//
//                    if (log)
//                        System.out.println(i + ": ADD row " + r1 + " + row " + r2 + "  nnz(D)=" + nnzD);
//
//                    mA.add(nrA);
//                    mD.add(nrD);
//                    ++combined_with_i;
//                }
//                    
//                if (type.isBasis() && combined_with_i>0)
//                    break;
//            }
//            checkInterrupted();
//
//            // Eliminate from [D|A] the rows in which the i-th column of A is not zero.
//            int rr = nRows;
//            while (rr > 0) {
//                rr--;
//                obs.advance(i, M+1, rr, nRows);
//                if (mA.get(rr)[i] == 0) {
//                    continue;
//                }
//                if (log)
//                    System.out.println(i + ": DEL row " + rr);
//                mA.remove(rr);
//                mD.remove(rr);
//            }
//            
//            // Eliminate frm [D|A] the rows that are not minimal, doing an exhaustive search.
//            if (!type.isBasis())
//                removeNonMinimalFlows(log, obs);
//        }
//        
//        if (type.isTrapsOrSiphons())
//            dropSupplementaryVariablesAndReduce(log, obs);
//        
//        if (log)
//            System.out.println("\nRESULT:\n"+this);
//
//        obs.advance(M+1, M+1, 1, 1);
//        
//        if (type.isBound()) {
//            computeBoundsFromInvariants();
////            scAlgo.compute(log, obs);
//        }
//        
//        setComputed();
//    }
//    
//    // Test all flows exaustively to check if they are minimal
//    // To do so, we take every pair of flows and we test if one is a
//    // linear component of another. If it is so, subtract the component flow
//    // from the other. When a flow becomes zero, it is removed from A|D.
//    private void removeNonMinimalFlows(boolean log, ProgressObserver obs) 
//            throws InterruptedException 
//    {
//        int rr = numFlows();
//        while (rr > 0) {
//            rr--;
//            obs.advance(M, M+1, numFlows()-rr, numFlows());
//            for (int i=0; i<numFlows(); i++) {
//                checkInterrupted();
//                if (i == rr)
//                    continue;
//                // Check if support(D[i]) subseteq support(D[rr])
//                boolean support_included = true;
//                for (int k=0; k<N && support_included; k++) {
//                    if (mD.get(i)[k] != 0) {
//                        if (mD.get(rr)[k] == 0)
//                            support_included = false;
//                    } 
//                }
//
//                if (support_included) {
//                    // rr was a linear combination of other flows. Remove it
//                    if (log)
//                        System.out.println("DEL row " + rr);
//                    mA.remove(rr);
//                    mD.remove(rr);
//                    break;
//                }
//            }
//        }        
//    }
//    
////    //-----------------------------------------------------------------------
////    // Compute all canonical semiflows, even if they are not minimal
////    public void computeAllCanonicalSemiflows(boolean log, ProgressObserver obs)//, int[][] annulers) 
////            throws InterruptedException 
////    {
////        assert type==PTFlows.Type.PLACE_SEMIFLOWS;
////        if (log)
////            System.out.println(this);
////        Set<Tuple<Integer, Integer>> dropped = new HashSet<>();
////        int initFlows = numFlows();
//////        if (log && annulers!=null) {
//////            for (int[] a : annulers)
//////                System.out.println("ANNULER: "+rowToString(a, null));
//////        }
////        
////        // Matrix A starts with the flow matrix, D is the identity.
////        // for every transition i=[0,M), repeat:
////        for (int i = 0; i < M; i++) {
////            if (log)
////                System.out.println("\nStep "+i+"/"+(M-1)+"\n"+this);
////            // Append to the matrix [D|A] every rows resulting as a non-negative
////            // linear combination of row pairs from [D|A] whose sum zeroes
////            // the i-th column of A.
//////            int nRows = numFlows();
//////            int combined_with_i = 0;
////            for (int r1 = 0; r1 < numFlows(); r1++) {
////                assert numFlows() < 1000;
////                if (mA.get(r1)[i] == 0) {
////                    continue;
////                }
////                obs.advance(i, M+1, r1, numFlows());
////                for (int r2 = r1 + 1; r2 < numFlows(); r2++) {
////                    checkInterrupted();
////                    // Find two rows r1 and r2 such that r1[i] and r2[i] have opposite signs.
////                    if (mA.get(r2)[i] == 0)
////                        continue;
////                    
////                    if (dropped.contains(new Tuple<>(r1, r2)))
////                        continue;
////                    
//////                    int mult1, mult2;
//////                    if (type.isSemiflow()) { // (non-negative) semiflows
////                    if (sign(mA.get(r1)[i]) == sign(mA.get(r2)[i]))
////                        continue;
//////                    mult1 = Math.abs(mA.get(r1)[i]);
//////                    mult2 = Math.abs(mA.get(r2)[i]);
//////                    mult1 = mult2 = 1; // Do all 1-steps
//////                    }
//////                    else { // integer flows
//////                        mult1 = Math.abs(mA.get(r1)[i]);
//////                        mult2 = Math.abs(mA.get(r2)[i]);
//////                        int gcd12 = gcd(mult1, mult2);
//////                        mult1 /= gcd12;
//////                        mult2 /= gcd12;
//////                        if (sign(mA.get(r1)[i]) == sign(mA.get(r2)[i]))
//////                            mult1 *= -1;
//////                    }
////
////                    // Create a new row nr' such that:
////                    //   nr = |r2[i]| * r1 + |ri[i]| * r2
////                    //   nr' = nr / gcd(nr)
////                    int[] nrA = new int[M];
////                    int[] nrD = new int[N];
////                    int gcdAD = -1;
////                    for (int k = 0; k < M; k++) {
////                        // Compute (with arithmetic overflow check):
//////                        nrA[k] = Math.addExact(Math.multiplyExact(mult2, mA.get(r1)[k]),
//////                                               Math.multiplyExact(mult1, mA.get(r2)[k]));
////                        nrA[k] = Math.addExact(mA.get(r1)[k], mA.get(r2)[k]);
////                        gcdAD = (k == 0) ? Math.abs(nrA[k]) : gcd(gcdAD, Math.abs(nrA[k]));
////                    }
//////                    assert nrA[i] == 0;
////                    for (int k = 0; k < N; k++) {
////                        // Compute (with arithmetic overflow check):
//////                        nrD[k] = Math.addExact(Math.multiplyExact(mult2, mD.get(r1)[k]),
//////                                               Math.multiplyExact(mult1, mD.get(r2)[k]));
////                        nrD[k] = Math.addExact(mD.get(r1)[k], mD.get(r2)[k]);
////                        gcdAD = gcd(gcdAD, Math.abs(nrD[k]));
////                    }
////                    // Make canonic
////                    if (gcdAD != 1) {
//////                        System.out.println("  gcdAD = " + gcdAD);
////                        for (int k = 0; k < M; k++)
////                            nrA[k] /= gcdAD;
////                        for (int k = 0; k < N; k++)
////                            nrD[k] /= gcdAD;
////                    }
////                    int nnzD = 0;
////                    for (int k = 0; k < N; k++) 
////                        if (nrD[k] != 0)
////                            nnzD++;
////                    if (nnzD == 0)
////                        continue; // drop empty row
////                    
////                    boolean dropVec = false;
////                    // check if an identical row already exists in D
////                    for (int hh=0; hh<numFlows() && !dropVec; hh++) {
////                        if (Arrays.equals(mD.get(hh), nrD)) {
////                            dropVec = true; // duplicated row
////                        }
////                    }
////                    // check if there is an identical support semiflow that is smaller
////                    if (!dropVec) {
////                        for (int k=initFlows; k<mD.size(); k++) {
////                            if (mA.get(k)[i] == 0) {
////                                if (checkParetoDominance(nrD, mD.get(k))) {
////                                    dropVec = true;
////                                    System.out.println("BOUND: "+rowToString(nrD, nrA)+
////                                                       " < "+rowToString(mD.get(k), mA.get(k)));
////                                    break;                                
////                                }
////                            }
////                        }
////                    }
////                    if (dropVec) {
////                        dropped.add(new Tuple<>(r1, r2));
////                        if (log) {
////                            System.out.println("DROP row:"+r1+" + row:" + r2 + 
////                                               "  nnz(D)=" + nnzD + "  gcdAD="+gcdAD);
//////                            System.out.println(String.format("--: %s", rowToString(nrD, nrA)));
////                        }
////                        continue;
////                    }
////
////                    if (log) {
////                        System.out.println("ADD  row:"+r1+" + row:" + r2 + 
////                                           "  nnz(D)=" + nnzD + "  gcdAD="+gcdAD);
////                        System.out.println(String.format("%2d: %s", mA.size(), rowToString(nrD, nrA)));
////                    }
////                    mA.add(nrA);
////                    mD.add(nrD);
//////                    ++combined_with_i;
////                }
////                    
//////                if (type.isBasis() && combined_with_i>0)
//////                    break;
////            }
////            checkInterrupted();
////            
////            // Eliminate from [D|A] the rows that are >= of the semiflows.
////            int rr = numFlows();
////            while (rr > 0) {
////                rr--;
////                boolean drop = false;
////                for (int jj=0; jj<numFlows() && !drop; jj++) { // loop htrough all semiflows
////                    if (jj != rr && mA.get(jj)[i] == 0) {
////                        if (checkParetoDominance(mD.get(rr), mD.get(jj))) {
////                            drop = true;
////                            System.out.println("DROPX row:"+rr+" >= row:" + jj);
////                            System.out.println(rowToString(mD.get(rr), mA.get(rr))+" >= "+
////                                               rowToString(mD.get(jj), mA.get(jj)));
////                        }
////                    }
////                }
////                if (drop) {
////                    mA.remove(rr);
////                    mD.remove(rr);
////                }
////            }
////
////            // Eliminate from [D|A] the rows in which the i-th column of A is not zero.
////            /*int rr = numFlows();
////            while (rr > 0) {
////                rr--;
////                obs.advance(i, M+1, rr, numFlows());
////                if (mA.get(rr)[i] == 0) {
////                    continue;
////                }
////                if (log)
////                    System.out.println(i + ": DEL row " + rr);
////                mA.remove(rr);
////                mD.remove(rr);
////            }*/
////            
//////            // Eliminate frm [D|A] the rows that are not minimal, doing an exhaustive search.
//////            if (!type.isBasis())
//////                removeNonMinimalFlows(log, obs);
////        }
////        
////        if (log)
////            System.out.println("\nRESULT:\n"+this);
////
////        // Remove all the initial flows
////        mA = new ArrayList<>(mA.subList(initFlows, mA.size()));
////        mD = new ArrayList<>(mD.subList(initFlows, mD.size()));
//////        if (log)
//////            System.out.println("\nREMOVING "+initFlows+" initial flows.");
////        
//////        if (type.isTrapsOrSiphons())
//////            dropSupplementaryVariablesAndReduce(log, obs);
////
////        obs.advance(M+1, M+1, 1, 1);
////        
//////        if (type.isBound()) {
//////            computeBoundsFromInvariants();
////////            scAlgo.compute(log, obs);
//////        }
////        
////        setComputed();
////    }
//////    //-----------------------------------------------------------------------
////    private boolean checkSameSupport(int[] vec1, int[] vec2) {
////        for (int i=0; i<vec1.length; i++)
////            if ((vec1[i]==0) != (vec2[i]==0))
////                return false; // different supports
////        return true;
////    }
////    //-----------------------------------------------------------------------
////    // check if vec1 Pareto-dominates vec2, i.e. iff vec1 >= vec2
////    private boolean checkParetoDominance(int[] vec1, int[] vec2) {
////        if (!checkSameSupport(vec1, vec2))
////            return false; // compare only if they have the same support
////        
////        for (int i=0; i<vec1.length; i++) {
////            if (vec1[i] < vec2[i])
////                return false;
////        }
////        return true;
////        
////       /* int eq = 0, greater = 0;
//////        boolean eq = true, greater = true;//, less = true;
////        for (int i=0; i<vec1.length; i++) {
////            if (vec1[i] == vec2[i])  eq++;
////            if (vec1[i] > vec2[i])   greater++;
//////            if (vec1[i] < vec2[i]) {
//////                eq = false;
//////                greater = false;
//////            }
//////            else if (vec1[i] > vec2[i]) {
//////                eq = false;
//////                // less = false;
//////            }
////        }
//////        if (less)
//////            System.out.println("### "+rowToString(vec1, null)+" < "+rowToString(vec2, null));
//////        if (greater)
//////            System.out.println("### "+rowToString(vec1, null)+" > "+rowToString(vec2, null));
////        boolean is_eq = (eq == vec1.length);
////        boolean is_greater_eq = (greater > 0) && (greater + eq == vec1.length);
////        
////        return is_eq || is_greater_eq;// || less;       */ 
////        
//////        boolean eq = true;
//////        for (int i=0; i<vec1.length; i++) {
//////            if (vec1[i] < vec2[i])
//////                eq = false;
//////            else if (vec1[i] > vec2[i])
//////                return true;
//////        }
//////        return !eq;
////        
//////        for (int i=0; i<semiflow.length; i++)
//////            if ((semiflow[i]==0) != (vec[i]==0))
//////                return false; // support is different
//////        // check that vec is not greater than @semiflow
//////        for (int i=0; i<semiflow.length; i++)
//////            if (vec[i] > semiflow[i])
//////                return true; // drop
//////        return false; 
////    }
//    
//    //-----------------------------------------------------------------------
//    
//    // Reduce all remaining flows after removing all supplementary variables
//    // Supplementary variable are located in D in the positions between N0 and N.
//    // Therefore reduce N to N0 (by truncating all rows), and then remove again all
//    // the non-minimal flows.
//    private void dropSupplementaryVariablesAndReduce(boolean log, ProgressObserver obs) 
//            throws InterruptedException 
//    {
//        // truncate and remove all supplementary variables in D
//        for (int rr=numFlows()-1; rr>=0; rr--) {
//            int[] newDi = new int[N0];
//            System.arraycopy(mD.get(rr), 0, newDi, 0, N0);
//            
//            int nnz = 0;
//            for (int k = 0; k < N0; k++)
//                nnz += (newDi[k] != 0 ? 1 : 0);
//            
//            if (nnz > 0) {
//                mD.set(rr, newDi);
//            }
//            else {
//                if (log)
//                    System.out.println("DEL row " + rr);
//                mA.remove(rr);
//                mD.remove(rr);
//            }
//        }
//        reduceN(N0);
//        // reduce the flows that are not minimal after removing the suppl. vars.
//        removeNonMinimalFlows(log, obs);
//    }
//    
//    private String rowToString(int[] rowD, int[] rowA) {
//        StringBuilder sb = new StringBuilder();
//        for (int j = 0; j < N; j++) {
//            sb.append(rowD[j] < 0 ? "" : " ").append(rowD[j]).append(" ");
//        }
//        if (rowA != null) {
//            sb.append("| ");
//            for (int j = 0; j < M; j++) {
//                sb.append(rowA[j] < 0 ? "" : " ").append(rowA[j]).append(" ");
//            }
//        }
//        return sb.toString();
//    }
//    
//    @Override
//    public String toString() {
//        StringBuilder sb = new StringBuilder();
//        for (int i = 0; i < numFlows(); i++) {
//            sb.append(String.format("%2d: ", i));
//            sb.append(rowToString(mD.get(i), mA.get(i)));
//            sb.append("\n");
//        }
//        return sb.toString();
//    }
//    
//    
//    void computeBoundsFromInvariants() {
//        lowerBnd = new int[N];
//        upperBnd = new int[N];
//        Arrays.fill(upperBnd, Integer.MAX_VALUE);
//        
//        // Read all place invariants
//        for (int i=0; i<numFlows(); i++) {
//            int[] inv = getFlowVector(i);
//            int tokenCnt = 0;
//            for (int p=0; p<N; p++) {
//                tokenCnt += inv[p] * initQuantity[p];
//            }
//            
////            System.out.print("PINV: ");
////            for (int p=0; p<N; p++)
////                if (inv[p] > 0)
////                    System.out.print(inv[p]+"*P"+p+"(m0="+initQuantity[p]+")  ");
////            System.out.println("   tc="+tokenCnt);
//            
//            int num_nnz = 0, kk = -1, last_p = -1;
//            for (int p=0; p<N; p++) {
//                if (inv[p] > 0) {
//                    last_p = p;
//                    num_nnz++;
//                    kk = tokenCnt / inv[p];
//                    upperBnd[p] = Math.min(upperBnd[p], kk);
//                }
//            }
//            if (num_nnz == 1 && kk > lowerBnd[last_p])
//                lowerBnd[last_p] = kk;
//        }
//        
////        for (int p=0; p<N; p++) {
////            System.out.println("Place "+p+" has bounds: ["+lowerBnd[p]+" - "+upperBnd[p]+"]");
////        }
//    }
//    
//    public int getUpperBoundOf(int p) {
//        assert isComputed() && type.isBound() && initQuantity != null && p >= 0 && p < N;
//        return upperBnd[p];
//    }
//    
//    public int getLowerBoundOf(int p) {
//        assert isComputed() && type.isBound() && initQuantity != null && p >= 0 && p < N;
//        return lowerBnd[p];
//    }
//    
//    
//    
//    
//    public String toLatexString(NetIndex netIndex, boolean showZeros) {
//        StringBuilder sb = new StringBuilder();
//        String[] symbols = type.getLatexFlowName();
//        
//        // header
//        sb.append("$\\begin{array}{r");
//        for (int f=0; f<numFlows(); f++)
//            sb.append("|c");
//        if (type.isPlace())
//            sb.append("|r");
//        sb.append("}\n ");
//        for (int f=0; f<numFlows(); f++)
//            sb.append("& ").append(symbols[0]).append("_{").append(f+1).append("}");
//        if (type.isPlace())
//            sb.append("& \\mathbf{m}_0");
//        sb.append("\\\\ \n\\hline\n");
//                
//        // row for a place/transition
//        for (int row=0; row<N; row++) {
//            if (type.isPlace())
//                sb.append(netIndex.places.get(row).getUniqueNameDecor().getLatexFormula().getLatex());
//            else
//                sb.append(netIndex.transitions.get(row).getUniqueNameDecor().getLatexFormula().getLatex());
//            
//            for (int f=0; f<numFlows(); f++) {
//                int[] flow = getFlowVector(f);
//                
//                sb.append(" &");
//                if (showZeros || flow[row]!=0) {
//                    String color;
//                    if (flow[row] > 0)
//                        color = "Blue";
//                    else if (flow[row] < 0)
//                        color = "Mahogany";
//                    else
//                        color = "Gray";
//                    sb.append("\\textcolor{").append(color).append("}{").append(flow[row]).append("}");
//                }
//            } 
//            
//            if (type.isPlace()) {
//                sb.append(" & \\mathbf{").append(initQuantity[row]).append("}");
//            }
//            sb.append("\\\\ \n\\hline\n");
//            
//        }
//        
//        // final row
//        if (type.isPlace()) {
//            sb.append("\n \\mathbf{m}_0 \\cdot ").append(symbols[1]).append(" & ");
//            
//            for (int f=0; f<numFlows(); f++) {
//                int[] flow = getFlowVector(f);
//                int sum = 0;
//                for (int j=0; j<flow.length; j++) {
//                    int initMark = initQuantity[j];
//                    sum += initMark * flow[j];
//                }
//                
//                sb.append("\\mathbf{").append(sum).append("} & ");
//            }
//            sb.append("\\\\ \n");
//        }
//        
//        sb.append("\\end{array}$");
//        return sb.toString();
//    }
//    
//    
//    // Initialization status
//    public boolean hasInhibitorEdges = false;
//    public boolean hasMarkDepEdges = false;
//    public boolean hasEdgeWithZeroCard = false;
//    public boolean hasContinuousEdges = false;
//    public boolean hasColoredPlaces = false;
//    public boolean hasNonIntegerInitMarks = false;
//    
//    // Support structures for building traps/siphons
//    private int[] trIndexStart, trIndexEnd;
//
//    
//    // Compute the initial sizes of the N,M
//    public static OldFlowsGenerator makeFor(PTFlows.Type type, NetIndex netIndex) {
//        int N, N0, M;
//        int[] trIndexStart = null, trIndexEnd = null;
//        
//        if (type.isTrapsOrSiphons()) {
//            N = N0 = netIndex.numPlaces();
//            M = 0;
//            // for traps/siphons, the incidence matrix is modified:
//            // each IA/OA arc generates a duplicate transition with an
//            // associated supplementary variable.
//            GspnEdge.Kind primaryKind = (type==PTFlows.Type.TRAPS ? GspnEdge.Kind.INPUT : GspnEdge.Kind.OUTPUT);
//            trIndexStart = new int[netIndex.transitions.size()];
//            trIndexEnd = new int[netIndex.transitions.size()];
//            int jj = 0;
//            for (int tt=0; tt<netIndex.transitions.size(); tt++) {
//                Transition trn = netIndex.transitions.get(tt);
//                trIndexStart[tt] = jj;
//                for (Edge ee : netIndex.net.edges) {
//                    if (ee instanceof GspnEdge) {
//                        GspnEdge e = (GspnEdge)ee;
//                        if (e.getConnectedTransition() == trn && e.getEdgeKind() == primaryKind) {
//                            N++; // add a supplementary variable
//                            M++; // add a new transition replica
//                            jj++; // increment the replication index
//                        }
//                    }
//                }
//                trIndexEnd[tt] = jj;
//            }
//        }
//        else if (type.isPlace()) {
//            N = N0 = netIndex.numPlaces();
//            M = netIndex.numTransition();
//        }
//        else {
//            N = N0 = netIndex.numTransition();
//            M = netIndex.numPlaces();
//        }
//        OldFlowsGenerator fg = new OldFlowsGenerator(N, N0, M, type);
//        fg.trIndexStart = trIndexStart;
//        fg.trIndexEnd = trIndexEnd;
//        return fg;
//    }
//
//    // Initialize the matrices from a gspn page
//    public void initialize(PTFlows.Type type, TemplateBinding varBindings, NetIndex netIndex) {
//        ParserContext context = new ParserContext(netIndex.net);
//        context.templateVarsBinding = varBindings;
//        context.bindingContext = context;
//        
//        int[] trSecondaryIndex = null;
//        if (type.isTrapsOrSiphons())
//            trSecondaryIndex = new int[netIndex.numTransition()];
//        
//        for (Edge edge : netIndex.net.edges) {
//            if (edge instanceof GspnEdge) {
//                GspnEdge ge = (GspnEdge)edge;
//                int p, t, card, sign;
//                
//                // Ignore inhibitor edges, but signal their presence.
//                if (ge.getEdgeKind() == GspnEdge.Kind.INHIBITOR) {
//                    hasInhibitorEdges = true;
//                    continue;
//                }
//                // Ignore flow edges, but signal their presence.
//                if (ge.isContinuous() || ge.isFiringFlow()) {
//                    hasContinuousEdges = true;
//                    continue;
//                }
//                // Ignore colored edges
//                if (!ge.getColorDomainOfConnectedPlace().isNeutralDomain()) {
//                    hasColoredPlaces = true;
//                    continue;
//                }
//                switch (ge.getEdgeKind()) {
//                    case INPUT:
////                        System.out.println("head="+ge.getHeadNode().getUniqueName()+" tail="+ge.getTailNode().getUniqueName());
//                        p = netIndex.place2index.get(ge.getConnectedPlace());
//                        t = netIndex.trn2index.get(ge.getConnectedTransition());
//                        sign = -1;
//                        try {
//                            card = ge.evaluateMultiplicity(context, null, null).getScalarInt();
//                        }
//                        catch (Exception e) {
//                            hasMarkDepEdges = true;
//                            card = 1;
//                        }
//                        break;
//                        
//                    case OUTPUT:
//                        p = netIndex.place2index.get(ge.getConnectedPlace());
//                        t = netIndex.trn2index.get(ge.getConnectedTransition());
//                        sign = 1;
//                        try {
//                            card = ge.evaluateMultiplicity(context, null, null).getScalarInt();
//                        }
//                        catch (Exception e) {
//                            hasMarkDepEdges = true;
//                            card = 1;
//                        }
//                        break;
//                        
//                    default:
//                        throw new IllegalStateException();
//                }
//                if (card == 0)
//                    hasEdgeWithZeroCard = true;
//                
//                if (!type.isTrapsOrSiphons()) {
//                    if (type.isPlace())
//                        addIncidence(p, t, card * sign);
//                    else
//                        addIncidence(t, p, card * sign);
//                }
//                else { // traps/siphons construction
//                    GspnEdge.Kind primaryKind = (type==PTFlows.Type.TRAPS ? GspnEdge.Kind.INPUT : GspnEdge.Kind.OUTPUT);
//                    int tt = netIndex.trn2index.get(ge.getConnectedTransition());
//                    if (ge.getEdgeKind() == primaryKind) {
//                        // on the primary edge kind (Input for traps, Output for siphons)
//                        // put a +1 on the jj-th transition column replica
//                        int jj = trIndexStart[tt] + trSecondaryIndex[tt];
//                        assert jj < trIndexEnd[tt];
//                        if (mA.get(p)[jj] == 0)
//                            setIncidence(p, jj, 1);
//                        ++trSecondaryIndex[tt]; // increment replica counter
//                    }
//                    else {
//                        // set the value of the secondary edge kind on all replicas of transition tt
//                        for (int jj=trIndexStart[tt]; jj<trIndexEnd[tt]; jj++)
//                            setIncidence(p, jj, -1);
//                    }
//                }
//            }            
//        }
//        trIndexStart = null;
//        trIndexEnd = null;
//        
//        if (type.isTrapsOrSiphons()) {
//            // Add the supplementary variables, i.e. the identity matrix 
//            // below the incidence. A = [C] over [I]
//            for (int i=0; i<M; i++)
//                setIncidence(i + N0, i, 1);
//        }
//        
//        if (type.isPlace()) {
//            // Prepare also the initial place quantities (for bound computation)
//            for (Place plc : netIndex.places) {
//                int m0;
//                try {
//                    m0 = plc.evaluateInitMarking(context).getScalarInt();
//                }
//                catch (Exception e) {
//                    hasNonIntegerInitMarks = true;
//                    m0 = 0;
//                }
//                setInitQuantity(netIndex.place2index.get(plc), m0);
//            }
//        }
//        
//    }
//    
//    public String flowToString(int i, NetIndex netIndex, boolean asInvariant, 
//            Color htmlForegroundColor, Color htmlBackgroundColor, Node selNode,
//            int activeIndex) 
//    {
//        StringBuilder repr = new StringBuilder();
//
//        int selK = -1;
//        if (selNode instanceof Place)
//            selK = netIndex.place2index.get((Place)selNode);
//        else if (selNode instanceof Transition)
//            selK = netIndex.trn2index.get((Transition)selNode);
//        int[] flow = getFlowVector(i);
//        
//        boolean selected = (selK == -1) || (flow[selK] != 0);
//        boolean active = (i == activeIndex);
////        System.out.println("selected="+selected+" selK="+selK+" activeIndex="+activeIndex+
////                " htmlForegroundColor="+htmlForegroundColor+
////                " htmlBackgroundColor="+htmlBackgroundColor);
//
//        boolean useHtml = (htmlForegroundColor != null);
//        String negClrHex = null, eqClrHex = null, posClrHex = null;
//        if (useHtml) {
//            Color posClr;
//            if (active)
//                posClr =  selected ? htmlBackgroundColor : 
//                          (Util.mix(htmlBackgroundColor, htmlForegroundColor, 0.5f));
//            else
//                posClr =  selected ? htmlForegroundColor : 
//                          (Util.mix(htmlForegroundColor, htmlBackgroundColor, 0.5f));
//                           
//            Color negClr = Util.mix(posClr, Color.MAGENTA, 0.5f);
//            Color eqClr = Util.mix(posClr, Color.CYAN, 0.5f);
//            posClrHex = Util.clrToHex(posClr);
//            negClrHex = Util.clrToHex(negClr);
//            eqClrHex = Util.clrToHex(eqClr);
//            repr.append("<html>");
//        }
//        
//        for (int k=0; k<flow.length; k++) {
//            if (flow[k] == 0)
//                continue;
//            if (repr.length() > 0)
//                repr.append(" ");
//            if (useHtml) {
//                repr.append("<font color='").append(flow[k] > 0 ? posClrHex : negClrHex).append("'>");
//                if (k == selK)
//                    repr.append("<b>");
//            }
//            if (flow[k] > 0) {
//                if (flow[k] != 1)
//                    repr.append(flow[k]).append("*");
//            }
//            else {
//                if (flow[k] == -1)
//                    repr.append("-");
//                else
//                    repr.append(flow[k]).append("*");                
//            }
//            if (type.isPlace())
//                repr.append(netIndex.places.get(k).getUniqueName());
//            else
//                repr.append(netIndex.transitions.get(k).getUniqueName());
//            if (useHtml) {
//                if (k == selK)
//                    repr.append("</b>");
//                repr.append("</font>");
//            }
//        }
//        
//        String equationSign = type.getInvariantSign();
//        if (asInvariant && equationSign!=null && initQuantity!=null) {
//            // Compute the invariant quantity
//            int q = 0;
//            for (int k=0; k<flow.length; k++)
//                q += flow[k] * initQuantity[k];
//            
//            if (useHtml)
//                repr.append("<font color='").append(eqClrHex).append("'>");
//            repr.append(" ").append(equationSign).append(" ").append(q);
//            if (useHtml)
//                repr.append("</font>");
//        }
//        
//        return repr.toString();
//    }
//
////    public static void main(String[] args) {
////        int NP = 14, MT = 10;
////        OldFlowsGenerator msa = new OldFlowsGenerator(NP, MT);
////        int[][] flow = {
////            {5, 4}, {1, 3}, // t1
////            {14, 7}, {6, 8}, // t2
////            {9}, {10, 11}, // t3
////            {2, 13}, {4}, // t4
////            {1}, {2}, // t5
////            {3}, {14}, // t6
////            {6}, {5}, // t7
////            {8}, {9}, // t8
////            {10, 12}, {7}, // t9
////            {11}, {12, 13} // t10
////        };
////        for (int t = 0; t < MT; t++) {
////            int[] in = flow[2 * t], out = flow[2 * t + 1];
////            for (int p = 0; p < in.length; p++) {
////                msa.addFlow(in[p] - 1, t, -1);
////            }
////            for (int p = 0; p < out.length; p++) {
////                msa.addFlow(out[p] - 1, t, +1);
////            }
////        }
////        msa.compute(true);
////    }
//    
//
//    
//    private static void printMat(ArrayList<int[]> mat) {
//        for (int[] row : mat) {
//            for (int j=0; j<row.length; j++) {
//                System.out.print((j==0 ? "{" : ", ")+row[j]);
//            }
//            System.out.println("},");
//        }
//    }
//    
//
//    public static void main(String[] args) throws InterruptedException {
//        ProgressObserver obs = (int step, int total, int s, int t) -> { };
//        FlowsGeneratorTestData.FlowProblem problem = FlowsGeneratorTestData.probSiphonBasis;
//        OldFlowsGenerator fg = new OldFlowsGenerator(problem.input, problem.probType);
//        if (problem.probType.isTrapsOrSiphons())
//            fg.N0 -= problem.input[0].length;
//        printMat(fg.mA);
//        fg.compute(true, obs);
//        printMat(fg.mD);
//        
//        // Compare the result matrix with the known results for that problem
//        if (problem.solution != null) {
//            int[][] mResult = fg.getAnnulers();
//            Comparator<int[]> comp = (int[] o1, int[] o2) -> Arrays.compare(o1, o2);
//            Arrays.sort(mResult, comp);
//            Arrays.sort(problem.solution, comp);
//
//            boolean equal = (mResult.length == problem.solution.length);
//            if (equal) {
//                for (int i=0; equal && i<mResult.length; i++) {
//                    equal = Arrays.equals(mResult[i], problem.solution[i]);
//                }
//            }
//            System.out.println("Check solution: "+equal);
//        }
//    }
//}
