/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.unfolding;

import common.Util;
import editor.Main;
import editor.domain.Edge;
import editor.domain.Node;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ColorVar;
import editor.domain.elements.ConstantID;
import editor.domain.elements.GspnEdge;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.TemplateVariable;
import editor.domain.elements.Transition;
import editor.domain.grammar.ColorVarsBinding;
import editor.domain.grammar.DomainElement;
import editor.domain.grammar.EvaluationException;
import editor.domain.grammar.ExpressionLanguage;
import editor.domain.grammar.OutOfBoundColorException;
import editor.domain.grammar.ParserContext;
import editor.domain.grammar.TemplateBinding;
import editor.domain.values.EvaluatedFormula;
import editor.domain.values.MultiSet;
import editor.domain.values.ValuedMultiSet;
import java.awt.geom.Point2D;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;

/** Structural unfolding of a colored Petri net.
 *
 * @author elvio
 */
public class Unfolding {
    
    // GSPN that will be unfolded
    public GspnPage gspn;
    
    // Result of the unfolding
    public GspnPage unfolded;
    
    private ParserContext context;
    
    // Attach long name or short color indices
    public boolean useLongUnfoldedNames = true;
    
    // dx/dy multipliers
    public double dxMult = 3.0, dyMult = 3.0;
    
    // Support structure that remembers the original colored place,
    // and its unfolding for a given color domain element.
    public Map<Place, Map<DomainElement, Place>> placeUnfolding = new HashMap<>();
//    public Map<Tuple<Place, DomainElement>, Place> placeUnfolding = new HashMap<>();
    
    // Support structure that remembers the original Transition and 
    // all its unfolded transitions.
    public Map<Transition, List<Transition>> trnUnfolding = new HashMap<>();
    
    // Use evaluation cache (speeds up unfolding)
    public boolean useEvaluationCache = false;
    
    public Unfolding(GspnPage gspn) {
        this.gspn = gspn;
        unfolded = new GspnPage();
    }
    
    private double next_dx(double dx, int comp, int color) {
        return dx + ((comp&1)==1 ? dxMult*color : 0);
    }
    private double next_dy(double dy, int comp, int color) {
        return dy + ((comp&1)==0 ? dyMult*color : 0);
    }
    private String getColorSuffixName(ColorClass domain, int color) {
        if (useLongUnfoldedNames)
            return domain.getColorName(color);
        else
            return ""+color;
    }
    private String getColorVarSuffixName(ColorClass domain, ColorVar colorVar, int color) {
        if (useLongUnfoldedNames)
            return colorVar.getUniqueName()+"_"+domain.getColorName(color);
        else
            return ""+color;
    }
    
    private void unfoldPlaceRecursively(Place plc, int comp, String prefix, 
                                        int[] colors, EvaluatedFormula initMark,
                                        double dx, double dy,
                                        Map<DomainElement, Place> unfMap) 
    {
        final ColorClass domain = plc.getColorDomain();
        if (comp == domain.getNumClassesInDomain()) {
            // End of recursion - unfolding place for a specific color
            DomainElement elem = new DomainElement(domain, colors);
            EvaluatedFormula unfoldMark = initMark.getMultiSetColorCard(elem);
            
            Place unfPlc = new Place(plc.getUniqueName()+prefix, unfoldMark.toString(), 
                                    plc.getType(), "", plc.getKroneckerPartition(), 
                                    new Point2D.Double(plc.getX()+dx, plc.getY()+dy));
            unfolded.nodes.add(unfPlc);
//            placeUnfolding.put(new Tuple<>(plc, elem), unfPlc);
//            Map<DomainElement, Place> unfMap = placeUnfolding.get(plc);
            unfMap.put(elem, unfPlc);
        }
        else {
            for (int c = 0; c < domain.getColorClass(comp).numColors(); c++) {
                String newPrefix = prefix + "_" +
                        getColorSuffixName(domain.getColorClass(comp), c);
                colors[comp] = c;
                unfoldPlaceRecursively(plc, comp + 1, newPrefix, colors, initMark, 
                                       next_dx(dx, comp, c), next_dy(dy, comp, c), unfMap);
            }
        }
    }
    
    private GspnEdge createNewEdge(GspnEdge oldEdge, Transition newTrn, Place newPlc,
                                   double dx, double dy, String multExpr) 
    {
        ArrayList<Point2D> points = new ArrayList<>();
        for (Point2D pt : oldEdge.points) // translate by dx, dy
            points.add(new Point2D.Double(pt.getX() + dx, pt.getY() + dy));
        
        GspnEdge newEdge = new GspnEdge(null, -1, null, -1, points, 
                oldEdge.isBroken, oldEdge.getEdgeKind(), multExpr);

        switch (oldEdge.getEdgeKind()) {
            case INPUT:
            case INHIBITOR:
                // Place is tail, Transition is head
                newEdge.connectToNode(newPlc, oldEdge.getTailMagnet(), Edge.EndPoint.TAIL);
                newEdge.connectToNode(newTrn, oldEdge.getHeadMagnet(), Edge.EndPoint.HEAD);
                break;
                
            case OUTPUT:
                // Place is head, Transition is tail
                newEdge.connectToNode(newPlc, oldEdge.getHeadMagnet(), Edge.EndPoint.HEAD);
                newEdge.connectToNode(newTrn, oldEdge.getTailMagnet(), Edge.EndPoint.TAIL);
                break;
                
            default:
                throw new UnsupportedOperationException();
        }
        return newEdge;
    }
    
    static class ColorVarComp implements Comparator<ColorVar> {
        @Override public int compare(ColorVar o1, ColorVar o2) {
            return o1.getUniqueName().compareTo(o2.getUniqueName());
        }
    }
    private static ColorVarComp s_colorVarComp = new ColorVarComp();
    
    private class TransitionUnfolder {
        Transition trn;
        Set<ColorVar> colorVarsSet = new HashSet<>();
        Set<GspnEdge> edges = new HashSet<>();
        ColorVarsBinding colorVarsBinding = new ColorVarsBinding();
        
        private ColorVar[] colorVars;
        private ColorClass[] domOfCVars;
        private MultiSet[][] enumeratedColors;
        
        private void unfoldTransition() throws CouldNotUnfoldException {
            // Get the array of the color variables for this transition
            colorVars = colorVarsSet.toArray(new ColorVar[colorVarsSet.size()]);
            // Sort the color variables in order to keep a stable order in the unfolding
            Arrays.sort(colorVars, s_colorVarComp);
            domOfCVars = new ColorClass[colorVars.length];
            enumeratedColors = new MultiSet[colorVars.length][];
            for (int cv=0; cv<colorVars.length; cv++) {
                ColorClass dom = colorVars[cv].findColorClass(gspn);
                domOfCVars[cv] = dom;
                // Enumerate all colors of this color class
                ArrayList<MultiSet> allMSets = new ArrayList<>();
                for (int col = 0; col < dom.numColors(); col++) {
                    DomainElement elem = new DomainElement(dom, new int[]{col});
                    Set<DomainElement> elemSet = new HashSet<>();
                    elemSet.add(elem);
                    allMSets.add(MultiSet.makeNew(dom, elemSet));
                }
                enumeratedColors[cv] = allMSets.toArray(new MultiSet[allMSets.size()]);
            }
            
            unfoldRecursively(0, 0, 0);
        }
    
        private void unfoldRecursively(int comp, double dx, double dy) throws CouldNotUnfoldException
        {
            if (comp < colorVars.length) { // Bound the next color variable
                // Enumerate colors of color variables
                MultiSet[] allColors = enumeratedColors[comp];
                for (int col = 0; col < allColors.length; col++) {
                    colorVarsBinding.bind(colorVars[comp], allColors[col]);
                    unfoldRecursively(comp + 1, next_dx(dx, comp, col), next_dy(dy, comp, col));
                }
//                ColorClass dom = colorVars[comp].findColorClass(gspn);            
//                for (int col = 0; col < dom.numColors(); col++) {
//                    DomainElement elem = new DomainElement(dom, new int[]{col});
//                    Set<DomainElement> elemSet = new HashSet<>();
//                    elemSet.add(elem);
//                    colorVarsBinding.bind(colorVars[comp], MultiSet.makeNew(dom, elemSet));
//                    unfoldRecursively(comp + 1, next_dx(dx, comp, col), next_dy(dy, comp, col));
//                }
            }
            else { // All color variables are bound
                try {
                    // Evaluate transition guard. In that case, partial evaluation is disabled since we need to
                    // arrive to a true/false value.
                    context.partialEvaluation = false;
                    boolean guardResult = trn.evaluateGuard(context, null, colorVarsBinding).getScalarBoolean();
                    context.partialEvaluation = true;
                    
//                    System.out.println("\n"+trn.getUniqueName()+"_"+mode.toString());
                    if (!guardResult) {
//                        System.out.println("  Evaluating guard: "+trn.getGuard()+"  ==>  False");
                        return;
                    }
//                    System.out.println("  Evaluating guard: "+trn.getGuard()+"  ==>  True");

                    // Generate the mode variable with form:  x_c1_y_c2_z_c3
                    StringBuilder mode = new StringBuilder();
                    for (int c=0; c<colorVars.length; c++) {
                        int color = colorVarsBinding.getValueBoundTo(colorVars[c]).getElement(0).getColor(0);
                        ColorClass colorDom = colorVarsBinding.getValueBoundTo(colorVars[c]).getDomain();
                        mode.append("_").append(getColorVarSuffixName(colorDom, colorVars[c], color));
                    }

                    //                System.out.println(trn.getUniqueName()+"_"+mode.toString()+"  -> "+reducedGuard);
                    Transition unfTrn = new Transition(trn.getUniqueName()+mode.toString(), trn.getType(), 
                            trn.hasDelay() ? trn.reduceDelay(context, null, colorVarsBinding, ExpressionLanguage.PNPRO) : "1.0",
                            trn.hasPriority() ? trn.reducePriority(context, null, colorVarsBinding, ExpressionLanguage.PNPRO) : "1",
                            trn.hasWeight() ? trn.reduceWeight(context, null, ExpressionLanguage.PNPRO) : "1.0",
                            trn.hasNumServers() ? trn.reduceNumServers(context, null, ExpressionLanguage.PNPRO) : "Infinite",
                            "True",
                            trn.getRotation(), new Point2D.Double(trn.getX() + dx, trn.getY() + dy));

                    // Reconnect all the edges to this transition
                    List<GspnEdge> newEdges = new LinkedList<>();
                    for (GspnEdge edge : edges) {
                        boolean trnIsHead = (edge.getHeadNode() == trn);
                        assert trnIsHead || (edge.getTailNode() == trn);
                        Place plc = edge.getConnectedPlace();

                        EvaluatedFormula arcMult = edge.evaluateMultiplicity(context, null, colorVarsBinding);
//                        System.out.println("     "+edge.getMultiplicity()+"  =>  "+arcMult);
                        if (plc.isInColorDomain()) {
                            // Arc multiplicity selects the proper unfolded place
                            if (!arcMult.isMultiSet())
                                throw new CouldNotUnfoldException("Could not unfold arc between transition "+trn.getUniqueName()+
                                        " and place "+plc.getUniqueName()+" with multiplicity "+edge.getMultiplicity()+
                                        ": multiplicity does not unfold to a fixed set of places.");

                            // Select the unfolded places connected to this edge
//                            int countNumEdges = 0;
                            ValuedMultiSet vmset = (ValuedMultiSet)arcMult;
//                            System.out.println(edge.getMultiplicity() + " = " + vmset);
                            for (int el=0; el < vmset.numElements(); el++) {
                                DomainElement elem = vmset.getElement(el);
                                EvaluatedFormula newArcMult = vmset.getValue(el);
                                if (newArcMult.equalsZero()) {
//                                    System.out.println("Arc of "+trn.getUniqueName()+" has mult=0 after unfolding.");
                                    //return;
                                    continue;
                                }
//                                Place unfPlc = placeUnfolding.get(new Tuple<>(plc, elem));
                                Place unfPlc = placeUnfolding.get(plc).get(elem);

                                GspnEdge newEdge = createNewEdge(edge, unfTrn, unfPlc, dx, dy, newArcMult.toString());
                                newEdges.add(newEdge);
//                                countNumEdges++;
                            }
//                            if (0 == countNumEdges) {
//                                // No places are in correspondence with this edge.
//                                // This may happen with loose color expression checks in PNML
//                                // when we have an expression like: <x++>, where x is
//                                // a color variable in an enumerative color class, and x++ is out-of-bound
//                                // In this case, all the transition should be thrown away, not only this edge.
//                                //  With the current code, this will not happen, since an OutOfBoundColorException
//                                // will be thrown. But anyway, it is reasonable to keep this code.
//                                return;
//                            }
                        }
                        else {
//                            Place unfPlc = placeUnfolding.get(new Tuple<>(plc, DomainElement.NEUTRAL_ELEMENT));
                            Place unfPlc = placeUnfolding.get(plc).get(DomainElement.NEUTRAL_ELEMENT);
                            assert unfPlc != null;
                            GspnEdge newEdge = createNewEdge(edge, unfTrn, unfPlc, dx, dy, arcMult.toString());
                            newEdges.add(newEdge);
                        }
                    }
                    
                    // Add all the nodes if everything went ok (no evaluation exceptions)
                    unfolded.nodes.add(unfTrn);
                    for (GspnEdge e : newEdges)
                        unfolded.edges.add(e);
                    storeUnfolded(unfTrn);
                }
                catch (OutOfBoundColorException e) {
                    System.out.print("Throwing away instance of "+trn.getUniqueName());
                    for (int c=0; c<colorVars.length; c++) {
                        int color = colorVarsBinding.getValueBoundTo(colorVars[c]).getElement(0).getColor(0);
                        ColorClass colorDom = colorVarsBinding.getValueBoundTo(colorVars[c]).getDomain();
                        System.out.print(" "+colorVars[c].getUniqueName()+"="+colorDom.getColorName(color));
                    }
                    System.out.println("");
                    // siliently throw away the transition...
                }
                catch (EvaluationException e) {
                    Main.logException(e, true);
                }
            }
        }
        
        private void storeUnfolded(Transition unfTrn) {
            List<Transition> l = trnUnfolding.get(trn);
            if (l == null) {
                l = new LinkedList<>();
                trnUnfolding.put(trn, l);                   
            }
            l.add(unfTrn);
        }
    }
    
    public void unfold() throws CouldNotUnfoldException {
        try {
            context = new ParserContext(gspn);
            context.templateVarsBinding = new TemplateBinding();
            if (useEvaluationCache) {
                context.evaluationCache = new HashMap<>();
                context.reduceCache = new HashMap<>();
            }
            gspn.compileParsedInfo(context);
        }
        catch (Exception e) {
            gspn.compileParsedInfo(null);
            throw new CouldNotUnfoldException("Color class definitions are parametric "
                    + "and not unfoldable.\nReason: "+e.getMessage());
        }
        try {
            // The rest of the unfolding procedure will not expand parameters in expressions
            context.partialEvaluation = true;

            // Unfold places
            for (Node node : gspn.nodes) {
                if (node instanceof Place) {
                    Place p = (Place)node;
                    Map<DomainElement, Place> unfMap = new HashMap<>();
                    placeUnfolding.put(p, unfMap);
                    if (p.isInColorDomain()) {
                        ColorClass dom = p.getColorDomain();
                        EvaluatedFormula initMark = p.evaluateInitMarking(context);
                        int[] colors = new int[dom.getNumClassesInDomain()];

                        unfoldPlaceRecursively(p, 0, "", colors, initMark, 0, 0, unfMap);
                    }
                    else { // Uncolored place, just copy it
                        Place copy = (Place)Util.deepCopy(p);
                        unfolded.nodes.add(copy);
//                        placeUnfolding.put(new Tuple<>(p, DomainElement.NEUTRAL_ELEMENT), copy);
                        unfMap.put(DomainElement.NEUTRAL_ELEMENT, copy);
                    }
                }
            }

            // Unfold transitions and edges
            for (Node node : gspn.nodes) {
                if (node instanceof Transition) {
                    TransitionUnfolder tu = new TransitionUnfolder();
                    tu.trn = (Transition)node;
                    // Color vars in transition guards and expressions
                    tu.colorVarsSet.addAll(tu.trn.getColorVarsInUse());
                    // Color vars in transitions edges
                    for (Edge e : gspn.edges) {
                        if (e instanceof GspnEdge && ((GspnEdge)e).getConnectedTransition() == tu.trn) {
                            tu.colorVarsSet.addAll(((GspnEdge)e).getColorVarsInUse());
                            tu.edges.add((GspnEdge)e);
                        }
                    }
                    tu.unfoldTransition();
                }
            }

            // Copy all the other elements that are not color definitions and color variables
            for (Node node : gspn.nodes) {
                if (node instanceof Place || node instanceof Transition ||
                    node instanceof ColorClass || node instanceof ColorVar)
                    continue;
                if (node instanceof ConstantID && ((ConstantID)node).isInColorDomain())
                    continue;
                if (node instanceof TemplateVariable && 
                    !((TemplateVariable)node).getColorDomain().isNeutralDomain())
                    continue;
                unfolded.nodes.add((Node)Util.deepCopy(node));
            }
        }
        finally {
            gspn.compileParsedInfo(null);
        }
    }
}
