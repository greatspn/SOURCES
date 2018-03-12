/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.play;

/** Some node that can be fired in the token game.
 *  It could be a GSPN transition or a DTA edge.
 *
 * @author elvio
 */
public interface Firable {
    // Is this firable identified with this name?
    public boolean isIdentifiedAs(String name);
    
    // Does this obecjt fire instantaneously, or in countinuous mode 
    // (like instantaneouse/flow transitions)?
    public boolean isFiringInstantaneous();
    public boolean isFiringFlow();
}
