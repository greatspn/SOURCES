/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.superposition;

import editor.domain.ProjectPage;
import editor.domain.grammar.ParserContext;
import javax.swing.Icon;

/**
 *
 * @author elvio
 */
public interface CompositionOperator {
    
    public String getOperatorName();
    public Icon getOperatorIcon();
    public Icon getPageIcon();
    
    // Is the number of composed nets fixed or variable?
    public boolean hasFixedNumOfOperators();
    
    // Get the fixed number of composed nets
    public int getFixedNumOfOperators();
    
    // Is this net composable?
    public boolean canComposeWith(ProjectPage page, MultiNetPage resultPage);
    
    // Is net replication count available
    public boolean useReplicaCount();
    
    // Net requires to instantiate color class parameters?
    public boolean requireParamBinding();
    
    // do the net composition
    public void compose(MultiNetPage mnPage, ParserContext context);
}
