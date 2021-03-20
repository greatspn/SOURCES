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
public class MultiNetCompositionPage extends MultiNetPage implements Serializable {

    @Override
    public String getPageTypeName() {
        return "MULTINETCOMPOSITIONPAGE";
    }
    
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
        setCompositionSuccessfull(null, viewProfile, null, null);
        setCompositionTarget(UNSUCCESSFULL_GSPN_TARGET);
    }
    
    //======================================================================

    // The list of nets that will be composed. Each of these nets could be a
    // simple NetPage, or the composed NetPage of a MultiNetPage.
    // Note:  compSubNets.size() == netsDescr.size()
//    private transient ArrayList<NetPage> compSubNets;
//    private transient ArrayList<String> subNetPrefixes;
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
    
//    private transient MultiNetPage mnPage;

    // do the net composition
    @Override
    protected void compose(ParserContext context) {
//        compSubNets = new ArrayList<>();
//        subNetPrefixes = new ArrayList<>();
        flattenedSubNets = new ArrayList<>();
        flattenedSubNetNames = new ArrayList<>();

        TemplateBinding rootBinding = new TemplateBinding();
        enumComponents("", this, rootBinding, context);
        
//        // Compose the subnets into a single net
//        final int numSubNets = compSubNets.size();
//        compData = new CompositionData[numSubNets];
//        for (int i=0; i<numSubNets; i++)
//            compData[i] = new CompositionData();
//        
//        // Enumerate node classes
//        Set<GroupClass> allClasses = new HashSet<>();
//        for (NetPage subnet: compSubNets)
//            for (Node node : subnet.nodes)
//                allClasses.add(node.getGroupClass());
//        
//        // Compose nodes, by classes
//        for (GroupClass grClass : allClasses)
//            composeNodesOfType(grClass);
        
        // Clear composition data
//        compData = null;

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
                    
//                    int dx2shift = (int)composedPageBounds.getWidth() + 2*textBoxExtraWidth + stepX + moveDx - (int)ithPageBounds.getX();
//                    int dy2shift = moveDy - (int)ithPageBounds.getY();//(int)pageBounds1.getHeight() + 5;
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
    }
    
//    // A visual component (a single unit of the multinet)
//    private static class NetPageUnit extends NetUnit {
//        // Elements in this component
//        public NetPage net;
//    }
    
//    // A synchronization unit, which is made by several components and the
//    // synchronization of nodes/edges
//    private static class NetUnit {
//        // Name/prefix of this component
//        public String prefix;
//        // sub-components of this component. Could be null for page units
//        public NetUnit[] subUnits;
//        // How nodes interacts between the sub-units
//        public ArrayList<NodeGroup> gNodes;
//        // How edges interact between the sub-units
//        
//        // Renaming table
//        Map<String, String> renaming = new HashMap<>();
//    }

//    // Support data for net composition
//    private static class CompositionData {
//        // ID conversion table
//        Map<String, String> idConv = new HashMap<>();
//    }
//    private transient CompositionData[] compData;
    

    //    private static class TaggedNodes {
//        // All nodes that share the same tag, divided by subnets.
//        LinkedList<Node>[] nodesBySubnet;
//
//        public TaggedNodes(int numSubnets) {
//            @SuppressWarnings("unchecked") LinkedList<Node>[] ll = new LinkedList[numSubnets];
//            this.nodesBySubnet = ll;
//        }
//    }
//    
//    private void insertNodeByTag(Map<String, TaggedNodes> table, Node node, String tag, int subnetId) {
//        TaggedNodes tn = table.get(tag);
//        if (tn == null) {
//            tn = new TaggedNodes(compSubNets.size());
//            table.put(tag, tn);
//        }
//        if (tn.nodesBySubnet[subnetId] == null)
//            tn.nodesBySubnet[subnetId] = new LinkedList<>();
//        tn.nodesBySubnet[subnetId].add(node);
//    }
//    
//    private static final String NON_COMPOSABLE = "@#!";
    
//    // Do simple node composition: nodes with the same name and type are fused toghether,
//    // and are assigned to a new unique name. Otherwise, they remain separated, and their
//    // names are prefixed with the subnet name.
//    private void composeNodesOfType(GroupClass type) {
//        System.out.println("composeNodesOfType "+type);
//        Map<String, TaggedNodes> taggedNode = new HashMap<>();
//        
//        int subnetId = 0;
//        for (NetPage subnet: compSubNets) {
//            for (Node node : subnet.nodes) {
//                GroupClass nodeType = node.getGroupClass();
//                if (nodeType != type)
//                    continue;
//                // Determine the node tag
//                String tag;
//                if (node.hasSuperPosTags()) {
//                    if (node.numTags() == 0)
//                        tag = NON_COMPOSABLE;
//                    else {
//                        if (node.numTags() > 1)
//                            addPageWarning("Multiple node tags are not supported.", node);
//                        tag = node.getTag(0);
//                    }
//                }
//                else
//                    tag = node.getUniqueName();
//                
//                // Insert the node in the composition structure, by tag.
//                insertNodeByTag(taggedNode, node, tag, subnetId);
//            }
//            subnetId++;
//        }
//        
//        for (Map.Entry<String, TaggedNodes> e : taggedNode.entrySet()) {
//            System.out.print(type+" "+e.getKey()+" ");
//            for (LinkedList<Node> l : e.getValue().nodesBySubnet) {
//                if (l == null)
//                    System.out.print("null ");
//                else {
//                    System.out.print("[");
//                    for (Node n : l) {
//                        System.out.print(n.getUniqueName()+" ");
//                    }
//                    System.out.print("] ");
//                }
//            }
//            System.out.println("");
//        }
//        
//        // Generate the node synchronization
//        Node[] synch = new Node[compSubNets.size()];
//        for (Map.Entry<String, TaggedNodes> e : taggedNode.entrySet()) {
//            if (e.getKey().equals(NON_COMPOSABLE))
//                continue;
//            generateSynchRecursively(e.getValue(), synch, e.getKey(), 0);
//        }
//        
//        // Copy non-synchronized nodes
//        TaggedNodes nonSynch = taggedNode.get(NON_COMPOSABLE);
//        if (nonSynch != null) {
//            for (int i=0; i<compSubNets.size(); i++) {
//                if (nonSynch.nodesBySubnet[i] != null) {
//                    for (Node nn : nonSynch.nodesBySubnet[i]) {
//                        synch[i] = nn;
//                        Node newNode = createNode(synch);
//                        compNet.nodes.add(newNode);
//                        synch[i] = null;
//                    }
//                }
//            }
//        }
//    }
//    
//    private void generateSynchRecursively(TaggedNodes tn, Node[] synch, String tag, int level) {
//        if (level == compSubNets.size()) {
//            Node newNode = createNode(synch);
//            compNet.nodes.add(newNode);
//        }
//        else {
//            if (tn.nodesBySubnet[level] == null)
//                generateSynchRecursively(tn, synch, tag, level+1);
//            else {
//                for (Node n : tn.nodesBySubnet[level]) {
//                    synch[level] = n;
//                    generateSynchRecursively(tn, synch, tag, level+1);
//                    synch[level] = null;
//                }
//            }
//        }
//    }
    
//    private Node createNode(Node[] synch) {
//        // Find the first not-null entry in synch and copy the node from it.
//        Node newNode = null, copiedNode = null;
//        for (Node n : synch) {
//            if (n != null) {
//                copiedNode = n;
//                newNode = (Node)Util.deepCopy(n);
//                break;
//            }
//        }
//        assert newNode != null;
//        
//        // Now emit warnings/errors if the fused nodes are inconsistent
//        for (Node nn : synch) {
//            if (nn == null || nn == copiedNode)
//                continue;
//            if (nn instanceof ConstantID) {
//                ConstantID c1 = (ConstantID)newNode;
//                ConstantID c2 = (ConstantID)nn;
//                
//                if (!c1.getConstantExpr().getExpr().equals(c2.getConstantExpr().getExpr())) {
//                    addPageWarning("Composition of constants with different values.", newNode);
//                }
//            }
//            else {
//                System.out.println("UNKNOWN NODE TYPE.");
//            }
//        }
//        return newNode;
//    }
    
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
                if (descr.net instanceof MultiNetCompositionPage) {
                    MultiNetCompositionPage mncp = (MultiNetCompositionPage)descr.net;
                    
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
    

    
//    // The procedure that actually does the composition.
//    private void compose() {
//        if (netsDescr.isEmpty()) {
//            groups = new HashSet<>();
//            return;
//        }
//        this.groups = new HashSet<>();
//        
//        // Index of synchronizable node groups, by tag name
//        @SuppressWarnings("unchecked")
//        MultiMap<String, NodeGroup>[] byLabel = new MultiMap[netsDescr.size()];
//        Set<String> allLabels = new HashSet<>();
//
//        for (int n=0; n<netsDescr.size(); n++) {
//            byLabel[n] = new MultiMap<>();
//            Iterator<NodeGroup> netGroups = netsDescr.get(n).net.groupIterator();
//            while (netGroups.hasNext()) {
//                NodeGroup group = netGroups.next();
//                // Nodes without tags pass directly without any synchronization
//                if (group.numTags() == 0 || !group.getGroupClass().isGroupable()) {
//                    groups.add(group);
//                }
//                else {
//                    // Index all nodes by their (multiple) tags
//                    for (int t=0; t<group.numTags(); t++) {
//                        byLabel[n].put(group.getTag(t), group);
//                        allLabels.add(group.getTag(t));
//                    }
//                }
//            }
//        }
//        
//        // Generate all combinations of synchronizable objects
//        // Proceed by generating the largest set of nodes in the same groupclass
//        // that share a single tag.
//        @SuppressWarnings("unchecked")
//        Set<NodeGroup>[] composed = new Set[netsDescr.size()];
//        for (int n=0; n<netsDescr.size(); n++)
//            composed[n] = new HashSet<>();
//        
//        // TODO: it is possible that two nodes with label: {a,b} || {a,b}
//        //  are synchronized twice. In this case, when doing synchronization for tag 'b'
//        // avoid it if the minimum set of common tags contains 'a' < 'b'.
//        
//        for (GroupClass groupClass : GroupClass.values()) {
//            if (!groupClass.isGroupable())
//                continue;
//            for (String tag : allLabels) {
//                // Reset the composition set
//                for (int n=0; n<netsDescr.size(); n++)
//                    composed[n].clear();
//
//                // Search for node groups with this tag and a specific group class
//                boolean shouldGenerate = false;
//                for (int n=0; n<netsDescr.size(); n++) {
//                    Collection<NodeGroup> candidates = byLabel[n].get(tag);
//                    Iterator<NodeGroup> candIt = candidates.iterator();
//                    while (candIt.hasNext()) {
//                        NodeGroup grp = candIt.next();
//                        if (grp.getGroupClass() != groupClass)
//                            continue; // same label, another group class. Ignore this node
//                        composed[n].add(grp);
//                        shouldGenerate = true;
//                    }
//                }
//                if (!shouldGenerate)
//                    continue; // No nodes with this tag in this group class
//
//                int numNetsInComposition = 0, netInd = 0;
//                for (int n=0; n<netsDescr.size(); n++)
//                    if (!composed[n].isEmpty())
//                        numNetsInComposition++;
//                assert numNetsInComposition > 0;
//                
//                if (numNetsInComposition == 1) {
//                    // All nodes in the same net, there is no synchronization.
//                    for (int n=0; n<netsDescr.size(); n++) {
//                        if (!composed[n].isEmpty()) {
//                            for (NodeGroup group : composed[n])
//                                groups.add(group);
//                            break;
//                        }
//                    }
//                }
//                else { // Nodes in multiple nets, there is a real synchronzation
//                    int[] netIndexes = new int[numNetsInComposition];
//                    for (int n=0; n<netsDescr.size(); n++)
//                        if (!composed[n].isEmpty())
//                            netIndexes[netInd++] = n;
//                    NodeGroup[] synchGroups = new NodeGroup[numNetsInComposition];
//
//                    // Generate the cross-product of all the nodes in the composition vector 
//                    generateSynchRecursively(netIndexes, 0, composed, synchGroups, groupClass);
//                }
//            }
//        }
//    }
//    
//    private void generateSynchRecursively(int[] netIndexes, int level,
//                                          Set<NodeGroup>[] composed, NodeGroup[] synchGroups,
//                                          GroupClass gc) 
//    {
//        if (level == netIndexes.length) { // Generate new group
//            // Duplicate nodes
//            NodeGroup[] dupSynchGroup = Arrays.copyOf(synchGroups, synchGroups.length);
//            NodeGroup newGroup = new SynchGroup(dupSynchGroup, netIndexes, gc);
//            if (!groups.contains(newGroup))
//                groups.add(newGroup);
//            else {
//                System.out.println("Duplicate synchronization group avoided!");
//            }
//        }
//        else { // Iterate over the cross-product elements
//            int nn = netIndexes[level];
//            for (NodeGroup ng : composed[nn]) {
//                synchGroups[level] = ng;
//                assert ng.getGroupClass() == gc;
//                generateSynchRecursively(netIndexes, level + 1, composed, synchGroups, gc);
//            }
//        }
//    }
}
