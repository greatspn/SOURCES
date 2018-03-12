/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.elements;

import java.util.Set;
import java.util.TreeSet;

/** The signature of a DTA. This object is NOT serializable, and has
 *  t be used as a transient reference.
 *
 * @author elvio
 */
public class DtaSignature {
    public DtaPage dta;
    public Set<String> clockVals = new TreeSet<>();
    public Set<String> actNames = new TreeSet<>();
    public Set<String> stateProps = new TreeSet<>();
    public boolean isUsed = false;
    public boolean isCorrect;

    public DtaSignature(DtaPage dta, boolean isCorrect) {
        this.dta = dta;
        this.isCorrect = isCorrect;
    }
}
