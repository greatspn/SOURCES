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
public class MultiNetCompositionOperator implements CompositionOperator, Serializable {

    @Override
    public String getOperatorName() {
        return "MultiNet Composition";
    }

    @Override
    public Icon getOperatorIcon() {
        return ResourceFactory.loadIcon32("page_multinet");
    }

    @Override public Icon getPageIcon() {
        return ResourceFactory.getInstance().getPageMultiNet16();
    }
    
    @Override
    public boolean hasFixedNumOfOperators() {
        return false;
    }

    @Override
    public int getFixedNumOfOperators() {
        throw new IllegalStateException();
    }
    
    @Override
    public boolean canComposeWith(ProjectPage page, MultiNetPage resultPage) {
        boolean canComp = (page != null) && (page != resultPage) && (page instanceof ComposableNet);
        return canComp;
    }

    @Override
    public boolean useReplicaCount() {
        return true;
    }
    
    
}
