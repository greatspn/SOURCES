/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.play;

import editor.domain.grammar.ColorVarsBinding;
import java.util.HashSet;
import java.util.Set;

/** A firable event with its color variable bindings.
 *
 * @author elvio
 * @param <FIR> The type of the firable event.
 */
public class FirableWithBindings<FIR extends Firable> {
    // The event that may fire
    public final FIR  firable;
    
    // The color variable bindings (or null if it is uncolored).
    public final Set<ColorVarsBinding> bindings = new HashSet<>();
    
    public FirableWithBindings(FIR firable) {
        this.firable = firable;
    }
}
