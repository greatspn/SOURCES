/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.semiflows;

import editor.domain.Edge;
import editor.domain.Node;
import editor.domain.elements.GspnEdge;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.Transition;
import editor.domain.grammar.ParserContext;
import editor.domain.grammar.TemplateBinding;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Map;

/** Martinez-Silva version of the Farkas algorithm for the computation
 *  of the minimal set of P(T)-semiflows in a Petri net.
 *
 * @author elvio
 */
public class MartinezSilvaAlgorithm extends StructuralAlgorithm {

    
    // Iteration matrix [ D(i) | A(i) ], where D(0)=I and A(0) = Flow matrix
    // In addition, keep the B matrix for the Martinez-Silva optimization.
    // K starts with K=N. After the computation, K is the number of semiflows.
    public ArrayList<int[]> mD;     // KxN matrix
    public ArrayList<int[]> mA;     // KxM matrix
    public ArrayList<boolean[]> mB; // KxM matrix
    
    // This extra informations keep the initial place marking, when
    // the method is used for P-invariants. It allows to derive the place bounds
    // from the P-invariants
    public boolean computeBounds = false;
    public int[] initQuantity; // array of N elements
    public int[] lowerBnd, upperBnd; // place bounds
    
    // Will compute semiflows or integer flows
    public boolean onlySemiflows = true;
    public boolean buildBasis = false;
    
    
//    SilvaColom88Algorithm scAlgo;
    

    // For P-semiflows: N=|P|, M=|T| (for T-semiflows: N=|T|, M=|P|)
    public MartinezSilvaAlgorithm(int N, int M) {
        super(N, M);
//        System.out.println("M="+M+", N="+N);
        mD = new ArrayList<>();
        mA = new ArrayList<>();
        mB = new ArrayList<>();
        for (int i = 0; i < N; i++) {
            mD.add(new int[N]);
            mA.add(new int[M]);
            mB.add(new boolean[M]);
            mD.get(i)[i] = 1;
        }
//        scAlgo = new SilvaColom88Algorithm(N, M);
    }

    // Add a flow from i to j with the specified cardinality
    @Override
    public void addFlow(int i, int j, int card) {
//        System.out.println("msa.addFlow("+i+", "+j+", "+card+");");
        mA.get(i)[j] += card;
        mB.get(i)[j] = (mA.get(i)[j] != 0);
        
//        scAlgo.addFlow(i, j, card);
    }
    
    // Add the initial token of a place (only for P-invariant computation)
    @Override
    public void setInitQuantity(int i, int quantity) {
        assert i < N && i >= 0;
        if (initQuantity == null) { // Not initialized yet
            initQuantity = new int[N];
            computeBounds = true;
        }
        assert initQuantity[i] == 0;
        initQuantity[i] = quantity;
        
//        scAlgo.setInitQuantity(i, quantity);
    }
    
    public int[] getSemiflow(int i) {
        return mD.get(i);
    }

    private static int gcd(int a, int b) {
        assert a >= 0 && b >= 0;
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
    
    private static int sign(int num) {
        if (num > 0)
            return +1;
        else if (num < 0)
            return -1;
        return 0;
    }
    

    @Override
    public void compute(boolean log, ProgressObserver obs) throws InterruptedException {
        if (log)
            System.out.println(this);
        // Matrix A starts with the flow matrix, D is the identity.
        // for every transition i=[0,M), repeat:
        for (int i = 0; i < M; i++) {
            if (log)
                System.out.println("\nStep "+i+"/"+(M-1)+"\n"+this);
            // Append to the matrix [D|A] every rows resulting as a non-negative
            // linear combination of row pairs from [D|A] whose sum zeroes
            // the i-th column of A.
            int nRows = numSemiflows();
            int combined_with_i = 0;
            for (int r1 = 0; r1 < nRows; r1++) {
                if (mA.get(r1)[i] == 0) {
                    continue;
                }
                obs.advance(i, M+1, r1, nRows);
                for (int r2 = r1 + 1; r2 < nRows; r2++) {
                    checkInterrupted();
                    // Find two rows r1 and r2 such that r1[i] and r2[i] have opposite signs.
                    if (mA.get(r2)[i] == 0)
                        continue;
                    
                    int mult1, mult2;
                    if (onlySemiflows) { // (non-negative) semiflows
                        if (sign(mA.get(r1)[i]) == sign(mA.get(r2)[i]))
                            continue;
                        mult1 = Math.abs(mA.get(r1)[i]);
                        mult2 = Math.abs(mA.get(r2)[i]);
                    }
                    else { // integer flows
                        mult1 = Math.abs(mA.get(r1)[i]);
                        mult2 = Math.abs(mA.get(r2)[i]);
                        int gcd12 = gcd(mult1, mult2);
                        mult1 /= gcd12;
                        mult2 /= gcd12;
                        if (sign(mA.get(r1)[i]) == sign(mA.get(r2)[i]))
                            mult1 *= -1;
                    }

                    // Create a new row nr' such that:
                    //   nr = |r2[i]| * r1 + |ri[i]| * r2
                    //   nr' = nr / gcd(nr)
                    //   nr(B) = logical union of B[r1] and B[r2]
                    int[] nrA = new int[M];
                    int[] nrD = new int[N];
                    boolean[] nrB = new boolean[M];
                    int gcdAD = -1;
                    for (int k = 0; k < M; k++) {
                        nrA[k] = mult2 * mA.get(r1)[k] + mult1 * mA.get(r2)[k];
                        gcdAD = (k == 0) ? Math.abs(nrA[k]) : gcd(gcdAD, Math.abs(nrA[k]));
                        nrB[k] = mB.get(r1)[k] || mB.get(r2)[k];
                    }
                    assert nrA[i] == 0;
                    for (int k = 0; k < N; k++) {
                        nrD[k] = mult2 * mD.get(r1)[k] + mult1 * mD.get(r2)[k];
                        gcdAD = gcd(gcdAD, Math.abs(nrD[k]));
                    }
                    if (gcdAD != 1) {
//                        System.out.println("  gcdAD = " + gcdAD);
                        for (int k = 0; k < M; k++) {
                            nrA[k] /= gcdAD;
                        }
                        for (int k = 0; k < N; k++) {
                            nrD[k] /= gcdAD;
                        }
                    }
                    int nnzD = 0, ntrueB = 0;
                    for (int k = 0; k < M; k++) {
                        ntrueB += (nrB[k] ? 1 : 0);
                    }
                    for (int k = 0; k < N; k++) {
                        nnzD += (nrD[k] != 0 ? 1 : 0);
                    }                    
                    if (nnzD == 0)
                        continue; // drop empty row

                    // Martinez-Silva optimization of the Farkas algorithm.
                    // The row is not a minimal support if the count of non-zero entries in D
                    // is greater than the number of TRUEs in B (for that row) + 1.
                    // If this happens, the row is not a minimal P(T)-semiflow and
                    // can be safely discarded.
                    if (nnzD > ntrueB+1) {
                        continue;
                    }
                    if (log)
                        System.out.println(i + ": ADD row " + r1 + " + row " + r2 +
                                           "  nnz(D)=" + nnzD + " r'=" + ntrueB);

                    mA.add(nrA);
                    mD.add(nrD);
                    mB.add(nrB);
                    ++combined_with_i;
                }
                    
                if (buildBasis && combined_with_i>0)
                    break;
            }
            checkInterrupted();

            // Eliminate from [D|A] the rows in which the i-th column of A is not zero.
            int rr = nRows;
            while (rr > 0) {
                rr--;
                obs.advance(i, M+1, rr, nRows);
                if (mA.get(rr)[i] == 0) {
                    continue;
                }
                if (log)
                    System.out.println(i + ": DEL row " + rr);
                mA.remove(rr);
                mD.remove(rr);
                mB.remove(rr);
            }
            
            // Eliminate frm [D|A] the rows that are not minimal, doing an exhaustive search.
            if (!buildBasis)
                removeNonMinimalSemiflows(log, obs);
        }
        if (log)
            System.out.println("\nRESULT:\n"+this);

        obs.advance(M+1, M+1, 1, 1);
        
        if (computeBounds) {
            computeBoundsFromInvariants();
//            scAlgo.compute(log, obs);
        }
        
        setComputed();
    }
    
    // Test all semiflows exaustively to check if they are minimal
    // To do so, we take every pair of semiflows and we test if one is a
    // linear component of another. If it is so, subtract the component semiflow
    // from the other. When a semiflow becomes zero, it is removed from A,D and B.
    private void removeNonMinimalSemiflows(boolean log, ProgressObserver obs) 
            throws InterruptedException 
    {
        int rr = numSemiflows();
        while (rr > 0) {
            rr--;
            obs.advance(M, M+1, numSemiflows()-rr, numSemiflows());
            for (int i=0; i<numSemiflows(); i++) {
                checkInterrupted();
                if (i == rr)
                    continue;
                // Check if the semiflow D[rr] contains D[i]
                // Check if support(D[i]) subseteq support(D[rr])
                boolean support_included = true;
                for (int k=0; k<N && support_included; k++) {
                    if (mD.get(i)[k] != 0) {
                        if (mD.get(rr)[k] == 0)
                            support_included = false;
                    } 
                }
//                int mult = -1;
//                boolean isComp = true; // Says if i is a linear component of rr
//                for (int k=0; k<N; k++) {
//                    if (mD.get(i)[k] != 0) {
//                        if (mD.get(i)[k] > mD.get(rr)[k]) {
//                            isComp = false;
//                            break; // i is not a linear component of rr
//                        }
//                        if (mult == -1)
//                            mult = mD.get(rr)[k] / mD.get(i)[k];
//                        else
//                            mult = Math.min(mult, mD.get(rr)[k] / mD.get(i)[k]);
//                    }
//                }
//                if (!isComp)
//                    continue;
//                // in matrix D:  D[rr] -= mult * D[i]
//                assert mult > 0;
//                boolean support_included = true;
//                for (int k=0; k<N; k++) {
//                    mD.get(rr)[k] -= mult * mD.get(i)[k];
//                    support_included = support_included && (mD.get(rr)[k] == 0);
//                }
                if (support_included) {
                    // rr was a linear combination of other semiflows. Remove it
                    if (log)
                        System.out.println("DEL row " + rr);
                    mA.remove(rr);
                    mD.remove(rr);
                    mB.remove(rr);
                    break;
                }
            }
        }        
    }

    public int numSemiflows() {
        return mD.size();
    }
    
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < numSemiflows(); i++) {
            sb.append(i < 10 ? " " : "").append(i).append(":  ");
            for (int j = 0; j < N; j++) {
                sb.append(mD.get(i)[j] < 0 ? "" : " ").append(mD.get(i)[j]).append(" ");
            }
            sb.append("| ");
            for (int j = 0; j < M; j++) {
                sb.append(mA.get(i)[j] < 0 ? "" : " ").append(mA.get(i)[j]).append(" ");
            }
            sb.append("|");
            for (int j = 0; j < M; j++) {
                sb.append(mB.get(i)[j] ? " T" : " .");
            }
            sb.append("\n");
        }
        return sb.toString();
    }
    
    
    void computeBoundsFromInvariants() {
        lowerBnd = new int[N];
        upperBnd = new int[N];
        Arrays.fill(upperBnd, Integer.MAX_VALUE);
        
        // Read all place invariants
        for (int i=0; i<numSemiflows(); i++) {
            int[] inv = getSemiflow(i);
            int tokenCnt = 0;
            for (int p=0; p<N; p++) {
                tokenCnt += inv[p] * initQuantity[p];
            }
            
//            System.out.print("PINV: ");
//            for (int p=0; p<N; p++)
//                if (inv[p] > 0)
//                    System.out.print(inv[p]+"*P"+p+"(m0="+initQuantity[p]+")  ");
//            System.out.println("   tc="+tokenCnt);
            
            int num_nnz = 0, kk = -1, last_p = -1;
            for (int p=0; p<N; p++) {
                if (inv[p] > 0) {
                    last_p = p;
                    num_nnz++;
                    kk = tokenCnt / inv[p];
                    upperBnd[p] = Math.min(upperBnd[p], kk);
                }
            }
            if (num_nnz == 1 && kk > lowerBnd[last_p])
                lowerBnd[last_p] = kk;
        }
        
//        for (int p=0; p<N; p++) {
//            System.out.println("Place "+p+" has bounds: ["+lowerBnd[p]+" - "+upperBnd[p]+"]");
//        }
    }
    
    public int getUpperBoundOf(int p) {
        assert isComputed() && computeBounds && initQuantity != null && p >= 0 && p < N;
        return upperBnd[p];
    }
    
    public int getLowerBoundOf(int p) {
        assert isComputed() && computeBounds && initQuantity != null && p >= 0 && p < N;
        return lowerBnd[p];
    }
    
    
    
    
    public String toLatexString(SemiFlows.Type type, NetIndex netIndex,
                                boolean showZeros) 
    {
        StringBuilder sb = new StringBuilder();
        
        // header
        sb.append("$\\begin{array}{r");
        for (int f=0; f<numSemiflows(); f++)
            sb.append("|c");
        if (type.isPlace())
            sb.append("|r");
        sb.append("}\n ");
        for (int f=0; f<numSemiflows(); f++)
            sb.append("& i_{").append(f+1).append("}");
        if (type.isPlace())
            sb.append("& \\mathbf{m}_0");
        sb.append("\\\\ \n\\hline\n");
                
        // row for a place/transition
        for (int row=0; row<N; row++) {
            if (type.isPlace())
                sb.append(netIndex.places.get(row).getUniqueNameDecor().getLatexFormula().getLatex());
            else
                sb.append(netIndex.transitions.get(row).getUniqueNameDecor().getLatexFormula().getLatex());
            
            for (int f=0; f<numSemiflows(); f++) {
                int[] semiflow = getSemiflow(f);
                
                sb.append(" &");
                if (showZeros || semiflow[row]!=0) {
                    String color;
                    if (semiflow[row] > 0)
                        color = "Blue";
                    else if (semiflow[row] < 0)
                        color = "Mahogany";
                    else
                        color = "Gray";
                    sb.append("\\textcolor{").append(color).append("}{").append(semiflow[row]).append("}");
                }
            } 
            
            if (type.isPlace()) {
                sb.append(" & \\mathbf{").append(initQuantity[row]).append("}");
            }
            sb.append("\\\\ \n\\hline\n");
            
        }
        
        // final row
        if (type.isPlace()) {
            sb.append("\n \\mathbf{m}_0 \\cdot I & ");
            
            for (int f=0; f<numSemiflows(); f++) {
                int[] semiflow = getSemiflow(f);
                int sum = 0;
                for (int j=0; j<semiflow.length; j++) {
                    int initMark = initQuantity[j];
                    sum += initMark * semiflow[j];
                }
                
                sb.append("\\mathbf{").append(sum).append("} & ");
            }
            sb.append("\\\\ \n");
        }
        
        sb.append("\\end{array}$");
        return sb.toString();
    }
    
    
    // Initialization status
    public boolean hasInhibitorEdges = false;
    public boolean hasMarkDepEdges = false;
    public boolean hasEdgeWithZeroCard = false;
    public boolean hasContinuousEdges = false;
    public boolean hasColoredPlaces = false;
    public boolean hasNonIntegerInitMarks = false;

    // Initialize the matrices from a gspn page
    public void initialize(SemiFlows.Type type, TemplateBinding varBindings, NetIndex netIndex) 
    {
        ParserContext context = new ParserContext(netIndex.net);
        context.templateVarsBinding = varBindings;
        context.bindingContext = context;
        
        for (Edge edge : netIndex.net.edges) {
            if (edge instanceof GspnEdge) {
                GspnEdge ge = (GspnEdge)edge;
                int p, t, card;
                
                // Ignore inhibitor edges, but signal their presence.
                if (ge.getEdgeKind() == GspnEdge.Kind.INHIBITOR) {
                    hasInhibitorEdges = true;
                    continue;
                }
                // Ignore flow edges, but signal their presence.
                if (ge.isContinuous() || ge.isFiringFlow()) {
                    hasContinuousEdges = true;
                    continue;
                }
                // Ignore colored edges
                if (!ge.getColorDomainOfConnectedPlace().isNeutralDomain()) {
                    hasColoredPlaces = true;
                    continue;
                }
                switch (ge.getEdgeKind()) {
                    case INPUT:
//                        System.out.println("head="+ge.getHeadNode().getUniqueName()+" tail="+ge.getTailNode().getUniqueName());
                        p = netIndex.place2index.get((Place)ge.getTailNode());
                        t = netIndex.trn2index.get((Transition)ge.getHeadNode());
                        try {
                            card = -ge.evaluateMultiplicity(context, null, null).getScalarInt();
                        }
                        catch (Exception e) {
                            hasMarkDepEdges = true;
                            card = 1;
                        }
                        break;
                        
                    case OUTPUT:
                        p = netIndex.place2index.get((Place)ge.getHeadNode());
                        t = netIndex.trn2index.get((Transition)ge.getTailNode());
                        try {
                            card = ge.evaluateMultiplicity(context, null, null).getScalarInt();
                        }
                        catch (Exception e) {
                            hasMarkDepEdges = true;
                            card = 1;
                        }
                        break;
                        
                    default:
                        throw new IllegalStateException();
                }
                if (card == 0)
                    hasEdgeWithZeroCard = true;
                if (type.isPlace())
                    addFlow(p, t, card);
                else
                    addFlow(t, p, card);
            }            
        }
        
        if (type.isPlace()) {
            // Prepare also the initial place quantities (for bound computation)
            for (Place plc : netIndex.places) {
                int m0;
                try {
                    m0 = plc.evaluateInitMarking(context).getScalarInt();
                }
                catch (Exception e) {
                    hasNonIntegerInitMarks = true;
                    m0 = 0;
                }
                setInitQuantity(netIndex.place2index.get(plc), m0);
            }
        }
        
    }
    
    public String flowToString(int i, SemiFlows.Type type, NetIndex netIndex) {
        int[] flow = getSemiflow(i);
        
        StringBuilder repr = new StringBuilder();
        for (int k=0; k<flow.length; k++) {
            if (flow[k] == 0)
                continue;
            if (repr.length() > 0)
                repr.append(" ");
            if (flow[k] > 0) {
                if (flow[k] != 1)
                    repr.append(flow[k]).append("*");
            }
            else {
                if (flow[k] == -1)
                    repr.append("-");
                else
                    repr.append(flow[k]).append("*");                
            }
            if (type.isPlace())
                repr.append(netIndex.places.get(k).getUniqueName());
            else
                repr.append(netIndex.transitions.get(k).getUniqueName());
        }
        return repr.toString();
    }

//    public static void main(String[] args) {
//        int NP = 14, MT = 10;
//        MartinezSilvaAlgorithm msa = new MartinezSilvaAlgorithm(NP, MT);
//        int[][] flow = {
//            {5, 4}, {1, 3}, // t1
//            {14, 7}, {6, 8}, // t2
//            {9}, {10, 11}, // t3
//            {2, 13}, {4}, // t4
//            {1}, {2}, // t5
//            {3}, {14}, // t6
//            {6}, {5}, // t7
//            {8}, {9}, // t8
//            {10, 12}, {7}, // t9
//            {11}, {12, 13} // t10
//        };
//        for (int t = 0; t < MT; t++) {
//            int[] in = flow[2 * t], out = flow[2 * t + 1];
//            for (int p = 0; p < in.length; p++) {
//                msa.addFlow(in[p] - 1, t, -1);
//            }
//            for (int p = 0; p < out.length; p++) {
//                msa.addFlow(out[p] - 1, t, +1);
//            }
//        }
//        msa.compute(true);
//    }
    
    public static MartinezSilvaAlgorithm init1() {
        int M=10, N=14;
        MartinezSilvaAlgorithm msa = new MartinezSilvaAlgorithm(N, M);
        msa.addFlow(0, 0, 1);
        msa.addFlow(0, 4, -1);
        msa.addFlow(1, 4, 1);
        msa.addFlow(1, 3, -1);
        msa.addFlow(12, 3, -1);
        msa.addFlow(3, 3, 1);
        msa.addFlow(3, 0, -1);
        msa.addFlow(2, 0, 1);
        msa.addFlow(4, 0, -1);
        msa.addFlow(2, 5, -1);
        msa.addFlow(13, 5, 1);
        msa.addFlow(13, 1, -1);
        msa.addFlow(5, 1, 1);
        msa.addFlow(5, 6, -1);
        msa.addFlow(4, 6, 1);
        msa.addFlow(6, 1, -1);
        msa.addFlow(7, 1, 1);
        msa.addFlow(7, 7, -1);
        msa.addFlow(8, 7, 1);
        msa.addFlow(8, 2, -1);
        msa.addFlow(9, 2, 1);
        msa.addFlow(10, 2, 1);
        msa.addFlow(10, 9, -1);
        msa.addFlow(11, 9, 1);
        msa.addFlow(12, 9, 1);
        msa.addFlow(11, 8, -1);
        msa.addFlow(9, 8, -1);
        msa.addFlow(6, 8, 1);
        return msa;
    }
    

    public static void main(String[] args) throws InterruptedException {
        MartinezSilvaAlgorithm msa = init1();
        ProgressObserver obs = new ProgressObserver() {
            @Override public void advance(int step, int total, int s, int t) { }
        };
        msa.onlySemiflows = false;
        msa.buildBasis = true;
        msa.compute(true, obs);
    }
}
