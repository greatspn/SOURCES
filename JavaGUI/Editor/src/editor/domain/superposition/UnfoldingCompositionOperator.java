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
public class UnfoldingCompositionOperator implements CompositionOperator, Serializable {

    @Override
    public String getOperatorName() {
        return "Unfolding";
    }

    @Override
    public Icon getOperatorIcon() {
        return ResourceFactory.getInstance().getUnfolding32();
    }

    @Override public Icon getPageIcon() {
        return ResourceFactory.getInstance().getPageUnfolding16();
    }
    
    @Override
    public boolean hasFixedNumOfOperators() {
        return true;
    }

    @Override
    public int getFixedNumOfOperators() {
        return 1;
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
