/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.semiflows;

import editor.domain.Selectable;
import java.awt.Color;

/** Visualized P/T semiflows, or place bounds computed from P-semiflows.
 *
 * @author elvio
 */
public interface PTFlows {
    
    public final char INFINITY_UNICODE = '\u221E';
    
    public enum Type {
        PLACE_SEMIFLOW, PLACE_FLOW, 
        TRANSITION_SEMIFLOWS, TRANSITION_FLOWS, 
        PLACE_BOUNDS_FROM_PINV;
        
        public String printableName() {
            switch (this) {
                case PLACE_SEMIFLOW: return "Place semiflows";
                case PLACE_FLOW: return "Place flows";
                case TRANSITION_SEMIFLOWS: return "Transition semiflows";
                case TRANSITION_FLOWS: return "Transition flows";
                case PLACE_BOUNDS_FROM_PINV: return "Place bounds";
                default:
                    throw new IllegalStateException();
            }
        }
        
        public boolean isPlace() {
            return this==PLACE_FLOW || this==PLACE_SEMIFLOW || this==PLACE_BOUNDS_FROM_PINV;
        }
        
        public boolean isTransition() {
            return this==TRANSITION_FLOWS || this==TRANSITION_SEMIFLOWS;
        }
        
        public boolean isSemiflow() {
            return this==TRANSITION_SEMIFLOWS || this==PLACE_SEMIFLOW || this==PLACE_BOUNDS_FROM_PINV;
        }
        
        
        public boolean isFlow() {
            return this==TRANSITION_FLOWS || this==PLACE_FLOW;
        }
    }
    
    // What type of informations is this?
    public Type getType();
    
    // Does the semiflow contain this place/transition/arc?
    public boolean contains(Selectable elem);
    
    // Cardinality of a place/transition, or upper bound
    public int getNodeCardinality(Selectable elem);
    
    // Lower bound of a place, or -1 if not in bound mode
    public int getNodeBound(Selectable elem, boolean lower);
    
    // Preferred color in visualization mode
    public Color getLineColor(int card);
    public Color getTextColor(int card);
    
    // Phase for animation of the dashed edges.
    public int getDashPhase();
}
