/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.superposition;

import editor.domain.NetPage;
import editor.domain.ProjectPage;
import editor.domain.grammar.ParserContext;
import editor.gui.ResourceFactory;
import java.io.Serializable;
import java.util.ArrayList;
import javax.swing.Icon;

/**
 *
 * @author elvio
 */
public class AlgebraCompositionPage extends MultiNetPage implements Serializable {

    @Override
    public String getPageTypeName() {
        return "ALGEBRACOMPOSITIONPAGE";
    }

    @Override
    public String getOperatorName() {
        return "Algebra";
    }

    @Override
    public Icon getOperatorIcon() {
        return ResourceFactory.getInstance().getSum32();
    }

    @Override public Icon getPageIcon() {
        return ResourceFactory.getInstance().getPageAlgebra16();
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
    public boolean canComposeWith(ProjectPage page) {
        boolean canComp = (page != null) && (page != this) && (page instanceof ComposableNet);
        return canComp;
    }

    @Override
    public boolean useReplicaCount() {
        return false;
    }

    @Override
    public boolean requireParamBinding() {
        return false;
    }
    
    //======================================================================
    
    // do the net composition
    @Override
    protected void compose(ParserContext context) {
        addPageError("Could not compose. Unimplemented.", null);
    }
}
