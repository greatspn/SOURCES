/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.struct;

import editor.domain.semiflows.StructuralAlgorithm;

/**
 *
 * @author elvio
 */
public class SilvaColom88Algorithm extends StructuralAlgorithm {
    
//    LpSolve lp;
    
    int[][] mC;
    int[] m0;

    public SilvaColom88Algorithm(int N, int M) {
        super(N, M);
        
        m0 = new int[N];
        mC = new int[N][M];
    }


    @Override
    public void addFlow(int i, int j, int card) {
        mC[i][j] += card;
    }

    @Override
    public void setInitQuantity(int i, int quantity) {
        m0[i] = quantity;
    }

    @Override
    public void compute(boolean log, ProgressObserver obs) throws InterruptedException {
        try {
            System.out.println("max: p0;");
            
            for (int p=0; p<N; p++) {
                System.out.print("p"+p+" = "+m0[p]);
                for (int t=0; t<M; t++) {
                    if (mC[p][t] != 0) {
                        char sgn = (mC[p][t] >= 0 ? '+' : '-');
                        System.out.print(" "+sgn);
                        if (Math.abs(mC[p][t]) != 1)
                            System.out.print(Math.abs(mC[p][t])+" ");
                        System.out.print("t"+t);
                    }
                }
                System.out.println(";");
            }
            for (int p=0; p<N; p++)
                System.out.println("p"+p+" >= 0;");
            for (int t=0; t<N; t++)
                System.out.println("t"+t+" >= 0;");
            
            System.out.print("int ");
            for (int p=0; p<N; p++)
                System.out.print("p"+p+", ");
            for (int t=0; t<N; t++)
                System.out.print("t"+t+(t==N-1 ? ";\n\n": ", "));
            
//            System.out.println("java.library.path = "+System.getProperty("java.library.path"));
//            
//            LpSolve lp = LpSolve.makeLp(0, M+N);
//            lp.setAddRowmode(true);
//            for (int n=0; n<M+N; n++) {
//                lp.setInt(n, true);
//                lp.setColName(n, n < N ? ("P"+n) : ("sigma"+(n-N)));
//            }
//
//            // There are M + N variables.
//            // max: m(p)
//            // subject to:  m = m0 + C * sigma
//            //              m >= 0, sigma >= 0
//            // There are M columns, and N * 2 + M rows (constraints).
//            
//            double[] row = new double[M+N];
//            
//            // Add the m = m0 + C * sigma rows  ==>  m - C * sigma = m0
//            for (int n=0; n<N; n++) {
//                Arrays.fill(row, 0);
//                row[n] = 1;
//                for (int j=0; j<M; j++)
//                    row[N+j] = -mC[n][j];
//                lp.addConstraint(row, LpSolve.EQ, m0[n]);
//            }
//            // Add the m >= 0 rows, and the sigma >= 0 rows
//            for (int n=0; n<M+N; n++) {
//                Arrays.fill(row, 0);
//                row[n] = 1;
//                lp.addConstraint(row, LpSolve.GE, 0);
//            }
//            lp.writeLp("/Users/elvio/Desktop/my.lp");
        }
        catch (Exception e) {
            throw new InterruptedException(e.getMessage());
        }
    }
}
