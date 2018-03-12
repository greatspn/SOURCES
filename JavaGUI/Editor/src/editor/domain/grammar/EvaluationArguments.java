/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.grammar;

import editor.domain.play.AbstractMarking;
import editor.domain.play.Firable;

/** Arguments passed to an evaluation stack frame, that are not part of the context.
 *
 * @author elvio
 */
public class EvaluationArguments {
    
    // Current joint state of a GSPN and a DTA (current marking and DTA location)
    // for the evaluation of marking-dependent expressions.
    public final AbstractMarking jointState;
    
    // The node that has fired (null if this is not a transition evalutation context).
    // Used to evaluate action-dependent axpressions.
    public final Firable firedNode;
    
    // The binding of the color variables
    public final ColorVarsBinding colorVarsBinding;

    public EvaluationArguments(AbstractMarking jointState) {
        this.jointState = jointState;
        this.firedNode = null;
        this.colorVarsBinding = null;
    }

    public EvaluationArguments(AbstractMarking jointState, Firable firedNode, ColorVarsBinding colorVarsBinding) {
        this.jointState = jointState;
        this.firedNode = firedNode;
        this.colorVarsBinding = colorVarsBinding;
    }
    
    public static final EvaluationArguments NO_ARGS = new EvaluationArguments(null, null, null);
    
    @Override
    public String toString() {
        String str = "";
        
        if (jointState != null)
            throw new UnsupportedOperationException();
        if (firedNode != null)
            throw new UnsupportedOperationException();
        
        if (colorVarsBinding != null)
            str += colorVarsBinding.toString();
        return str + "@@";
    }
}
