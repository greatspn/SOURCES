/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.superposition;

import editor.domain.ProjectPage;
import editor.gui.ResourceFactory;
import java.io.Serializable;
import javax.swing.Icon;

/**
 *
 * @author elvio
 */
public class AlgebraCompositionOperator implements CompositionOperator, Serializable {

    @Override
    public String getOperatorName() {
        return "Algebra";
    }

    @Override
    public Icon getOperatorIcon() {
        return ResourceFactory.loadIcon32("sum");
    }

    
    
    @Override
    public boolean hasFixedNumOfOperators() {
        return true;
    }

    @Override
    public int getFixedNumOfOperators() {
        return 2;
    }
    
    @Override
    public boolean canComposeWith(ProjectPage page, MultiNetPage resultPage) {
        boolean canComp = (page != null) && (page != resultPage) && (page instanceof ComposableNet);
        return canComp;
    }

    @Override
    public boolean useReplicaCount() {
        return false;
    }
    
    
}
