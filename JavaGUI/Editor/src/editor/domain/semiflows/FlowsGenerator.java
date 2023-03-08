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
import java.util.Comparator;
import java.util.Iterator;
import java.util.Set;
import java.util.TreeSet;

/** Farkas algorithm for the computation
 *  of the minimal set of P(T)-(semi)flows in a Petri net.
 *
 * @author elvio
 */
public class FlowsGenerator extends StructuralAlgorithm {
    
    // Iteration matrix [ D(i) | A(i) ], where D(0)=I and A(0) = Flow matrix
    // At the beginning, K=N. After the computation, K is the number of flows.
    private ArrayList<Row> initMat; // [D|A] = [E|L]
    Set<Row> rows;
    private ArrayList<Row> flowsMat; // Results
    
    // This extra informations keep the initial place marking, when
    // the method is used for P-invariants. It allows to derive the place bounds
    // from the P-invariants
    public int[] initQuantity; // array of N elements
    public int[] lowerBnd, upperBnd; // place bounds
    
    // Will compute semiflows or integer flows
    public final PTFlows.Type type;
    

    // For P-flows: N=|P|, M=|T| (for T-flows: N=|T|, M=|P|)
    // If supplementary variables are added, N0 keeps the value of N(|P| or |T|) 
    // while N will count the basic variables plus the supplementary ones.
    public FlowsGenerator(int N, int N0, int M, PTFlows.Type type) {
        super(N, N0, M);
        this.type = type;
//        System.out.println("M="+M+", N="+N);
        initMat = new ArrayList<>();
        for (int i = 0; i < N; i++) {
            Row row = new Row(N, M, false);
            row.e[i] = 1;
            initMat.add(row);
        }
    }
    
    // Initialize from an N*M matrix
    public FlowsGenerator(int[][] mat, PTFlows.Type type) {
        this(mat.length, mat.length, mat[0].length, type);
        for (int i=0; i<mat.length; i++) {
            for (int j=0; j<mat[i].length; j++)
                setIncidence(i, j, mat[i][j]);
        }
    }

    // Add an element to the incidence matrix from i to j with the specified cardinality
    public void addIncidence(int i, int j, int value) {
        initMat.get(i).l[j] += value;
    }
    // Set an element to the incidence matrix from i to j with the specified cardinality
    public final void setIncidence(int i, int j, int value) {
        initMat.get(i).l[j] = value;
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
        return flowsMat.size(); 
    }

    public int[] getFlowVector(int i) {
        return flowsMat.get(i).e;
    }
    
    // only for generation of all non-minimal flows
    public boolean isFlow(int i) {
        return flowsMat.get(i).is_l_zero();
    }
    
    // get a compact matrix of all real flows
    public int[][] getAnnulers() {
        int num=0, i=0;
        for (int f=0; f<numFlows(); f++)
            if (isFlow(f))
                num++;
        int[][] annulers = new int[num][];
        for (int f=0; f<numFlows(); f++)
            if (isFlow(f))
                annulers[i++] = getFlowVector(f);
        return annulers;
    }
    
    private static int sign(int num) {
        if (num > 0)
            return +1;
        else if (num < 0)
            return -1;
        return 0;
    }

    @Override
    public void compute(boolean log, ProgressObserver obs) throws InterruptedException, TooManyRowsException {
        rows = new TreeSet<>(Row.LEX_COMPARATOR);
        rows.addAll(initMat);
        initMat.clear();
        initMat = null;
        
        if (log)
            System.out.println(this);
        // Matrix A starts with the flow matrix, D is the identity.
        // for every transition i=[0,M), repeat:
        int columnCounter = 0;
        for (int i=nextPivot(); i>=0; i=nextPivot()) {
            Set<Row> newRows = new TreeSet<>(Row.LEX_COMPARATOR);
            obs.advance(columnCounter++, M+1, 0, 0);
//            System.out.println("Step: "+columnCounter+"/"+(M)+"  rows="+rows.size());
//            System.out.println("pivot: "+i+"/"+M);
            if (log)
                System.out.println("\nStep "+i+"/"+(M-1)+"\n"+this);
            // Append to the matrix [D|A] every rows resulting as a non-negative
            // linear combination of row pairs from [D|A] whose sum zeroes
            // the i-th column of A.
            int combined_with_i = 0;
            for (Row row1 : rows) {
                if (row1.l[i] == 0) {
                    continue;
                }
                for (Row row2 : rows) {
                    checkInterrupted();
                    if (row1 == row2)
                        break; // reached the half-visit
                    // Find two rows r1 and r2 such that r1[i] and r2[i] have opposite signs.
                    if (row2.l[i] == 0)
                        continue;
                    
                    if (type.isSemiflow()) { // (non-negative) semiflows
                        if (sign(row1.l[i]) == sign(row2.l[i]))
                            continue;
                    }
                    int mult1 = Math.abs(row1.l[i]);
                    int mult2 = Math.abs(row2.l[i]);
                    int gcd12 = Row.scalarGCD(mult1, mult2);
                    mult1 /= gcd12;
                    mult2 /= gcd12;
                    if (type.isFlow()) {
                        if (sign(row1.l[i]) == sign(row2.l[i]))
                            mult1 *= -1;
                    }

                    // Create a new row nr' such that:
                    //   nr = |r2[i]| * r1 + |ri[i]| * r2
                    //   nr' = nr / gcd(nr)
                    Row newRow = new Row(mult2, row1, mult1, row2);
                    newRow.canonicalize();
                    if (newRow.is_e_zero())
                        continue; // can this really happen??

                    if (log)
                        System.out.println(i + ": " + mult2 + "*" + row1 + " + " + mult1 + "*" + row2 + " = " + newRow);

                    newRows.add(newRow);
                    if (maxRows>0 && rows.size() + newRows.size() > maxRows)
                        throw new TooManyRowsException(); // suspend computation
                    ++combined_with_i;
                }
                    
                if (type.isBasis() && combined_with_i>0)
                    break;
            }
            checkInterrupted();

            // Eliminate from [D|A] the rows in which the i-th column of A is not zero.
            Iterator<Row> iter = rows.iterator();
            while (iter.hasNext()) {
                Row checked = iter.next();
//                obs.advance(i, M+1, rr, nRows);
                if (checked.l[i] != 0) {
                    if (log)
                        System.out.println(i + ": DEL " + checked);
                    iter.remove();
                }
            }
            
            rows.addAll(newRows);
            
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
        }
        
        flowsMat = new ArrayList<>(rows);
        rows.clear();
        rows = null;
        
        setComputed();
    }
    
    // Test all flows exaustively to check if they are minimal
    // To do so, we take every pair of flows and we test if one is a
    // linear component of another. If it is so, subtract the component flow
    // from the other. When a flow becomes zero, it is removed from A|D.
    private void removeNonMinimalFlows(boolean log, ProgressObserver obs) 
            throws InterruptedException 
    {
        FastSupportTestTable fstt = new FastSupportTestTable(N);
        for (Row row : rows) {
            if (null == fstt.smallerSupport(row))
                fstt.insertRow(row);
        }
        
        Iterator<Row> iter = rows.iterator();
        while (iter.hasNext()) {
            Row row = iter.next();
            Row smallerRow = fstt.smallerSupport(row);
            if (smallerRow != null) {
                if (log)
                    System.out.println("   DEL " + row + " by " + smallerRow);
                iter.remove();
            }
        }
    }
    
    //-----------------------------------------------------------------------

    private int nextPivot() {
        int[] pos = new int[M], neg = new int[M];
        for (Row row : rows) {
            for (int j=0; j<M; j++) {
                if (row.l[j]>0)
                    pos[j]++;
                else if  (row.l[j]<0)
                    neg[j]++;
            }
        }
        int nextCol = -1;
        int nextOps = -1;
        for (int j=0; j<M; j++) {
//            System.out.println("   j="+j+"  pos="+pos[j]+" neg="+neg[j]);
            if (pos[j]>0 || neg[j]>0) { // still something to do on col j
                int ops = pos[j] * neg[j];
                if (nextCol==-1 || ops < nextOps) {
                    nextOps = ops;
                    nextCol = j;
                }
            }
        }
        return nextCol;
    }
    
    //-----------------------------------------------------------------------
    
    // Reduce all remaining flows after removing all supplementary variables
    // Supplementary variable are located in D in the positions between N0 and N.
    // Therefore reduce N to N0 (by truncating all rows), and then remove again all
    // the non-minimal flows.
    private void dropSupplementaryVariablesAndReduce(boolean log, ProgressObserver obs) 
            throws InterruptedException 
    {
        // truncate and remove all supplementary variables in D
        Iterator<Row> iter = rows.iterator();
        while (iter.hasNext()) {
            Row row = iter.next();
            row.truncate_e(N0);
            if (row.is_e_zero()) {
                iter.remove();
            }
        }
        reduceN(N0);
        // reduce the flows that are not minimal after removing the suppl. vars.
        removeNonMinimalFlows(log, obs);
    }
    
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        if (initMat != null) {
            for (int i = 0; i < initMat.size(); i++) {
                sb.append(String.format("%2d: ", i));
                sb.append(initMat.get(i));
                sb.append("\n");
            }
        }
        if (rows != null) {
            for (Row row : rows) {
                sb.append(row);
                sb.append("\n");
            }
        }
        if (flowsMat != null) {
            for (int i = 0; i < numFlows(); i++) {
                sb.append(String.format("%2d: ", i));
                sb.append(flowsMat.get(i));
                sb.append("\n");
            }
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
        String[] symbols = type.getLatexFlowName();
        
        // header
        sb.append("$\\begin{array}{r");
        for (int f=0; f<numFlows(); f++)
            sb.append("|c");
        if (type.isPlace())
            sb.append("|r");
        sb.append("}\n ");
        for (int f=0; f<numFlows(); f++)
            sb.append("& ").append(symbols[0]).append("_{").append(f+1).append("}");
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
            sb.append("\n \\mathbf{m}_0 \\cdot ").append(symbols[1]).append(" & ");
            
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
//                        if (mA.get(p)[jj] == 0)
                        if (initMat.get(p).l[jj] == 0)
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
            Color htmlForegroundColor, Color htmlBackgroundColor, Node selNode,
            int activeIndex) 
    {
        StringBuilder repr = new StringBuilder();

        int selK = -1;
        if (selNode instanceof Place)
            selK = netIndex.place2index.get((Place)selNode);
        else if (selNode instanceof Transition)
            selK = netIndex.trn2index.get((Transition)selNode);
        int[] flow = getFlowVector(i);
        
        boolean selected = (selK == -1) || (flow[selK] != 0);
        boolean active = (i == activeIndex);
//        System.out.println("selected="+selected+" selK="+selK+" activeIndex="+activeIndex+
//                " htmlForegroundColor="+htmlForegroundColor+
//                " htmlBackgroundColor="+htmlBackgroundColor);

        boolean useHtml = (htmlForegroundColor != null);
        String negClrHex = null, eqClrHex = null, posClrHex = null;
        if (useHtml) {
            Color posClr;
            if (active)
                posClr =  selected ? htmlBackgroundColor : 
                          (Util.mix(htmlBackgroundColor, htmlForegroundColor, 0.5f));
            else
                posClr =  selected ? htmlForegroundColor : 
                          (Util.mix(htmlForegroundColor, htmlBackgroundColor, 0.5f));
                           
            Color negClr = Util.mix(posClr, Color.MAGENTA, 0.5f);
            Color eqClr = Util.mix(posClr, Color.CYAN, 0.5f);
            posClrHex = Util.clrToHex(posClr);
            negClrHex = Util.clrToHex(negClr);
            eqClrHex = Util.clrToHex(eqClr);
            repr.append("<html>");
        }
        
        for (int k=0; k<flow.length; k++) {
            if (flow[k] == 0)
                continue;
            if (repr.length() > 0)
                repr.append(" ");
            if (useHtml) {
                repr.append("<font color='").append(flow[k] > 0 ? posClrHex : negClrHex).append("'>");
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
        
        String equationSign = type.getInvariantSign();
        if (asInvariant && equationSign!=null && initQuantity!=null) {
            // Compute the invariant quantity
            int q = 0;
            for (int k=0; k<flow.length; k++)
                q += flow[k] * initQuantity[k];
            
            if (useHtml)
                repr.append("<font color='").append(eqClrHex).append("'>");
            repr.append(" ").append(equationSign).append(" ").append(q);
            if (useHtml)
                repr.append("</font>");
        }
        
        return repr.toString();
    }

    

    
    private static void printMat(int[][] mat) {
        for (int[] row : mat) {
            for (int j=0; j<row.length; j++) {
                System.out.print((j==0 ? "{" : ", ")+row[j]);
            }
            System.out.println("},");
        }
    }
    

    public static void main(String[] args) throws InterruptedException, TooManyRowsException {
        long start = System.nanoTime();
        testProblem(FlowsGeneratorTestData.prob1);
        testProblem(FlowsGeneratorTestData.probAnisimov);
        testProblem(FlowsGeneratorTestData.probCSP);
        testProblem(FlowsGeneratorTestData.probComp);
        testProblem(FlowsGeneratorTestData.probSiphonBasis);
        testProblem(FlowsGeneratorTestData.probM33N);
        testProblem(FlowsGeneratorTestData.probM33I);
        testProblem(FlowsGeneratorTestData.probM44N);
        testProblem(FlowsGeneratorTestData.probM44I);
//        FlowsGeneratorTestData.FlowProblem fp = new FlowsGeneratorTestData.FlowProblem(myMat, null, PTFlows.Type.PLACE_FLOWS);
//        testProblem(fp);
        System.out.println("Total Time: " + (System.nanoTime() - start)/1e9);
    }
    
    private static void testProblem(FlowsGeneratorTestData.FlowProblem problem) throws InterruptedException, TooManyRowsException {
        boolean verbose = false;
        ProgressObserver obs = (int step, int total, int s, int t) -> { };
        FlowsGenerator fg = new FlowsGenerator(problem.input, problem.probType);
        if (problem.probType.isTrapsOrSiphons())
            fg.N0 -= problem.input[0].length;
//        printMat(fg.mA);
        fg.compute(verbose, obs);
//        printMat(fg.mD);
        
        // Compare the result matrix with the known results for that problem
        if (problem.solution != null) {
            int[][] mResult = fg.getAnnulers();
            Comparator<int[]> comp = (int[] o1, int[] o2) -> Arrays.compare(o1, o2);
            Arrays.sort(mResult, comp);
            Arrays.sort(problem.solution, comp);
            
            boolean equal = (mResult.length == problem.solution.length);
            if (equal) {
                for (int i=0; equal && i<mResult.length; i++) {
                    equal = Arrays.equals(mResult[i], problem.solution[i]);
                }
            }
            if (verbose || !equal)  {
                System.out.println("mResult");
                printMat(mResult);
                System.out.println("problem.solution");
                printMat(problem.solution);
                Thread.sleep(100);
            }

            System.out.println("Check solution: "+equal);
            if (!equal)
                throw new IllegalStateException();
        }
    }
    
//    private static int[][] myMat = {
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0 },
//{  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0 },
//{  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1 },
//{  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{ -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0, -1 },
//{ -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
//{  1,  1,  1,  1,  1,  1,  1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
//{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
//
//    };
}
