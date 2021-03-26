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
        msa.setIncidence(12, 0, 1);
        msa.setIncidence(2, 0, 1);
        msa.setIncidence(0, 0, 1);
        msa.setIncidence(32, 0, 1);
        msa.setIncidence(18, 0, -1);
        msa.setIncidence(3, 0, -1);
        msa.setIncidence(31, 0, -1);
        msa.setIncidence(17, 1, 1);
        msa.setIncidence(12, 1, -1);
        msa.setIncidence(12, 2, 1);
        msa.setIncidence(17, 2, -1);
        msa.setIncidence(16, 3, 1);
        msa.setIncidence(12, 3, -1);
        msa.setIncidence(17, 3, -1);
        msa.setIncidence(14, 4, 1);
        msa.setIncidence(1, 4, 1);
        msa.setIncidence(16, 4, -1);
        msa.setIncidence(19, 4, -1);
        msa.setIncidence(15, 5, 1);
        msa.setIncidence(27, 5, 3);
        msa.setIncidence(14, 5, -1);
        msa.setIncidence(28, 5, -3);
        msa.setIncidence(1, 6, 1);
        msa.setIncidence(26, 6, 1);
        msa.setIncidence(15, 6, -1);
        msa.setIncidence(19, 6, -1);
        msa.setIncidence(6, 7, 1);
        msa.setIncidence(5, 7, 1);
        msa.setIncidence(7, 7, -2);
        msa.setIncidence(4, 7, -1);
        msa.setIncidence(7, 8, 2);
        msa.setIncidence(6, 8, -1);
        msa.setIncidence(4, 9, 2);
        msa.setIncidence(15, 9, 1);
        msa.setIncidence(28, 9, 1);
        msa.setIncidence(5, 9, -2);
        msa.setIncidence(8, 9, -1);
        msa.setIncidence(27, 9, -1);
        msa.setIncidence(8, 10, 1);
        msa.setIncidence(30, 10, 1);
        msa.setIncidence(9, 10, -1);
        msa.setIncidence(29, 10, -1);
        msa.setIncidence(8, 11, 1);
        msa.setIncidence(29, 11, 2);
        msa.setIncidence(10, 11, -1);
        msa.setIncidence(30, 11, -2);
        msa.setIncidence(11, 12, 1);
        msa.setIncidence(12, 12, 1);
        msa.setIncidence(31, 12, 7);
        msa.setIncidence(13, 12, -1);
        msa.setIncidence(14, 12, -1);
        msa.setIncidence(32, 12, -7);
        msa.setIncidence(13, 13, 1);
        msa.setIncidence(9, 13, 1);
        msa.setIncidence(14, 13, -1);
        msa.setIncidence(12, 13, -1);
        msa.setIncidence(19, 14, 1);
        msa.setIncidence(18, 14, 1);
        msa.setIncidence(20, 14, -1);
        msa.setIncidence(1, 14, -1);
        msa.setIncidence(20, 15, 1);
        msa.setIncidence(21, 15, -1);
        msa.setIncidence(21, 16, 1);
        msa.setIncidence(22, 16, -1);
        msa.setIncidence(22, 17, 1);
        msa.setIncidence(19, 17, 1);
        msa.setIncidence(23, 17, -1);
        msa.setIncidence(1, 17, -1);
        msa.setIncidence(23, 18, 1);
        msa.setIncidence(3, 18, 1);
        msa.setIncidence(25, 18, -1);
        msa.setIncidence(2, 18, -1);
        msa.setIncidence(1, 19, 7);
        msa.setIncidence(25, 19, 7);
        msa.setIncidence(24, 19, -1);
        msa.setIncidence(24, 20, 1);
        msa.setIncidence(19, 20, -7);
        msa.setIncidence(26, 20, -4);
        msa.setIncidence(0, 20, -7);
        msa.setIncidence(9, 21, 1);
        msa.setIncidence(10, 21, 1);
        msa.setIncidence(11, 21, -1);
        msa.setIncidence(12, 21, -1);
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
