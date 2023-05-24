/*
 * Implementation of the Hilbert basis computation
 */
package editor.domain.semiflows;
import static editor.domain.semiflows.Row.DEGLEX_COMPARATOR;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Iterator;
import java.util.Random;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeSet;

/**
 *
 * @author elvio
 */
public class HilbertBasis {  
    private ArrayList<Row> rows;
    private final int N, M;
    private boolean verbose = false;
    private boolean keepCpCm = false; // Keep the C+ and C- monoid bases
    
    //-----------------------------------------------------------------------
    // Empty constructor: Initialize [identity(N*N) | zero(N*M)]
    public HilbertBasis(int N, int M) {
        this.N = N;
        this.M = M;
        rows = new ArrayList<>();
        for (int n=0; n<N; n++) {
            Row row = new Row(N, M, true);
            row.e[n] = 1;
            rows.add(row);
        }
    }
    
    // Initialize from an N*M matrix
    public HilbertBasis(int[][] mat) {
        this(mat.length, mat[0].length);
        for (int i=0; i<mat.length; i++) {
            System.arraycopy(mat[i], 0, rows.get(i).l, 0, mat[i].length);
        }
    }

    //-----------------------------------------------------------------------
    public void setKeepCpCm() {
        keepCpCm = true;
    }
    public void setVerbose() {
        verbose = true;
    }
    public void addToL(int i, int j, int val) {
        rows.get(i).l[j] += val;
    }
    public int numRows() { 
        return rows.size(); 
    }
    public int[] getBasisVec(int i) {
        return rows.get(i).e;
    }
    public boolean isHilbertBasisVec(int i) {
        return rows.get(i).is_l_zero();
    }
    public void removeInitialRows() {
        rows.removeIf(row -> row.initial);
    }
    
    //-----------------------------------------------------------------------

    private boolean partialOrderCmp(Row row1, Row row2, int jMax) {
//        for (int j=0; j<row1.l.length; j++) {
        for (int j=0; j<=jMax; j++) {
            if (!(row1.l[j] * row2.l[j] >= 0 && 
                  Math.abs(row1.l[j]) <= Math.abs(row2.l[j])))
                return false;
        }
        return row1.less_equal_e(row2);
        
//        return (row1.l[j] * row2.l[j] >= 0 && 
//                Math.abs(row1.l[j]) <= Math.abs(row2.l[j]) &&
//                row1.less_equal_e(row2));
    }
        
    //-----------------------------------------------------------------------
    // Generate all sums of C+/C- rows with a single row1, and append the new rows into G
    private void S_vectors(ArrayList<Row> C, Row row1, ArrayList<Row> G) {
        for (Row row2 : C) {
//            boolean doPairwiseSum = true;
//            for (int j=0; j<=jMax && doPairwiseSum; j++) {
//                if (row1.l[j] * row2.l[j] > 0) // same sign
//                    doPairwiseSum = false;
//            }
//            if (!doPairwiseSum)
//                continue;
            
            appendUnique(new Row(row1, row2), G);
        }
    }
    
    //-----------------------------------------------------------------------
    
    private Row isIrreducible(Row r, int j, ArrayList<Row> C0,
                              ArrayList<Row> Cp, ArrayList<Row> Cm) 
    {
        // C0 may reduce every row
        for (Row r2 : C0)
            if (partialOrderCmp(r2, r, j))
                return r2;
        // If the row has a non-negative l[j], in can be reduced by C+
        if (r.l[j] > 0)
            for (Row r2 : Cp)
                if (partialOrderCmp(r2, r, j))
                    return r2;
        // If the row has a non-positive l[j], in can be reduced by C-
        if (r.l[j] < 0)
            for (Row r2 : Cm)
                if (partialOrderCmp(r2, r, j))
                    return r2;
        // row r is irreducible w.r.t. C0/C+/C-
        return null;
    }
    
    //-----------------------------------------------------------------------
    private static boolean appendUnique(Row row, ArrayList<Row> rows) {
        for (Row row2 : rows) {
            if (row2.equal(row))
                return false; // Already exists
        }
        rows.add(row);
        return true;
    }
    
    //-----------------------------------------------------------------------
    private static void appendAllUnique(ArrayList<Row> toAppend, ArrayList<Row> rows) {
        for (Row r : toAppend) {
            appendUnique(r, rows);
//            if (appendUnique(r, rows)) {
//                System.out.println("APPEND: "+r);
//                System.exit(-1);
//            }
        }
    }

    //-----------------------------------------------------------------------
    private void HilbertFMcol(int j) {
        ArrayList<Row> C0 = new ArrayList<>(); // C0 = rows with c[j] == 0
        ArrayList<Row> Cp = new ArrayList<>(); // C+ = rows with c[j] > 0
        ArrayList<Row> Cm = new ArrayList<>(); // C- = rows with c[j] < 0

        ArrayList<Row> G = new ArrayList<>(rows);
        // modified Pottier algorithm
        while (!G.isEmpty()) {
            G.sort(Row.DEGLEX_COMPARATOR);
            Row el = G.get(0);
            G.remove(0);
            Row redux = isIrreducible(el, j, C0, Cp, Cm);
            if (redux == null) {
                if (el.l[j] == 0) {
                    appendUnique(el, C0);
                    if (verbose)
                        System.out.println("C0 <- C0 U "+el);
                }
                else if (el.l[j] > 0) {
                    appendUnique(el, Cp);
                    S_vectors(Cm, el, G); // G U ([e|l] * C-)
                    if (verbose)
                        System.out.println("C+ <- C+ U "+el);
                }
                else if (el.l[j] < 0) {
                    appendUnique(el, Cm);
                    S_vectors(Cp, el, G); // G U ([e|l] * C+)
                    if (verbose)
                        System.out.println("C- <- C- U "+el);
                }
            }
            else {
                if (verbose)
                    System.out.println("DROP       "+el+" by "+redux);
            }
        }
        ArrayList<Row> D = new ArrayList<>();
        D.addAll(C0);
        if (keepCpCm) {
            D.addAll(Cp);
            D.addAll(Cm);
        }
        int s1 = D.size();
        appendAllUnique(rows, D); // keep all rows that started the iteration
        int s2 = D.size();
        if (keepCpCm && s1!=s2) { 
            System.out.println("ERROR: D should contain all the initial rows!"); 
//            throw new IllegalStateException();
        }
        
        rows = D;
    }
    
    //-----------------------------------------------------------------------
    // Heuristic that chooses the next pivot of L from [E|L]
    private int nextPivot(boolean[] colReduced) {
//        int[] Lp = new int[M];
//        int[] Lm = new int[M];
//        for (Row el : rows) {
//            for (int j=0; j<M; j++) {
//                if (el.l[j] > 0)
//                    Lp[j] += el.l[j];
//                else
//                    Lm[j] += -el.l[j];
//            }
//        }
//        int pivot = -1, smallestValue = Integer.MAX_VALUE;
//        for (int j=0; j<M; j++) {
//            if (colReduced[j])
//                continue;
//            if (Lp[j]==0 && Lm[j]==0)
//                continue;
//            
//            int value = Lp[j] * Lm[j];
//            if (value < smallestValue) {
//                smallestValue = value;
//                pivot = j;
//            }
//        }
//        return pivot;
        for (int j=0; j<M; j++) {
            if (colReduced[j])
                continue;
            return j;
        }
        return -1;
    }
    
    //-----------------------------------------------------------------------
    public void HilbertFM() {
        boolean[] colReduced = new boolean[M];
        int j;
        while ((j = nextPivot(colReduced)) >= 0) {
            colReduced[j] = true;
            if (verbose)
                System.out.println("\nnext pivot: "+j);
            HilbertFMcol(j);
//            System.out.println(rows.size());
            if (verbose)
                System.out.print(this);
        }
    }
    
    //-----------------------------------------------------------------------
    public String toString() {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < rows.size(); i++) {
            sb.append(String.format("%2d: ", i+1));
            sb.append(rows.get(i));
            sb.append("\n");
        }
        return sb.toString();
    }
    
    
    
    //-----------------------------------------------------------------------
    // Generate all sums of C rows with a single row1, and append the new rows into G
    private void S_vectors_B(ArrayList<Row> C, Row row1, int j, Set<Row> G, FastPreceqTable fpt) {
        for (Row row2 : C) {
            // Can we compose row1 with row2?
            boolean compose = false;
            for (int k=j; k<=j; k++) {
//            for (int k=0; k<row1.l.length; k++) {
                if (!(row1.l[k] * row2.l[k] >= 0 && 
                      Math.abs(row1.l[k]) <= Math.abs(row2.l[k])))
                {
                    compose = true;
                    break;
                }
            }
                
            if (compose) {
                Row newRow = new Row(row1, row2);
                if (null==findReducer(newRow, j, fpt)) {
                    G.add(newRow);
                }
            }
        }
    }
    
    //-----------------------------------------------------------------------
    
    private static boolean checkUnique(Row row, ArrayList<Row> rows) {
        for (Row row2 : rows) {
            if (row2.equal(row))
                return false; // Already exists
        }
        return true; // is unique
    }
//    private Row isIrreducible(Row r, int j, ArrayList<Row> C) 
//    {
//        // C may reduce every row
//        for (Row r2 : C)
//            if (partialOrderCmp(r2, r, j))
//                return r2;
//        // row r is irreducible w.r.t. C
//        return null;
//    }
    
    private Row findReducer(Row r, int j, FastPreceqTable le) 
    {
        Row preceq = le.preceq(r);
        if (preceq != null) {
//            System.out.println("r =      "+r);
//            System.out.println("preceq = "+preceq);
            assert partialOrderCmp(preceq, r, j);
        }
        return preceq;
    }
    
    //-----------------------------------------------------------------------
    public void Buchberger(int j) {
        FastPreceqTable fpt = new FastPreceqTable(M);
        ArrayList<Row> C = new ArrayList<>(); 
        SortedSet<Row> G = new TreeSet<>(DEGLEX_COMPARATOR);
        G.addAll(rows);
        int step = 0;

        while (!G.isEmpty()) {
            Iterator<Row> it = G.iterator();
            Row el = it.next();
            it.remove();
            Row redux = findReducer(el, j, fpt);
            if (redux == null) { // [e|l] is irreducible
                if (verbose)
                    System.out.println("C <- C U "+el);
                S_vectors_B(C, el, j, G, fpt); // G U ([e|l] * C)
                assert checkUnique(el, C);
                C.add(el);
                fpt.insertRow(el);
            }
            else { // [e|l] is reducible by redux
                if (verbose)
                    System.out.println("DROP     "+el+" by "+redux);
            }
            
            step++;
//            if (step % 1000 == 0 || G.isEmpty()) {
//                System.out.println("step: "+step+"  |G|="+G.size()+"  |C|="+C.size());
//            }
        }
        rows = C;
    }
    
    //-----------------------------------------------------------------------
    // TEST MODULE
    //-----------------------------------------------------------------------
    public static void main(String[] args) {
        Random rand = new Random(System.currentTimeMillis());
        int[][] matA1 = {{-2}, {3}};
        int[][] solA1 = { {3, 2} };
        int[][] matK1 = {
            { 3 ,  1 ,  -5,   1},
            { -2,  1 ,  -4,  -3},
            { 5 ,  -2,   3,  -2},
            { -7,  -3,   2,   1}
        };
        
                        
//        testHilbert(HilbertTestData.mat33, HilbertTestData.sol33, true); //OK
//        testHilbert(HilbertTestData.matA3, HilbertTestData.solA3, true); //OK
//        testHilbert(HilbertTestData.matA4, HilbertTestData.solA4, true); //OK
//        testHilbert(HilbertTestData.matAnisim, HilbertTestData.solAnisim, true);
//        testHilbert(HilbertTestData.matSinch, HilbertTestData.solSinch, true); 

//        testHilbert(matA1, solA1, true);
//        testHilbert(HilbertTestData.mat33v2, HilbertTestData.sol33v2, true); //OK 2310 slow
//        testHilbert(HilbertTestData.matA1, HilbertTestData.solA1, true); // ?? very slow
//        testHilbert(HilbertTestData.matm33, HilbertTestData.solm33, true);
//        testHilbert(HilbertTestData.matmagic33, HilbertTestData.solmagic33, true); //OK
//        testHilbert(HilbertTestData.matmagic3x3, HilbertTestData.solmagic3x3, true); //OK
//        testHilbert(HilbertTestData.mat44, HilbertTestData.sol44, true);
        //testHilbert(HilbertTestData.mat55, null, false);
//        testHilbert(matK1, null, true); // slow 1533

        testBuchberger(HilbertTestData.mat33, HilbertTestData.sol33); // OK (361), fast
//        testBuchberger(HilbertTestData.matA3, HilbertTestData.solA3); // OK (12), fast
//        testBuchberger(HilbertTestData.matA4, HilbertTestData.solA4); // OK (34), fast
//        testBuchberger(HilbertTestData.mat33v2, HilbertTestData.sol33v2); // OK, 2311 fast
//        testBuchberger(HilbertTestData.matA1, HilbertTestData.solA1); // very slow
//        testBuchberger(matK1, null); // OK, 1533
//        testBuchberger(HilbertTestData.mat44, HilbertTestData.sol44); // OK (12887), slow
//        testBuchberger(HilbertTestData.mat55, null); // out-of-memory

        /*int[][] matBreaks = {
            { 1,  1,  0,  1,  1,  0,  1 },
            { 1,  1,  1,  0,  1,  1,  1 },
            { 1,  1,  1,  1,  0,  1,  0 },
            {-1,  0,  0, -1,  0, -1, -1 },
            { 0, -1, -1,  0,  0,  0, -1 },
            { 0, -1,  0, -1,  0,  0,  0 },
            { 0, -1,  0,  0, -1, -1,  0 },
        };
        testHilbert(matBreaks, null, true); */
        
//        int[][] matATPN = { {1}, {2}, {-2} };
//        testHilbert(matATPN, null, true);
        
//        int[][] mat = {
////            {-1,0}, {-1,1}, {0,-1}, {2,0}
//            {1,0}, {-1,-1}, {0,1}
//        };
//        testHilbert(mat, null, true);

//        for (int h=0; h<10; h++) {
//            testCommutativity(HilbertTestData.mat33v2,rand);
////            testCommutativity(HilbertTestData.mat33,rand);
//        }


//        int[][] matA5 = {{1,1,1}, {-1,0,0}, {0,-1,0}};
//        for (int h=0; h<10; h++) { testCommutativity(matA5, rand); }


//        int[][] m = HilbertTestData.matmagic3x3, m2=null;
//        for (int i=0; i<1000; i++) {
//            try {
//                m2 = dropRandom(m, rand);
//                HilbertBasis H = new HilbertBasis(m2);  
//                H.setKeepCpCm();
//                H.HilbertFM();
//            }
//            catch (Exception e) {
//                System.out.println(new HilbertBasis(m2));
//                m = m2;
//            }
//        }
    }
    
    private static void testCommutativity(int[][] matIn, Random rand) {
        int[] index = new int[matIn[0].length];
        for (int i=0; i<index.length; i++)
            index[i] = i;
        for (int i=0; i<index.length; i++) {
            int j = rand.nextInt(index.length);
            int temp = index[i];
            index[i] = index[j];
            index[j] = temp;
        }
        int[][] mat = new int[matIn.length][index.length];
        for (int i=0; i<matIn.length; i++)
            for (int j=0; j<index.length; j++)
                mat[i][j] = matIn[i][index[j]];
//        int[][] sol = new int[matRes.length][index.length];
//        for (int i=0; i<matRes.length; i++)
//            for (int j=0; j<index.length; j++)
//                sol[i][j] = matRes[i][index[j]];
            
        HilbertBasis H = new HilbertBasis(matIn);  
//        H.setVerbose();
//        System.out.println(H);
        H.setKeepCpCm();
//        H.HilbertFM();
        for (int j=0; j<mat[0].length; j++)
            H.Buchberger(j);
//        System.out.println("\nResult:\n"+H);
        
        int hilbertB0sz = 0;
        for (int i=0; i<H.numRows(); i++)
            if (H.isHilbertBasisVec(i))
                hilbertB0sz++;
        
        for (int i=0; i<index.length; i++)
            System.out.print(index[i] + " ");
        System.out.println(":  "+H.numRows()+" "+hilbertB0sz);
    }
    
    private static void testHilbert(int[][] matIn, int[][] matRes, boolean keepCpCm) {
        System.out.println("\n\n---------------------");
        HilbertBasis H = new HilbertBasis(matIn);  
        H.setVerbose();
        System.out.println(H);
        if (keepCpCm)
            H.setKeepCpCm();
        H.HilbertFM();
        System.out.println("\nResult:\n"+H);
        
        // Separate the Hilbert basis B0
        ArrayList<int[]> avB0 = new ArrayList<>();
        for (int i=0; i<H.numRows(); i++)
            if (H.isHilbertBasisVec(i))
                avB0.add(H.getBasisVec(i));
        int[][] B0 = avB0.toArray(new int[avB0.size()][]);
        sortArrays(B0);
        System.out.println("Computed Hilbert basis has "+B0.length+" entries.");
//        for (int i=0; i<H.numRows(); i++) {
//            if (H.isHilbertBasisVec(i)) {
//                System.out.println(H.rows.get(i));
//            }
//        }

        if (matRes != null) {
            sortArrays(matRes);
            boolean equal = true;
            for (int i=0; equal && i<B0.length; i++) {
                equal = equal && Arrays.compare(B0[i], matRes[i])==0;
            }
            System.out.println("Check solution: "+equal);
            if (!equal)
                throw new IllegalStateException();
        }
    }
    
    private static void testBuchberger(int[][] matIn, int[][] matRes) {
        System.out.println("\n\n---------------------");
        HilbertBasis H = new HilbertBasis(matIn);  
        H.setVerbose();
        System.out.println(H);
        long start = System.currentTimeMillis();
        for (int j=0; j<matIn[0].length; j++)
            H.Buchberger(j);
//        H.Buchberger(matIn[0].length - 1);
        System.out.println("\nResult:\n"+H);
        System.out.println("\nRows: "+H.numRows());
        long totalTime = System.currentTimeMillis() - start;
        System.out.println("Total time: "+(totalTime/1000.0));
        
        // Separate the Hilbert basis B0
        ArrayList<int[]> avB0 = new ArrayList<>();
        for (int i=0; i<H.numRows(); i++)
            if (H.isHilbertBasisVec(i))
                avB0.add(H.getBasisVec(i));
        int[][] B0 = avB0.toArray(new int[avB0.size()][]);
        sortArrays(B0);
        System.out.println("Computed Hilbert basis has "+B0.length+" entries.");
        for (int i=0; i<H.numRows(); i++) {
            if (H.isHilbertBasisVec(i)) {
                System.out.println(H.rows.get(i));
            }
        }

        if (matRes != null) {
            sortArrays(matRes);
            boolean equal = true;
            for (int i=0; equal && i<B0.length; i++) {
                equal = equal && Arrays.compare(B0[i], matRes[i])==0;
            }
            System.out.println("Check solution: "+equal);
            if (!equal)
                throw new IllegalStateException();
        }
    }
    
    private static void sortArrays(int[][] mat) {
        Comparator<int[]> comp = (int[] o1, int[] o2) -> Arrays.compare(o1, o2);
        Arrays.sort(mat, comp);
    }
    
    private static int[][] dropRandom(int[][] mat, Random rnd) {
        int R = mat.length, C = mat[0].length;
        int skiprow=-1, skipcol=-1;
        if (rnd.nextBoolean()) {
            skiprow = rnd.nextInt(R);
            R--;
//            System.out.println("skiprow="+skiprow+" R="+R);
        } else {
            skipcol = rnd.nextInt(C);
            C--;
//            System.out.println("skipcol="+skipcol+" C="+C);
        }
        int[][] newmat = new int[R][C];
        for (int i=0,r=0; i<mat.length; i++) {
            if (i==skiprow) 
                continue;
            for (int j=0,c=0; j<mat[0].length; j++) {
                if (j==skipcol)
                    continue;
                newmat[r][c] = mat[i][j];
                c++;
            }
            r++;
        }
        return newmat;
    }
    
  }
