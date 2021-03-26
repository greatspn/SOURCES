/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.composition;

import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.Selectable;
import editor.domain.Selectable.DummyNamedSelectable;
import editor.domain.ViewProfile;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.Transition;
import editor.domain.grammar.ParserContext;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import editor.domain.measures.SolverParams;
import editor.domain.unfolding.Algebra;
import editor.domain.unfolding.MergePolicy;
import editor.gui.ResourceFactory;
import java.awt.Color;
import java.awt.geom.Rectangle2D;
import java.io.Serializable;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import javax.swing.Icon;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

/**
 *
 * @author elvio
 */
public class TagBasedCompositionPage extends MultiNetPage implements Serializable {

    @Override
    public String getPageTypeName() {
        return "TAGBASEDCOMPOSITIONPAGE";
    }

    @Override
    public String getOperatorName() {
        return "Tag-based composition";
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
    
    // Composition tags
    public Set<String> selTagsP = new HashSet<>();
    public Set<String> selTagsT = new HashSet<>();
    
    // Alignment options
    public static enum Alignment { HORIZONTAL, VERTICAL, CUSTOM }
    public Alignment alignment = Alignment.HORIZONTAL;
    public final SolverParams.IntExpr alignDx = new SolverParams.IntExpr("10");
    public final SolverParams.IntExpr alignDy = new SolverParams.IntExpr("10");
    
    // Use broken edges
    public boolean useBrokenEdges = true;
        
    
    // Tags extracted from the operand nets
    transient public Set<String> commonTagsP;
    transient public Set<String> commonTagsT;

    @Override
    protected void checkPageFieldsCorrectness(boolean isNewOrModified, boolean dependenciesAreOk, ProjectData proj) 
    {
        Set<String> net1TagsP = new HashSet<>();
        Set<String> net1TagsT = new HashSet<>();
        Set<String> net2TagsP = new HashSet<>();
        Set<String> net2TagsT = new HashSet<>();
        commonTagsP = new HashSet<>();
        commonTagsT = new HashSet<>();
        
        // reconstruct the tagLists
        if (!dependenciesAreOk)
            return;
        getNetTags(netsDescr.get(0).net.getComposedNet(), net1TagsP, net1TagsT);
        getNetTags(netsDescr.get(1).net.getComposedNet(), net2TagsP, net2TagsT);

        // Get the intersection of the set of tags
        for (String tag : net1TagsP)
            if (net2TagsP.contains(tag))
                commonTagsP.add(tag);
        for (String tag : net1TagsT)
            if (net2TagsT.contains(tag))
                commonTagsT.add(tag);
        
        // Remove from the selected sets the tags that are not in common
        Iterator<String> it = selTagsP.iterator();
        while (it.hasNext()) {
            String tag = it.next();
            if (!commonTagsP.contains(tag))
                it.remove();
        }
        it = selTagsT.iterator();
        while (it.hasNext()) {
            String tag = it.next();
            if (!commonTagsT.contains(tag))
                it.remove();
        }
        
        // Validate expression ojects
        ParserContext context = new ParserContext(this);
//        System.out.println("alignDx="+alignDx.getExpr());
        alignDx.checkExprCorrectness(context, this, horizSelectable);
//        System.out.println("getNumErrorsAndWarnings()="+getNumErrorsAndWarnings());
        alignDy.checkExprCorrectness(context, this, vertSelectable);
//        for (String tag : commonTagsP)
//            System.out.println(getPageName()+" tagsP="+tag);
    }
    
    private static Selectable horizSelectable = new DummyNamedSelectable("Horizontal offset");
    private static Selectable vertSelectable = new DummyNamedSelectable("Vertical offset");
    
    private void getNetTags(NetPage net, Set<String> tagsP, Set<String> tagsT) {
        if (net == null)
            return;
        for (Node node : net.nodes) {
            if (node instanceof Place) {
                for (int t=0; t<node.numTags(); t++)
                    tagsP.add(node.getTag(t));
            }
            else if (node instanceof Transition) {
                for (int t=0; t<node.numTags(); t++)
                    tagsT.add(node.getTag(t));
            }
        }
    }
    
    //======================================================================
    
    // do the net composition
    @Override
    protected void compose(ParserContext context) {
//        System.out.println("getNumErrorsAndWarnings()="+getNumErrorsAndWarnings());
//        addPageError("Could not compose. Unimplemented.", null);
//            compNet.preparePageCheck();
//            compNet.checkPage(null, null, compNet, null);
//            if (compNet.isPageCorrect()) {
        NetPage net1 = netsDescr.get(0).net.getComposedNet();
        NetPage net2 = netsDescr.get(1).net.getComposedNet();
        if (!(net1 instanceof GspnPage)) {
            addPageError(net1.getPageName()+" is not a GSPN page. Cannot compose", null);
            return;
        }
        if (!(net2 instanceof GspnPage)) {
            addPageError(net2.getPageName()+" is not a GSPN page. Cannot compose", null);
            return;
        }

        int dx2shift = 0, dy2shift = 0;
        Rectangle2D pageBounds1 = net1.getPageBounds();
        switch (alignment) {
            case HORIZONTAL: 
                dx2shift = (int)pageBounds1.getWidth() + 5;
                break;
            case VERTICAL: 
                dy2shift = (int)pageBounds1.getHeight() + 5;
                break;
            case CUSTOM:
                dx2shift = Integer.parseInt(alignDx.getExpr());
                dy2shift = Integer.parseInt(alignDy.getExpr());
                break;
        }
        Algebra a = new Algebra(MergePolicy.BY_TAG, (GspnPage)net1, (GspnPage)net2, 
                selTagsT.isEmpty() ? null : selTagsT.toArray(new String[selTagsT.size()]),
                selTagsP.isEmpty() ? null : selTagsP.toArray(new String[selTagsP.size()]),
                dx2shift, dy2shift, useBrokenEdges, false);
        a.compose();
        a.result.setSelectionFlag(false);
        
        String uniqueName = net1.getPageName()+"+"+net2.getPageName();
        a.result.setPageName(uniqueName);
        
        ViewProfile newProfile = net1.viewProfile.combineWith(net2.viewProfile);

        setCompositionSuccessfull(a.result, newProfile,
                new String[]{uniqueName}, new NetPage[]{a.result});
        
        for (String w : a.warnings) {
            addPageWarning("Composition: "+w, null);
        }
    }
    
    //======================================================================
    
    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir);
        bindXMLAttrib(this, el, exDir, "use-broken-edges", "useBrokenEdges", false);
        bindXMLAttrib(this, el, exDir, "align-dx", "alignDx.@Expr", "10");
        bindXMLAttrib(this, el, exDir, "align-dy", "alignDy.@Expr", "10");
        bindXMLAttrib(this, el, exDir, "alignment", "alignment", null, Alignment.class);
        
        Document doc = exDir.getDocument();
        if (exDir.FieldsToXml()) {
            Element placeTagListElem = doc.createElement("place-tags");
            el.appendChild(placeTagListElem);
            for (String selTag : selTagsP) {
                Element tagElem = doc.createElement("tag");
                tagElem.setAttribute("name", selTag);
                placeTagListElem.appendChild(tagElem);
            }
            
            Element transTagListElem = doc.createElement("transition-tags");
            el.appendChild(transTagListElem);
            for (String selTag : selTagsT) {
                Element tagElem = doc.createElement("tag");
                tagElem.setAttribute("name", selTag);
                transTagListElem.appendChild(tagElem);
            }
            
//            el.setAttribute("alignment", alignment.toString());
        }
        else {
            NodeList placeTagListElemL = el.getElementsByTagName("place-tags");
            if (placeTagListElemL.getLength() > 0) {
                org.w3c.dom.Node placeTagListNode = placeTagListElemL.item(0);
                if (placeTagListNode.getNodeType() == org.w3c.dom.Node.ELEMENT_NODE) {
                    Element placeTagListElem = (Element)placeTagListNode;

                    NodeList tagElemList = placeTagListElem.getElementsByTagName("tag");
                    for (int i=0; i<tagElemList.getLength(); i++) {
                        org.w3c.dom.Node tagElemNode = tagElemList.item(i);
                        if (tagElemNode.getNodeType() == org.w3c.dom.Node.ELEMENT_NODE) {
                            Element tagElem = (Element)tagElemNode;
                            String tag = tagElem.getAttribute("name");
                            if (tag != null) {
                                selTagsP.add(tag);
                            }
                        }
                    }
                }
            }
            
            NodeList transTagListElemL = el.getElementsByTagName("transition-tags");
            if (transTagListElemL.getLength() > 0) {
                org.w3c.dom.Node transTagListNode = transTagListElemL.item(0);
                if (transTagListNode.getNodeType() == org.w3c.dom.Node.ELEMENT_NODE) {
                    Element transTagListElem = (Element)transTagListNode;

                    NodeList tagElemList = transTagListElem.getElementsByTagName("tag");
                    for (int i=0; i<tagElemList.getLength(); i++) {
                        org.w3c.dom.Node tagElemNode = tagElemList.item(i);
                        if (tagElemNode.getNodeType() == org.w3c.dom.Node.ELEMENT_NODE) {
                            Element tagElem = (Element)tagElemNode;
                            String tag = tagElem.getAttribute("name");
                            if (tag != null) {
                                selTagsT.add(tag);
                            }
                        }
                    }
                }
            }
            
//            alignment = Alignment.valueOf(safeParseString(el.getAttribute("alignment"), Alignment.HORIZONTAL.toString()));
        }
    }
}
