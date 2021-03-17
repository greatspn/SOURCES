/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.superposition;

import common.Util;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.ViewProfile;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.Transition;
import editor.domain.grammar.ParserContext;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import editor.domain.unfolding.Algebra;
import editor.gui.ResourceFactory;
import java.awt.Dimension;
import java.io.Serializable;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import javax.swing.Icon;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

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
    
    public Set<String> selTagsP = new HashSet<>();
    public Set<String> selTagsT = new HashSet<>();
    
    transient public Set<String> commonTagsP;
    transient public Set<String> commonTagsT;

    @Override
    protected boolean checkPageCorrectness(boolean isNewOrModified, ProjectData proj, 
            Set<ProjectPage> changedPages, ProjectPage invokerPage) 
    {
        boolean res = super.checkPageCorrectness(isNewOrModified, proj, changedPages, invokerPage); 
        
        Set<String> net1TagsP = new HashSet<>();
        Set<String> net1TagsT = new HashSet<>();
        Set<String> net2TagsP = new HashSet<>();
        Set<String> net2TagsT = new HashSet<>();
        commonTagsP = new HashSet<>();
        commonTagsT = new HashSet<>();
        
        // reconstruct the tagLists
        if (netsDescr.size()!=2) 
            return res;    
        if (netsDescr.get(0).net != null)
            getNetTags(netsDescr.get(0).net.getComposedNet(), net1TagsP, net1TagsT);
        if (netsDescr.get(1).net != null)
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
        
//        for (String tag : commonTagsP)
//            System.out.println(getPageName()+" tagsP="+tag);
        
        return res;
    }
    
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
//        addPageError("Could not compose. Unimplemented.", null);
        GspnPage net1 = (GspnPage)netsDescr.get(0).net.getComposedNet();
        GspnPage net2 = (GspnPage)netsDescr.get(1).net.getComposedNet();

        int dx2shift = 20;
        int dy2shift = 20;
        boolean propBrokenEdges = true;
        Algebra a = new Algebra(net1, net2, 
                selTagsT.isEmpty() ? null : selTagsT.toArray(new String[selTagsT.size()]),
                selTagsP.isEmpty() ? null : selTagsP.toArray(new String[selTagsP.size()]),
                dx2shift, dy2shift, propBrokenEdges, false);
        a.compose();
        
        String uniqueName = net1.getPageName()+"+"+net2.getPageName();
        a.result.setPageName(uniqueName);

        setCompositionSuccessfull(a.result, 
                new String[]{uniqueName}, new NetPage[]{a.result});
        
        for (String w : a.warnings) {
            addPageWarning("Composition: "+w, null);
        }
    }
    
    //======================================================================
    
    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
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
        }
    }
}
