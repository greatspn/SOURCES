/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import editor.domain.semiflows.FlowsGenerator;
import editor.domain.semiflows.PTFlows;
import editor.domain.semiflows.StructuralAlgorithm.ProgressObserver;
import editor.domain.semiflows.TooManyRowsException;

/**
 *
 * @author elvio
 */
public class GPPPTest {
    public static FlowsGenerator init() {
        int M=22, N=33;        
        FlowsGenerator fg = new FlowsGenerator(N, N, M, PTFlows.Type.PLACE_SEMIFLOWS);
        fg.addIncidence(12, 0, 1);
        fg.addIncidence(2, 0, 1);
        fg.addIncidence(0, 0, 1);
        fg.addIncidence(32, 0, 1);
        fg.addIncidence(18, 0, -1);
        fg.addIncidence(3, 0, -1);
        fg.addIncidence(31, 0, -1);
        fg.addIncidence(17, 1, 1);
        fg.addIncidence(12, 1, -1);
        fg.addIncidence(12, 2, 1);
        fg.addIncidence(17, 2, -1);
        fg.addIncidence(16, 3, 1);
        fg.addIncidence(12, 3, -1);
        fg.addIncidence(17, 3, -1);
        fg.addIncidence(14, 4, 1);
        fg.addIncidence(1, 4, 1);
        fg.addIncidence(16, 4, -1);
        fg.addIncidence(19, 4, -1);
        fg.addIncidence(15, 5, 1);
        fg.addIncidence(27, 5, 3);
        fg.addIncidence(14, 5, -1);
        fg.addIncidence(28, 5, -3);
        fg.addIncidence(1, 6, 1);
        fg.addIncidence(26, 6, 1);
        fg.addIncidence(15, 6, -1);
        fg.addIncidence(19, 6, -1);
        fg.addIncidence(6, 7, 1);
        fg.addIncidence(5, 7, 1);
        fg.addIncidence(7, 7, -2);
        fg.addIncidence(4, 7, -1);
        fg.addIncidence(7, 8, 2);
        fg.addIncidence(6, 8, -1);
        fg.addIncidence(4, 9, 2);
        fg.addIncidence(15, 9, 1);
        fg.addIncidence(28, 9, 1);
        fg.addIncidence(5, 9, -2);
        fg.addIncidence(8, 9, -1);
        fg.addIncidence(27, 9, -1);
        fg.addIncidence(8, 10, 1);
        fg.addIncidence(30, 10, 1);
        fg.addIncidence(9, 10, -1);
        fg.addIncidence(29, 10, -1);
        fg.addIncidence(8, 11, 1);
        fg.addIncidence(29, 11, 2);
        fg.addIncidence(10, 11, -1);
        fg.addIncidence(30, 11, -2);
        fg.addIncidence(11, 12, 1);
        fg.addIncidence(12, 12, 1);
        fg.addIncidence(31, 12, 7);
        fg.addIncidence(13, 12, -1);
        fg.addIncidence(14, 12, -1);
        fg.addIncidence(32, 12, -7);
        fg.addIncidence(13, 13, 1);
        fg.addIncidence(9, 13, 1);
        fg.addIncidence(14, 13, -1);
        fg.addIncidence(12, 13, -1);
        fg.addIncidence(19, 14, 1);
        fg.addIncidence(18, 14, 1);
        fg.addIncidence(20, 14, -1);
        fg.addIncidence(1, 14, -1);
        fg.addIncidence(20, 15, 1);
        fg.addIncidence(21, 15, -1);
        fg.addIncidence(21, 16, 1);
        fg.addIncidence(22, 16, -1);
        fg.addIncidence(22, 17, 1);
        fg.addIncidence(19, 17, 1);
        fg.addIncidence(23, 17, -1);
        fg.addIncidence(1, 17, -1);
        fg.addIncidence(23, 18, 1);
        fg.addIncidence(3, 18, 1);
        fg.addIncidence(25, 18, -1);
        fg.addIncidence(2, 18, -1);
        fg.addIncidence(1, 19, 7);
        fg.addIncidence(25, 19, 7);
        fg.addIncidence(24, 19, -1);
        fg.addIncidence(24, 20, 1);
        fg.addIncidence(19, 20, -7);
        fg.addIncidence(26, 20, -4);
        fg.addIncidence(0, 20, -7);
        fg.addIncidence(9, 21, 1);
        fg.addIncidence(10, 21, 1);
        fg.addIncidence(11, 21, -1);
        fg.addIncidence(12, 21, -1);
        return fg;
    }
    
    public static void main(String[] args) throws InterruptedException, TooManyRowsException {
        FlowsGenerator fg = init();
        ProgressObserver obs = new ProgressObserver() {
            @Override
            public void advance(int step, int total, int s, int t) {
            }
        };
        fg.compute(true, obs);
    }
}
