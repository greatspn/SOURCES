/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.composition;

import common.Util;
import editor.domain.Expr;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.Selectable;
import editor.domain.elements.GspnPage;
import editor.domain.grammar.ParserContext;
import editor.domain.grammar.TemplateBinding;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import editor.domain.measures.SolverParams;
import editor.domain.unfolding.CouldNotUnfoldException;
import editor.domain.unfolding.Unfolding;
import editor.gui.ResourceFactory;
import java.awt.Color;
import java.io.Serializable;
import java.util.Map;
import javax.swing.Icon;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

/**
 *
 * @author elvio
 */
public class UnfoldingCompositionPage extends MultiNetPage implements Serializable {
    
    public final SolverParams.IntExpr[] dxMult;
    public final SolverParams.IntExpr[] dyMult;
    
    //======================================================================
    
    public static final int NUM_OFFSET_ROWS = 4;
    
    public UnfoldingCompositionPage() {
        dxMult = new SolverParams.IntExpr[NUM_OFFSET_ROWS];
        dyMult = new SolverParams.IntExpr[NUM_OFFSET_ROWS];
        for (int i=0; i<NUM_OFFSET_ROWS; i++) {
            dxMult[i] = new SolverParams.IntExpr();
            dyMult[i] = new SolverParams.IntExpr();
        }
        resetOffsetMatrix();
    }
    
    public void resetOffsetMatrix() {
        for (int i=0; i<NUM_OFFSET_ROWS; i++) {
            dxMult[i].setExpr(i%2==0 ? "0" : "3");
            dyMult[i].setExpr(i%2==0 ? "3" : "0");
        }
    }

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
    public boolean useTagRewritingRules() {
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
        
        for (SolverParams.IntExpr expr : dxMult)
            expr.checkExprCorrectness(context, this, horizSelectable);
        for (SolverParams.IntExpr expr : dyMult)
            expr.checkExprCorrectness(context, this, vertSelectable);

    }

    private static final Selectable horizSelectable = new Selectable.DummyNamedSelectable("Horizontal multiplier");
    private static final Selectable vertSelectable = new Selectable.DummyNamedSelectable("Vertical multiplier");

    @Override
    protected void resetCompositionTargets() {
        setCompositionSuccessfull(null, null, null, null);
        setCompositionTarget(UNSUCCESSFULL_GSPN_TARGET);
    }
    
    @Override
    public Color colorOfMergedNode(Node node) {
        return null;
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
                    double[][] gfxMultipliers = { 
                        { Integer.parseInt(dxMult[0].getExpr()), Integer.parseInt(dyMult[0].getExpr()) }, 
                        { Integer.parseInt(dxMult[1].getExpr()), Integer.parseInt(dyMult[1].getExpr()) },
                        { Integer.parseInt(dxMult[2].getExpr()), Integer.parseInt(dyMult[2].getExpr()) },
                        { Integer.parseInt(dxMult[3].getExpr()), Integer.parseInt(dyMult[3].getExpr()) }
                    };
                    u.gfxMultipliers = gfxMultipliers;
//                    for (int r=0; r<gfxMultipliers.length; r++) {
//                        for (int c=0; c<gfxMultipliers[r].length; c++)
//                            System.out.print(gfxMultipliers[r][c]+" ");
//                        System.out.println("");
//                    }
                    
                    u.unfold();
                    String uniqueName = "Unfolding of "+compNet.getPageName();
                    u.unfolded.setPageName(uniqueName);
                    u.unfolded.setSelectionFlag(false);
                    u.unfolded.viewProfile.copyFrom(compNet.viewProfile);

                    setCompositionSuccessfull(u.unfolded, compNet.viewProfile,
                            new String[]{uniqueName}, new NetPage[]{u.unfolded});
                }
                catch (CouldNotUnfoldException e) {
                    addPageError(e.getMessage(), null);
                }
            }
            else {
                addPageError("Could not prepare "+compNet.getPageName()+" for the unfolding.", null);
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
        super.exchangeXML(el, exDir);
        Document doc = exDir.getDocument();
        if (exDir.FieldsToXml()) {
            Element offsetListElem = doc.createElement("offset-list");
            el.appendChild(offsetListElem);
            for (int i=0; i<NUM_OFFSET_ROWS; i++) {
                Element offsetElem = doc.createElement("offset");
                offsetElem.setAttribute("dx", dxMult[i].getExpr());
                offsetElem.setAttribute("dy", dyMult[i].getExpr());
                offsetListElem.appendChild(offsetElem);
            }
        }
        else {
            NodeList offsetListElemL = el.getElementsByTagName("offset-list");
            if (offsetListElemL.getLength() > 0) {
                org.w3c.dom.Node offsetListNode = offsetListElemL.item(0);
                if (offsetListNode.getNodeType() == org.w3c.dom.Node.ELEMENT_NODE) {
                    Element offsetListElem = (Element)offsetListNode;

                    NodeList tagElemList = offsetListElem.getElementsByTagName("offset");
                    int j = 0;
                    for (int i=0; i<tagElemList.getLength() && j<NUM_OFFSET_ROWS; i++) {
                        org.w3c.dom.Node tagElemNode = tagElemList.item(i);
                        if (tagElemNode.getNodeType() == org.w3c.dom.Node.ELEMENT_NODE) {
                            Element tagElem = (Element)tagElemNode;
                            String tag = tagElem.getAttribute("dx");
                            if (tag != null) {
                                dxMult[j].setExpr(tag);
                            }
                            tag = tagElem.getAttribute("dy");
                            if (tag != null) {
                                dyMult[j].setExpr(tag);
                            }
                            j++;
                        }
                    }
                }
            }
        }
    }
}
