/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.composition;

import common.Util;
import editor.domain.Edge;
import editor.domain.Expr;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.ViewProfile;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.TextBox;
import editor.domain.elements.Transition;
import editor.domain.grammar.ParserContext;
import editor.domain.grammar.TemplateBinding;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import editor.domain.unfolding.Algebra;
import editor.domain.unfolding.MergePolicy;
import editor.gui.ResourceFactory;
import java.awt.Color;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import javax.swing.Icon;
import org.w3c.dom.Element;

/**
 *
 * @author elvio
 */
public class NameBasedCompositionPage extends MultiNetPage implements Serializable {

    @Override
    public String getPageTypeName() {
        return "NAMEBASEDCOMPOSITIONPAGE";
    }
    
    @Override
    public String getOperatorName() {
        return "Name-based Composition";
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
    public boolean canComposeWith(ProjectPage page) {
        boolean canComp = (page != null) && (page != this) && (page instanceof ComposableNet);
        return canComp;
    }

    @Override
    public boolean useReplicaCount() {
        return true;
    }
    
    @Override
    public boolean requireParamBinding() {
        return true;
    }
    
    @Override
    protected void checkPageFieldsCorrectness(boolean isNewOrModified, boolean dependenciesAreOk, ProjectData proj) {        
        // nothing to check...
    }
    
    @Override
    protected void resetCompositionTargets() {
        setCompositionSuccessfull(null, null, null, null);
        setCompositionTarget(UNSUCCESSFULL_GSPN_TARGET);
    }
    
    //======================================================================

    // The 'flattened' list of components in this multipage, which is what is actually shown
    // Note:  flattenedSubNets.size() >= netsDescr.size()
    private transient ArrayList<NetPage> flattenedSubNets;
    private transient ArrayList<String> flattenedSubNetNames;
    
    private transient Set<String> placeNames;
    private transient Set<String> transNames;
    
    private transient Map<String, Color> colorOfPlace;
    private transient Map<String, Color> colorOfTrans;
    private transient int placePalIndex, transPalIndex;
    
    private void updateMergeNames(NetPage net) {
        for (Node node : net.nodes) {
            if (node instanceof Place) {
                if (!placeNames.contains(node.getUniqueName()))
                    placeNames.add(node.getUniqueName());
                else {
                    if (!colorOfPlace.containsKey(node.getUniqueName()))
                        colorOfPlace.put(node.getUniqueName(), 
                                BLUE_PALETTE[(placePalIndex++) % BLUE_PALETTE.length]);
                }
            }
            else if (node instanceof Transition) {
                if (!transNames.contains(node.getUniqueName()))
                    transNames.add(node.getUniqueName());
                else {
                    if (!colorOfTrans.containsKey(node.getUniqueName()))
                        colorOfTrans.put(node.getUniqueName(), 
                                RED_PALETTE[(transPalIndex++) % RED_PALETTE.length]);
                }
            }
        }
    }
    
    @Override
    public Color colorOfMergedNode(Node node) {
        if (node instanceof Place && colorOfPlace!=null && colorOfPlace.containsKey(node.getUniqueName()))
            return colorOfPlace.get(node.getUniqueName());
        else if (node instanceof Transition && colorOfTrans!=null && colorOfTrans.containsKey(node.getUniqueName()))
            return colorOfTrans.get(node.getUniqueName());
        else 
            return null;
    }
    
    //======================================================================

    // do the net composition
    @Override
    protected void compose(ParserContext context) {
        flattenedSubNets = new ArrayList<>();
        flattenedSubNetNames = new ArrayList<>();

        TemplateBinding rootBinding = new TemplateBinding();
        enumComponents("", this, rootBinding, context);
        
        placeNames = new HashSet<>();
        transNames = new HashSet<>();
        colorOfPlace = new HashMap<>();
        colorOfTrans = new HashMap<>();
        placePalIndex = transPalIndex = 0;

        for (NetPage subnet : flattenedSubNets) {
            subnet.preparePageCheck();
            subnet.checkPage(null, null, subnet, null);
        }
        
        for (int i=0; i<netsDescr.size(); i++) {
            ComposableNet net = netsDescr.get(i).net;
            String name = netsDescr.get(i).targetNetName;
            if (net == null) {
                addPageError(name+" is missing. Cannot compose", null);
            }
            else if (net.getComposedNet() == null) {
                addPageError(name+" is not a valid GSPN page. Cannot compose", null);
            }
            else if (!(net.getComposedNet() instanceof GspnPage)) {
                addPageError(name+" is not a GSPN page. Cannot compose", null);
                return;
            }
        }
        
        NetPage composedNet = UNSUCCESSFULL_GSPN_TARGET;
        ViewProfile newProfile = viewProfile;
        final int ptMoveDx = 1, ptMoveDy = 1;
        final int textBoxExtraWidth = 2, textBoxExtraHeight = 3;
        int posX = 1, posY = 1;
        if (flattenedSubNets.size() > 0 && isPageCorrect()) {
            composedNet = (NetPage)Util.deepCopy(flattenedSubNets.get(0));
            newProfile = composedNet.viewProfile;
            composedNet.preparePageCheck();
            composedNet.checkPage(null, null, composedNet, null);
            composedNet.setPageName(flattenedSubNetNames.get(0));
            updateMergeNames(composedNet);
            Rectangle2D composedPageBounds = composedNet.computeIntegerPageBounds();
//            System.out.println("composedPageBounds="+composedPageBounds+"\n");
            double xShift = posX - composedPageBounds.getX() + ptMoveDx;
            double yShift = posY - composedPageBounds.getY() + ptMoveDy;
            for (Node node : composedNet.nodes)
                node.getCenterHandle(composedNet).moveBy(xShift, yShift, true);
            for (Edge edge : composedNet.edges)
                edge.getCenterHandle(composedNet).moveBy(xShift, yShift, true);
            
            // Wrap the composedNet with the TextBox            
            TextBox textBox = new TextBox(flattenedSubNetNames.get(0), 
                    new Point2D.Double(posX, posY), 
                    composedNet.generateUniqueNodeName(true, "__textBox"));
            textBox.setWidth(composedPageBounds.getWidth()+textBoxExtraWidth);
            textBox.setHeight(composedPageBounds.getHeight()+textBoxExtraHeight);
            textBox.getLockEditable().setValue(null, null, Boolean.TRUE);
            textBox.getBorderColorEditable().setValue(null, null, new Color(0, 64, 255));
            composedNet.nodes.add(textBox);
//            System.out.println("composedPageBounds="+composedPageBounds+"\n");
            final int stepX = 2;
            posX += composedPageBounds.getWidth() + textBoxExtraWidth + stepX;
//            System.out.println("posX = "+posX);
            
            if (!composedNet.isPageCorrect()) {
                addPageError("Could not prepare "+composedNet.getPageName()+" for the composition.", null);
            }
            else {            
                for (int index=1; index<flattenedSubNets.size() && isPageCorrect(); index++) {
                    NetPage ithNet = flattenedSubNets.get(index);
                    Rectangle2D ithPageBounds = ithNet.computeIntegerPageBounds();
                    
                    int dx2shift = posX - (int)ithPageBounds.getX() + ptMoveDx;
                    int dy2shift = posY - (int)ithPageBounds.getY() + ptMoveDy;
//                    System.out.println("ithPageBounds="+ithPageBounds);
//                    System.out.println("dx2shift="+dx2shift+" dy2shift="+dy2shift);
                    boolean useBrokenEdges = true;
                    Algebra a = new Algebra(MergePolicy.BY_NAME, (GspnPage)composedNet, (GspnPage)ithNet, 
                            null, null, dx2shift, dy2shift, useBrokenEdges, false);
                    a.compose();
                    a.result.setSelectionFlag(false);
                    
                    String uniqueName = composedNet.getPageName()+"+"+flattenedSubNetNames.get(index);
                    a.result.setPageName(uniqueName);
                    for (String w : a.warnings) {
                        addPageWarning("Composition with "+ithNet.getPageName()+": "+w, null);
                    }
                    composedNet = a.result;
                    newProfile = newProfile.combineWith(ithNet.viewProfile);
                    composedNet.preparePageCheck();
                    composedNet.checkPage(null, null, composedNet, null);
                    if (!composedNet.isPageCorrect()) {
                        addPageError("Could not compose with "+flattenedSubNetNames.get(index)+".", null);
                    }
                    updateMergeNames(ithNet);
                    
                    // Wrap the ithNet in the composedNet with a new TextBox
                    textBox = new TextBox(flattenedSubNetNames.get(index), 
                            new Point2D.Double(posX, posY), 
                            composedNet.generateUniqueNodeName(true, "__textBox"));
                    textBox.setWidth(ithPageBounds.getWidth()+textBoxExtraWidth);
                    textBox.setHeight(ithPageBounds.getHeight()+textBoxExtraHeight);
                    textBox.getLockEditable().setValue(null, null, Boolean.TRUE);
                    textBox.getBorderColorEditable().setValue(null, null, new Color(0, 64, 255));
                    composedNet.nodes.add(textBox);
                    
                    // finally, update the composed Net bounds
                    composedPageBounds = composedNet.computeIntegerPageBounds();
                    posX += ithPageBounds.getWidth()+ textBoxExtraWidth + stepX;
//                    System.out.println("composedPageBounds="+composedPageBounds+"\n");
//                    System.out.println("posX = "+posX);
                }
            }
        }
        
        if (!isPageCorrect())
            composedNet = UNSUCCESSFULL_GSPN_TARGET;

        setCompositionSuccessfull(composedNet, newProfile,
                flattenedSubNetNames.toArray(new String[flattenedSubNetNames.size()]), 
                flattenedSubNets.toArray(new NetPage[flattenedSubNets.size()]));

        placeNames = null;
        transNames = null;
//
////        GspnPage gspn = new GspnPage();
////        gspn.setPageName("comp");
//        setCompositionSuccessfull(null,//gspn, 
//                flattenedSubNetNames.toArray(new String[flattenedSubNetNames.size()]), 
//                flattenedSubNets.toArray(new NetPage[flattenedSubNets.size()]));
//        addPageWarning("Composition is unimplemented.", null);
    }
    
    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir);
    }
    
    
    // The procedure that enumerates (and flattens) the hierarchical structure of subnets,
    // with net copying, renaming and partial parameter instantiation
    private void enumComponents(String prefix, MultiNetPage thisPage,
                                TemplateBinding binding,
                                ParserContext multiPageContext) 
    {
//        assert nets != null && numReplicas >= 1 && instParams != null;
        Set<String> tvarWarnSet = new HashSet<>();
        // Are we enumerating the nets that will be composed (the first level in the hierarchy)?
        final boolean isFirstLevel = (prefix.isEmpty());
        
        for (NetInstanceDescriptor descr : thisPage.netsDescr) {
            assert descr.net != null;
            
            // Try to evaluate the replica count
            Integer numReplicas = descr.evalNumReplicas(multiPageContext);
//            if (numReplicas == null) { 
//                // Parametric count
//                areReplicaCountParametric = true;
//                
//                // Add the first instantiated subnet, for parameter instantiation.
//                throw new UnsupportedOperationException("To implement.");
//            }
//            else {
            if (numReplicas == null || numReplicas < 1) {
                thisPage.addPageError("Cannot have "+numReplicas
                        +" replicas of net \""+descr.targetNetName+"\".", null);
                continue;
            }
            String netPrefixBase = uniqueNameForSubNet(thisPage, descr);
            for (int inst=0; inst < numReplicas; inst++) {
                // List the parameter instantiations
                TemplateBinding effectiveBinding = new TemplateBinding();
                effectiveBinding.binding.putAll(binding.binding);
                for (Map.Entry<String, Expr> bind : descr.getBindingOfReplica(inst).binding.entrySet()) {
                    if (effectiveBinding.binding.containsKey(bind.getKey())) {
                        if (!tvarWarnSet.contains(bind.getKey())) {
                            thisPage.addPageWarning("Template parameter instantiation of "+bind.getKey()+
                                    " is hidden by an internal instantiation.", null);
                            tvarWarnSet.add(bind.getKey());
                        }
                    }
                    Expr value = bind.getValue();
//                        if (value.contains("$$"))
//                            value = value.replaceAll("$$", ""+inst);
                    effectiveBinding.bindSingleValue(bind.getKey(), bind.getValue());
                }

                // Generate the subnet name
                String netPrefix = netPrefixBase;
                if (numReplicas > 1)
                    netPrefix += "[" + inst + "]";
                if (!prefix.isEmpty())
                    netPrefix = prefix + "." + netPrefix;
//                if (isFirstLevel)
//                    subNetPrefixes.add(netPrefix);

                // Compose hierarchically
                if (descr.net instanceof NameBasedCompositionPage) {
                    NameBasedCompositionPage mncp = (NameBasedCompositionPage)descr.net;
                    
//                    if (isFirstLevel)
//                        compSubNets.add(compPage);
                    enumComponents(netPrefix, mncp, 
                                   effectiveBinding, multiPageContext);
                }
                else /*(descr.net instanceof NetPage)*/ {
                    // Insert the netpage (copying it)
                    NetPage npage = (NetPage)Util.deepCopy(descr.net.getComposedNet());

                    // Apply parameter substitution
                    thisPage.substituteParameters(npage, effectiveBinding);

                    flattenedSubNets.add(npage);
                    flattenedSubNetNames.add(netPrefix);
//                    if (isFirstLevel)
//                        compSubNets.add(npage);
                }
//                else if (descr.net instanceof MultiNetPage) {
//                    // Discend recursively
//                    MultiNetPage mnpage = (MultiNetPage)descr.net;
//                    NetPage compPage = (NetPage)Util.deepCopy(mnpage.getComposedNet());
//                    assert compPage != null;
//
//                    // Apply parameter substitution
//                    thisPage.substituteParameters(compPage, effectiveBinding);
//
//                    if (isFirstLevel)
//                        compSubNets.add(compPage);
//                    enumComponents(netPrefix, thisPage, 
//                                   effectiveBinding, multiPageContext);
//                }
//                else throw new UnsupportedOperationException("Unknown composable net!");
            }
        }
//        }
    }
    
    // Generate a unique name for the net descriptor @netsDescr[i]
    // Will be thenet name if the net is not repeated in the multinetpage definition,
    // or will be netName/num if it repeats multiple times.
    private String uniqueNameForSubNet(MultiNetPage thisPage, NetInstanceDescriptor di) {
        boolean isRepeated = false;
        for (NetInstanceDescriptor descr : thisPage.netsDescr)
            if (descr != di)
                if (descr.net == di.net)
                    isRepeated = true;
        if (!isRepeated)
            return di.targetNetName;
        // count copies before the i-th net
        int prevInst = 0;
        for (int j=0; thisPage.netsDescr.get(j) != di; j++)
            if (thisPage.netsDescr.get(j).net == di.net)
                prevInst++;
        return di.targetNetName+"/"+prevInst;
    }
    
}
