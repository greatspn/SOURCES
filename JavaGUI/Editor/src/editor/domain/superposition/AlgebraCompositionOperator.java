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
public class AlgebraCompositionOperator implements CompositionOperator, Serializable {

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
    public boolean canComposeWith(ProjectPage page, MultiNetPage resultPage) {
        boolean canComp = (page != null) && (page != resultPage) && (page instanceof ComposableNet);
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
    public void compose(MultiNetPage mnPage, ParserContext context) {
        mnPage.addPageError("Could not compose. Unimplemented.", null);
    }
}
