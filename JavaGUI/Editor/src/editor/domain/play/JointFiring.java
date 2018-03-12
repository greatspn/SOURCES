/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.play;

import common.Tuple;
import editor.domain.elements.GspnEdge;
import editor.domain.NetObject;
import editor.domain.Node;
import editor.domain.elements.DtaEdge;
import editor.domain.elements.Place;
import editor.domain.elements.DtaLocation;
import editor.domain.elements.Transition;
import editor.domain.grammar.ColorVarsBinding;
import editor.domain.values.EvaluatedFormula;
import java.io.Serializable;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

/** A firing of a GSPN (optionally followed by a firing of a DTA).
 *
 * @author elvio
 */
public class JointFiring implements Serializable {
    // The node that fired
    public final Firable firedNode;
    // The color binding assigned to the fired event (null if there is no binding)
    public final ColorVarsBinding colorVarsBinding;
    // The elapsed time from the last event
    public final double elapsedTime;
    // The state reached after the firing
    public final JointState reachedState;
    
    // Intermediate states used to play the token/flow animation.
    // The initial marking, before the flow phase
    public Map<Place, EvaluatedFormula> markingAtFlowBegin = new HashMap<>();
    // Marking at the end of the flow phase
    public Map<Place, EvaluatedFormula> markingAtFlowEnd = new HashMap<>();
    // Marking during the transition firing
    public Map<Place, EvaluatedFormula> markingDuringFiring = new HashMap<>();
    // The continuous transitions enabled during this firing
    public Set<FirableWithBindings<Transition>> enabledFlowTransitions = new HashSet<>();
    
    public double startTime;
    public DtaLocation startLoc;
    
//    public Map<Place, Double> startingFluidLevels = new HashMap<Place, Double>();
//    public Map<Place, EvaluatedFormula> tokenCountDuringFlowPhase = new HashMap<Place, EvaluatedFormula>();
//    // Fliud levels after
//    public Map<Place, Double> fluidLevelsBeforeFiringPhase = new HashMap<Place, Double>();
//    // tokens have been removed from input places, but not yet added to the output places
//    public Map<Place, EvaluatedFormula> tokenCountDuringFiringPhase = new HashMap<Place, EvaluatedFormula>();
    
    // State where tokens have been removed from input places, but not yet
    // added to the output places. Used for animation purposes.
//    public JointState intermState;
    
    
    // Fluid levels reached before the instantaneous transition firing
//    public Map<Place, Double> fluidLevelsBeforeFiring = new HashMap<Place, Double>();
    // The last tangible state seen in the path trace
    public JointState lastTangibleState;
    public Node lastTimedTransitionFired;
    // Tokens moved on GSPN edges
    public Map<GspnEdge, EvaluatedFormula> tokensPassed = new HashMap<>();
    public double maxInputEdgeLength = 0;
    public double maxOutputEdgeLength = 0;
    public double speedup;
    // Flow moved during the elapsed time 
    public Map<GspnEdge, EvaluatedFormula> flowPassed = new HashMap<>();
    // Inner/Boundary edge of the DTA that has been triggered
//    public DtaEdge triggeredEdge;

    public JointFiring(JointState startState, Firable firedNode, ColorVarsBinding colorVarsBinding,
                       JointFiring prevFiring, double elapsedTime, double speedup) {
        // NEVER DO A DEEPCOPY: IT COPIES Places and Transitions !!
        // this.reachedState = (JointState)Util.deepCopy(startState);
        this.firedNode = firedNode;
        this.colorVarsBinding = colorVarsBinding;
        this.elapsedTime = elapsedTime;
        this.speedup = speedup;
        reachedState = new JointState();
        
        assert (colorVarsBinding != null && firedNode instanceof Transition) ||
               (colorVarsBinding == null && firedNode instanceof DtaEdge) ||
               (colorVarsBinding == null && firedNode instanceof TimeElapse);
        
//        intermState = new JointState();
        Iterator<Map.Entry<Place, EvaluatedFormula>> it = startState.marking.entrySet().iterator();
        while (it.hasNext()) {
            Map.Entry<Place, EvaluatedFormula> entry = it.next();
            reachedState.marking.put(entry.getKey(), entry.getValue());
//            intermState.marking.put(entry.getKey(), entry.getValue());
        }
        reachedState.currLoc = startState.currLoc;
        startLoc = startState.currLoc;
        reachedState.stateNum = startState.stateNum + 1;
        reachedState.isTimedSimulation = startState.isTimedSimulation;
        startTime = startState.time;
        reachedState.time = startState.time + elapsedTime;
        
        if (startState.isTangible()) {
            lastTangibleState = startState;
            if (firedNode instanceof Transition) {
                lastTimedTransitionFired = (Transition)firedNode;
                assert ((Transition)firedNode).isTimed();
            }
            else lastTimedTransitionFired = null;
        }
        else if (prevFiring != null) {
            lastTangibleState = prevFiring.lastTangibleState;
            lastTimedTransitionFired = prevFiring.lastTimedTransitionFired;
        }
        else {
            lastTangibleState = null;
            lastTimedTransitionFired = null;
        }
    }
    
    private int getFlowAnimationTime() {
        if (enabledFlowTransitions.isEmpty())
            return 0;
        return (int)(GspnDtaPlayEngine.ANIM_CONTINUOUS_FLOW * speedup);
    }

    public int getTotalAnimationTime() {
        if (reachedState.isErrorState())
            return 0;
        int firingTime = (int) (GspnDtaPlayEngine.ANIM_FIRED_TOKENS_SPEED * (maxInputEdgeLength + maxOutputEdgeLength) * speedup);
        return getFlowAnimationTime() + firingTime;
    }

    public Tuple<Double,Double> getAnimationPhases(int elapsedTime) {        
        int flowAnimTime = getFlowAnimationTime();
        double flowPhase, firingPhase;
        if (flowAnimTime > elapsedTime) {
            flowPhase = ((double)elapsedTime) / flowAnimTime;
            firingPhase = 0.0;
        }
        else {
            flowPhase = 0.0;
            elapsedTime -= flowAnimTime;
            
            // 0.0 - 0.5 : input tokens move to the transition
            // 0.5 - 1.0 : tokens move from the transition to the output places
            elapsedTime /= speedup;
            double length = elapsedTime / (double) GspnDtaPlayEngine.ANIM_FIRED_TOKENS_SPEED;
            length = NetObject.clamp(length, 0.0, maxInputEdgeLength + maxOutputEdgeLength);
            if (length < maxInputEdgeLength) {
                firingPhase = 0.5 * (length / maxInputEdgeLength);
            } else {
                if (maxOutputEdgeLength == 0)
                    firingPhase = 1;
                else
                    firingPhase = 0.5 + 0.5 * ((length - maxInputEdgeLength) / maxOutputEdgeLength);
            }
            if (firingPhase <= 0.0001)
                firingPhase = 0.01;
        }
        
        return new Tuple<>(flowPhase, firingPhase);
    }
    
    public FirableWithBindings<Transition> getEnabledFlowBindingsOf(Transition trn) {
        for (FirableWithBindings<Transition> fwb : enabledFlowTransitions)
            if (fwb.firable == trn)
                return fwb;
        return null; // trn is not enabled
    }
}
