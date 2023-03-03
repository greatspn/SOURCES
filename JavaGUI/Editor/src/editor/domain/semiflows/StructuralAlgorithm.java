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
    // when there are supplementary variables, N0 is the original N.
    public int N;
    public int N0, M;
    
    // Has already been computed?
    private boolean computed = false;
    private String failureReason = "";
    protected int maxRows = -1;

    public StructuralAlgorithm(int N, int N0, int M) {
        assert N0 <= N;
        this.N = N;
        this.N0 = N0;
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
    
    public void setFailed(String reason) {
        this.computed = false;
        this.failureReason = reason;
    }
    
    public String getFailureReason() {
        return failureReason;
    }

    public void setMaxRows(int maxRows) {
        this.maxRows = maxRows;
    }

    public int getMaxRows() {
        return maxRows;
    }
    
    protected void reduceN(int N) {
        assert N >= this.N0;
        this.N = N;
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

    public abstract void compute(boolean log, ProgressObserver obs) throws InterruptedException, TooManyRowsException;
}
