/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.grammar;

import editor.domain.Node;
import editor.domain.elements.ColorClass;
import java.util.Iterator;

/** A namespace where nodes may be searched for.
 *
 * @author elvio
 */
public interface NodeNamespace {
    
    public Node getNodeByUniqueName(String id);
    
    public Iterator<ColorClass> colorClassIterator();
}
