/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.unfolding;

import common.Tuple;
import common.Util;
import editor.domain.Edge;
import editor.domain.Node;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ColorVar;
import editor.domain.elements.ConstantID;
import editor.domain.elements.GspnEdge;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.TemplateVariable;
import editor.domain.elements.TextBox;
import editor.domain.elements.TokenType;
import editor.domain.elements.Transition;
import java.awt.geom.Point2D;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 *
 * @author elvio
 */
public class Algebra {
    
    // Algebra input net operators
    private final GspnPage gspn1, gspn2;
    
    // Restriction sets
    private final String[] restSetTr, restSetPl;
    
    // net2 coordinate shifts
    private final int dx2, dy2;

    private final boolean verbose;
    
    // Output: combined net
    public final GspnPage result;

    // Output: combination messages & warnings
    public final ArrayList<String> warnings;

    //=========================================================================
    // Fields thah help in the composition of the result net

    // Map places from net1 to result
    private final Map<Place, List<Place>> plc1InProd = new HashMap<>();
    // Map places from net2 to result
    private final Map<Place, List<Place>> plc2InProd = new HashMap<>();

    // Map transitions from net1 to result
    private final Map<Transition, List<Transition>> trn1InProd = new HashMap<>();
    // Map transitions from net2 to result
    private final Map<Transition, List<Transition>> trn2InProd = new HashMap<>();
    
    // Maps <place, trans> of composed net with the source edges from net1, net2
    private final Map<Tuple<Place, Transition>, Tuple<GspnEdge, GspnEdge>> edgeMapInput = new HashMap<>();
    private final Map<Tuple<Place, Transition>, Tuple<GspnEdge, GspnEdge>> edgeMapOutput = new HashMap<>();
    private final Map<Tuple<Place, Transition>, Tuple<GspnEdge, GspnEdge>> edgeMapInhibitor = new HashMap<>();
    
    // Duplicated color variables
    private final Map<String, ColorVar> dupColorVars = new HashMap<>();
    private final Set<ColorVar> usedDupColorVar = new HashSet<>();
    
    // Unique symbols in the result net
    private final Set<String> uniqueNamesResult = new HashSet<>();

    //=========================================================================
    // Determine if two nodes with tags (i.e. places or transitions) share at
    // least a common tag in the restricted list, which means that the
    // two nodes will be composed in the @result net.
    private boolean shareRestrictedTag(Node node1, Node node2, String[] restList) {
        for (int n1=0; n1<node1.numTags(); n1++) {
            for (int n2=0; n2<node2.numTags(); n2++) {
                if (node1.getTag(n1).equals(node2.getTag(n2))) {
                    for (String tag : restList)
                        if (tag.equals(node1.getTag(n1)))
                            return true;
                }
            }
        }
        return false;
    }
    
    // Build the union list of all tags of @node1 and @node2
    private String mergeTags(Node node1, Node node2) {
        String tags = node1.getSuperPosTags();        
        for (int n2=0; n2<node2.numTags(); n2++) {
            boolean found = false;
            for (int n1=0; n1<node1.numTags() && !found; n1++) {
                if (node1.getTag(n1).equals(node2.getTag(n2))) {
                    found = true;
                }
            }
            if (!found)
                tags += "|" + node2.getTag(n2);
        }
//        System.out.println("mergeTags "+node1.getSuperPosTags()+" "+node2.getSuperPosTags()+" -> "+tags);
        return tags;
    }

    //=========================================================================
    private Node shiftNode(Node n) {
        n.setX(n.getX() + dx2);
        n.setY(n.getY() + dy2);
        return n;
    }
    
    private Point2D shiftPoint2D(Point2D pt) {
        pt.setLocation(pt.getX() + dx2, pt.getY() + dy2);
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
            if (!uniqueNamesResult.contains(newName) &&
                gspn1.getNodeByUniqueName(newName) == null &&
                gspn2.getNodeByUniqueName(newName) == null)
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

    //=========================================================================
    public Algebra(GspnPage gspn1, GspnPage gspn2, String[] restSetTr, String[] restSetPl, 
                   int dx2, int dy2, boolean verbose) 
    {
        this.gspn1 = gspn1;
        this.gspn2 = gspn2;
        this.restSetTr = restSetTr;
        this.restSetPl = restSetPl;
        this.dx2 = dx2;
        this.dy2 = dy2;
        this.verbose = verbose;
        
        result = new GspnPage();
        warnings = new ArrayList<>();
    }
    
    //=========================================================================
    private void joinColorClasses() {
        Map<String, ColorClass> map1 = new HashMap<>();
        Map<String, ColorClass> map2 = new HashMap<>();
        
        for (Node node1 : gspn1.nodes) 
            if (node1 instanceof ColorClass) 
                map1.put(node1.getUniqueName(), (ColorClass)node1);
        for (Node node2 : gspn2.nodes) 
            if (node2 instanceof ColorClass) 
                map2.put(node2.getUniqueName(), (ColorClass)node2);
        
        for (Map.Entry<String, ColorClass> ee1 : map1.entrySet()) {
            if (map2.containsKey(ee1.getKey())) {
                // common object: join
                ColorClass c1 = (ColorClass)ee1.getValue();
                ColorClass c2 = (ColorClass)map2.get(ee1.getKey());
                ColorClass newColorClass = (ColorClass)Util.deepCopy(c1);
                if (!c1.getColorClassDef().getExpr().equals(c2.getColorClassDef().getExpr())) 
                {
                    warnings.add("Composition conflict for color class "+newColorClass.getUniqueName());
                }
                result.nodes.add(makeNodeNameUnique(newColorClass));
            }
            else {
                ColorClass newColorClass = (ColorClass)Util.deepCopy(ee1.getValue());
                result.nodes.add(makeNodeNameUnique(newColorClass));
            }
        }
        for (Map.Entry<String, ColorClass> ee2 : map2.entrySet()) {
            if (!map1.containsKey(ee2.getKey())) {
                ColorClass newColorClass = (ColorClass)Util.deepCopy(ee2.getValue());
                shiftNode(newColorClass);
                result.nodes.add(makeNodeNameUnique(newColorClass));
            }
        }        
    }

    //=========================================================================
    private void joinColorVars() {
        Map<String, ColorVar> map1 = new HashMap<>();
        Map<String, ColorVar> map2 = new HashMap<>();
        
        for (Node node1 : gspn1.nodes) 
            if (node1 instanceof ColorVar) 
                map1.put(node1.getUniqueName(), (ColorVar)node1);
        for (Node node2 : gspn2.nodes) 
            if (node2 instanceof ColorVar) 
                map2.put(node2.getUniqueName(), (ColorVar)node2);
        
        for (Map.Entry<String, ColorVar> ee1 : map1.entrySet()) {
            if (map2.containsKey(ee1.getKey())) {
                // common object: join
                ColorVar c1 = (ColorVar)ee1.getValue();
                ColorVar c2 = (ColorVar)map2.get(ee1.getKey());
                ColorVar newColorVar = (ColorVar)Util.deepCopy(c1);
                if (!c1.getDomainExpr().getExpr().equals(c2.getDomainExpr().getExpr())) 
                {
                    warnings.add("Composition conflict for color variable "+newColorVar.getUniqueName());
                }
                result.nodes.add(makeNodeNameUnique(newColorVar));
                
                // create a duplicated color var for later use, and register its unique name
                ColorVar dupColorVar = (ColorVar)Util.deepCopy(newColorVar);
                makeNodeNameTotallyUnique(dupColorVar);
                dupColorVar.setX(dupColorVar.getX() + 5);
                dupColorVars.put(c1.getUniqueName(), dupColorVar);
            }
            else {
                ColorVar newColorVar = (ColorVar)Util.deepCopy(ee1.getValue());
                result.nodes.add(makeNodeNameUnique(newColorVar));
            }
        }
        for (Map.Entry<String, ColorVar> ee2 : map2.entrySet()) {
            if (!map1.containsKey(ee2.getKey())) {
                ColorVar newColorVar = (ColorVar)Util.deepCopy(ee2.getValue());
                shiftNode(newColorVar);
                result.nodes.add(makeNodeNameUnique(newColorVar));
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
        Map<String, ConstantID> map1 = new HashMap<>();
        Map<String, ConstantID> map2 = new HashMap<>();
        
        for (Node node1 : gspn1.nodes) 
            if (node1 instanceof ConstantID) 
                map1.put(node1.getUniqueName(), (ConstantID)node1);
        for (Node node2 : gspn2.nodes) 
            if (node2 instanceof ConstantID) 
                map2.put(node2.getUniqueName(), (ConstantID)node2);
        
        for (Map.Entry<String, ConstantID> ee1 : map1.entrySet()) {
            if (map2.containsKey(ee1.getKey())) {
                // common object: join
                ConstantID c1 = (ConstantID)ee1.getValue();
                ConstantID c2 = (ConstantID)map2.get(ee1.getKey());
                ConstantID newConst = (ConstantID)Util.deepCopy(c1);
                if (c1.getConstType() != c2.getConstType() ||
                    !c1.getColorDomainName().equals(c2.getColorDomainName()) ||
                    !c1.getConstantExpr().getExpr().equals(c2.getConstantExpr().getExpr())) 
                {
                    warnings.add("Composition conflict for constant "+newConst.getUniqueName());
                }
                result.nodes.add(makeNodeNameUnique(newConst));
            }
            else {
                ConstantID newConst = (ConstantID)Util.deepCopy(ee1.getValue());
                result.nodes.add(makeNodeNameUnique(newConst));
            }
        }
        for (Map.Entry<String, ConstantID> ee2 : map2.entrySet()) {
            if (!map1.containsKey(ee2.getKey())) {
                ConstantID newConst = (ConstantID)Util.deepCopy(ee2.getValue());
                shiftNode(newConst);
                result.nodes.add(makeNodeNameUnique(newConst));
            }
        }
    }
    
    //=========================================================================
    private void joinTemplateVariables() {
        Map<String, TemplateVariable> map1 = new HashMap<>();
        Map<String, TemplateVariable> map2 = new HashMap<>();
        
        for (Node node1 : gspn1.nodes) 
            if (node1 instanceof TemplateVariable) 
                map1.put(node1.getUniqueName(), (TemplateVariable)node1);
        for (Node node2 : gspn2.nodes) 
            if (node2 instanceof TemplateVariable) 
                map2.put(node2.getUniqueName(), (TemplateVariable)node2);
        
        for (Map.Entry<String, TemplateVariable> ee1 : map1.entrySet()) {
            if (map2.containsKey(ee1.getKey())) {
                // common object: join
                TemplateVariable c1 = (TemplateVariable)ee1.getValue();
                TemplateVariable c2 = (TemplateVariable)map2.get(ee1.getKey());
                TemplateVariable newTVar = (TemplateVariable)Util.deepCopy(c1);
                if (c1.getType()!= c2.getType()) 
                {
                    warnings.add("Composition conflict for template variable "+newTVar.getUniqueName());
                }
                result.nodes.add(makeNodeNameUnique(newTVar));
            }
            else {
                TemplateVariable newTVar = (TemplateVariable)Util.deepCopy(ee1.getValue());
                result.nodes.add(makeNodeNameUnique(newTVar));
            }
        }
        for (Map.Entry<String, TemplateVariable> ee2 : map2.entrySet()) {
            if (!map1.containsKey(ee2.getKey())) {
                TemplateVariable newTVar = (TemplateVariable)Util.deepCopy(ee2.getValue());
                shiftNode(newTVar);
                result.nodes.add(makeNodeNameUnique(newTVar));
            }
        }
    }
    
    //=========================================================================
    private void joinTextBoxes() {
        int new_id = 0;
        
        for (Node node1 : gspn1.nodes) {
            if (node1 instanceof TextBox) { 
                TextBox newTBox = (TextBox)Util.deepCopy(node1);
                newTBox.setUniqueName("textbox__"+(new_id++));
                result.nodes.add(newTBox);
            }
        }
        for (Node node2 : gspn2.nodes) {
            if (node2 instanceof TextBox) { 
                TextBox newTBox = (TextBox)Util.deepCopy(node2);
                newTBox.setUniqueName("textbox__"+(new_id++));
                result.nodes.add(shiftNode(newTBox));
            }
        }
    }
        
    //=========================================================================
    private void joinPlaces() {
        for (Node node1 : gspn1.nodes) {
            if (node1 instanceof Place) {
                final Place p1 = (Place)node1;
                List<Place> crossList1 = null;
                int j = 0;
                for (Node node2 : gspn2.nodes) {
                    if (node2 instanceof Place) {
                        if (shareRestrictedTag(node1, node2, restSetPl)) {
                            final Place p2 = (Place)node2;
                            List<Place> crossList2 = plc2InProd.get(p2);
                            if (crossList1 == null)
                                crossList1 = new LinkedList<>();
                            if (crossList2 == null)
                                crossList2 = new LinkedList<>();

                            Place newPlace = (Place)Util.deepCopy(p1);
                            newPlace.setUniqueName(p1.getUniqueName()+"_"+p2.getUniqueName());
                            makeNodeNameUnique(newPlace);
                            newPlace.setSuperPosTags(mergeTags(node1, node2));
                            newPlace.setX(newPlace.getX() + j*3);
                            newPlace.setY(newPlace.getY() + j*3);
                            if (!p1.getColorDomainName().equals(p2.getColorDomainName()) ||
                                p1.getType() != p2.getType() ||
                                !p1.getInitMarkingExpr().equals(p2.getInitMarkingExpr()) ||
                                !p1.getKroneckerPartition().equals(p2.getKroneckerPartition())) 
                            {
                                warnings.add("Composition conflict for place "+newPlace.getUniqueName());
                            }
                            result.nodes.add(newPlace);
                            j++;

                            crossList1.add(newPlace);
                            crossList2.add(newPlace);
                            plc2InProd.put((Place)node2, crossList2);
                        }
                    }
                }
                
                if (crossList1 != null)
                    plc1InProd.put((Place)node1, crossList1);
            }
        }
        
        for (Node node1 : gspn1.nodes) {
            if (node1 instanceof Place) {
                if (!plc1InProd.containsKey((Place)node1)) {
                    Place newPlace = (Place)Util.deepCopy(node1);
                    makeNodeNameUnique(newPlace);
                    result.nodes.add(newPlace);
                    
                    LinkedList<Place> list = new LinkedList<>();
                    list.add(newPlace);
                    plc1InProd.put((Place)node1, list);
                }
            }
        }
        for (Node node2 : gspn2.nodes) {
            if (node2 instanceof Place) {
                if (!plc2InProd.containsKey((Place)node2)) {
                    Place newPlace = (Place)Util.deepCopy(node2);
                    makeNodeNameUnique(newPlace);
                    shiftNode(newPlace);
                    result.nodes.add(newPlace);
                    
                    LinkedList<Place> list = new LinkedList<>();
                    list.add(newPlace);
                    plc2InProd.put((Place)node2, list);
                }
            }
        }
    }

   //=========================================================================
    private void joinTransitions() {
        for (Node node1 : gspn1.nodes) {
            if (node1 instanceof Transition) {
                final Transition t1 = (Transition)node1;
                List<Transition> crossList1 = null;
                int j = 0;
                for (Node node2 : gspn2.nodes) {
                    if (node2 instanceof Transition) {
                        if (shareRestrictedTag(node1, node2, restSetTr)) {
                            final Transition t2 = (Transition)node2;
                            List<Transition> crossList2 = trn2InProd.get(t2);
                            if (crossList1 == null)
                                crossList1 = new LinkedList<>();
                            if (crossList2 == null)
                                crossList2 = new LinkedList<>();

                            Transition newTransition = (Transition)Util.deepCopy(t1);
                            newTransition.setUniqueName(t1.getUniqueName()+"_"+t2.getUniqueName());
                            makeNodeNameUnique(newTransition);
                            newTransition.setSuperPosTags(mergeTags(node1, node2));
                            newTransition.setX(newTransition.getX() + j*3);
                            newTransition.setY(newTransition.getY() + j*3);
                            if (t1.getType() != t2.getType() ||
                                !t1.getDelay().equals(t2.getDelay()) ||
                                !t1.getPriority().equals(t2.getPriority()) ||
                                !t1.getWeight().equals(t2.getWeight()) ||
                                !t1.getNumServers().equals(t2.getNumServers()) ||
                                !t1.getGuard().equals(t2.getGuard()) ||
                                t1.getRotation() != t2.getRotation()) 
                            {
                                warnings.add("Composition conflict for transition "+newTransition.getUniqueName());
                            }
                            result.nodes.add(newTransition);
                            j++;

                            crossList1.add(newTransition);
                            crossList2.add(newTransition);
                            trn2InProd.put((Transition)node2, crossList2);
                        }
                    }
                }
                
                if (crossList1 != null)
                    trn1InProd.put((Transition)node1, crossList1);
            }
        }
        
        for (Node node1 : gspn1.nodes) {
            if (node1 instanceof Transition) {
                if (!trn1InProd.containsKey((Transition)node1)) {
                    Transition newTransition = (Transition)Util.deepCopy(node1);
                    makeNodeNameUnique(newTransition);
                    result.nodes.add(newTransition);
                    
                    LinkedList<Transition> list = new LinkedList<>();
                    list.add(newTransition);
                    trn1InProd.put((Transition)node1, list);
                }
            }
        }
        for (Node node2 : gspn2.nodes) {
            if (node2 instanceof Transition) {
                if (!trn2InProd.containsKey((Transition)node2)) {
                    Transition newTransition = (Transition)Util.deepCopy(node2);
                    makeNodeNameUnique(newTransition);
                    shiftNode(newTransition);
                    result.nodes.add(newTransition);
                    
                    LinkedList<Transition> list = new LinkedList<>();
                    list.add(newTransition);
                    trn2InProd.put((Transition)node2, list);
                }
            }
        }
    }

    //=========================================================================
    private void joinEdges(GspnEdge.Kind kind, 
                           Map<Tuple<Place, Transition>, Tuple<GspnEdge, GspnEdge>> edgeMap) 
    {
        // prepare all edges of type @kind from net1
        for (Edge edge1 : gspn1.edges) {
            GspnEdge e1 = (GspnEdge)edge1;
            if (kind != e1.getEdgeKind())
                continue;
            Place p1 = e1.getConnectedPlace();
            List<Place> listPlace1 = plc1InProd.get(p1);
            Transition t1 = e1.getConnectedTransition();
            List<Transition> listTr1 = trn1InProd.get(t1);

            for (Place plr : listPlace1) {
                for (Transition trr : listTr1) {
                    Tuple<Place, Transition> key = new Tuple<>(plr, trr);
                    edgeMap.put(key, new Tuple<>(e1, null));
                }
            }
        }

        // prepare all edges of type @kind from net2
        for (Edge edge2 : gspn2.edges) {
            GspnEdge e2 = (GspnEdge)edge2;
            if (kind != e2.getEdgeKind())
                continue;
            Place p2 = e2.getConnectedPlace();
            List<Place> listPlace2 = plc2InProd.get(p2);
            Transition t2 = e2.getConnectedTransition();
            List<Transition> listTr2 = trn2InProd.get(t2);

            for (Place plr : listPlace2) {
                for (Transition trr : listTr2) {
                    Tuple<Place, Transition> key = new Tuple<>(plr, trr);
                    if (edgeMap.containsKey(key))
                        edgeMap.put(key, new Tuple<>(edgeMap.get(key).x, e2));
                    else
                        edgeMap.put(key, new Tuple<>(null, e2));
                }
            }
        }
        
        if (verbose)
            printEdgeHelpers(kind, edgeMap);

        // read back all edges, then compose and insert in the result net
        for (Map.Entry<Tuple<Place, Transition>, Tuple<GspnEdge, GspnEdge>> ee : edgeMap.entrySet()) {
            Place resultPlace = ee.getKey().x;
            Transition resultTrans = ee.getKey().y;
            GspnEdge e1 = ee.getValue().x, e2 = ee.getValue().y;
            int tailMagnet, headMagnet;
            ArrayList<Point2D> points;
            boolean isBroken;
            String mult;

            if (e1 == null) { // only edge from net2
                headMagnet = e2.getHeadMagnet();
                tailMagnet = e2.getTailMagnet();
                mult = e2.getMultiplicity();
                isBroken = e2.isBroken;
                points = composeEdgePoints(e1, null);
            }
            else if (e2 == null) { // only edge from net1
                headMagnet = e1.getHeadMagnet();
                tailMagnet = e1.getTailMagnet();
                mult = e1.getMultiplicity();
                isBroken = e1.isBroken;
                points = composeEdgePoints(null, e2);
            }
            else { // combine edge from net1 + edge from net2
                headMagnet = e1.getHeadMagnet();
                tailMagnet = e1.getTailMagnet();
                isBroken = e1.isBroken || e2.isBroken;
                points = composeEdgePoints(e1, null); // NOTE: use only e1, do not use e2!

                // Combine multiplicities
                String m1 = e1.getMultiplicity(), m2 = e2.getMultiplicity();
                if (!e1.getConnectedPlace().getColorDomainName().equals(e2.getConnectedPlace().getColorDomainName())) {
                    warnings.add("Composing arcs from place "+e1.getConnectedPlace().getUniqueName()+
                                 " in net1 with place "+e2.getConnectedPlace().getUniqueName()+
                                 " with different color domains.");
                    mult = m1+" + "+m2;
                }
                else {
                    if (e1.getConnectedPlace().isInNeutralDomain()) {
                        try {
                            if (e1.getTypeOfConnectedPlace() == TokenType.DISCRETE) {
                                int i1 = Integer.parseInt(m1);
                                int i2 = Integer.parseInt(m2);
                                mult = "" + (i1 + i2);
                            }
                            else {
                                double i1 = Double.parseDouble(m1);
                                double i2 = Double.parseDouble(m2);
                                mult = "" + (i1 + i2);                                
                            }
                        }
                        catch (NumberFormatException e) {
                            mult = m1+" + "+m2;
                        }
                    }
                    else { // color expressions
                        for (ColorVar cvar2 : e2.getColorVarsInUse()) {
                            for (ColorVar cvar1 : e1.getColorVarsInUse()) {
                                if (cvar1.getUniqueName().equals(cvar2.getUniqueName())) {
                                    // replace all occurrences of cvar1 in m2 with dupCvar
                                    ColorVar dupCvar = dupColorVars.get(cvar1.getUniqueName());
                                    usedDupColorVar.add(dupCvar);
                                    
                                    // rewrite cvar1 -> dupCvar
                                    // regex from: group 1 = any non-alphanumeric char, or start of string
                                    //             group 2 = searched identifier
                                    //             group 3 = any non-alphanumeric char, or end of string
                                    // regex to: group 1 + replaced identifier + group 3
                                    String from = "([^a-zA-Z0-9_]|^)("+cvar1.getUniqueName()+")([^a-zA-Z0-9_]|$)";
                                    String to = "$1"+dupCvar.getUniqueName()+"$3";
                                    m2 = m2.replaceAll(from, to);
                                    
                                    break;
                                }
                            }
                        }
                        
                        mult = m1+" + "+m2;
                    }
                }
            }

            GspnEdge newEdge = new GspnEdge(null, tailMagnet, null, headMagnet, points, isBroken, kind, mult);
            newEdge.setConnectedPlace(resultPlace, kind);
            newEdge.setConnectedTransition(resultTrans, kind);
            result.edges.add(newEdge);
        }
    }
    
//    public static void main(String[] args) {
//        System.out.println("<c1> + <c11>".replaceAll("[^a-zA-Z0-9_]", "#"));
//        System.out.println("c1 <c1> + <c11> c1".replaceAll("([^a-zA-Z0-9_]|^)(c1)([^a-zA-Z0-9_]|$)", "$1c2$3"));
//    }
    
    //=========================================================================
    private ArrayList<Point2D> composeEdgePoints(GspnEdge e1, GspnEdge e2) {
        ArrayList<Point2D> points = new ArrayList<>();
        
        points.add(new Point2D.Double(0, 0));
        
        if (e1 != null) {
            for (int ii=1; ii<e1.points.size()-1; ii++)
                points.add(new Point2D.Double(e1.points.get(ii).getX(), 
                                              e1.points.get(ii).getY()));
        }
        if (e2 != null) {
            for (int ii=1; ii<e2.points.size()-1; ii++)
                points.add(shiftPoint2D(new Point2D.Double(e2.points.get(ii).getX(), 
                                                           e2.points.get(ii).getY())));
        }
        
        points.add(new Point2D.Double(0, 0));
        
        return points;
    }

    //=========================================================================
    public void compose() {
        // Join non-place and non-transition objects
        joinColorClasses();
        joinColorVars();
        joinConstants();
        joinTemplateVariables();
        joinTextBoxes();
        
        // Join and compose places and transitions
        joinPlaces();
        joinTransitions();
        if (verbose)
            printNodeHelpers();

        // Generate all the edges connecting the composed places and transitions
        joinEdges(GspnEdge.Kind.INPUT, edgeMapInput);
        joinEdges(GspnEdge.Kind.OUTPUT, edgeMapOutput);
        joinEdges(GspnEdge.Kind.INHIBITOR, edgeMapInhibitor);
        
        // Add the color variables that where used in expression rewritings
        joinDuplicatedColorVarsUsed();
    }
    
    
    
    //=========================================================================
    private void printEdgeHelpers(GspnEdge.Kind kind, 
                                  Map<Tuple<Place, Transition>, Tuple<GspnEdge, GspnEdge>> edgeMap) 
    {
        System.out.println(kind+" EDGES:");
        for (Map.Entry<Tuple<Place, Transition>, Tuple<GspnEdge, GspnEdge>> ee : edgeMap.entrySet()) {
            System.out.print("  ");
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
    }
    
    //=========================================================================
    private void printNodeHelpers() {
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
    }

    
}