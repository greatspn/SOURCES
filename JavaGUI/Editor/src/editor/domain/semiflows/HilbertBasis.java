/*
 * Implementation of the Hilbert basis computation
 */
package editor.domain.semiflows;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Random;

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
    public boolean isRealBasisVec(int i) {
        return rows.get(i).is_l_zero();
    }
    public void removeInitialRows() {
        rows.removeIf(row -> row.initial);
    }
    
    //-----------------------------------------------------------------------

    private boolean partialOrderCmp(Row row1, Row row2, int j) {
        for (int jj=0; jj<=j; jj++) {
            if (!(row1.l[jj] * row2.l[jj] >= 0 && 
                  Math.abs(row1.l[jj]) <= Math.abs(row2.l[jj])))
                return false;
        }
        return row1.less_equal_e(row2);
        
//        return (row1.l[j] * row2.l[j] >= 0 && 
//                Math.abs(row1.l[j]) <= Math.abs(row2.l[j]) &&
//                row1.less_equal_e(row2));
    }
        
    //-----------------------------------------------------------------------
//    // Generate all sums of R+ rows with R- rows into C
//    private void S_vectors(ArrayList<Row> Rp, ArrayList<Row> Rm, ArrayList<Row> C) {
//        for (Row rowp : Rp) 
//            for (Row rowm : Rm) 
//                appendUnique(new Row(rowp, rowm), C);
//    }
    // Generate all sums of R+/R- rows with a single row1 into C
    private void S_vectors(ArrayList<Row> R, Row row1, ArrayList<Row> C) {
        for (Row row2 : R) 
            appendUnique(new Row(row1, row2), C);
    }
    
    //-----------------------------------------------------------------------
    
    private boolean isIrreducible(Row r, int j, ArrayList<Row> C0,
                                  ArrayList<Row> Cp, ArrayList<Row> Cm) 
    {
        for (Row r2 : C0)
            if (partialOrderCmp(r2, r, j))
                return false;
        for (Row r2 : Cp)
            if (partialOrderCmp(r2, r, j))
                return false;
        for (Row r2 : Cm)
            if (partialOrderCmp(r2, r, j))
                return false;
        return true;
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
        for (Row r : toAppend)
            appendUnique(r, rows);
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
            if (isIrreducible(el, j, C0, Cp, Cm)) {
                if (el.l[j] == 0) {
                    appendUnique(el, C0);
                    if (verbose)
                        System.out.println("C0 <- C0 U "+el);
                }
                else if (el.l[j] > 0) {
                    appendUnique(el, Cp);
                    S_vectors(Cm, el, G); // G U (C- * [e|l])
                    if (verbose)
                        System.out.println("C+ <- C+ U "+el);
                }
                else if (el.l[j] < 0) {
                    appendUnique(el, Cm);
                    S_vectors(Cp, el, G); // G U (C+ * [e|l])
                    if (verbose)
                        System.out.println("C- <- C- U "+el);
                }
            }
            else {
                if (verbose)
                    System.out.println("DROP       "+el);
            }
        }
        ArrayList<Row> D = new ArrayList<>();
        D.addAll(C0);
        if (keepCpCm) {
            D.addAll(Cp);
            D.addAll(Cm);
        }
        appendAllUnique(rows, D); // keep all rows that started the iteration
        
        rows = D;
    }
    
    //-----------------------------------------------------------------------
    // Heuristic that chooses the next pivot of L from [E|L]
    private int nextPivot(boolean[] colReduced) {
        int[] Lp = new int[M];
        int[] Lm = new int[M];
        for (Row el : rows) {
            for (int j=0; j<M; j++) {
                if (el.l[j] > 0)
                    Lp[j] += el.l[j];
                else
                    Lm[j] += -el.l[j];
            }
        }
        int pivot = -1, smallestValue = Integer.MAX_VALUE;
        for (int j=0; j<M; j++) {
            if (colReduced[j])
                continue;
            return j;
//            if (Lp[j]==0 && Lm[j]==0)
//                continue;
//            
//            int value = Lp[j] * Lm[j];
//            if (value < smallestValue) {
//                smallestValue = value;
//                pivot = j;
//            }
        }
        return pivot;
    }
    
    //-----------------------------------------------------------------------
    public void HilbertFM() {
//        for (int j=0; j<M; j++) {
        boolean[] colReduced = new boolean[M];
        int j;
        while ((j = nextPivot(colReduced)) >= 0) {
            colReduced[j] = true;
            if (verbose)
                System.out.println("\nnext pivot: "+j);
            HilbertFMcol(j);
            if (verbose)
                System.out.print(this);
        }
    }
    
    //-----------------------------------------------------------------------
    public String toString() {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < rows.size(); i++) {
            sb.append(String.format("%2d: ", i));
            sb.append(rows.get(i));
            sb.append("\n");
        }
        return sb.toString();
    }
    
    
    public static void main(String[] args) {
        Random rand = new Random(1);
        int[][] matA1 = {{-2}, {3}};
        int[][] solA1 = { {3, 2} };
        int[][] matK1 = {
            { 3 ,  1 ,  -5,   1},
            { -2,  1 ,  -4,  -3},
            { 5 ,  -2,   3,  -2},
            { -7,  -3,   2,   1}
        };
        int[][] matSinch = {{1,0}, {0,1}, {-1,-1}};
        
                        
//        testHilbert(matA1, solA1, true);
//        testHilbert(HilbertTestData.mat33, HilbertTestData.sol33, true); //OK
//        testHilbert(HilbertTestData.mat33v2, HilbertTestData.sol33v2, true); //OK 2310 slow
//        testHilbert(HilbertTestData.matA1, HilbertTestData.solA1, true); // ?? very slow
//        testHilbert(HilbertTestData.matA3, HilbertTestData.solA3, true); //OK
//        testHilbert(HilbertTestDaa.matA4, HilbertTestData.solA4, true); //OK
//        testHilbert(HilbertTestData.matm33, HilbertTestData.solm33, true);
//        testHilbert(HilbertTestData.matmagic33, HilbertTestData.solmagic33, true); //OK
//        testHilbert(HilbertTestData.matmagic3x3, HilbertTestData.solmagic3x3, true); //OK
//        testHilbert(HilbertTestData.mat44, HilbertTestData.sol44, true);
        //testHilbert(HilbertTestData.mat55, null, false);
//        testHilbert(matK1, null, true); // slow
        testHilbert(matSinch, null, true); 
        
//        int[][] mat = {
////            {-1,0}, {-1,1}, {0,-1}, {2,0}
//            {1,0}, {-1,-1}, {0,1}
//        };
//        testHilbert(mat, null, true);

//        for (int h=0; h<10; h++)
//            testCommutativity(HilbertTestData.mat33,rand);
    }
    
    private static void testCommutativity(int[][] matIn, Random rand) {
        int[] index = new int[matIn[0].length];
        for (int i=0; i<index.length; i++)
            index[i] = i;
        for (int i=0; i<index.length; i++) {
            int j = rand.nextInt(index.length - 1);
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
        H.HilbertFM();
//        System.out.println("\nResult:\n"+H);
        
        int hilbertB0sz = 0;
        for (int i=0; i<H.numRows(); i++)
            if (H.isRealBasisVec(i))
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
            if (H.isRealBasisVec(i))
                avB0.add(H.getBasisVec(i));
        int[][] B0 = avB0.toArray(new int[avB0.size()][]);
        sortArrays(B0);
        System.out.println("Computed Hilbert basis has "+B0.length+" entries.");

        if (matRes != null) {
            sortArrays(matRes);
            boolean equal = true;
            for (int i=0; equal && i<B0.length; i++) {
                equal = equal && Arrays.compare(B0[i], matRes[i])==0;
            }
            System.out.println("Check solution: "+equal);
        }
    }
    
    private static void sortArrays(int[][] mat) {
        Comparator<int[]> comp = (int[] o1, int[] o2) -> Arrays.compare(o1, o2);
        Arrays.sort(mat, comp);
    }
}
