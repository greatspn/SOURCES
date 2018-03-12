///*
// * To change this license header, choose License Headers in Project Properties.
// * To change this template file, choose Tools | Templates
// * and open the template in the editor.
// */
//
//package editor.domain.superposition;
//
//import editor.domain.Node;
//import editor.domain.elements.ColorClass;
//import editor.domain.elements.ColorVar;
//import editor.domain.elements.ConstantID;
//import editor.domain.elements.Place;
//import editor.domain.elements.TemplateVariable;
//import editor.domain.elements.TokenType;
//import editor.domain.elements.Transition;
//import java.awt.geom.Point2D;
//
///**
// *
// * @author elvio
// */
//public class Algebra {
//    
//    public static boolean areMergeable(NodeGroup g1, NodeGroup g2) {
//        return g1.getSynchClass() == g2.getSynchClass();
//    }
//    
//    // A group of nodes that share a set of tags and are mergeable between them
////    public static final class NodeGroup {
//        
//    
////    private final NetPage[] nets;
////    private final ArrayList<PageErrorWarning> errors;
////
////    public Algebra(NetPage[] nets, ArrayList<PageErrorWarning> errors) {
////        this.nets = nets;
////        this.errors = errors;
////    }
////    
////    public void addError(String msg, Node[] nodes) {
////        errors.add(PageErrorWarning.newErrorSet(msg, mkSet(nodes)));
////    }
////    public void addError(String msg, Node node) {
////        errors.add(PageErrorWarning.newError(msg, node));
////    }
////    public void addWarning(String msg, Node[] nodes) {
////        errors.add(PageErrorWarning.newWarningSet(msg, mkSet(nodes)));
////    }
////    
////    private static Set<Selectable> mkSet(Node[] nodes) {
////        Set<Selectable> set = new HashSet<>();
////        set.addAll(Arrays.asList(nodes));
////        return set;
////    }
////    
//////    static class SpNode {
//////        public final Node node;
//////        public final String[] labels;
//////
//////        public SpNode(Node node, String[] labels) {
//////            this.node = node;
//////            this.labels = labels;
//////        }
//////    }
////    
////    static class TagClass {
////        public String tag;
////        public Class nodeClass;
////
////        public TagClass(String tag, Class nodeClass) {
////            this.tag = tag;
////            this.nodeClass = nodeClass;
////        }
////        public TagClass duplicate() {
////            return new TagClass(tag, nodeClass);
////        }
////    }
////    
////    private static final String[] tag1 = new String[1];
////    private static String[] generateTagList(Node node) {
////        String tagList = node.getSuperPosTags();
////        String[] tags;
////        if (tagList.isEmpty()) {
////            tagList = node.getUniqueName();
////            tags = tag1;
////            tags[0] = tagList;
////        }
////        else {
////            tags = tagList.split(",");
////            for (int i=0; i<tags.length; i++) 
////                tags[i] = tags[i].trim();
////        }
////        
////        return tags;
////    }
////    
////    public void apply() {
////        assert nets!=null && nets.length>0;
////
////        // Index of unique labels per page
////        @SuppressWarnings("unchecked")
////        Map<TagClass, Node>[] uniqueLabels = (HashMap<TagClass, Node>[])new HashMap[nets.length];
////        
////        Set<Node> superposedNodes = new HashSet<>();
////        Map<Node, Node> replacements = new HashMap<>();
////        
////        // Index of superposable nodes
////
////        // Make the index of superposable nodes
////        int netInd = 0;
////        TagClass tcKey = new TagClass(null, null);
////
////        Node[] errList = new Node[2];
////        for (NetPage np : nets) {
////            uniqueLabels[netInd] = new HashMap<>();
////            for (Node node : np.nodes) {
////                if (node.hasSuperPosTags()) {
////                    String[] tags = generateTagList(node);
////                    for (String tag : tags) {
////                        tcKey.nodeClass = node.getSuperpositionClass();
////                        tcKey.tag = tag;
////                        if (uniqueLabels[netInd].containsKey(tcKey)) {
////                            Node otherNode = uniqueLabels[netInd].get(tcKey);
////                            if (otherNode != null) {
////                                errList[0] = node;
////                                errList[1] = otherNode;
////                                addWarning("Label '"+tag+"' is not unique.", errList);
////                            }
////                            uniqueLabels[netInd].put(tcKey, null); // Not unique
////                        }
////                        else
////                            uniqueLabels[netInd].put(tcKey, node); // New label
////                    }
////                }
////            }
////            netInd++;
////        }
////
////        // For each page-unique label, find the set of nodes that have 
////        // the same label in other pages, and have the same class. These nodes
////        // are marked to be superposed.
////        Node[] chain = new Node[nets.length];
////        netInd = 0;
////        for (NetPage np : nets) {
////            for (Node node : np.nodes) {
////                assert !superposedNodes.contains(node);
////                if (replacements.containsKey(node))
////                    continue; // already superposed
////                if (node.hasSuperPosTags()) {
////                    // TEST: node should not be already composed
////                    
////                    // TODO: color domains of superposed places must match.
////
////                    String[] tags = generateTagList(node);
////                    for (String tag : tags) {
////                        // Is this a unique tag?
////                        tcKey.nodeClass = node.getSuperpositionClass();
////                        tcKey.tag = tag;
////                        boolean validTag = true;
////                        for (Map<TagClass, Node> uniqInd : uniqueLabels) {
////                            if (uniqInd.containsKey(tcKey)) {
////                                validTag = validTag && uniqInd.get(tcKey) != null;
////                                if (!validTag)
////                                    break;
////                            }
////                        }
////                        if (!validTag)
////                            continue;
////
////                        Arrays.fill(chain, null);
////
////                        // Search other nodes in the net list with the same class/tag
////                        chain[netInd] = node;
////                        boolean hasChain = false;
////                        for (int j=netInd + 1; j<nets.length; j++) {
////                            chain[j] = uniqueLabels[j].get(tcKey);
////                            hasChain = hasChain || (chain[j] != null);
////                        }
////                        if (!hasChain)
////                            continue; // This tag is not shared
////
////                        // The selected nodes can be superposed togheter
////                        Node jn = mergeNodes(chain, tag);
////                        superposedNodes.add(jn);
////                        for (int j=netInd + 1; j<nets.length; j++) {
////                            if (chain[j] != null) {
////                                replacements.put(chain[j], jn);
////                            }
////                        }
////                    }
////                }
////            }
////            netInd++;
////        }
////        
////        // Replace superposed nodes with node references
////        for (NetPage np : nets) {
////            for (int i=0; i<np.nodes.size(); i++) {
////                Node rep = replacements.get(np.nodes.get(i));
////                if (rep != null) {
////                    np.nodes.set(i, new NodeRef(rep, rep.getUniqueName(), positionOf(rep)));
////                }
////            }
////        }
////        
////        // Reconnect edges of superposed nodes, and remove the old nodes.
////        for (NetPage np : nets) {
////            for (Edge edge : np.edges) {
////                if (replacements.containsKey(edge.getHeadNode()))
////                    edge.connectToNode(replacements.get(edge.getHeadNode()), 
////                                                        edge.getHeadMagnet(), 
////                                                        Edge.EndPoint.HEAD);
////                if (replacements.containsKey(edge.getTailNode()))
////                    edge.connectToNode(replacements.get(edge.getTailNode()), 
////                                                        edge.getTailMagnet(), 
////                                                        Edge.EndPoint.TAIL);
////            }
////        }
////        
////        // TODO: create a new set of Nodes for the superposed ones.
////    }
//    
//    public static enum SynchClass {
//        PLACE, TRANSITION, CONSTANT, COLOR_CLASS, COLOR_VAR
//    }
//    
//    
//    public static SynchClass getSynchClassOf(Node node) {
//        if (node instanceof Place)
//            return SynchClass.PLACE;
//        else if (node instanceof Transition)
//            return SynchClass.TRANSITION;
//        else if (node instanceof ConstantID)
//            return SynchClass.CONSTANT;
//        else if (node instanceof ColorClass)
//            return SynchClass.COLOR_CLASS;
//        else if (node instanceof ColorVar)
//            return SynchClass.COLOR_VAR;
//        else if (node instanceof TemplateVariable) {
//            switch (((TemplateVariable)node).getType()) {
//                case INTEGER:
//                case REAL:
//                    return SynchClass.CONSTANT;
//                case COLOR_CLASS:
//                    return SynchClass.COLOR_CLASS;
//                default:
//                    throw new UnsupportedOperationException("Missing template case.");
//            }
//        }
//        throw new UnsupportedOperationException(" Missing node type.");
//    }
//    
//    private Class getClassOfTemplate(TemplateVariable tvar) {
//        switch (tvar.getType()) {
//            case INTEGER:
//            case REAL:
//                return editor.domain.elements.ConstantID.class;
//                
//            case COLOR_CLASS:
//                return ColorClass.class;
//
//            default:
//                return null;
//        }
//    }
//    
//    // Create a new node that is the "fusion" of the selected nodes
//    private Node mergeNodes(Node[] nodes, String tag) {
//        assert nodes.length > 0;
//        Class typeClass = null;
//        boolean haveTemplate = false, classMismatch = false;
//        for (Node n : nodes) {
//            if (n instanceof TemplateVariable) {
//                haveTemplate = true;
//            }
//            else if (typeClass == null)
//                typeClass = n.getClass();
//            else if (n.getClass() != typeClass) {
//                addError("Type mismatch in node union.", nodes);
//                break;
//            }
//        }
//        if (typeClass == null) // All nodes are incomplete definitions (template variables)
//            typeClass = TemplateVariable.class;
//        
//        // Node chains may contain nulls. Get the index of the first not-null node
//        int fnn = 0;
//        for (; fnn < nodes.length; fnn++)
//            if (nodes[fnn] != null && nodes[fnn].getClass() == typeClass)
//                break;
//        assert nodes[fnn] != null;
//        
//        Node merged;
//        if (typeClass == Place.class)
//            merged = mergePlaces(nodes, fnn, tag);
//        else if (typeClass == ConstantID.class)
//            merged = mergeConstants(nodes, fnn, tag);
//        else if (typeClass == TemplateVariable.class)
//            merged = mergeParams(nodes, fnn, tag);
//        else 
//            throw new UnsupportedOperationException("Missing node type.");
//        
//        return merged;
//    }
//    
//    private Point2D positionOf(Node node) {
//        return new Point2D.Double(node.getX(), node.getY());
//    }
//    
//    private Place mergePlaces(Node[] nodes, int fnn, String newName) {
//        String initMark = ((Place)nodes[fnn]).getInitMarkingEditable().getValue().toString();
//        TokenType tt = ((Place)nodes[fnn]).getType();
//        String colorDom = ((Place)nodes[fnn]).getColorDomainName();
//        String partition = ((Place)nodes[fnn]).getKroneckerPartition();
//
//        boolean initMarkOk = true, typeOk = true, colorDomOk = true, partitionOk = true;
//
//        for (int i=0; i<nodes.length; i++) {
//            if (nodes[i] == null || i==fnn)
//                continue;
//            if (!((Place)nodes[i]).getInitMarkingEditable().getValue().toString().equals(initMark))
//                initMarkOk = false;
//            if (((Place)nodes[i]).getType() != tt)
//                typeOk = false;
//            if (!((Place)nodes[i]).getColorDomainName().equals(colorDom))
//                colorDomOk = false;
//            if (!((Place)nodes[i]).getKroneckerPartition().equals(partition))
//                partitionOk = false;
//        }
//
//        if (!initMarkOk)
//            addWarning("Joined places do not have the same initial marking.", nodes);
//        if (!typeOk)
//            addError("Joined places do not share the same type.", nodes);
//        if (!colorDomOk)
//            addError("Joined places have different color domains.", nodes);
//        if (!partitionOk)
//            addWarning("Joined places do not have the same partition.", nodes);
//
//        return new Place(newName, initMark, tt, colorDom, partition, positionOf(nodes[fnn]));
//    }
//    
//    private Node mergeConstants(Node[] nodes, int fnn, String newName) {
//        assert nodes[fnn] instanceof ConstantID;
//        ConstantID cnn = (ConstantID)nodes[fnn];
//        ConstantID.ConstType ctype = cnn.getConstType();
//        String value = cnn.getConstantExpr().getExpr();
//        String colorDom = cnn.getColorDomainName();
//
//        boolean typeOk = true, valueOk = true, colorDomOk = true;
//
//        for (int i=0; i<nodes.length; i++) {
//            if (nodes[i] == null || i==fnn)
//                continue;
//            if (nodes[i] instanceof ConstantID) {
//                ConstantID c = (ConstantID)nodes[i];
//                if (!c.getColorDomainName().equals(colorDom))
//                    colorDomOk = false;
//                if (!c.getConstantExpr().getExpr().equals(value))
//                    valueOk = false;
//                if (c.getConstType() != ctype)
//                    typeOk = false;
//            }
//            else {
//                assert nodes[i] instanceof TemplateVariable;
//                TemplateVariable tv = (TemplateVariable)nodes[i]; 
//                if (!tv.isCovertibleToConst())
//                    addError("Parameter "+tv+" cannot be merged into a constant.", tv);
//                else if (TemplateVariable.toConstType(tv.getType()) != ctype)
//                    typeOk = false;
//            }
//        }
//        if (!typeOk)
//            addError("Joined constants do not have the same type.", nodes);
//        if (!valueOk)
//            addWarning("Joined constants do not share the same value.", nodes);
//        if (!colorDomOk)
//            addError("Joined constants have different color domains", nodes);
//
//        return new ConstantID(ctype, newName, value, colorDom, positionOf(nodes[fnn]));
//    }
//    
//    private Node mergeParams(Node[] nodes, int fnn, String newName) {
//        TemplateVariable tvnn = (TemplateVariable)nodes[fnn];
//        TemplateVariable.Type type = tvnn.getType();
//        boolean typeOk = true;
//        for (int i=0; i<nodes.length; i++) {
//            if (nodes[i] == null || i==fnn)
//                continue;
//            assert nodes[i] instanceof TemplateVariable;
//            TemplateVariable tv = (TemplateVariable)nodes[i]; 
//            if (tv.getType() != type)
//                typeOk = false;
//        }
//        if (!typeOk)
//            addError("Joined paarmeters do not have the same type.", nodes);
//
//        return new TemplateVariable(type, newName, "", positionOf(nodes[fnn]));
//    }
//}
