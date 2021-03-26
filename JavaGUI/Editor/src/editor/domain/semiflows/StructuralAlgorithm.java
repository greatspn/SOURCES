/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.semiflows;

/**
 *
 * @author elvio
 */
public abstract class StructuralAlgorithm {
    // For P-semiflows: N=|P|, M=|T| (for T-semiflows: N=|T|, M=|P|)
    public final int N, M;
    
    // Has already been computed?
    private boolean computed = false;

    public StructuralAlgorithm(int N, int M) {
        this.N = N;
        this.M = M;
    }
    
//    public abstract void addFlow(int i, int j, int card);
//    
//    public abstract void setInitQuantity(int i, int quantity);

    public void setComputed() {
        this.computed = true;
    }

    public boolean isComputed() {
        return computed;
    }
    
    
    public interface ProgressObserver {
        public void advance(int step, int total, int subStep, int subTotal);
    }
    
    boolean interrupted = false;
    public synchronized void setInterrupted() {
        interrupted = true;
    }
    protected synchronized void checkInterrupted() throws InterruptedException {
        if (interrupted)
            throw new InterruptedException("Computation thread interrupted by the user.");
    }

    public abstract void compute(boolean log, ProgressObserver obs) throws InterruptedException;
}
