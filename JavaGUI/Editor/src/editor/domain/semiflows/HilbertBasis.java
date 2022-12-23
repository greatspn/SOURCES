/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package editor.domain.semiflows;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;

/**
 *
 * @author elvio
 */
public class HilbertBasis {  
    private ArrayList<Row> rows;
    private final int N, M;
    private boolean verbose = false;
    private boolean keepRpRm = false;
    
    //-----------------------------------------------------------------------
    // Empty constructor: Initialize [identity(N*N) | zero(N*M)]
    public HilbertBasis(int N, int M) {
        this.N = N;
        this.M = M;
        rows = new ArrayList<>();
        for (int n=0; n<N; n++) {
            Row row = new Row(N, M);
            row.d[n] = 1;
            rows.add(row);
        }
    }
    
    // Initialize from an N*M matrix
    public HilbertBasis(int[][] mat) {
        this(mat.length, mat[0].length);
        for (int i=0; i<mat.length; i++) {
            for (int j=0; j<mat[i].length; j++) {
                rows.get(i).c[j] = mat[i][j];
            }
        }
    }

    //-----------------------------------------------------------------------
    public void setKeepRpRm() {
        keepRpRm = true;
    }
    public void addToC(int i, int j, int val) {
        rows.get(i).c[j] += val;
    }
    public int numRows() { 
        return rows.size(); 
    }
    public int[] getBasisVec(int i) {
        return rows.get(i).d;
    }
    public boolean isRealBasisVec(int i) {
        return rows.get(i).is_c_zero();
    }
    
    //-----------------------------------------------------------------------
    // Generate all sums of R+ rows with R- rows into C
    private void S_vectors(ArrayList<Row> Rp, ArrayList<Row> Rm, ArrayList<Row> C) {
        for (Row rowp : Rp) {
            for (Row rowm : Rm) {
                Row rowSum = new Row(N, M);
                rowSum.add(rowp);
                rowSum.add(rowm);
                C.add(rowSum);
            }
        }
    }
    
    //-----------------------------------------------------------------------
    // Normal form
    private boolean normalForm(Row row) {
        for (Row reducer : rows) {
            // remove the (max multiple of the) reducer.d from dc.d
            int alpha = Integer.MAX_VALUE;
            for (int j=0; j<N && alpha>0; j++) {
                if (reducer.d[j] != 0) {
                    alpha = Math.min(alpha, row.d[j] / reducer.d[j]);
                }
            }
            assert alpha != Integer.MAX_VALUE;
            if (alpha > 0) {
                row.addMult(reducer, -alpha);
                
                // If d has been zeroed, return false;
                if (row.is_d_zero())
                    return false;
            }
        }
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
    private void HilbertFMcol(int j) {
        ArrayList<Row> Rp = new ArrayList<>(); // R+ = rows with c[j] > 0
        ArrayList<Row> Rm = new ArrayList<>(); // R- = rows with c[j] < 0
        {
            ArrayList<Row> R0 = new ArrayList<>(); // R0 = rows with c[j] == 0
            for (Row dc : rows) {
                if (dc.c[j] > 0)
                    Rp.add(dc);
                else if (dc.c[j] < 0)
                    Rm.add(dc);
                else
                    R0.add(dc);
            }
            rows = R0;
            if (verbose)
                System.out.println("|R+|:"+Rp.size()+" |R-|:"+Rm.size()+" |G|:"+rows.size());
        }
        
        // Pottier algorithm
        ArrayList<Row> C = new ArrayList<>();
        ArrayList<Row> supp = new ArrayList<>();
        S_vectors(Rp, Rm, C); // generate first candidates
        // normalize and recombine all candidates in C
        while (!C.isEmpty()) {
            Row dc = C.get(0);
            C.remove(0);
            if (normalForm(dc)) {
                if (dc.c[j] > 0) {
                    if (appendUnique(dc, Rp)) {
                        supp.add(dc);
                        S_vectors(supp, Rm, C);
                        supp.clear();
                        if (verbose)
                            System.out.println("R+ <- R+ U "+dc);
                    }
                }
                else if (dc.c[j] < 0) {
                    if (appendUnique(dc, Rm)) {
                        supp.add(dc);
                        S_vectors(Rp, supp, C);
                        supp.clear();
                        if (verbose)
                            System.out.println("R- <- R- U "+dc);
                    }
                }
                else {
                    if (appendUnique(dc, rows)) {
                        if (verbose)
                            System.out.println("G  <- G  U "+dc);
                    }
                }
            }
            else {
                if (verbose)
                    System.out.println("DROP");
            }
        }
        if (keepRpRm) {
            rows.addAll(Rp);
            rows.addAll(Rm);
        }
    }
    
    //-----------------------------------------------------------------------
    // Heuristic thah chooses the next pivot of C from D|C
    private int nextPivot(boolean[] colReduced) {
        int[] Cp = new int[M];
        int[] Cm = new int[M];
        for (Row dc : rows) {
            for (int j=0; j<M; j++) {
                if (dc.c[j] > 0)
                    Cp[j] += dc.c[j];
                else
                    Cm[j] += -dc.c[j];
            }
        }
        int pivot = -1, smallestValue = Integer.MAX_VALUE;
        for (int j=0; j<M; j++) {
            if (colReduced[j])
                continue;
            if (Cp[j]==0 && Cm[j]==0)
                continue;
            
            int value = Cp[j] * Cm[j];
            if (value < smallestValue) {
                smallestValue = value;
                pivot = j;
            }
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
                System.out.println("next pivot: "+j);
            HilbertFMcol(j);
            if (verbose)
                System.out.println(this);
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
        int[][] matA1 = {{-2}, {3}};
        int[][] solA1 = { {3, 2} };
        
        int[][] mat33 = {
            { 1,  1,  0,  1,  1,  0,  1},
            { 1,  1,  1,  0,  1,  1,  1},
            { 1,  1,  1,  1,  0,  1,  0},
            {-1,  0, -1,  0,  0,  0,  0},
            {-1,  0,  0, -1,  0, -1, -1},
            {-1,  0,  0,  0, -1,  0,  0},
            { 0, -1, -1,  0,  0,  0, -1},
            { 0, -1,  0, -1,  0,  0,  0},
            { 0, -1,  0,  0, -1, -1,  0},
        };
        int[][] sol33 = {
            {0, 2, 1, 2, 1, 0, 1, 0, 2},
            {1, 0, 2, 2, 1, 0, 0, 2, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 2, 0, 0, 1, 2, 2, 0, 1},
            {2, 0, 1, 0, 1, 2, 1, 2, 0},            
        };
        
        int[][] mat44 = {
            { 1,  1,  1,  0,  1,  1,  1,  0,  1},
            { 1,  1,  1,  1,  0,  1,  1,  1,  1},
            { 1,  1,  1,  1,  1,  0,  1,  1,  1},
            { 1,  1,  1,  1,  1,  1,  0,  1,  0},
            {-1,  0,  0, -1,  0,  0,  0,  0,  0},
            {-1,  0,  0,  0, -1,  0,  0, -1,  0},
            {-1,  0,  0,  0,  0, -1,  0,  0, -1},
            {-1,  0,  0,  0,  0,  0, -1,  0,  0},
            { 0, -1,  0, -1,  0,  0,  0,  0,  0},
            { 0, -1,  0,  0, -1,  0,  0,  0, -1},
            { 0, -1,  0,  0,  0, -1,  0, -1,  0},
            { 0, -1,  0,  0,  0,  0, -1,  0,  0},
            { 0,  0, -1, -1,  0,  0,  0,  0, -1},
            { 0,  0, -1,  0, -1,  0,  0,  0,  0},
            { 0,  0, -1,  0,  0, -1,  0,  0,  0},
            { 0,  0, -1,  0,  0,  0, -1, -1,  0},
        };
        int[][] sol44 = {
            {0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0},
            {0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0},
            {0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0},
            {0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
            {0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0},
            {0, 0, 1, 1, 0, 1, 1, 0, 2, 0, 0, 0, 0, 1, 0, 1},
            {0, 0, 2, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1},
            {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0},
            {0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1},
            {0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0},
            {0, 1, 0, 1, 2, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1},
            {0, 2, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0},
            {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0},
            {1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 2, 0, 0},
            {1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 2, 0},
            {1, 0, 1, 0, 0, 0, 0, 2, 0, 1, 1, 0, 1, 1, 0, 0},
            {1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1},
            {1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 2, 1, 0, 1, 0},
            {1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1},
        };
        
        int[][] mat55 = { // very hard, takes too much time
            { 1,  1,  1,  1,  0,  1,  1,  1,  1,  0,  1 },
            { 1,  1,  1,  1,  1,  0,  1,  1,  1,  1,  1 },
            { 1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  1 },
            { 1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  1 },
            { 1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  0 },
            {-1,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0 },
            {-1,  0,  0,  0,  0, -1,  0,  0,  0, -1,  0 },
            {-1,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0 },
            {-1,  0,  0,  0,  0,  0,  0, -1,  0,  0, -1 },
            {-1,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0 },
            { 0, -1,  0,  0, -1,  0,  0,  0,  0,  0,  0 },
            { 0, -1,  0,  0,  0, -1,  0,  0,  0,  0,  0 },
            { 0, -1,  0,  0,  0,  0, -1,  0,  0, -1, -1 },
            { 0, -1,  0,  0,  0,  0,  0, -1,  0,  0,  0 },
            { 0, -1,  0,  0,  0,  0,  0,  0, -1,  0,  0 },
            { 0,  0, -1,  0, -1,  0,  0,  0,  0,  0,  0 },
            { 0,  0, -1,  0,  0, -1,  0,  0,  0,  0, -1 },
            { 0,  0, -1,  0,  0,  0, -1,  0,  0,  0,  0 },
            { 0,  0, -1,  0,  0,  0,  0, -1,  0, -1,  0 },
            { 0,  0, -1,  0,  0,  0,  0,  0, -1,  0,  0 },
            { 0,  0,  0, -1, -1,  0,  0,  0,  0,  0, -1 },
            { 0,  0,  0, -1,  0, -1,  0,  0,  0,  0,  0 },
            { 0,  0,  0, -1,  0,  0, -1,  0,  0,  0,  0 },
            { 0,  0,  0, -1,  0,  0,  0, -1,  0,  0,  0 },
            { 0,  0,  0, -1,  0,  0,  0,  0, -1, -1,  0 },
        };
                        
        testHilbert(matA1, solA1);
        testHilbert(mat33, sol33);
        testHilbert(mat44, sol44);
    }
    
    private static void testHilbert(int[][] matIn, int[][] matRes) {
        HilbertBasis H = new HilbertBasis(matIn);  
//        H.verbose = true;
        System.out.println(H);
//        H.setKeepRpRm();
        H.HilbertFM();
        System.out.println(H);
        
        sortArrays(matRes);
        int[][] sol = new int[H.numRows()][];
        for (int i=0; i<sol.length; i++)
            sol[i] = H.getBasisVec(i);
        sortArrays(sol);
        
        boolean equal = true;
        for (int i=0; i<sol.length; i++)
            equal = equal && Arrays.compare(sol[i], matRes[i])==0;
        System.out.println("Check solution: "+equal);
    }
    
    private static void sortArrays(int[][] mat) {
        Comparator<int[]> comp = new Comparator<int[]>() {
            @Override public int compare(int[] o1, int[] o2) {
                return Arrays.compare(o1, o2);
            }
        };
        Arrays.sort(mat, comp);
    }
}
