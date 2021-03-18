/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.composition;

import common.Util;
import editor.domain.Expr;
import editor.domain.NetPage;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.Selectable;
import editor.domain.elements.GspnPage;
import editor.domain.grammar.ParserContext;
import editor.domain.grammar.TemplateBinding;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import editor.domain.measures.SolverParams;
import editor.domain.unfolding.CouldNotUnfoldException;
import editor.domain.unfolding.Unfolding;
import editor.gui.ResourceFactory;
import java.io.Serializable;
import java.util.Map;
import javax.swing.Icon;
import org.w3c.dom.Element;

/**
 *
 * @author elvio
 */
public class UnfoldingCompositionPage extends MultiNetPage implements Serializable {
    
    public final SolverParams.IntExpr dxMult = new SolverParams.IntExpr("3");
    public final SolverParams.IntExpr dyMult = new SolverParams.IntExpr("3");
    
    //======================================================================

    @Override
    public String getPageTypeName() {
        return "UNFOLDINGPAGE";
    }

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
    public boolean canComposeWith(ProjectPage page) {
        boolean canComp = (page != null) && (page != this) && (page instanceof ComposableNet);
        if (canComp) {
            ComposableNet net = (ComposableNet)page;
            NetPage compNet = net.getComposedNet();
            canComp = (compNet != null) && compNet.canBeUnfolded();
        }
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
    
    @Override
    protected void checkPageFieldsCorrectness(boolean isNewOrModified, boolean dependenciesAreOk, ProjectData proj) {        
        // Validate expression ojects
        ParserContext context = new ParserContext(this);
        dxMult.checkExprCorrectness(context, this, horizSelectable);
        dyMult.checkExprCorrectness(context, this, vertSelectable);

    }

    private static Selectable horizSelectable = new Selectable.DummyNamedSelectable("Horizontal multiplier");
    private static Selectable vertSelectable = new Selectable.DummyNamedSelectable("Vertical multiplier");

    @Override
    protected void resetCompositionTargets() {
        setCompositionSuccessfull(null, null, null);
        setCompositionTarget(UNSUCCESSFULL_GSPN_TARGET);
    }
    
    //======================================================================
    // do the net composition
    @Override
    protected void compose(ParserContext context) {
        assert netsDescr.size() == 1;
        GspnPage compNetBase = (GspnPage)netsDescr.get(0).net.getComposedNet();
        
        // Insert the netpage (copying it)
        GspnPage compNet = (GspnPage)Util.deepCopy(compNetBase);

        // Apply parameter substitution
        TemplateBinding effectiveBinding = new TemplateBinding();
        effectiveBinding.binding.putAll(netsDescr.get(0).instParams.binding);
        for (Map.Entry<String, Expr> bind : effectiveBinding.binding.entrySet()) {
            Expr value = bind.getValue();
            value.checkExprCorrectness(context, this, null);
//            effectiveBinding.bindSingleValue(bind.getKey(), bind.getValue());
        }
        substituteParameters(compNet, effectiveBinding);
        
        // Check that all expressions were evaluated correctly
        if (isPageCorrect()) {
            compNet.preparePageCheck();
            compNet.checkPage(null, null, compNet, null);
            if (compNet.isPageCorrect()) {
                try {
                    final Unfolding u = new Unfolding(compNet);
                    u.dxMult = Integer.parseInt(dxMult.getExpr());
                    u.dyMult = Integer.parseInt(dyMult.getExpr()); 
                    
                    u.unfold();
                    String uniqueName = "Unfolding of "+compNet.getPageName();
                    u.unfolded.setPageName(uniqueName);
                    u.unfolded.setSelectionFlag(false);

                    setCompositionSuccessfull(u.unfolded, 
                            new String[]{uniqueName}, new NetPage[]{u.unfolded});
                }
                catch (CouldNotUnfoldException e) {
                    addPageError(e.getMessage(), null);
                }
            }
            else {
                addPageError("Could not pepare "+compNet.getPageName()+" for the unfolding.", null);
            }
        }
//                netNames.toArray(new String[netNames.size()]), 
//                nets.toArray(new NetPage[nets.size()]));
        
//        
//        ArrayList<NetPage> nets = new ArrayList<>();
//        ArrayList<String> netNames = new ArrayList<>();
//        
//        for (NetInstanceDescriptor di : mnPage.netsDescr) {
//            NetPage compNet = di.net.getComposedNet();
//            nets.add(compNet);
//            netNames.add("Unfolding of "+compNet.getPageName());
//        }
//        
//        mnPage.setCompositionSuccessfull(new GspnPage(), 
//                netNames.toArray(new String[netNames.size()]), 
//                nets.toArray(new NetPage[nets.size()]));
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        bindXMLAttrib(this, el, exDir, "mult-dx", "dxMult.@Expr", "3");
        bindXMLAttrib(this, el, exDir, "mult-dy", "dyMult.@Expr", "3");
    }
}
