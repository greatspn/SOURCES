/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.unfolding;

import common.IntTuple;
import common.Triple;
import common.Tuple;
import common.Util;
import editor.domain.Edge;
import editor.domain.Node;
import editor.domain.SuperpositionTag;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ColorVar;
import editor.domain.elements.ConstantID;
import editor.domain.elements.GspnEdge;
import editor.domain.elements.GspnEdge.Kind;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.TemplateVariable;
import editor.domain.elements.TextBox;
import editor.domain.elements.TokenType;
import editor.domain.elements.Transition;
import editor.domain.semiflows.HilbertBasis;
import java.awt.geom.Point2D;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 *
 * @author elvio
 */
public class Algebra2 {
    
    public enum Policy {
        // Nodes are composed when they have conjugated tags:
        //   aa|bb  joins with  aa?  and  bb?
        UNARY_CONJUGATED_MINIMAL("Unary conjugated (minimal)"), 
        // As above, but it is not restricted to the minimal semiflows
        UNARY_CONJUGATED_ALL("Unary conjugated (all)"), 
        // Nodes are joined from parallel composition if they share the same tag
        BINARY_PARALLEL("N-ary structured composition"),
        NONE("None"); 
        
        private final String visualizedName;

        private Policy(String visualizedName) {
            this.visualizedName = visualizedName;
        }

        @Override
        public String toString() {
            return visualizedName;
        }
        
        public boolean isConjugated() { 
            return this==UNARY_CONJUGATED_ALL || this==UNARY_CONJUGATED_MINIMAL;
        }
    }
    
    // Algebra input net operators
    private final GspnPage[] nets;
    
    // Combination functions for places and transitions
    private final TagRewritingFunction[] relabelFn;
    // Synchronization tags
    private final String[] syncSetPl, syncSetTr;
    
    // net2 coordinate shifts
    private final Point2D[] deltaCoords;
    
    // use broken edges between the two composed nets
    private final boolean useBrokenEdges;

//    // Merge by shared tags or by common name
//    private final MergePolicy mergePolicy;
    
    // Apply restrictions on the selected synchronization tags
    private final boolean restrictTags;
    
    // Semantics for interpreting net tags
    private final Policy policy;
    
    // Do not allow single-net synchronization. Only sync from multiple nets are possible
    private final boolean avoidSingleNetSynch;
    
    // For CCS semantics, generate only the minimal synchronization sets
    // instead of generating any possible combinations of the synch sets
//    private final boolean onlyMinimalSynch;

    private final boolean verbose;
    
    // Output: combined net
    public final GspnPage result;

    // Output: combination messages & warnings
    public final ArrayList<String> warnings;
    
    //=========================================================================
    // Fields that help in the composition of the result net
    
    // Map source places with the list of composed places in the result net
    private final Map<Place, List<Tuple<Integer, Place>>> orig2CompPlcs = new HashMap<>();
    // Same for the transitions
    private final Map<Transition, List<Tuple<Integer, Transition>>> orig2CompTrns = new HashMap<>();
    
    // P/T nodes that are just copied into the result net without changes
    private final Map<Node, Node> simpleNode2Orig = new HashMap<>();
    // From which net operand each simple copied P/T cames from
    private final Map<Node, Integer> simpleNode2NetId = new HashMap<>();
    
    // which net owns the originating edge
    private final Map<GspnEdge, Integer> edge2NetId = new HashMap<>();
    
    // Associate to each new P/T pair a multiset of edges that are merged in the composed net
    private final Map<Triple<Place, Transition, Kind>,
                      List<Tuple<Integer, GspnEdge>>> edgeMap = new HashMap<>();
    
    // Duplicated color variables. All ColorVars from net2 are initially duplicated,
    // then only the one used (stored in @usedDupColorVar) actually end up in the @result net
    private final Map<String, ColorVar> dupColorVars = new HashMap<>();
    private final Set<ColorVar> usedDupColorVar = new HashSet<>();
    
    // Rules to duplicate color variables for all edges & expressions 
    // of a net N for a given @result transition
    private final Map<Tuple<Transition, Integer>, Set<String>> trnClrVarsToDup = new HashMap<>();
    
    // Unique symbols in the @result net
    private final Set<String> uniqueNamesResult = new HashSet<>();
    
    // Positions of the generated nodes
    private final Set<IntTuple> positions = new HashSet<>();
    
    // P/T sync tags associated to consecutive indices
    private final Map<String, Integer> plcTag2Id = new HashMap<>();
    private final Map<String, Integer> trnTag2Id = new HashMap<>();
    
    //=========================================================================
    // A synchronization of a multiset of nodes
    private static class SynchMultiset {
        public String nodeTags; // merged tags
        public String nodeName; // merged names
        // multiset of the synchronized nodes
        public List<Tuple<Integer, Node>> multiset = new LinkedList<>();

        @Override
        public String toString() {
            String s = nodeName+" "+nodeTags+" = ";
            for (Tuple<Integer, Node> entry : multiset)
                s += "("+entry.x+" "+entry.y.getUniqueName()+") ";
            return s;
        }
    }

    //=========================================================================
    private void storePlaceSource(Place newPlace, int card, Place origPlace) {
        List<Tuple<Integer, Place>> lst = orig2CompPlcs.get(origPlace);
        if (lst == null) {
            lst = new LinkedList<>();
            orig2CompPlcs.put(origPlace, lst);
        }
        lst.add(new Tuple<>(card, newPlace));
    }
    private void storeTransitionSource(Transition newTransition, int card, Transition origTransition) {
        List<Tuple<Integer, Transition>> lst = orig2CompTrns.get(origTransition);
        if (lst == null) {
            lst = new LinkedList<>();
            orig2CompTrns.put(origTransition, lst);
        }
        lst.add(new Tuple<>(card, newTransition));
    }
    
    //=========================================================================
    private IntTuple getNodePosition(Node node) {
        return new IntTuple((int)(node.getX()/2), (int)(node.getY()/2));
    }
    private void storePosition(Node node) {
        positions.add(getNodePosition(node));
    }
    private void relocateAndStorePosition(Node node) {
        boolean relocate = true;
        while (relocate) {
            IntTuple pos = getNodePosition(node);
            relocate = positions.contains(pos);
            if (relocate) {
                node.setNodePosition(node.getX() + 3, node.getY());
            }
        }
        storePosition(node);
    }
    
    
    //=========================================================================
    // Build the union list of all tags of @node1 and @node2
    private String mergeTagsConj(List<Tuple<Integer, Node>> multiset) {
        ArrayList<SuperpositionTag> tags = new ArrayList<>();
        
        for (Tuple<Integer, Node> entry : multiset) {
            for (int t=0; t<entry.y.numTags(); t++) {
                tags.add(new SuperpositionTag(entry.y.getTag(t), 
                                              entry.x * entry.y.getTagCard(t)));
            }
        }
        
        SuperpositionTag[] mergedTags = SuperpositionTag.mergeTags(tags);
        StringBuilder builder = new StringBuilder();
        for (SuperpositionTag st : mergedTags) {
            if (st.getCard() != 0) { 
                // Only merge tags that have not been composed (card != 0)
                builder.append(builder.length() == 0 ? "" : "|").append(st.toCanonicalString());
            }
        }
        return builder.toString();
    }
    //=========================================================================
    // Build the union list of all tags of @node1 and @node2
    private String mergeTagsCSP(List<Tuple<Integer, Node>> multiset, String syncTag, 
                                Map<String, Integer> restrictedTagsSet) 
    {
        ArrayList<SuperpositionTag> tags = new ArrayList<>();

        // Join all non-restricted tags (exclude also syncTag)
        for (Tuple<Integer, Node> entry : multiset) {
            for (int t=0; t<entry.y.numTags(); t++) {
                String tag = entry.y.getTag(t);
                if (!tag.equals(syncTag) && !restrictedTagsSet.containsKey(tag))
                    tags.add(new SuperpositionTag(tag, entry.x * Math.abs(entry.y.getTagCard(t))));
            }
        }
        tags.add(new SuperpositionTag(syncTag, 1));
        
        SuperpositionTag[] mergedTags = SuperpositionTag.mergeTags(tags);
        StringBuilder builder = new StringBuilder();
        for (SuperpositionTag st : mergedTags) {
//            if (st.getTag().equals(syncTag) || !restrictedTagsSet.containsKey(st.getTag())) {
                if (st.getCard() != 0) { 
                    // Only merge tags that have not been composed (card != 0)
                    builder.append(builder.length() == 0 ? "" : "|").append(st.toCanonicalString());
                }
//            }
        }
        return builder.toString();
    }

    //=========================================================================
    // Warn for different attributes that cannot be merged
    private void checkAttributeConflict(Node n1, Node n2, Node comb, String attr1, String attr2, String what) {
        if (!attr1.equals(attr2)) {
            String message = comb.getClass().getSimpleName()+
                    " "+comb.getUniqueName()+": could not combine "+what+" of "+
                    n1.getUniqueName();
//            +" (net "+gspn1.getPageName()+ ") and "+n2.getUniqueName()+" (net "+gspn2.getPageName()+")";
            warnings.add(message);
        }
    }

    //=========================================================================
    private Node shiftNode(Node n, int netId) {
        n.setX(n.getX() + deltaCoords[netId].getX());
        n.setY(n.getY() + deltaCoords[netId].getY());
        return n;
    }
    
    private Point2D shiftPoint2D(Point2D pt, int netId) {
        pt.setLocation(pt.getX() + deltaCoords[netId].getX(), 
                       pt.getY() + deltaCoords[netId].getY());
        return pt;
    }
    
    //=========================================================================
    // generate a new unique name for the final combined net
    private String generateUniqueCombinedName(String name) {
        String newName = name;
        int ii = 0;
        while(true) {
            if (!uniqueNamesResult.contains(newName))
                break; // newName is unique
            
            newName = name + "_" + ii;
            ii++;
        }
//        System.out.println("generateUniqueCombinedName "+name+" -> "+newName);
        return newName;
    }
    
    // unique in the final net
    private Node makeNodeNameUnique(Node n) {
        n.setUniqueName(generateUniqueCombinedName(n.getUniqueName()));
        uniqueNamesResult.add(n.getUniqueName());
        return n;
    }

    //=========================================================================
    // generate a new name that is unique for both the input and the final nets
    private String generateTotallyUniqueCombinedName(String name) {
        String newName = name;
        int ii = 0;
        while(true) {
            boolean isUnique;
            isUnique = !uniqueNamesResult.contains(newName);
            for (int i=0; isUnique && i<nets.length; i++)
                isUnique = isUnique && (nets[i].getNodeByUniqueName(newName) == null);
                
            if (isUnique)
                break; // newName is unique
            
            newName = name + "_" + ii;
            ii++;
        }
        return newName;
    }

    // unique in the final net and in the input nets
    private Node makeNodeNameTotallyUnique(Node n) {
        n.setUniqueName(generateTotallyUniqueCombinedName(n.getUniqueName()));
        uniqueNamesResult.add(n.getUniqueName());
        return n;
    }
    
    // policy for generating the merged name from the two operand names
    private String mergeNames(List<Tuple<Integer, Node>> multiset) {
        StringBuilder name = new StringBuilder();
        for (Tuple<Integer, Node> entry : multiset) {
            if (name.length() > 0)
                name.append("_plus_");
            if (entry.x > 1)
                name.append(entry.x).append("_times_");
            name.append(entry.y.getUniqueName());
        }
        return name.toString();
    }    

    //=========================================================================
    public Algebra2(GspnPage[] nets, TagRewritingFunction[] relabelFn, 
                    Point2D[] deltaCoords, 
                    String[] syncSetPl, String[] syncSetTr,
                    Policy policy,
                    boolean useBrokenEdges, 
                    boolean restrictTags,
                    boolean avoidSingleNetSynch,
                    boolean verbose) 
    {
        this.nets = nets;
        this.relabelFn = relabelFn;
        this.deltaCoords = deltaCoords;
        this.syncSetPl = syncSetPl;
        this.syncSetTr = syncSetTr;
        this.policy = policy;
        this.useBrokenEdges = useBrokenEdges;
        this.restrictTags = restrictTags;
        this.avoidSingleNetSynch = avoidSingleNetSynch;
        this.verbose = verbose;
        
        assert this.nets.length == this.relabelFn.length;
        assert this.nets.length == this.deltaCoords.length;
        
        result = new GspnPage();
        warnings = new ArrayList<>();
        
        // make all nets unique. The implementation assumes that Place/Transition/Edges
        // pointers are unique, hence if we compose a net with itself, we need to 
        // duplicate the net first
        for (int i=0; i<nets.length; i++) {
            for (int j=i+1; j<nets.length; j++) {
                if (nets[i] == nets[j]) { // same object, duplicate and revalidate
                    nets[j] = (GspnPage)Util.deepCopy(nets[j]);                    
                    nets[j].preparePageCheck();
                    nets[j].checkPage(null, null, nets[j], null);
                }
            }
        }
    }
    
    //=========================================================================
    private boolean syncMultisetFromSingleSourceNets(List<Tuple<Integer, Node>> multiset) {
        assert !multiset.isEmpty();
        int origNet = simpleNode2NetId.get(multiset.get(0).y);
        for (Tuple<Integer, Node> entry : multiset)
            if (simpleNode2NetId.get(entry.y) != origNet)
                return false; // from multiple nets
        return true;
    }
    
    //=========================================================================
    private void generateSynchSetsCSP(ArrayList<ArrayList<Node>> nodes, int level, int[] sel,
                                      ArrayList<SynchMultiset> syncMultisets,
                                      String tag, Map<String, Integer> tagIds) 
    {
        if (level == nets.length) {
            SynchMultiset sm = new SynchMultiset();
            for (int i=0; i<nets.length; i++) {
                Node ithNode = nodes.get(i).get(sel[i]);
                int weight = 1;
//                int weight = ithNode.getTagCard(ithNode.findTag(tag));
                sm.multiset.add(new Tuple<>(weight, ithNode));
            }
            sm.nodeName = tag + "_" + mergeNames(sm.multiset);
            sm.nodeTags = mergeTagsCSP(sm.multiset, tag, tagIds);                
//            System.out.println(tag+": "+sm);
            syncMultisets.add(sm);
        }
        else {
            // visit recursively
            for (int i=0; i<nodes.get(level).size(); i++) {
                sel[level] = i;
                generateSynchSetsCSP(nodes, level + 1, sel, syncMultisets, tag, tagIds);
            }
        }        
    }
        
    //=========================================================================
    private ArrayList<SynchMultiset>
         synchronizeNodes(ArrayList<Node> nodeIds, 
                          Map<String, Integer> tagIds) 
    {
        ArrayList<SynchMultiset> syncMultisets = new ArrayList<>();
        
        switch (policy) {
            case BINARY_PARALLEL:
                // For each tag, generate a single synchronization node
                for (String tag : tagIds.keySet()) {
                    ArrayList<ArrayList<Node>> nodesPerNet = new ArrayList<>();
                    for (int i=0; i<nets.length; i++)
                        nodesPerNet.add(new ArrayList<>());
                    // Add all nodes that have the tag
                    for (Node node : nodeIds) {
                        boolean has_tag = false;
                        for (int t=0; t<node.numTags(); t++) {
                            if (tag.equals(node.getTag(t))) {
                                has_tag = true;
                                break;
                            }
                        }
                        if (has_tag)
                            nodesPerNet.get(simpleNode2NetId.get(node)).add(node);
                    }
                    boolean canSync = true;
                    for (ArrayList<Node> nodeList : nodesPerNet)
                        if (nodeList.isEmpty())
                            canSync = false; // nothing to synchronize

                    if (canSync)
                        generateSynchSetsCSP(nodesPerNet, 0, new int[nets.length], 
                                             syncMultisets, tag, tagIds);
                }
                break;
                
            case UNARY_CONJUGATED_ALL: 
            case UNARY_CONJUGATED_MINIMAL:
                {
                    // Setup the Unary conjugated Hilbert composition problem.
                    HilbertBasis H;
                    {
                        int M=tagIds.size(), N=nodeIds.size();
                        H = new HilbertBasis(N, M);
                    }
                    for (int nodeId=0; nodeId<nodeIds.size(); nodeId++) {
                        Node node = nodeIds.get(nodeId);
                        for (int t=0; t<node.numTags(); t++) {
                            if (tagIds.containsKey(node.getTag(t))) {
                                int tagId = tagIds.get(node.getTag(t));
                                int card = node.getTagCard(t);
                                H.addToL(nodeId, tagId, card);
                            }
                        }
                    }
                    H.setVerbose();
                    if (policy == Policy.UNARY_CONJUGATED_ALL)
                        H.setKeepCpCm();
                    H.HilbertFM();
                    H.removeInitialRows();

                    // Generate the synchronization nodes
                    for (int phase=0; phase<2; phase++) {
                        for (int ff=0; ff < H.numRows(); ff++) {
                            // phase 0 -> insert real flows
                            // phase 1 -> insert incomplete syncrhonizations
                            if (H.isHilbertBasisVec(ff) == (phase==0)) {
                                int[] syncVec = H.getBasisVec(ff);
                                assert syncVec.length == nodeIds.size();
                                SynchMultiset sm = new SynchMultiset();
                                for (int nodeId=0; nodeId<nodeIds.size(); nodeId++) {
                                    if (syncVec[nodeId] != 0) {
                                        int card = syncVec[nodeId];
                                        Node compPl = nodeIds.get(nodeId);
                                        assert simpleNode2NetId.containsKey(compPl);
                                        sm.multiset.add(new Tuple<>(card, compPl));
                                    }
                                }
                                assert !sm.multiset.isEmpty();
                                if (avoidSingleNetSynch && syncMultisetFromSingleSourceNets(sm.multiset))
                                    continue; // only one source net     

                                sm.nodeName = mergeNames(sm.multiset);
                                sm.nodeTags = mergeTagsConj(sm.multiset);
                                syncMultisets.add(sm);
                            }
                        }
                    }                    
//                    // Setup the synchronization problem.
//                    FlowsGenerator fg;//, fgM=null;
//                    {
//                        int M=tagIds.size(), N=nodeIds.size();
//                        fg = new FlowsGenerator(N, N, M, PTFlows.Type.PLACE_SEMIFLOWS);
////                        if (policy == Policy.UNARY_CONJUGATED_ALL)
////                            fgM = new FlowsGenerator(N, N, M, PTFlows.Type.PLACE_SEMIFLOWS);
//                    }
//                    for (int nodeId=0; nodeId<nodeIds.size(); nodeId++) {
//                        Node node = nodeIds.get(nodeId);
//                        for (int t=0; t<node.numTags(); t++) {
//                            if (tagIds.containsKey(node.getTag(t))) {
//                                int tagId = tagIds.get(node.getTag(t));
//                                int card = node.getTagCard(t);
//                                fg.addIncidence(nodeId, tagId, card);
////                                if (fgM != null)
////                                    fgM.addIncidence(nodeId, tagId, card);
//                            }
//                        }
//                    }
//                    StructuralAlgorithm.ProgressObserver obs = (int step, int total, int s, int t) -> { };
//                    try {
//                        if (policy == Policy.UNARY_CONJUGATED_MINIMAL)
//                            fg.compute(false, obs); // minimal semiflows 
////                        if (fgM != null) {
////                            fgM.computeAllCanonicalSemiflows(true, obs, fg.getAnnulers());
////                            fg = fgM;
////                        }   
//                        else // all semiflows
//                            fg.computeAllCanonicalSemiflows(true, obs);
//                    }
//                    catch (InterruptedException e) { throw new IllegalStateException("Should not happen."); }

//                    // Generate the synchronization nodes
//                    for (int phase=0; phase<2; phase++) {
//                        for (int ff=0; ff < fg.numFlows(); ff++) {
//                            // phase 0 -> insert real flows
//                            // phase 1 -> insert incomplete syncrhonizations
//                            if (fg.isFlow(ff) == (phase==0)) {
//                                int[] syncVec = fg.getFlowVector(ff);
//                                assert syncVec.length == nodeIds.size();
//                                SynchMultiset sm = new SynchMultiset();
//                                for (int nodeId=0; nodeId<nodeIds.size(); nodeId++) {
//                                    if (syncVec[nodeId] != 0) {
//                                        int card = syncVec[nodeId];
//                                        Node compPl = nodeIds.get(nodeId);
//                                        assert simpleNode2NetId.containsKey(compPl);
//                                        sm.multiset.add(new Tuple<>(card, compPl));
//                                    }
//                                }
//                                assert !sm.multiset.isEmpty();
//                                if (avoidSingleNetSynch && syncMultisetFromSingleSourceNets(sm.multiset))
//                                    continue; // only one source net     
//
//                                sm.nodeName = mergeNames(sm.multiset);
//                                sm.nodeTags = mergeTagsConj(sm.multiset);
//                                syncMultisets.add(sm);
//                            }
//                        }
//                    }
                }
                break;
        }
        return syncMultisets;
    }    
    
    //=========================================================================
    private void joinColorClasses() {
        Map<String, ColorClass> joinMap = new HashMap<>();
        
        for (int nn=0; nn<nets.length; nn++) {
            for (Node node : nets[nn].nodes) {
                if (node instanceof ColorClass) {
                    ColorClass cc = (ColorClass)node;
                    if (joinMap.containsKey(cc.getUniqueName())) {
                        // common object: join
                        ColorClass newCc = (ColorClass)joinMap.get(cc.getUniqueName());
                        checkAttributeConflict(cc, newCc, newCc, 
                                cc.getColorClassDef().getExpr(), 
                                newCc.getColorClassDef().getExpr(),
                                "color class definitions");
                    }
                    else {
                        ColorClass newColorClass = (ColorClass)Util.deepCopy(cc);
                        shiftNode(newColorClass, nn);
                        result.nodes.add(makeNodeNameUnique(newColorClass));
                        joinMap.put(newColorClass.getUniqueName(), newColorClass);
                    }
                }
            }
        }
    }

    //=========================================================================
    private void joinColorVars() {
        Map<String, ColorVar> joinMap = new HashMap<>();
        
        for (int nn=0; nn<nets.length; nn++) {
            for (Node node : nets[nn].nodes) {
                if (node instanceof ColorVar) {
                    ColorVar cvar = (ColorVar)node;
                    if (joinMap.containsKey(cvar.getUniqueName())) {
                        // common object: join
                        ColorVar newCvar = (ColorVar)joinMap.get(cvar.getUniqueName());
                        checkAttributeConflict(cvar, newCvar, newCvar, 
                                cvar.getDomainExpr().getExpr(), 
                                newCvar.getDomainExpr().getExpr(),
                                "color variable domains");
                        
                        // create a duplicated color var for later use, and register its unique name
                        ColorVar dupColorVar = (ColorVar)Util.deepCopy(cvar);
                        makeNodeNameTotallyUnique(dupColorVar);
                        dupColorVar.setX(dupColorVar.getX() + 5);
                        dupColorVars.put(cvar.getUniqueName(), dupColorVar);
                    }
                    else {
                        ColorVar newColorVar = (ColorVar)Util.deepCopy(cvar);
                        shiftNode(newColorVar, nn);
                        result.nodes.add(makeNodeNameUnique(newColorVar));
                        joinMap.put(newColorVar.getUniqueName(), newColorVar);
                    }
                }
            }
        }
    }
    
    private void joinDuplicatedColorVarsUsed() {
        for (ColorVar cv : usedDupColorVar) {
            // cv.name is already made totally unique, do not call makeNodeNameUnique()
            result.nodes.add(cv); 
        }
    }

    //=========================================================================
    private void joinConstants() {
        Map<String, ConstantID> joinMap = new HashMap<>();
        
        for (int nn=0; nn<nets.length; nn++) {
            for (Node node : nets[nn].nodes) {
                if (node instanceof ConstantID) {
                    ConstantID constId = (ConstantID)node;
                    if (joinMap.containsKey(constId.getUniqueName())) {
                        // common object: join
                        ConstantID newConstId = (ConstantID)joinMap.get(constId.getUniqueName());
                        checkAttributeConflict(constId, newConstId, newConstId, 
                                constId.getConstType().toString(), 
                                newConstId.getConstType().toString(),
                                "types");
                        checkAttributeConflict(constId, newConstId, newConstId, 
                                constId.getColorDomainName(), 
                                newConstId.getColorDomainName(),
                                "color domains");
                        checkAttributeConflict(constId, newConstId, newConstId, 
                                constId.getConstantExpr().getExpr(), 
                                newConstId.getConstantExpr().getExpr(),
                                "definitions");
                    }
                    else {
                        ConstantID newConstantID = (ConstantID)Util.deepCopy(constId);
                        shiftNode(newConstantID, nn);
                        result.nodes.add(makeNodeNameUnique(newConstantID));
                        joinMap.put(newConstantID.getUniqueName(), newConstantID);
                    }
                }
            }
        }
    }
    
    //=========================================================================
    private void joinTemplateVariables() {
        Map<String, TemplateVariable> joinMap = new HashMap<>();
        
        for (int nn=0; nn<nets.length; nn++) {
            for (Node node : nets[nn].nodes) {
                if (node instanceof TemplateVariable) {
                    TemplateVariable tvar = (TemplateVariable)node;
                    if (joinMap.containsKey(tvar.getUniqueName())) {
                        // common object: join
                        TemplateVariable newTvar = (TemplateVariable)joinMap.get(tvar.getUniqueName());
                        checkAttributeConflict(tvar, newTvar, newTvar, 
                                tvar.getType().toString(), 
                                newTvar.getType().toString(),
                                "types");
                    }
                    else {
                        TemplateVariable newTemplateVariable = (TemplateVariable)Util.deepCopy(tvar);
                        shiftNode(newTemplateVariable, nn);
                        result.nodes.add(makeNodeNameUnique(newTemplateVariable));
                        joinMap.put(newTemplateVariable.getUniqueName(), newTemplateVariable);
                    }
                }
            }
        }
    }
    
    //=========================================================================
    private void joinTextBoxes() {
        int new_id = 0;
        
        for (int nn=0; nn<nets.length; nn++) {
            for (Node node : nets[nn].nodes) {
                if (node instanceof TextBox) {
                    TextBox newTBox = (TextBox)Util.deepCopy(node);
                    newTBox.setUniqueName("textbox__"+(new_id++));
                    shiftNode(newTBox, nn);
                    result.nodes.add(newTBox);
                }
            }
        }
    }
            
    //=========================================================================
    private void joinPlaces() {
        for (int nn=0; nn<nets.length; nn++) {
            for (Node node : nets[nn].nodes) {
                if (node instanceof Place) {
                    Place newPlace = (Place)Util.deepCopy(node);
                    makeNodeNameUnique(newPlace);
                    newPlace.setSuperPosTags(relabelFn[nn].getKeptTags(node));
                    shiftNode(newPlace, nn);
                    result.nodes.add(newPlace);
                    storePlaceSource(newPlace, 1, (Place)node);
                    storePosition(newPlace);
                    simpleNode2Orig.put(newPlace, (Place)node);
                    simpleNode2NetId.put(newPlace, nn);
                }
            }
        }
        
        if (syncSetPl==null || syncSetPl.length == 0)
            return;
        
        ArrayList<Node> placeIds = new ArrayList<>();
        // Determine which places have synchronization tags
        for (Node newNode : result.nodes) {
            if (newNode instanceof Place) {
                for (int t=0; t<newNode.numTags(); t++) {
                    if (plcTag2Id.containsKey(newNode.getTag(t))) {
                        placeIds.add((Place)newNode);
                        break;
                    }
                }
            }
        }
        // Compute the synchronization place multisets
        ArrayList<SynchMultiset> syncMultisets = synchronizeNodes(placeIds, plcTag2Id);
        
        // Generate the synchronization place nodes
        for (SynchMultiset sm : syncMultisets) {
            // Generate the new place from the synchronization multiset
            Place newPlace = (Place)Util.deepCopy(sm.multiset.get(0).y);
            newPlace.setUniqueName(sm.nodeName);
            makeNodeNameUnique(newPlace);
            newPlace.setSuperPosTags(sm.nodeTags);
            
            // Check incompatibilities with the multiset members
            Iterator<Tuple<Integer, Node>> iter = sm.multiset.iterator();
            iter.next(); // skip cloned node
            while (iter.hasNext()) {
                Place p2 = (Place)iter.next().y;
                
                checkAttributeConflict(newPlace, p2, newPlace, 
                        newPlace.getColorDomainName(), p2.getColorDomainName(), "color domains");
                checkAttributeConflict(newPlace, p2, newPlace, 
                        newPlace.getType().toString(), p2.getType().toString(), "types");
                checkAttributeConflict(newPlace, p2, newPlace, 
                        newPlace.getKroneckerPartition(), p2.getKroneckerPartition(), "Kronecker partitions");
                
                // Combine the initial markings
                String init1 = newPlace.getInitMarkingExpr();
                String init2 = p2.getInitMarkingExpr(), newInit;
                if (newPlace.isInNeutralDomain()) {
                    newInit = simpleNeutralExprSum(init1, init2, newPlace.getType());
                }
                else { // color expressions
                    newInit = simpleColorExprSum(init1, init2, newPlace.getType());
                }
                //System.out.println("init1="+init1+" init2="+init2+" newInit="+newInit);
                newPlace.getInitMarkingEditable().setValue(null, null, newInit);
                
                newPlace.setNodePosition(newPlace.getX() + p2.getX(), 
                                         newPlace.getY() + p2.getY());
            }
            
            newPlace.setNodePosition(newPlace.getX() / sm.multiset.size(),
                                     newPlace.getY() / sm.multiset.size());
            relocateAndStorePosition(newPlace);
            result.nodes.add(newPlace);
            for (Tuple<Integer, Node> entry : sm.multiset) {
                Place origPlc = (Place)simpleNode2Orig.get(entry.y);
                assert origPlc != null;
                storePlaceSource(newPlace, entry.x, origPlc);
            }
        }
    }

   //=========================================================================
    private void joinTransitions() {
        for (int nn=0; nn<nets.length; nn++) {
            for (Node node : nets[nn].nodes) {
                if (node instanceof Transition) {
                    Transition newTransition = (Transition)Util.deepCopy(node);
                    makeNodeNameUnique(newTransition);
                    newTransition.setSuperPosTags(relabelFn[nn].getKeptTags(node));
                    shiftNode(newTransition, nn);
                    result.nodes.add(newTransition);
                    storeTransitionSource(newTransition, 1, (Transition)node);
                    storePosition(newTransition);
                    simpleNode2Orig.put(newTransition, (Transition)node);
                    simpleNode2NetId.put(newTransition, nn);
                }
            }
        }
        
        if (syncSetTr==null || syncSetTr.length == 0)
            return;
        
        ArrayList<Node> trnIds = new ArrayList<>();
        // Determine which transitions have synchronization tags
        for (Node newNode : result.nodes) {
            if (newNode instanceof Transition) {
                for (int t=0; t<newNode.numTags(); t++) {
                    if (trnTag2Id.containsKey(newNode.getTag(t))) {
                        trnIds.add((Transition)newNode);
                        break;
                    }
                }
            }
        }
        // Compute the synchronization transitions multisets 
        ArrayList<SynchMultiset> syncMultisets = synchronizeNodes(trnIds, trnTag2Id);
        
        // Generate the synchronization transition nodes
        for (SynchMultiset sm : syncMultisets) {
            // Generate the new transition from the synchronization multiset
            Transition newTransition = (Transition)Util.deepCopy(sm.multiset.get(0).y);
            newTransition.setUniqueName(sm.nodeName);
            makeNodeNameUnique(newTransition);
            newTransition.setSuperPosTags(sm.nodeTags);
            
            // Check incompatibilities with the multiset members
            Iterator<Tuple<Integer, Node>> iter = sm.multiset.iterator();
            iter.next(); // skip cloned node
            while (iter.hasNext()) {
                Transition t2 = (Transition)iter.next().y;
                
                checkAttributeConflict(newTransition, t2, newTransition, 
                    newTransition.getType().toString(), t2.getType().toString(), "types");
                checkAttributeConflict(newTransition, t2, newTransition, 
                    newTransition.getDelay(), t2.getDelay(), "delays");
                checkAttributeConflict(newTransition, t2, newTransition, 
                    newTransition.getPriority(), t2.getPriority(), "priorities");
                checkAttributeConflict(newTransition, t2, newTransition, 
                    newTransition.getWeight(), t2.getWeight(), "weights");
                checkAttributeConflict(newTransition, t2, newTransition, 
                    newTransition.getNumServers(), t2.getNumServers(), "number of servers");
                
                newTransition.getGuardEditable().setValue(null, null, mergeGuards(newTransition.getGuard(), t2.getGuard()));
//                checkAttributeConflict(newTransition, t2, newTransition, 
//                    newTransition.getGuard(), t2.getGuard(), "guards");
                
                newTransition.setNodePosition(newTransition.getX() + t2.getX(), 
                                              newTransition.getY() + t2.getY());
            }
            
            newTransition.setNodePosition(newTransition.getX() / sm.multiset.size(), 
                                          newTransition.getY() / sm.multiset.size());  
            relocateAndStorePosition(newTransition);
            result.nodes.add(newTransition);
            for (Tuple<Integer, Node> entry : sm.multiset) {
                Transition origTrn = (Transition)simpleNode2Orig.get(entry.y);
                assert origTrn != null;
                storeTransitionSource(newTransition, entry.x, origTrn);
            }
        }
    }
    
    private String mergeGuards(String g1, String g2) {
        if (g1.equals("True"))
            return g2;
        else if (g2.equals("True"))
            return g1;
        else return "("+g1+") && ("+g2+")";
    }

    //=========================================================================
    private void joinEdges() {
        // Prepare all the edges
        for (int nn=0; nn<nets.length; nn++) {
            for (Edge edge : nets[nn].edges) {
                GspnEdge e = (GspnEdge)edge;
                
                Place plc = e.getConnectedPlace();
                List<Tuple<Integer, Place>> listPlcs = orig2CompPlcs.get(plc);
                Transition trn = e.getConnectedTransition();
                List<Tuple<Integer, Transition>> listTrns = orig2CompTrns.get(trn);
                
                // Generate the cross product of edges
                for (Tuple<Integer, Place> entryP : listPlcs) {
                    for (Tuple<Integer, Transition> entryT : listTrns) {
                        Triple<Place, Transition, Kind> key = new Triple<>(entryP.y, entryT.y, e.getEdgeKind());
                        
                        List<Tuple<Integer, GspnEdge>> list = edgeMap.get(key);
                        if (list == null) {
                            list = new LinkedList<>();
                            edgeMap.put(key, list);
                        }
                        list.add(new Tuple<>(entryP.x * entryT.x, e));
                    }
                }
                
                edge2NetId.put(e, nn);
            }
        }
        
        determineColorVarsToBeDuplicated();

        // read back all edges, then compose and insert in the result net
        for (Map.Entry<Triple<Place, Transition, Kind>,
                       List<Tuple<Integer, GspnEdge>>> ee : edgeMap.entrySet()) 
        {
            Place resultPlace = ee.getKey().x;
            Transition resultTrans = ee.getKey().y;
            Kind kind = ee.getKey().z;
            List<Tuple<Integer, GspnEdge>> edges = ee.getValue();
            int tailMagnet, headMagnet;
            ArrayList<Point2D> points;
            boolean isBroken;
            String mult;
            isBroken = useBrokenEdges && !(simpleNode2Orig.containsKey(resultPlace) && simpleNode2Orig.containsKey(resultTrans));
            
            if (edges.size() == 1) { // only one originating edge
                Tuple<Integer, GspnEdge> entry0 = edges.get(0);
                headMagnet = entry0.y.getHeadMagnet();
                tailMagnet = entry0.y.getTailMagnet();
                mult = simpleNeutralMult(entry0.x, entry0.y.getMultiplicity(), entry0.y.getTypeOfConnectedPlace()); 
                points = composeEdgePoints2(entry0.y, null);
            }
            else { // combine multiple edges
                headMagnet = -1;
                tailMagnet = -1;
                if (edges.size() == 2) {
                    points = composeEdgePoints2(edges.get(0).y, edges.get(1).y);
                } else {
                    points = composeEdgePoints2(null, null);
                }
                
                // Combine multiplicities
                Tuple<Integer, GspnEdge> entry0 = edges.get(0);
                mult = simpleNeutralMult(entry0.x, entry0.y.getMultiplicity(), entry0.y.getTypeOfConnectedPlace()); 
                Iterator<Tuple<Integer, GspnEdge>> iter = edges.iterator();
                iter.next(); // skip entry 0
                while (iter.hasNext()) {
                    Tuple<Integer, GspnEdge> entryN = iter.next();
                    String multN = entryN.y.getMultiplicity();
                    if (!entry0.y.getConnectedPlace().getColorDomainName().equals(entryN.y.getConnectedPlace().getColorDomainName())) {
                        warnings.add("Composing arcs from place "+entry0.y.getConnectedPlace().getUniqueName()+
                                     " in net1 with place "+entryN.y.getConnectedPlace().getUniqueName()+
                                     " with different color domains.");
                        mult = mult+" + "+multN;
                    }
                    else {
                        if (entry0.y.getConnectedPlace().isInNeutralDomain()) {
                            mult = simpleNeutralExprSum(mult, multN, entry0.y.getTypeOfConnectedPlace());
                        }
                        else { // color expressions
                            int netId = edge2NetId.get(entryN.y);
                            multN = duplicateCommonColorVars(resultTrans, multN, netId);
                            mult = simpleColorExprSum(mult, multN, entry0.y.getTypeOfConnectedPlace());
                        }
                    }
                }
            }
            
            GspnEdge newEdge = new GspnEdge(null, tailMagnet, null, headMagnet, points, isBroken, kind, mult);
            newEdge.setConnectedPlace(resultPlace, kind);
            newEdge.setConnectedTransition(resultTrans, kind);
            result.edges.add(newEdge);
        }
    }
    
    //=========================================================================
    // apply restrictions: remove all nodes that have the syncronization tags
    private void applyRestrictions() {
        if (!restrictTags)
            return; // nothing to do
        // Remove nodes
        Set<Node> removedNodes = new HashSet<>();
        Iterator<Node> iterN = result.nodes.iterator();
        while (iterN.hasNext()) {
            Node node = iterN.next();
            boolean remove = false;
            if (node instanceof Place) {
                if (policy==Policy.BINARY_PARALLEL && !simpleNode2Orig.containsKey(node)) 
                    continue; // keep this node
                for (int t=0; t<node.numTags(); t++) {
                    if (plcTag2Id.containsKey(node.getTag(t))) {
                        remove = true;
                        break;
                    }
                }
            }
            else if (node instanceof Transition) {
                if (policy==Policy.BINARY_PARALLEL && !simpleNode2Orig.containsKey(node)) 
                    continue; // keep this node
                for (int t=0; t<node.numTags(); t++) {
                    if (trnTag2Id.containsKey(node.getTag(t))) {
                        remove = true;
                        break;
                    }
                }
            }
            if (remove) {
                iterN.remove();
                removedNodes.add(node);
            }
        }
        // Remove the edges connected to removed nodes
        Iterator<Edge> iterE = result.edges.iterator();
        while (iterE.hasNext()) {
            Edge edge = iterE.next();
            if (removedNodes.contains(edge.getHeadNode()) ||
                removedNodes.contains(edge.getTailNode())) 
            {
                iterE.remove();
            }
        }
    }
    
    //=========================================================================
    // determine in each result transition, which color variables need to be duplicated
    private void determineColorVarsToBeDuplicated() {
        // all the color variables from edges of net1/2 used by a @result transition
        ArrayList<Map<Transition, Set<ColorVar>>> clrVarsOfNet = new ArrayList<>(nets.length);
        for (int nn=0; nn<nets.length; nn++)
            clrVarsOfNet.add(new HashMap<>());
        
        // Determine all color variables used in each transition relation,
        // on all the edges from net1 and net2
        for (Map.Entry<Triple<Place, Transition, Kind>,
                       List<Tuple<Integer, GspnEdge>>> ee : edgeMap.entrySet()) 
        {
            Transition trr = ee.getKey().y;
            
            for (Tuple<Integer, GspnEdge> edge : ee.getValue()) {
                int netId = edge2NetId.get(edge.y);
                Set<ColorVar> set = clrVarsOfNet.get(netId).get(trr);
                if (set == null) {
                    set = new HashSet<>();
                    clrVarsOfNet.get(netId).put(trr, set);
                }

                set.addAll(edge.y.getColorVarsInUse());
            }
        }
        
        // Determine, in each result transition, which color variables
        // from the composing nets needs to be duplicated
        for (Node node : result.nodes) {
            if (node instanceof Transition) {
                Transition trn = (Transition)node;
                Set<String> usedCvarNames = new HashSet<>();
                
                // Follow the sequence of the composed nets
                for (int nn=0; nn<nets.length; nn++) {
                    if (clrVarsOfNet.get(nn).containsKey(trn)) {
                        Set<ColorVar> set = clrVarsOfNet.get(nn).get(trn);
                        Set<String> clrVarsToDup = new HashSet<>();
                        
                        for (ColorVar cvar : set) {
                            if (usedCvarNames.contains(cvar.getUniqueName())) {
                                // duplicate color variable name
                                clrVarsToDup.add(cvar.getUniqueName());
                                ColorVar dupCvar = dupColorVars.get(cvar.getUniqueName());
                                usedDupColorVar.add(dupCvar);
                            }
                            else {
                                // use original color variable name
                                usedCvarNames.add(cvar.getUniqueName());
                            }
                        }
                        if (!clrVarsToDup.isEmpty()) {
                            Tuple<Transition, Integer> key = new Tuple<>(trn, nn);
                            trnClrVarsToDup.put(key, clrVarsToDup);

                            if (verbose) {
                                for (String cvarName : clrVarsToDup) {
                                    System.out.println("IN TRANSITION "+trn.getUniqueName()+
                                            " ALL INSTANCES OF "+cvarName+
                                            " FROM EDGES OF NET2 WILL BE REPLACED WITH "+
                                            dupColorVars.get(cvarName).getUniqueName());
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    //=========================================================================
    // replace in @expr all the occurrences of color variables in @colorVars2
    // that are also present in @colorVars2
    private String duplicateCommonColorVars(Transition trr, String expr, int edgeNetId) {
        Tuple<Transition, Integer> key = new Tuple<>(trr, edgeNetId);
        Set<String> clrVarsToDup = trnClrVarsToDup.get(key);
        if (clrVarsToDup == null)
            return expr; // nothing to do
        
        for (String cvarName : clrVarsToDup) {
            // replace all occurrences of cvar1 in m2 with dupCvar
            ColorVar dupCvar = dupColorVars.get(cvarName);
            usedDupColorVar.add(dupCvar);

            // rewrite cvar1 -> dupCvar
            // regex from: group 1 = any non-alphanumeric char, or start of string
            //             group 2 = searched identifier
            //             group 3 = any non-alphanumeric char, or end of string
            // regex to: group 1 + replaced identifier + group 3
            String from = "([^a-zA-Z0-9_]|^)("+cvarName+")([^a-zA-Z0-9_]|$)";
            String to = "$1"+dupCvar.getUniqueName()+"$3";
            expr = expr.replaceAll(from, to);
        }
        return expr;
    }
    
    //=========================================================================
    // simplified sum of color terms
    private String simpleColorExprSum(String expr1, String expr2, TokenType type) {
        if (expr1.isEmpty())
            return expr2;
        if (expr2.isEmpty())
            return expr1;
        // Simple case 1: 
        // n * <All, .., All> + m * <All, ..., All> -> (n+m)<All, ..., All>
        Pattern r = Pattern.compile("(\\d*)\\s*(<All[\\s*,\\s*All]*>)");
        Matcher m1 = r.matcher(expr1);
        Matcher m2 = r.matcher(expr2);
        if (m1.find() && m2.find()) {
            String s1 = m1.group(1);
            String s2 = m2.group(1);
            String all1 = m1.group(2);
            if (s1.length() == 0)
                s1 = "1";
            if (s2.length() == 0)
                s2 = "1";
            try {
                if (type == TokenType.DISCRETE) {
                    int n1 = Integer.parseInt(s1);
                    int n2 = Integer.parseInt(s2);
                    return (n1+n2)+all1;
                }
                else {
                    double d1 = Double.parseDouble(s1);
                    double d2 = Double.parseDouble(s2);
                    return (d1+d2)+all1;                 
                }
            }
            catch (NumberFormatException e) {
                return expr1+" + "+expr2;
            }
        }
        
        return expr1+" + "+expr2;
    }

    //=========================================================================
    // simplified sum of non-colored terms
    private String simpleNeutralExprSum(String expr1, String expr2, TokenType type) {
        boolean blank1 = expr1.isBlank(), blank2 = expr2.isBlank();
        if (blank1 && blank2)
            return "";
        if (blank1)
            return expr2;
        if (blank2)
            return expr1;
        
        try {
            if (type == TokenType.DISCRETE) {
                int i1 = Integer.parseInt(expr1);
                int i2 = Integer.parseInt(expr2);
                return "" + (i1 + i2);
            }
            else {
                double i1 = Double.parseDouble(expr1);
                double i2 = Double.parseDouble(expr2);
                return "" + (i1 + i2);                                
            }
        }
        catch (NumberFormatException e) { }
        return expr1+" + "+expr2;    
    }
    
    //=========================================================================
    // simpified product of a positive integer with a scalar edge cardinality
    private String simpleNeutralMult(int mult, String expr, TokenType type) {
        if (mult == 1)
            return expr;
        
        try {
            if (type == TokenType.DISCRETE) {
                int i1 = Integer.parseInt(expr);
                return "" + (mult * i1);
            }
            else {
                double i1 = Double.parseDouble(expr);
                return "" + (mult * i1);                                
            }
        }
        catch (NumberFormatException e) { }
        return mult+" * "+expr;
    }
     
//    public static void main(String[] args) {
////        System.out.println("<c1> + <c11>".replaceAll("[^a-zA-Z0-9_]", "#"));
////        System.out.println("c1 <c1> + <c11> c1".replaceAll("([^a-zA-Z0-9_]|^)(c1)([^a-zA-Z0-9_]|$)", "$1c2$3"));
//
//        Pattern r = Pattern.compile("(\\d*)\\s*(<All[\\s*,\\s*All]*>)");
//        String[] lines = {
//            "<All>", "2<All>", "3 <All, All>", "4", "4<All,All, All>"
//        };
//        for (String line : lines) {
//            Matcher m = r.matcher(line);
//            boolean found = m.find();
//            System.out.print(line+" -> "+found);
//            if (found) {
//                System.out.print("  $1="+m.group(1)+" $2="+m.group(2));
//            }
//            System.out.println("");
//        }
//    }
    
    //=========================================================================
    private ArrayList<Point2D> composeEdgePoints2(GspnEdge e1, GspnEdge e2) {
        ArrayList<Point2D> points = new ArrayList<>();
        
        points.add(new Point2D.Double(0, 0));
        
        if (e1 != null) {
            int net1 = edge2NetId.get(e1);
            for (int ii=1; ii<e1.points.size()-1; ii++)
                points.add(shiftPoint2D(new Point2D.Double(e1.points.get(ii).getX(), 
                                                           e1.points.get(ii).getY()), net1));
        }
        if (e2 != null) {
            int net2 = edge2NetId.get(e2);
            for (int ii=1; ii<e2.points.size()-1; ii++)
                points.add(shiftPoint2D(new Point2D.Double(e2.points.get(ii).getX(), 
                                                           e2.points.get(ii).getY()), net2));
        }
        
        points.add(new Point2D.Double(0, 0));
        
        return points;
    }

    //=========================================================================
    public void compose() {
        // Prepare synchronization tags
        if (syncSetPl != null)
            for (int ii=0; ii<syncSetPl.length; ii++)
                plcTag2Id.put(syncSetPl[ii], ii);
        if (syncSetTr != null)
            for (int ii=0; ii<syncSetTr.length; ii++)
                trnTag2Id.put(syncSetTr[ii], ii);
        
        // Join non-place and non-transition objects
        joinColorClasses();
        joinColorVars();
        joinConstants();
        joinTemplateVariables();
        joinTextBoxes();
        
        // Join and compose places and transitions
        joinPlaces();
        joinTransitions();
        /*if (verbose)
            printNodeHelpers();*/

        // Generate all the edges connecting the composed places and transitions
        joinEdges();
        
        // Add the color variables that where used in expression rewritings
        joinDuplicatedColorVarsUsed();
        
        // Apply restriction rules
        applyRestrictions();
    }
    
    
    
    //=========================================================================
    /*private void printEdgeHelpers() 
    {
//        System.out.println(kind+" EDGES:");
        for (Map.Entry<Triple<Place, Transition, Kind>, Tuple<GspnEdge, GspnEdge>> ee : edgeMap.entrySet()) {
            System.out.print("  "+ee.getKey().z+" ");
            // source edges
            if (ee.getValue().x == null)
                System.out.print("...");
            else
                System.out.print(ee.getValue().x.getConnectedPlace().getUniqueName()+"+"+
                                 ee.getValue().x.getConnectedTransition().getUniqueName()+"("+
                                 ee.getValue().x.getMultiplicity()+")");
            System.out.print(" + ");
            if (ee.getValue().y == null)
                System.out.print("...");
            else
                System.out.print(ee.getValue().y.getConnectedPlace().getUniqueName()+"+"+
                                 ee.getValue().y.getConnectedTransition().getUniqueName()+"("+
                                 ee.getValue().y.getMultiplicity()+")");
            System.out.print("  ->  ");
            // result edges
            System.out.print(ee.getKey().x.getUniqueName()+" ");
            System.out.print(ee.getKey().y.getUniqueName()+" ");
            System.out.println();
        }
        System.out.println("");
    }*/
    
    //=========================================================================
    /*private void printNodeHelpers() {
        System.out.println("NET1 PLACE MAP:");
        for (Map.Entry<Place, List<Place>> ee : plc1InProd.entrySet()) {
            System.out.print("  "+ee.getKey().getUniqueName()+"  ->  ");
            for (Place n : ee.getValue())
                System.out.print(n.getUniqueName()+" ");
            System.out.println("");
        }
        System.out.println("NET2 PLACE MAP:");
        for (Map.Entry<Place, List<Place>> ee : plc2InProd.entrySet()) {
            System.out.print("  "+ee.getKey().getUniqueName()+"  ->  ");
            for (Place n : ee.getValue())
                System.out.print(n.getUniqueName()+" ");
            System.out.println("");
        }
        System.out.println("NET1 TRANSITION MAP:");
        for (Map.Entry<Transition, List<Transition>> ee : trn1InProd.entrySet()) {
            System.out.print("  "+ee.getKey().getUniqueName()+"  ->  ");
            for (Transition n : ee.getValue())
                System.out.print(n.getUniqueName()+" ");
            System.out.println("");
        }
        System.out.println("NET2 TRANSITION MAP:");
        for (Map.Entry<Transition, List<Transition>> ee : trn2InProd.entrySet()) {
            System.out.print("  "+ee.getKey().getUniqueName()+"  ->  ");
            for (Transition n : ee.getValue())
                System.out.print(n.getUniqueName()+" ");
            System.out.println("");
        }   
        System.out.println("");
    }*/

    
}
