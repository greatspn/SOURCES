/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.semiflows;

import editor.domain.Node;
import editor.domain.Selectable;
import editor.gui.ResourceFactory;
import java.awt.Color;
import javax.swing.ImageIcon;

/** Visualized P/T semiflows, or place bounds computed from P-semiflows.
 *
 * @author elvio
 */
public interface PTFlows {
    
    public final char INFINITY_UNICODE = '\u221E';
    
    public enum Type {
        PLACE_SEMIFLOWS, 
        PLACE_FLOWS, 
        TRANSITION_SEMIFLOWS, 
        TRANSITION_FLOWS, 
        PLACE_BOUNDS_FROM_PINV,
        PLACE_BASIS,
        TRANSITION_BASIS,
        TRAPS,
        SIPHONS;
        
        public String printableName() {
            switch (this) {
                case PLACE_SEMIFLOWS:        return "Place semiflows";
                case PLACE_FLOWS:            return "Place flows";
                case TRANSITION_SEMIFLOWS:   return "Transition semiflows";
                case TRANSITION_FLOWS:       return "Transition flows";
                case PLACE_BOUNDS_FROM_PINV: return "Place bounds";
                case PLACE_BASIS:            return "Place basis";
                case TRANSITION_BASIS:       return "Place basis";
                case TRAPS:                  return "Minimal traps";
                case SIPHONS:                return "Minimal siphons";
                default:
                    throw new IllegalStateException();
            }
        }
        
        public ImageIcon getIcon32() {
            ResourceFactory rf = ResourceFactory.getInstance();
            switch (this) {
                case PLACE_SEMIFLOWS:        return rf.getPinv_N32();
                case PLACE_FLOWS:            return rf.getPinv_Z32();
                case TRANSITION_SEMIFLOWS:   return rf.getTinv_N32();
                case TRANSITION_FLOWS:       return rf.getTinv_Z32();
                case PLACE_BOUNDS_FROM_PINV: return rf.getBound32();
                case PLACE_BASIS:            return rf.getPinv_B32();
                case TRANSITION_BASIS:       return rf.getTinv_B32();
                case TRAPS:                  return rf.getTrap();
                case SIPHONS:                return rf.getSiphon();
                default:
                    throw new IllegalStateException();
            }
        }
        
        public String[] getLatexFlowName() {
            switch (this) {
                case PLACE_SEMIFLOWS:        return new String[]{"\\phi", "\\boldsymbol{\\Phi}"};
                case PLACE_FLOWS:            return new String[]{"f", "\\mathbf{F}"};
                case TRANSITION_SEMIFLOWS:   return new String[]{"\\psi"/*, "\\boldsymbol{\\Psi}"*/};
                case TRANSITION_FLOWS:       return new String[]{"y"/*, "\\mathbf{Y}"*/};
                case PLACE_BOUNDS_FROM_PINV: return null;
                case PLACE_BASIS:            return new String[]{"b", "\\mathbf{B}"};
                case TRANSITION_BASIS:       return new String[]{"d"/*, "\\mathbf{D}"*/};
                case TRAPS:                  return new String[]{"t", "\\mathbf{T}"};
                case SIPHONS:                return new String[]{"s", "\\mathbf{S}"};
                default:
                    throw new IllegalStateException();
            }
        }
        
        public boolean isPlace() {
            return this==PLACE_FLOWS || this==PLACE_SEMIFLOWS || this==PLACE_BOUNDS_FROM_PINV
                    || this==PLACE_BASIS || this==TRAPS || this==SIPHONS;
        }
        
        public boolean isTransition() {
            return this==TRANSITION_FLOWS || this==TRANSITION_SEMIFLOWS || this==TRANSITION_BASIS;
        }
        
        public boolean isSemiflow() {
            return this==TRANSITION_SEMIFLOWS || this==PLACE_SEMIFLOWS || this==PLACE_BOUNDS_FROM_PINV 
                    || this==TRAPS || this==SIPHONS;
        }
        
        public boolean isFlow() {
            return this==TRANSITION_FLOWS || this==PLACE_FLOWS || this==PLACE_BASIS || this==TRANSITION_BASIS;
        }
        
        public boolean isTrapsOrSiphons() {
            return this==TRAPS || this==SIPHONS;
        }
        
        public boolean isBasis() {
            return this==PLACE_BASIS || this==TRANSITION_BASIS;
        }
        
        public boolean isBound() {
            return this==PLACE_BOUNDS_FROM_PINV;
        }
        
        public String getInvariantSign() {
            switch (this) {
                case PLACE_SEMIFLOWS:
                case PLACE_FLOWS:
                case PLACE_BASIS: 
                    return "=";
                case TRANSITION_SEMIFLOWS: 
                case TRANSITION_FLOWS: 
                case PLACE_BOUNDS_FROM_PINV:
                case TRANSITION_BASIS: 
                case TRAPS:
                case SIPHONS: 
                    return null;
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
    public int getNodeBound(Selectable elem, boolean lower);
    
    // Preferred color in visualization mode
    public Color getLineColor(int card);
    public Color getTextColor(int card);
    public Color getBorderColor(Node node);
    
    // Phase for animation of the dashed edges.
    public int getDashPhase();
}
