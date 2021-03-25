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
public interface SemiFlows {
    
    public final char INFINITY_UNICODE = '\u221E';
    
    public enum Type {
        PLACE_SEMIFLOW, TRANSITION_SEMIFLOWS, PLACE_BOUNDS_FROM_PINV;
        
        public String printableName() {
            switch (this) {
                case PLACE_SEMIFLOW: return "Place semiflows";
                case TRANSITION_SEMIFLOWS: return "Transition semiflows";
                case PLACE_BOUNDS_FROM_PINV: return "Place bounds";
                default:
                    throw new IllegalStateException();
            }
        }
    }
    
    // What type of informations is this?
    public Type getType();
    
    // Does the semiflow contain this place/transition/arc?
    public boolean contains(Selectable elem);
    
    // Cardinality of a place/transition, or upper bound
    public int getNodeCardinality(Selectable elem);
    
    // Lower bound of a place, or -1 if not in bound mode
    public int getNodeLowerBound(Selectable elem);
    
    // Preferred color in visualization mode
    public Color getLineColor();
    public Color getTextColor();
    
    // Phase for animation of the dashed edges.
    public int getDashPhase();
}
