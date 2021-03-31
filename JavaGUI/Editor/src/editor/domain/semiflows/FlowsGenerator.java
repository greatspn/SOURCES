/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.semiflows;

import common.Util;
import editor.domain.Edge;
import editor.domain.Node;
import editor.domain.elements.GspnEdge;
import editor.domain.elements.Place;
import editor.domain.elements.Transition;
import editor.domain.grammar.ParserContext;
import editor.domain.grammar.TemplateBinding;
import java.awt.Color;
import java.util.ArrayList;
import java.util.Arrays;

/** Farkas algorithm for the computation
 *  of the minimal set of P(T)-(semi)flows in a Petri net.
 *
 * @author elvio
 */
public class FlowsGenerator extends StructuralAlgorithm {
    
    // Iteration matrix [ D(i) | A(i) ], where D(0)=I and A(0) = Flow matrix
    // At the beginning, K=N. After the computation, K is the number of flows.
    public ArrayList<int[]> mD;     // KxN matrix
    public ArrayList<int[]> mA;     // KxM matrix
    
    // This extra informations keep the initial place marking, when
    // the method is used for P-invariants. It allows to derive the place bounds
    // from the P-invariants
    public int[] initQuantity; // array of N elements
    public int[] lowerBnd, upperBnd; // place bounds
    
    // Will compute semiflows or integer flows
    public final PTFlows.Type type;
    
//    SilvaColom88Algorithm scAlgo;
    

    // For P-flows: N=|P|, M=|T| (for T-flows: N=|T|, M=|P|)
    // If supplementary variables are added, N0 keeps the value of N(|P| or |T|) 
    // while N will count the basic variables plus the supplementary ones.
    public FlowsGenerator(int N, int N0, int M, PTFlows.Type type) {
        super(N, N0, M);
        this.type = type;
//        System.out.println("M="+M+", N="+N);
        mD = new ArrayList<>();
        mA = new ArrayList<>();
        for (int i = 0; i < N; i++) {
            mD.add(new int[N]);
            mA.add(new int[M]);
            mD.get(i)[i] = 1;
        }
//        scAlgo = new SilvaColom88Algorithm(N, M);
    }

    // Add an element to the incidence matrix from i to j with the specified cardinality
    public void addIncidence(int i, int j, int card) {
        mA.get(i)[j] += card;
    }
    // Set an element to the incidence matrix from i to j with the specified cardinality
    public void setIncidence(int i, int j, int value) {
        mA.get(i)[j] = value;
    }
    
    // Add the initial token of a place (only for P-invariant computation)
    public void setInitQuantity(int i, int quantity) {
        assert i < N && i >= 0;
        if (initQuantity == null) { // Not initialized yet
            initQuantity = new int[N];
//            computeBounds = true;
        }
        assert initQuantity[i] == 0;
        initQuantity[i] = quantity;
        
//        scAlgo.setInitQuantity(i, quantity);
    }
    
    public int numFlows() {
        return mD.size();
    }

    public int[] getFlowVector(int i) {
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
            int nRows = numFlows();
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
                    if (type.isSemiflow()) { // (non-negative) semiflows
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
                    int[] nrA = new int[M];
                    int[] nrD = new int[N];
                    int gcdAD = -1;
                    for (int k = 0; k < M; k++) {
                        // Compute (with arithmetic overflow check):
                        //   nrA[k] = mult2 * mA.get(r1)[k] + mult1 * mA.get(r2)[k];
                        nrA[k] = Math.addExact(Math.multiplyExact(mult2, mA.get(r1)[k]),
                                               Math.multiplyExact(mult1, mA.get(r2)[k]));
                        gcdAD = (k == 0) ? Math.abs(nrA[k]) : gcd(gcdAD, Math.abs(nrA[k]));
                    }
                    assert nrA[i] == 0;
                    for (int k = 0; k < N; k++) {
                        // Compute (with arithmetic overflow check):
                        //   nrD[k] = mult2 * mD.get(r1)[k] + mult1 * mD.get(r2)[k];
                        nrD[k] = Math.addExact(Math.multiplyExact(mult2, mD.get(r1)[k]),
                                               Math.multiplyExact(mult1, mD.get(r2)[k]));
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
                    int nnzD = 0;
                    for (int k = 0; k < N; k++) {
                        nnzD += (nrD[k] != 0 ? 1 : 0);
                    }                    
                    if (nnzD == 0)
                        continue; // drop empty row

                    if (log)
                        System.out.println(i + ": ADD row " + r1 + " + row " + r2 + "  nnz(D)=" + nnzD);

                    mA.add(nrA);
                    mD.add(nrD);
                    ++combined_with_i;
                }
                    
                if (type.isBasis() && combined_with_i>0)
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
            }
            
            // Eliminate frm [D|A] the rows that are not minimal, doing an exhaustive search.
            if (!type.isBasis())
                removeNonMinimalFlows(log, obs);
        }
        
        if (type.isTrapsOrSiphons())
            dropSupplementaryVariablesAndReduce(log, obs);
        
        if (log)
            System.out.println("\nRESULT:\n"+this);

        obs.advance(M+1, M+1, 1, 1);
        
        if (type.isBound()) {
            computeBoundsFromInvariants();
//            scAlgo.compute(log, obs);
        }
        
        setComputed();
    }
    
    // Test all flows exaustively to check if they are minimal
    // To do so, we take every pair of flows and we test if one is a
    // linear component of another. If it is so, subtract the component flow
    // from the other. When a flow becomes zero, it is removed from A|D.
    private void removeNonMinimalFlows(boolean log, ProgressObserver obs) 
            throws InterruptedException 
    {
        int rr = numFlows();
        while (rr > 0) {
            rr--;
            obs.advance(M, M+1, numFlows()-rr, numFlows());
            for (int i=0; i<numFlows(); i++) {
                checkInterrupted();
                if (i == rr)
                    continue;
                // Check if support(D[i]) subseteq support(D[rr])
                boolean support_included = true;
                for (int k=0; k<N && support_included; k++) {
                    if (mD.get(i)[k] != 0) {
                        if (mD.get(rr)[k] == 0)
                            support_included = false;
                    } 
                }

                if (support_included) {
                    // rr was a linear combination of other flows. Remove it
                    if (log)
                        System.out.println("DEL row " + rr);
                    mA.remove(rr);
                    mD.remove(rr);
                    break;
                }
            }
        }        
    }
    
    // Reduce all remaining flows after removing all supplementary variables
    // Supplementary variable are located in D in the positions between N0 and N.
    // Therefore reduce N to N0 (by truncating all rows), and then remove again all
    // the non-minimal flows.
    private void dropSupplementaryVariablesAndReduce(boolean log, ProgressObserver obs) 
            throws InterruptedException 
    {
        // truncate and remove all supplementary variables in D
        for (int i=0; i<numFlows(); i++) {
            int[] newDi = new int[N0];
            System.arraycopy(mD.get(i), 0, newDi, 0, N0);
            mD.set(i, newDi);
        }
        reduceN(N0);
        // reduce the flows that are not minimal after removing the suppl. vars.
        removeNonMinimalFlows(log, obs);
    }
    
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < numFlows(); i++) {
            sb.append(i < 10 ? " " : "").append(i).append(":  ");
            for (int j = 0; j < N; j++) {
                sb.append(mD.get(i)[j] < 0 ? "" : " ").append(mD.get(i)[j]).append(" ");
            }
            sb.append("| ");
            for (int j = 0; j < M; j++) {
                sb.append(mA.get(i)[j] < 0 ? "" : " ").append(mA.get(i)[j]).append(" ");
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
        for (int i=0; i<numFlows(); i++) {
            int[] inv = getFlowVector(i);
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
        assert isComputed() && type.isBound() && initQuantity != null && p >= 0 && p < N;
        return upperBnd[p];
    }
    
    public int getLowerBoundOf(int p) {
        assert isComputed() && type.isBound() && initQuantity != null && p >= 0 && p < N;
        return lowerBnd[p];
    }
    
    
    
    
    public String toLatexString(NetIndex netIndex, boolean showZeros) {
        StringBuilder sb = new StringBuilder();
        
        // header
        sb.append("$\\begin{array}{r");
        for (int f=0; f<numFlows(); f++)
            sb.append("|c");
        if (type.isPlace())
            sb.append("|r");
        sb.append("}\n ");
        for (int f=0; f<numFlows(); f++)
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
            
            for (int f=0; f<numFlows(); f++) {
                int[] flow = getFlowVector(f);
                
                sb.append(" &");
                if (showZeros || flow[row]!=0) {
                    String color;
                    if (flow[row] > 0)
                        color = "Blue";
                    else if (flow[row] < 0)
                        color = "Mahogany";
                    else
                        color = "Gray";
                    sb.append("\\textcolor{").append(color).append("}{").append(flow[row]).append("}");
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
            
            for (int f=0; f<numFlows(); f++) {
                int[] flow = getFlowVector(f);
                int sum = 0;
                for (int j=0; j<flow.length; j++) {
                    int initMark = initQuantity[j];
                    sum += initMark * flow[j];
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
    
    // Support structures for building traps/siphons
    private int[] trIndexStart, trIndexEnd;

    
    // Compute the initial sizes of the N,M
    public static FlowsGenerator makeFor(PTFlows.Type type, NetIndex netIndex) {
        int N, N0, M;
        int[] trIndexStart = null, trIndexEnd = null;
        
        if (type.isTrapsOrSiphons()) {
            N = N0 = netIndex.numPlaces();
            M = 0;
            // for traps/siphons, the incidence matrix is modified:
            // each IA/OA arc generates a duplicate transition with an
            // associated supplementary variable.
            GspnEdge.Kind primaryKind = (type==PTFlows.Type.TRAPS ? GspnEdge.Kind.INPUT : GspnEdge.Kind.OUTPUT);
            trIndexStart = new int[netIndex.transitions.size()];
            trIndexEnd = new int[netIndex.transitions.size()];
            int jj = 0;
            for (int tt=0; tt<netIndex.transitions.size(); tt++) {
                Transition trn = netIndex.transitions.get(tt);
                trIndexStart[tt] = jj;
                for (Edge ee : netIndex.net.edges) {
                    if (ee instanceof GspnEdge) {
                        GspnEdge e = (GspnEdge)ee;
                        if (e.getConnectedTransition() == trn && e.getEdgeKind() == primaryKind) {
                            N++; // add a supplementary variable
                            M++; // add a new transition replica
                            jj++; // increment the replication index
                        }
                    }
                }
                trIndexEnd[tt] = jj;
            }
        }
        else if (type.isPlace()) {
            N = N0 = netIndex.numPlaces();
            M = netIndex.numTransition();
        }
        else {
            N = N0 = netIndex.numTransition();
            M = netIndex.numPlaces();
        }
        FlowsGenerator fg = new FlowsGenerator(N, N0, M, type);
        fg.trIndexStart = trIndexStart;
        fg.trIndexEnd = trIndexEnd;
        return fg;
    }

    // Initialize the matrices from a gspn page
    public void initialize(PTFlows.Type type, TemplateBinding varBindings, NetIndex netIndex) {
        ParserContext context = new ParserContext(netIndex.net);
        context.templateVarsBinding = varBindings;
        context.bindingContext = context;
        
        int[] trSecondaryIndex = null;
        if (type.isTrapsOrSiphons())
            trSecondaryIndex = new int[netIndex.numTransition()];
        
        for (Edge edge : netIndex.net.edges) {
            if (edge instanceof GspnEdge) {
                GspnEdge ge = (GspnEdge)edge;
                int p, t, card, sign;
                
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
                        p = netIndex.place2index.get(ge.getConnectedPlace());
                        t = netIndex.trn2index.get(ge.getConnectedTransition());
                        sign = -1;
                        try {
                            card = ge.evaluateMultiplicity(context, null, null).getScalarInt();
                        }
                        catch (Exception e) {
                            hasMarkDepEdges = true;
                            card = 1;
                        }
                        break;
                        
                    case OUTPUT:
                        p = netIndex.place2index.get(ge.getConnectedPlace());
                        t = netIndex.trn2index.get(ge.getConnectedTransition());
                        sign = 1;
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
                
                if (!type.isTrapsOrSiphons()) {
                    if (type.isPlace())
                        addIncidence(p, t, card * sign);
                    else
                        addIncidence(t, p, card * sign);
                }
                else { // traps/siphons construction
                    GspnEdge.Kind primaryKind = (type==PTFlows.Type.TRAPS ? GspnEdge.Kind.INPUT : GspnEdge.Kind.OUTPUT);
                    int tt = netIndex.trn2index.get(ge.getConnectedTransition());
                    if (ge.getEdgeKind() == primaryKind) {
                        // on the primary edge kind (Input for traps, Output for siphons)
                        // put a +1 on the jj-th transition column replica
                        int jj = trIndexStart[tt] + trSecondaryIndex[tt];
                        assert jj < trIndexEnd[tt];
                        if (mA.get(p)[jj] == 0)
                            setIncidence(p, jj, 1);
                        ++trSecondaryIndex[tt]; // increment replica counter
                    }
                    else {
                        // set the value of the secondary edge kind on all replicas of transition tt
                        for (int jj=trIndexStart[tt]; jj<trIndexEnd[tt]; jj++)
                            setIncidence(p, jj, -1);
                    }
                }
            }            
        }
        trIndexStart = null;
        trIndexEnd = null;
        
        if (type.isTrapsOrSiphons()) {
            // Add the supplementary variables, i.e. the identity matrix 
            // below the incidence. A = [C] over [I]
            for (int i=0; i<M; i++)
                setIncidence(i + N0, i, 1);
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
    
    public String flowToString(int i, NetIndex netIndex, boolean asInvariant, 
            Color htmlForegroundColor, Color htmlBackgroundColor, Node selNode) 
    {
        StringBuilder repr = new StringBuilder();

        int selK = -1;
        if (selNode instanceof Place)
            selK = netIndex.place2index.get((Place)selNode);
        else if (selNode instanceof Transition)
            selK = netIndex.trn2index.get((Transition)selNode);
        int[] flow = getFlowVector(i);
        
        boolean selected = (selK == -1) || (flow[selK] != 0);


        boolean useHtml = (htmlForegroundColor != null);
        String negClrHex = null, eqClrHex = null, foreClrHex = null;
        if (useHtml) {
            Color foreClr = selected ? htmlForegroundColor : Util.mix(htmlForegroundColor, htmlBackgroundColor, 0.5f);
            Color negClr = Util.mix(foreClr, Color.MAGENTA, 0.5f);
            Color eqClr = Util.mix(foreClr, Color.CYAN, 0.5f);
            foreClrHex = String.format("#%06x", foreClr.getRGB() & 0xFFFFFF);
            negClrHex = String.format("#%06x", negClr.getRGB() & 0xFFFFFF);
            eqClrHex = String.format("#%06x", eqClr.getRGB() & 0xFFFFFF);
            repr.append("<html>");
        }
        
        for (int k=0; k<flow.length; k++) {
            if (flow[k] == 0)
                continue;
            if (repr.length() > 0)
                repr.append(" ");
            if (useHtml) {
                repr.append("<font color='").append(flow[k] > 0 ? foreClrHex : negClrHex).append("'>");
                if (k == selK)
                    repr.append("<b>");
            }
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
            if (useHtml) {
                if (k == selK)
                    repr.append("</b>");
                repr.append("</font>");
            }
        }
        
        String invSign = type.getInvariantSign();
        if (asInvariant && invSign!=null && initQuantity!=null) {
            // Compute the invariant quantity
            int q = 0;
            for (int k=0; k<flow.length; k++)
                q += flow[k] * initQuantity[k];
            
            if (useHtml)
                repr.append("<font color='").append(eqClrHex).append("'>");
            repr.append(" ").append(invSign).append(" ").append(q);
            if (useHtml)
                repr.append("</font>");
        }
        
        return repr.toString();
    }

//    public static void main(String[] args) {
//        int NP = 14, MT = 10;
//        FlowsGenerator msa = new FlowsGenerator(NP, MT);
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
    
    public static FlowsGenerator init1() {
        int M=10, N=14;
        FlowsGenerator fg = new FlowsGenerator(N, N, M, PTFlows.Type.PLACE_SEMIFLOWS);
        fg.addIncidence(0, 0, 1);
        fg.addIncidence(0, 4, -1);
        fg.addIncidence(1, 4, 1);
        fg.addIncidence(1, 3, -1);
        fg.addIncidence(12, 3, -1);
        fg.addIncidence(3, 3, 1);
        fg.addIncidence(3, 0, -1);
        fg.addIncidence(2, 0, 1);
        fg.addIncidence(4, 0, -1);
        fg.addIncidence(2, 5, -1);
        fg.addIncidence(13, 5, 1);
        fg.addIncidence(13, 1, -1);
        fg.addIncidence(5, 1, 1);
        fg.addIncidence(5, 6, -1);
        fg.addIncidence(4, 6, 1);
        fg.addIncidence(6, 1, -1);
        fg.addIncidence(7, 1, 1);
        fg.addIncidence(7, 7, -1);
        fg.addIncidence(8, 7, 1);
        fg.addIncidence(8, 2, -1);
        fg.addIncidence(9, 2, 1);
        fg.addIncidence(10, 2, 1);
        fg.addIncidence(10, 9, -1);
        fg.addIncidence(11, 9, 1);
        fg.addIncidence(12, 9, 1);
        fg.addIncidence(11, 8, -1);
        fg.addIncidence(9, 8, -1);
        fg.addIncidence(6, 8, 1);
        return fg;
    }
    

    public static void main(String[] args) throws InterruptedException {
        FlowsGenerator fg = init1();
        ProgressObserver obs = (int step, int total, int s, int t) -> { };
        fg.compute(true, obs);
    }
}
