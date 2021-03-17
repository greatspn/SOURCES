/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.superposition;

import editor.domain.ProjectPage;
import javax.swing.Icon;

/**
 *
 * @author elvio
 */
public interface CompositionOperator {
    
    public String getOperatorName();
    public Icon getOperatorIcon();
    
    // Is the number of composed nets fixed or variable?
    public boolean hasFixedNumOfOperators();
    
    // Get the fixed number of composed nets
    public int getFixedNumOfOperators();
    
    // Is this net composable?
    public boolean canComposeWith(ProjectPage page, MultiNetPage resultPage);
    
    // Is net replication count available
    public boolean useReplicaCount();
}
