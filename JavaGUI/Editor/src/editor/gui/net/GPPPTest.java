/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import editor.domain.semiflows.MartinezSilvaAlgorithm;
import editor.domain.semiflows.StructuralAlgorithm.ProgressObserver;

/**
 *
 * @author elvio
 */
public class GPPPTest {
    public static MartinezSilvaAlgorithm init() {
        int M=22, N=33;        
        MartinezSilvaAlgorithm msa = new MartinezSilvaAlgorithm(N, M);
        msa.addFlow(12, 0, 1);
        msa.addFlow(2, 0, 1);
        msa.addFlow(0, 0, 1);
        msa.addFlow(32, 0, 1);
        msa.addFlow(18, 0, -1);
        msa.addFlow(3, 0, -1);
        msa.addFlow(31, 0, -1);
        msa.addFlow(17, 1, 1);
        msa.addFlow(12, 1, -1);
        msa.addFlow(12, 2, 1);
        msa.addFlow(17, 2, -1);
        msa.addFlow(16, 3, 1);
        msa.addFlow(12, 3, -1);
        msa.addFlow(17, 3, -1);
        msa.addFlow(14, 4, 1);
        msa.addFlow(1, 4, 1);
        msa.addFlow(16, 4, -1);
        msa.addFlow(19, 4, -1);
        msa.addFlow(15, 5, 1);
        msa.addFlow(27, 5, 3);
        msa.addFlow(14, 5, -1);
        msa.addFlow(28, 5, -3);
        msa.addFlow(1, 6, 1);
        msa.addFlow(26, 6, 1);
        msa.addFlow(15, 6, -1);
        msa.addFlow(19, 6, -1);
        msa.addFlow(6, 7, 1);
        msa.addFlow(5, 7, 1);
        msa.addFlow(7, 7, -2);
        msa.addFlow(4, 7, -1);
        msa.addFlow(7, 8, 2);
        msa.addFlow(6, 8, -1);
        msa.addFlow(4, 9, 2);
        msa.addFlow(15, 9, 1);
        msa.addFlow(28, 9, 1);
        msa.addFlow(5, 9, -2);
        msa.addFlow(8, 9, -1);
        msa.addFlow(27, 9, -1);
        msa.addFlow(8, 10, 1);
        msa.addFlow(30, 10, 1);
        msa.addFlow(9, 10, -1);
        msa.addFlow(29, 10, -1);
        msa.addFlow(8, 11, 1);
        msa.addFlow(29, 11, 2);
        msa.addFlow(10, 11, -1);
        msa.addFlow(30, 11, -2);
        msa.addFlow(11, 12, 1);
        msa.addFlow(12, 12, 1);
        msa.addFlow(31, 12, 7);
        msa.addFlow(13, 12, -1);
        msa.addFlow(14, 12, -1);
        msa.addFlow(32, 12, -7);
        msa.addFlow(13, 13, 1);
        msa.addFlow(9, 13, 1);
        msa.addFlow(14, 13, -1);
        msa.addFlow(12, 13, -1);
        msa.addFlow(19, 14, 1);
        msa.addFlow(18, 14, 1);
        msa.addFlow(20, 14, -1);
        msa.addFlow(1, 14, -1);
        msa.addFlow(20, 15, 1);
        msa.addFlow(21, 15, -1);
        msa.addFlow(21, 16, 1);
        msa.addFlow(22, 16, -1);
        msa.addFlow(22, 17, 1);
        msa.addFlow(19, 17, 1);
        msa.addFlow(23, 17, -1);
        msa.addFlow(1, 17, -1);
        msa.addFlow(23, 18, 1);
        msa.addFlow(3, 18, 1);
        msa.addFlow(25, 18, -1);
        msa.addFlow(2, 18, -1);
        msa.addFlow(1, 19, 7);
        msa.addFlow(25, 19, 7);
        msa.addFlow(24, 19, -1);
        msa.addFlow(24, 20, 1);
        msa.addFlow(19, 20, -7);
        msa.addFlow(26, 20, -4);
        msa.addFlow(0, 20, -7);
        msa.addFlow(9, 21, 1);
        msa.addFlow(10, 21, 1);
        msa.addFlow(11, 21, -1);
        msa.addFlow(12, 21, -1);
        return msa;
    }
    
    public static void main(String[] args) throws InterruptedException {
        MartinezSilvaAlgorithm msa = init();
        ProgressObserver obs = new ProgressObserver() {
            @Override
            public void advance(int step, int total, int s, int t) {
            }
        };
        msa.compute(true, obs);
    }
}
