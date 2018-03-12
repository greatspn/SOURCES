/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.superposition;

import editor.domain.NetPage;
import editor.domain.elements.TemplateVariable;
import editor.domain.grammar.TemplateBinding;
import java.util.Iterator;
import java.util.Set;

/** A net page that is composable (by P/T superposition) into other net pages.
 *
 * @author elvio
 */
public interface ComposableNet {
    
    // Iterate over subnets
    public Iterator<ComposableNet> subnetsIterator();
    public int numSubnets();
    public String getPageName();
    
    // Iterate over groups
    public Iterator<NodeGroup> groupIterator();
    public int numNodeGroups();
    
    // Enumerate parameters
    public Set<TemplateVariable> enumerateParams();
    
    // Replace parameters with constant values
    public void instantiateParams(TemplateBinding binding);
    
    // Get the single (flattened) composed net.
    public NetPage getComposedNet();
    // Tell is this net contains subnets
    public boolean hasSubnets();
}
