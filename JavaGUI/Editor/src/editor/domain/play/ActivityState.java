/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.play;

/** The current activity of the net in the play engine.
 * This class is used to pass arguments to the print() methods.
 *
 * @author elvio
 */
public class ActivityState {
    public JointState state;
    public JointFiring firing;
    
    private final double pulseCoeff;
    private final double flowCoeff;
    private final double firingCoeff;

    public ActivityState(JointState state, double pulseCoeff) {
        this.state = state;
        this.pulseCoeff = pulseCoeff;
        this.flowCoeff = this.firingCoeff = -1.0;
    }

    public ActivityState(JointState state, JointFiring firing, double flowCoeff, double firingCoeff) {
        assert state != null && firing != null;
//        System.out.println("ActivityState:  flowCoeff="+flowCoeff+"  firingCoeff="+firingCoeff);
        assert flowCoeff >= 0 && firingCoeff >= 0 && flowCoeff <= 1 && firingCoeff <= 1;
        assert flowCoeff != 0 || firingCoeff != 0;
        this.state = state;
        this.firing = firing;
        this.pulseCoeff = -1.0;
        this.flowCoeff = flowCoeff;
        this.firingCoeff = firingCoeff;
    }

    public double getPulseCoeff() {
        assert firing == null;
        return pulseCoeff;
    }

    public double getFlowCoeff() {
        assert firing != null;
        return flowCoeff;
    }

    public double getFiringCoeff() {
        assert firing != null;
        return firingCoeff;
    }
    
    public boolean isFlowPhase() {
        return getFlowCoeff() > 0.0;
    }
    
    public boolean isFiringPhase() {
        return getFiringCoeff() > 0.0;
    }
}
