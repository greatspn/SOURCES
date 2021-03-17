/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.superposition;

import editor.domain.NetPage;
import editor.domain.ProjectPage;
import editor.domain.elements.GspnPage;
import editor.domain.grammar.ParserContext;
import editor.gui.ResourceFactory;
import java.io.Serializable;
import java.util.ArrayList;
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
    
    @Override
    public boolean requireParamBinding() {
        return true;
    }
    
    //======================================================================
    // do the net composition
    public void compose(MultiNetPage mnPage, ParserContext context) {
        ArrayList<NetPage> nets = new ArrayList<>();
        ArrayList<String> netNames = new ArrayList<>();
        
        for (NetInstanceDescriptor di : mnPage.netsDescr) {
            NetPage compNet = di.net.getComposedNet();
            nets.add(compNet);
            netNames.add("Unfolding of "+compNet.getPageName());
        }
        
        mnPage.setCompositionSuccessfull(new GspnPage(), 
                netNames.toArray(new String[netNames.size()]), 
                nets.toArray(new NetPage[nets.size()]));
    }
}
