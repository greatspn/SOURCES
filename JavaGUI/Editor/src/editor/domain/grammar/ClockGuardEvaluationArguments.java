/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.grammar;

import editor.domain.play.AbstractMarking;
import editor.domain.play.GspnDtaPlayEngine;

/**
 *
 * @author elvio
 */
public class ClockGuardEvaluationArguments extends EvaluationArguments {
    
    // The next time bound
    public double timeBound = -1;

    public ClockGuardEvaluationArguments(AbstractMarking jointState) {
        super(jointState);
    }
    
    public void setTimeBound(double clockValue, double barrier) {
        if (clockValue < barrier + GspnDtaPlayEngine.EPSILON_VALUE) {
            double newBound = (barrier - clockValue);
            if (timeBound == -1) {
                if (timeBound < newBound)
                    timeBound = newBound;
            }
        }
    }
}
