/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.play;

import common.Tuple;
import editor.domain.elements.DtaEdge;
import editor.domain.elements.DtaLocation;
import editor.domain.elements.DtaPage;
import editor.domain.Edge;
import editor.domain.elements.GspnEdge;
import editor.domain.elements.GspnPage;
import editor.domain.Node;
import editor.domain.elements.Place;
import editor.domain.elements.Transition;
import static editor.domain.elements.GspnEdge.Kind.INHIBITOR;
import static editor.domain.elements.GspnEdge.Kind.INPUT;
import static editor.domain.elements.GspnEdge.Kind.OUTPUT;
import editor.domain.elements.ClockVar;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ColorVar;
import editor.domain.grammar.ColorVarsBinding;
import editor.domain.grammar.DomainElement;
import editor.domain.values.EvaluatedFormula;
import editor.domain.grammar.EvaluationException;
import editor.domain.grammar.ParserContext;
import editor.domain.grammar.TemplateBinding;
import editor.domain.values.Bound;
import editor.domain.values.IntScalarValue;
import editor.domain.values.ListOfBounds;
import editor.domain.values.MultiSet;
import editor.domain.values.RealScalarValue;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.TreeSet;

/** The Token Game engine for GSPNs and DTAs, used by the "Play" command.
 *
 * @author elvio
 */
public class GspnDtaPlayEngine {
    
    // Await animation speed
    public final static int ANIM_PULSE_SPEED = 500; // millisec

    // Tokens speed during firing animation.
    public final static int ANIM_FIRING_SPEED = 25; // millisec
    public final static int ANIM_FIRED_TOKENS_SPEED = 40; //millisec per unit
    
    // Total flow duration
    public final static int ANIM_CONTINUOUS_FLOW = 600; // millisec
    
    public final static double EPSILON_VALUE = 0.00001;
    public final static EvaluatedFormula EPSILON = RealScalarValue.makeNew(EPSILON_VALUE);
    

    public static JointState getInitialState(GspnPage gspn, TemplateBinding gspnBinding,
                                             DtaPage dta, TemplateBinding dtaBinding,
                                             boolean isTimedSimulation) {
        JointState initState = new JointState();
        initState.stateNum = 0;
        initState.isTimedSimulation = isTimedSimulation;
        initState.time = 0.0;
        initState.nextTimeBound = -1;
        
        try {
            ParserContext gspnContext = new ParserContext(gspn);
            ParserContext dtaContext = new ParserContext(dta);
            gspnContext.templateVarsBinding = gspnBinding;
            dtaContext.templateVarsBinding = dtaBinding;
            dtaContext.bindingContext = gspnContext;  // DTA -> GSPN
            gspnContext.bindingContext = gspnContext; // GSPN -> GSPN
            gspn.compileParsedInfo(gspnContext);
            if (dta != null)
                dta.compileParsedInfo(dtaContext);
            
            for (Node n : gspn.nodes) {
                if (n instanceof Place) {
                    Place place = (Place)n;
                    EvaluatedFormula ef = place.evaluateInitMarking(gspnContext);
                    initState.marking.put(place, ef);
                }
            }        

            if (dta != null) {
                // Initialize the clock variables to zero.
                if (isTimedSimulation) {
                    for (Node node : dta.nodes) {
                        if (node instanceof ClockVar) {
                            ClockVar x = (ClockVar)node;
                            initState.continuousVars.put(x, 0.0);
                        }
                    }
                }
                if (initState.isTangible()) {
                    // Find the accepting initial location
                    initState.currLoc = initialLocationMatching(dta, initState, dtaContext);
                    if (initState.isErrorState())
                        return initState;
                }
            }
            
            initState.markingText = initState.getMarkingRepr(gspnContext);
            prepareEnabledSet(gspnContext, gspn, dtaContext, dta, initState, null);
        }
        catch (EvaluationException ee) {
            initState.setErrorState(ee.getMessage());
        }
        finally {
            gspn.compileParsedInfo(null);
            if (dta != null)
                dta.compileParsedInfo(null);
        }
        return initState;
    }
    
    
    private static DtaLocation initialLocationMatching(DtaPage dta, JointState state, ParserContext dtaContext) {
        DtaLocation initLoc = null;
        for (Node n : dta.nodes) {
            if (n instanceof DtaLocation) {
                DtaLocation loc = (DtaLocation)n;
                if (loc.isInitial() && loc.evaluateStatePropositionExpr(dtaContext, state).getScalarBoolean()) {
                    if (initLoc != null) {
                        state.setErrorState("Initial DTA location is not unique.");
                        return JointState.BOTTOM_LOCATION;
                    }
                    initLoc = loc;
                }
            }
        }
        if (initLoc != null)
            return initLoc;
        return JointState.BOTTOM_LOCATION;
    }
    
    
//    private static enum TypeMode {
//        DISCRETE_PLACE_INSTANTANEOUS_TRANSITION,
//        DISCRETE_PLACE_FLOW_TRANSITION,
//        CONTINUOUS_PLACE_INSTANTANEOUS_TRANSITION,
//        CONTINUOUS_PLACE_FLOW_TRANSITION;
//    }
//    
//    private static TypeMode typeModeOf(Place p, Transition t) {
//        if (p.isDiscrete()) {
//            if (t.getFiringMode() == FiringMode.INSTANTANEOUS)
//                return TypeMode.DISCRETE_PLACE_INSTANTANEOUS_TRANSITION;
//            else
//                return TypeMode.DISCRETE_PLACE_FLOW_TRANSITION;
//        }
//        else { // p is continuous
//            if (t.getFiringMode() == FiringMode.INSTANTANEOUS)
//                return TypeMode.CONTINUOUS_PLACE_INSTANTANEOUS_TRANSITION;
//            else
//                return TypeMode.CONTINUOUS_PLACE_FLOW_TRANSITION;
//        }
//    }
    
    // Determine the list of color variable bindings that could enable a transition
    private static Set<ColorVarsBinding> getBindingsOfTransition(GspnPage gspn, ParserContext gspnContext, 
                                                                 Transition trn)
    {
        Set<ColorVar> trnColorVars = new HashSet<>();
        if (trn.getColorVarsInUse() != null)
            trnColorVars.addAll(trn.getColorVarsInUse());
        
        for (Edge ee : gspn.edges) {
            if (!(ee instanceof GspnEdge))
                continue;
            
            GspnEdge e = (GspnEdge)ee;
            if (e.getConnectedTransition() != trn)
                continue;
            
            trnColorVars.addAll(e.getColorVarsInUse());
        }
        
        Set<ColorVarsBinding> outBindings = new HashSet<>();
        if (trnColorVars.isEmpty()) {
            outBindings.add(ColorVarsBinding.EMPTY_BINDING);
            return outBindings;
        }
        
        // Generate all the possible color bindings of the involved color variables.
        ColorVar[] vars = new ColorVar[trnColorVars.size()];
        trnColorVars.toArray(vars);
        int[] domainSizes = new int[vars.length];
        ColorClass[] colorClasses = new ColorClass[vars.length];
        MultiSet[] values = new MultiSet[vars.length];
        for (int i=0; i<vars.length; i++) {
            colorClasses[i] = vars[i].findColorClass(gspn); 
            domainSizes[i] = colorClasses[i].numColors();
        }
        
        generateBindings(gspnContext, 0, vars, domainSizes, 
                         values, colorClasses, outBindings);
        
        return outBindings;
    }
    
    private static void generateBindings(ParserContext gspnContext, int level, ColorVar[] vars, int[] domainSizes, 
                                          MultiSet[] values, ColorClass[] colorClasses,
                                          Set<ColorVarsBinding> outBindings) 
    {
        if (level == vars.length) {
            ColorVarsBinding cvb = new ColorVarsBinding();
            for (int i=0; i<vars.length; i++)
                cvb.bind(vars[i], values[i]);
            outBindings.add(cvb);
        }
        else {
            Set<DomainElement> set = new TreeSet<>();
            int[] color = new int[1];
            for (int i=0; i<domainSizes[level]; i++) {
                color[0] = i;
                set.add(new DomainElement(colorClasses[level], color));
                values[level] = MultiSet.makeNew(colorClasses[level], set);
                
                generateBindings(gspnContext, level + 1, vars, domainSizes, 
                                 values, colorClasses, outBindings);
                
                set.clear();
            }
        }
    }
    
    // determines if a given GSPN transition has concession in a marking
    private static boolean hasTransitionConcession(GspnPage gspn, ParserContext gspnContext, 
                                                   Transition trn, ColorVarsBinding binding, 
                                                   JointState state)
    {
        if (trn.hasGuard() && !trn.evaluateGuard(gspnContext, state, binding).getScalarBoolean())
            return false;
        
        for (Edge ee : gspn.edges) {
            if (ee instanceof GspnEdge) {
                GspnEdge e = (GspnEdge)ee;
                boolean enabled = true;
                
                switch (e.getEdgeKind()) {
                    case INPUT: {
                        Place p = (Place)e.getTailNode();
                        Transition t = (Transition)e.getHeadNode();
                        if (trn != t)
                            break;
                        
                        EvaluatedFormula inMult = e.evaluateMultiplicity(gspnContext, state, binding);
                        if (inMult.equalsZero()) {
                            //enabled = false;
                            break;
                        }
                        if (p.isDiscrete()) {
                            enabled = state.getMarkingOfPlace(p).greaterEqual(inMult).getScalarBoolean();
                        }
                        else if (t.isFiringInstantaneous()) { // continuous place, instantaneous transition
                            enabled = state.getMarkingOfPlace(p).greaterEqual(inMult).getScalarBoolean();
                        }
                        else { // continuous place, flow transition
                            enabled = state.getMarkingOfPlace(p).greater(EPSILON).getScalarBoolean();
                        }
                        break;
                    }
                    case OUTPUT: {
                        break;
                    }
                    case INHIBITOR: {
                        Place p = (Place)e.getTailNode();
                        Transition t = (Transition)e.getHeadNode();
                        if (trn != t)
                            break;
                        EvaluatedFormula outMult = e.evaluateMultiplicity(gspnContext, state, binding);
                        if (p.isDiscrete()) {
                            enabled = state.getMarkingOfPlace(p).less_ZeroAsInf(outMult).getScalarBoolean();
                        }
                        else if (t.isFiringInstantaneous()) { // continuous place, instantaneous transition
                            enabled = state.getMarkingOfPlace(p).less_ZeroAsInf(outMult).getScalarBoolean();
                        }
                        else { // continuous place, flow transition
                            enabled = state.getMarkingOfPlace(p).less_ZeroAsInf(outMult.subtract(EPSILON)).getScalarBoolean();
                        }
                        break;
                    }
                }
                
                if (!enabled) 
                    return false;
            }
        }
        return true;
    }
    
    
    // Return the overall flow of a continuous place moved by enabled flow transitions.
    private static EvaluatedFormula flowOfPlace(GspnPage gspn, ParserContext gspnContext, 
                                                Place plc, JointState state) {
        assert plc.isContinuous() && state.isTangible() && state.isTimedSimulation;
        EvaluatedFormula flow = EvaluatedFormula.makeZero(EvaluatedFormula.Type.REAL, plc.getColorDomain());
        
        // Determine the flow of the continuous places from the enabled timed transitions
        for (Edge ee : gspn.edges) {
            if (ee instanceof GspnEdge) {
                GspnEdge e = (GspnEdge)ee;
                if (e.isDiscrete() || e.isFiringInstantaneous())
                    continue;
                switch (e.getEdgeKind()) {
                    case INPUT: {
                        Place p = (Place)e.getTailNode();
                        Transition t = (Transition)e.getHeadNode();
                        if (p != plc)
                            break;

                        FirableWithBindings<Transition> enabledBind = state.getEnabledFirableBindingsOf(t);
                        if (enabledBind != null) {
                            for (ColorVarsBinding binding : enabledBind.bindings) {
                                // Remove the input flow from the place
                                EvaluatedFormula inMult = e.evaluateMultiplicity(gspnContext, state, binding);
                                flow = flow.subtract(inMult);
                            }
                        }
//                        EvaluatedFormula inMult = e.evaluateMultiplicity(gspnContext, state);
//                        if (state.enabledTransitions.contains(t))
//                            flow -= inMult.getReal();
                        break;
                    }
                    case OUTPUT: {
                        Place p = (Place)e.getHeadNode();
                        Transition t = (Transition)e.getTailNode();
                        if (p != plc)
                            break;

                        FirableWithBindings<Transition> enabledBind = state.getEnabledFirableBindingsOf(t);
                        if (enabledBind != null) {
                            for (ColorVarsBinding binding : enabledBind.bindings) {
                                // Add the output flow to the place
                                EvaluatedFormula outMult = e.evaluateMultiplicity(gspnContext, state, binding);
                                flow = flow.add(outMult);
                            }
                        }
//                        // Add the output flow to the place
//                        EvaluatedFormula outMult = e.evaluateMultiplicity(gspnContext, state);
//                        if (state.enabledTransitions.contains(t)) 
//                            flow += outMult.getReal();
                        break;
                    }
                    case INHIBITOR:
                        break;
                }
            }
        }
        System.out.println("Place "+plc.getUniqueName()+" has flow "+flow);
        return flow;
    }
    
    
    // Returns the time bound when this transition will become 
    // enabled/disabled/inhibited due to the continuous flow
    private static double timeBoundOfTransition(GspnPage gspn, ParserContext gspnContext, 
                                                Transition trn, boolean isEnabled,
                                                ColorVarsBinding binding, JointState state) 
    {
        assert state.isTangible() && state.isTimedSimulation;
//        boolean isEnabled = state.enabledTransitions.contains(trn);
        double upperBound = -1;
        for (Edge ee : gspn.edges) {
            if (ee instanceof GspnEdge) {
                GspnEdge e = (GspnEdge) ee;
//                double newUpperBound = -1;
                EvaluatedFormula allBounds = null;
                if (/*e.isFiringFlow() && */e.isContinuous()) {
                    switch (e.getEdgeKind()) {
                        case INPUT: {
                            Place p = (Place) e.getTailNode();
                            Transition t = (Transition)e.getHeadNode();
                            if (t != trn)
                                break;
//                            EvaluatedFormula mult = e.evaluateMultiplicity(gspnContext, state);
                            EvaluatedFormula placeFlow = state.flowFunction.get(p);

                            if (e.isFiringFlow()) {
                                if (isEnabled) { // Place p is being drained, test when it goes to 0
                                    allBounds = state.getMarkingOfPlace(p).safeDivide(placeFlow.opposite());
                                }
//                                if (placeFlow < 0.0 && isEnabled) { // Place p is being drained, test when it goes to 0
//                                    newUpperBound = state.getMarkingOfPlace(p).getReal() / -placeFlow;
//                                    assert newUpperBound > 0.0;
//                                }
                            }
                            else { // instantaneous transition
                                if (isEnabled) {
                                    // Test when trn will become disabled
                                    EvaluatedFormula inMult = e.evaluateMultiplicity(gspnContext, state, binding);
                                    EvaluatedFormula placeMark = state.getMarkingOfPlace(p);
//                                    assert !placeMark.lessEqual(inMult).getScalarBoolean();
                                    allBounds = placeMark.subtract(inMult).safeDivide(placeFlow.opposite());
                                }
                                else {
                                    // Transition could become enabled
                                    EvaluatedFormula inMult = e.evaluateMultiplicity(gspnContext, state, binding);
                                    EvaluatedFormula placeMark = state.getMarkingOfPlace(p);
//                                    assert !placeMark.greaterEqual(inMult).getScalarBoolean();
                                    allBounds = placeMark.subtract(inMult).safeDivide(placeFlow);
                                }
                                
//                                if (placeFlow < 0.0 && isEnabled) { 
//                                    // Test when trn will become disabled
//                                    EvaluatedFormula inMult = e.evaluateMultiplicity(gspnContext, state, binding);
//                                    EvaluatedFormula placeMark = state.getMarkingOfPlace(p);
//                                    assert !state.getMarkingOfPlace(p).lessEqual(inMult).getScalarBoolean();
//                                    newUpperBound = (state.getMarkingOfPlace(p).getReal() - inMult.getReal()) / -placeFlow;
//                                    assert newUpperBound > 0.0;
//                                }
//                                else if (placeFlow > 0.0 && !isEnabled) {
//                                    // Transition could become enabled
//                                    EvaluatedFormula inMult = e.evaluateMultiplicity(gspnContext, state, binding);
//                                    assert state.getMarkingOfPlace(p).getReal() < inMult.getReal();
//                                    newUpperBound = (inMult.getReal() - state.getMarkingOfPlace(p).getReal()) / placeFlow;
//                                    assert newUpperBound > 0.0;
//                                }
                            }
                            break;
                        }
                        
                        case OUTPUT: 
                            break;

                        case INHIBITOR: {
                            Place p = (Place) e.getTailNode();
                            Transition t = (Transition)e.getHeadNode();
                            if (t != trn)
                                break;
                            EvaluatedFormula mult = e.evaluateMultiplicity(gspnContext, state, binding);
                            EvaluatedFormula placeFlow = state.flowFunction.get(p);
                            EvaluatedFormula placeMark = state.getMarkingOfPlace(p);
//                            EvaluatedFormula inhibLevel = mult.getReal();

                            if (placeMark.less(mult.add(EPSILON)).getScalarBoolean() && isEnabled) {
                                // Transition will be inhibited
                                allBounds = mult.subtract(placeMark).safeDivide(placeFlow);
                            }
                            else if (placeMark.greaterEqual(mult.add(EPSILON)).getScalarBoolean() && !isEnabled) {
                                // Transition could become enabled
                                allBounds = placeMark.subtract(mult).safeDivide(placeFlow.opposite());
                            }
//                            
//                            if (placeMark < inhibLevel + EPSILON_VALUE && placeFlow > 0.0 && isEnabled) {
//                                // Transition will be inhibited
//                                newUpperBound = (inhibLevel - placeMark) / placeFlow;
//                                assert newUpperBound > 0.0;
//                            } 
//                            else if (placeMark >= inhibLevel - EPSILON_VALUE && placeFlow < 0.0 && !isEnabled) {
//                                // Transition could become enabled
//                                newUpperBound = (placeMark - inhibLevel) / -placeFlow;
//                                assert newUpperBound > 0.0;
//                            }

                            break;
                        }
                    }
                }
                if (allBounds != null) {
//                    System.out.println("allBounds = "+allBounds);
                    double newUpperBound = -1;
                    if (allBounds.isScalar()) {
                        newUpperBound = allBounds.getScalarRealOrIntAsReal();
                    }
                    else {
                        allBounds.multisetFilterNegatives();
                        if (!((MultiSet)allBounds).isEmpty()) {
                            newUpperBound = allBounds.multisetMin().getScalarReal();
                        }
                    }
                    if (newUpperBound > 0) {
                        if (upperBound == -1 || newUpperBound < upperBound)
                            upperBound = newUpperBound;
                    }
                }
            }
        }
        // Consider the bound of general (deterministic) transitions
        if (trn.isGeneral() && isEnabled) {
            double age = 0.0, delayBound;
            Tuple<Transition, ColorVarsBinding> trnBind = new Tuple<>(trn, binding);
            if (state.ageOfTransitions.containsKey(trnBind))
                age = state.ageOfTransitions.get(trnBind);
            delayBound = trn.evaluateDelayBound(gspnContext, state, binding).getUpperBound();
            if (delayBound != Double.MAX_VALUE) {
                double fireBound = (delayBound - age);
                assert fireBound >= 0.0;
                if (upperBound == -1 || fireBound < upperBound)
                    upperBound = fireBound;
            }
        }
        
        System.out.println("Bound of "+trn.getUniqueName()+binding+"  is "+upperBound);
        return upperBound;
    }
    

    private static void prepareEnabledSet(ParserContext gspnContext, GspnPage gspn, 
                                          ParserContext dtaContext, DtaPage dta, 
                                          JointState state, JointFiring firing) 
    {
        state.enabledTransitions.clear();
        state.enabledDtaEdges.clear();
        state.nextTimeBound = -1;
        state.priorityLevel = 0;
        state.avgTimeNextTransition = -1;
        
        if (state.isTop() || state.isBot())
            return;
        
        // Determine the enabled GSPN transitions
        for (Node node : gspn.nodes) {
            if (node instanceof Transition) {
                Transition trn = (Transition)node;
                // Generate all the possible bindings that could enable this transition
                Set<ColorVarsBinding> allBindings = getBindingsOfTransition(gspn, gspnContext, trn);
                FirableWithBindings<Transition> firable = new FirableWithBindings<>(trn);
                for (ColorVarsBinding binding : allBindings) {
                    if (hasTransitionConcession(gspn, gspnContext, trn, binding, state)) {
                        // Transition has concession in marking. Determine if it is enabled
                        int trnPrio;
                        if (trn.isImmediate()) {
                            trnPrio = trn.evaluatePriority(gspnContext, state, binding).getScalarInt();
                            // Sanity check: avoid non-positive immediate transition priority
                            if (trnPrio <= 0) {
                                state.setErrorState("Transition "+trn.getUniqueName()+
                                        " has a non-positive priority of "+trnPrio+".");
                                return;
                            }
                        }
                        else
                            trnPrio = 0;

                        if (trnPrio >= state.priorityLevel) {
//                            state.enabledTransitions.add(trn);
                            firable.bindings.add(binding);
                            if (trnPrio > state.priorityLevel) {
                                // Update the priority level
                                state.priorityLevel = trnPrio;
                                // Remove transition that have less priority than trn
                                Iterator<FirableWithBindings<Transition>> trnIt = state.enabledTransitions.iterator();
                                while (trnIt.hasNext()) {
                                    FirableWithBindings<Transition> fwb = trnIt.next();
                                    if (fwb.firable.isTimed() || state.priorityLevel > fwb.firable.evaluatePriority(gspnContext, state, binding).getScalarInt()) {
                                        trnIt.remove();
                                    }
                                }
                            }
                        }                        
                    }
                }
                if (!firable.bindings.isEmpty())
                    state.enabledTransitions.add(firable);
            }
        }

        if (dta != null && firing != null && state.isTangible() && firing.firedNode instanceof Transition) {
            // An Inner edge must fire.
            state.enabledTransitions.clear();
            state.isDtaInnerMove = true;
            for (Edge e : dta.edges) {
                if (e instanceof DtaEdge) {
                    DtaEdge edge = (DtaEdge)e;
                    if (edge.isInner() && 
                        edge.getTailNode() == state.currLoc &&
                        edge.evaluateActionSetExpr(dtaContext, state, (Transition)firing.firedNode, 
                                                   firing.colorVarsBinding).getScalarBoolean() &&
                        ((DtaLocation)edge.getHeadNode()).evaluateStatePropositionExpr
                                    (dtaContext, firing.reachedState).getScalarBoolean()) 
                    {
                        state.enabledDtaEdges.add(edge);
//                        if (innerEdge != null) {
//                            firing.reachedState.setErrorState("Inner edge choice is not deterministic.");
//                            state.currLoc = JointState.BOTTOM_LOCATION;
//                            return;
//                        }
//                        innerEdge = edge;
                    }
                }
            }
            if (state.enabledDtaEdges.isEmpty()) {
                state.currLoc = JointState.BOTTOM_LOCATION;
            }
//            else
//                state.enabledInnerEdges.add(innerEdge);
            return;
        }
        
        // Prepare the flow function for the continuous places 
        // from the enabled continuous transitions
        for (Node node : gspn.nodes) {
            if (node instanceof Place && ((Place)node).isContinuous()) {
                Place plc = (Place)node;
                if (state.isTangible() && state.isTimedSimulation) {
                    EvaluatedFormula flow = flowOfPlace(gspn, gspnContext, plc, state);
                    state.flowFunction.put(plc, flow);
                }
                else
                    state.flowFunction.put(plc, EvaluatedFormula.makeZero(EvaluatedFormula.Type.REAL, 
                                                                           plc.getColorDomain()));
            }
        }

        if (state.isTangible() && state.isTimedSimulation) {
            // Determine the time bound when the continuous flow will change
            // the enabling of some transition.
            for (Node node : gspn.nodes) {
                if (node instanceof Transition) {
                    Transition trn = (Transition)node;
                    Set<ColorVarsBinding> allBindings = getBindingsOfTransition(gspn, gspnContext, trn);
                    FirableWithBindings<Transition> enabledBindings = state.getEnabledFirableBindingsOf(trn);
                    for (ColorVarsBinding binding : allBindings) {
                        boolean isEnabled = (enabledBindings != null &&
                                             enabledBindings.bindings.contains(binding));
                        double bound = timeBoundOfTransition(gspn, gspnContext, trn, 
                                                             isEnabled, binding, state);
                        if (bound != -1) {
                            if (state.nextTimeBound == -1 || state.nextTimeBound > bound)
                                state.nextTimeBound = bound;
                        }
                    }
                }
            }
            
            // Find the time bound of the DTA Inner edges.
            if (dta != null) {
                for (Edge e : dta.edges) {
                    if (e instanceof DtaEdge) {
                        DtaEdge edge = (DtaEdge)e;
                        if (edge.isBoundary()) {
                            double bound = edge.getBoundOf(dtaContext, state);
                            if (bound != -1) {
                                if (state.nextTimeBound == -1 || state.nextTimeBound > bound)
                                    state.nextTimeBound = bound;
                            }
                        }
                    }
                }
            }
        }        

        // Add DTA boundary edges
        if (dta != null) {
            for (Edge e : dta.edges) {
                if (e instanceof DtaEdge) {
                    DtaEdge edge = (DtaEdge)e;
                    JointState evalState = firing==null ? state : firing.reachedState;
                    if (edge.isBoundary() && 
                        edge.getTailNode() == state.currLoc &&
                        ((DtaLocation)edge.getHeadNode()).evaluateStatePropositionExpr
                                        (dtaContext, evalState).getScalarBoolean()) 
                    {
                        // For untimed simulation, always add the boundary edges.
                        // For timed simulation, the edge must also be enabled before
                        // the upperBound.
                        double bound = -1;
                        if (state.isTimedSimulation) {
                            bound = edge.getBoundOf(dtaContext, state);
                            if (bound != -1 && bound > state.nextTimeBound)
                                continue;
                            if (bound != -1 && bound <= 0 + EPSILON_VALUE) {
                                // Boundary edge is urgent!
                                state.enabledTransitions.clear();
                                state.priorityLevel = Integer.MAX_VALUE;
                                state.boundaryDtaEdgesAreUrgent = true;
                                assert !state.isTangible();
                            }
                        }
                        
                        state.enabledDtaEdges.add(edge);
                        if (bound != -1)
                            state.firingTimeSupport.put(new Tuple<Firable, ColorVarsBinding>(edge, null), 
                                    new Bound(bound, bound));
                    }
                }
            }
        }
        
        System.out.println("Next time bound is "+state.nextTimeBound);
        
        // Manage general transitions
        if (state.isTimedSimulation && state.isTangible()) {
            // Clear the memory of the disabled general transitions
            Iterator<Map.Entry<Tuple<Transition, ColorVarsBinding>, Double>> ageIt;
            ageIt = state.ageOfTransitions.entrySet().iterator();
            while (ageIt.hasNext()) {
                Map.Entry<Tuple<Transition, ColorVarsBinding>, Double> e = ageIt.next();
                FirableWithBindings<Transition> enabledBindings = state.getEnabledFirableBindingsOf(e.getKey().x);
//                if (!state.enabledTransitions.contains(e.getKey()))
                if (enabledBindings==null || !enabledBindings.bindings.contains(e.getKey().y))
                    ageIt.remove();
            }
            
            // Set of urgent general transitions, that will fire immediately
            Set<FirableWithBindings<Transition>> urgentTransitions 
                    = new HashSet<>();
            
            // Add the memory of the newly enabled general transitions
            Iterator<FirableWithBindings<Transition>> fwbIt = state.enabledTransitions.iterator();
            while (fwbIt.hasNext()) {
                FirableWithBindings<Transition> fwb = fwbIt.next();
                FirableWithBindings<Transition> urgentFirable = new FirableWithBindings<>(fwb.firable);
                if (fwb.firable.isGeneral()) {
                    boolean mayFire = false;
                    for (ColorVarsBinding binding : fwb.bindings) {
                        Tuple<Transition, ColorVarsBinding> trnBind = new Tuple<>(fwb.firable, binding);
                        if (!state.ageOfTransitions.containsKey(trnBind)) {
                            // t is newly enabled
                            System.out.println("Transition "+fwb.firable.getUniqueName()+binding+" is newly enabled.");
                            state.ageOfTransitions.put(trnBind, 0.0);
                        }
                        // Now remove those deterministic/general timed transitions
                        // that are enabled but cannot fire before the nextTimeBound.
                        // In that case, the transition is removed, but its age is kept.
                        double age = state.ageOfTransitions.get(trnBind);
                        ListOfBounds fwbBounds = fwb.firable.evaluateDelayBound(gspnContext, state, binding);
                        double maxTimeToFire = fwbBounds.getUpperBound() - age;
                        double minTimeToFire = fwbBounds.getLowerBound() - age;
                        assert maxTimeToFire+EPSILON_VALUE >= 0;
                        

                        for (Bound b : fwbBounds.bounds) {
                            // This interval starts before the nextTimeBound
                            if (b.a - age < state.nextTimeBound + EPSILON_VALUE) {
                                // Add this time bound to the list of time bounds for this event
                                state.firingTimeSupport.put(new Tuple<Firable, ColorVarsBinding>
                                            (fwb.firable, binding), 
                                            new Bound(b.a - age, Math.min(b.b - age, state.nextTimeBound)));
//                                // Add a fire point for each Dirac impulse of the general event
//                                if (b.isImpulse()) {
//                                    
//                                    state.fixedTimeToFire.put(new Tuple<Firable, ColorVarsBinding>
//                                            (fwb.firable, binding), new Bound(b.a - age, b.b - age));
//                                }
//                                else {
//                                    // Uniform/Rectangular support before the nextTimeToFire
//                                    // should be treated as non-urgent behaviours
//                                }
                                mayFire = true;
                            }
                        }
                        if (mayFire && maxTimeToFire < EPSILON_VALUE) {
//                          // Treat this general transition like an immediate transition
                            urgentFirable.bindings.add(binding);
                        }

                        
//                        if (state.nextTimeBound < minTimeToFire - EPSILON_VALUE) {
//                            // There is no time to fire! Remove this transition
//                            fwbIt.remove();
//                        }
//                        else { // May fire
//                            if (maxTimeToFire < EPSILON_VALUE) {
//                                // Treat this general transition like an immediate transition
//                                urgentFirable.bindings.add(binding);
////                                urgentTransitions.add(trnBind);
//                            }
//                            state.fixedTimeToFire.put(new Tuple<Firable, ColorVarsBinding>(fwb.firable, binding), maxTimeToFire);
//                            
//                            // Consider also those events that have a urgent bound but also a non-urgent behaviour,
//                            // like a Uniform[a, b] event.
//                            if (minTimeToFire < maxTimeToFire)
//                                nonUrgentGeneralFirable.bindings.add(binding);
//                        }
                    }
                    if (!mayFire) {
                        // There is no time to fire! Remove this transition binding
                        fwbIt.remove();
                    }
                    if (!urgentFirable.bindings.isEmpty())
                        urgentTransitions.add(urgentFirable);
//                    if (!nonUrgentGeneralFirable.bindings.isEmpty())
//                        state.enabledTransitions.add(nonUrgentGeneralFirable);
                }
            }
            for (Map.Entry<Tuple<Transition, ColorVarsBinding>, Double> e : state.ageOfTransitions.entrySet())
                System.out.println("Age of "+e.getKey().x.getUniqueName()+e.getKey().y+" is "+e.getValue());
            
            // Do we have some urgent transitions?
            if (!urgentTransitions.isEmpty()) {
                state.enabledTransitions = urgentTransitions;
                state.priorityLevel = 1;
                state.timedEventsAreUrgent = true;
            }
        }
        
        // Finally, setup the suggested timing of the next event
        if (state.isTimedSimulation && state.isTangible()) {
            state.avgTimeNextTransition = 0.0;
            for (FirableWithBindings<Transition> fwb : state.enabledTransitions) {
                if (fwb.firable.isFiringInstantaneous()) {
                    // Evaluate only the first binding
                    ColorVarsBinding binding = fwb.bindings.iterator().next();
                    if (fwb.firable.isGeneral()) {
                        double delay = fwb.firable.evaluateDelayBound(gspnContext, state, binding).getUpperBound();
                        if (delay != Double.MAX_VALUE)
                            state.avgTimeNextTransition += delay;
                    }
                    else if (fwb.firable.isExponential()) {
                        double rate = fwb.firable.evaluateDelay(gspnContext, state, binding).getScalarReal();
                        state.avgTimeNextTransition += 1.0 / rate;
                    }
                    else throw new UnsupportedOperationException("Missing suggested timing function.");
                }
            }
            state.avgTimeNextTransition /= state.enabledTransitions.size();
            if (Double.isInfinite(state.avgTimeNextTransition) ||
                Double.isNaN(state.avgTimeNextTransition) ||
                state.avgTimeNextTransition <= EPSILON_VALUE)
                state.avgTimeNextTransition = 1.0; // Default value
        }
    }
    
    
    // Apply a continuous transition firing on a marking
    private static void fireContinuousTransition(GspnPage gspn, ParserContext gspnContext,
                                                 Transition trn, ColorVarsBinding binding, 
                                                 JointState startState, JointFiring firing, 
                                                 double elapsedTime)
    {
        assert trn.isFiringFlow();
        assert startState.getEnabledFirableBindingsOf(trn).bindings.contains(binding);
        assert elapsedTime > 0.0 && startState.isTangible();
        
        EvaluatedFormula elapsedTimeReal = RealScalarValue.makeNew(elapsedTime);
        if (!startState.isTimedSimulation)
            return;
        
        for (Edge ee : gspn.edges) {
            if (ee instanceof GspnEdge) {
                GspnEdge e = (GspnEdge)ee;
                if (e.isDiscrete())
                    continue; // Must be a test arc anyway, so @e has no effect.
                switch (e.getEdgeKind()) {
                    case INPUT: {
                        Place p = (Place)e.getTailNode();
                        Transition t = (Transition)e.getHeadNode();
                        if (t != trn)
                            break;
                        
                        EvaluatedFormula passingFlow = e.evaluateMultiplicity(gspnContext, startState, binding).multiply(elapsedTimeReal);
                        EvaluatedFormula newFluidLevel = firing.reachedState.marking.get(p).subtract(passingFlow);
//                        double passingFlow = e.evaluateMultiplicity(gspnContext, startState).getReal()  * elapsedTime;
//                        double newFluidLevel = firing.reachedState.marking.get(p).getReal() - passingFlow;
//                        EvaluatedFormula efFludLevel = new EvaluatedFormula(newFluidLevel);
                        firing.reachedState.marking.put(p, newFluidLevel);
                        firing.markingAtFlowEnd.put(p, newFluidLevel);
                        firing.markingDuringFiring.put(p, newFluidLevel);
                        firing.flowPassed.put(e, passingFlow);

                        break;
                    }

                    case OUTPUT: {
                        Place p = (Place)e.getHeadNode();
                        Transition t = (Transition)e.getTailNode();
                        if (t != trn)
                            break;

                        EvaluatedFormula passingFlow = e.evaluateMultiplicity(gspnContext, startState, binding).multiply(elapsedTimeReal);
                        EvaluatedFormula newFluidLevel = firing.reachedState.marking.get(p).add(passingFlow);
//                        double passingFlow = e.evaluateMultiplicity(gspnContext, startState).getReal()  * elapsedTime;
//                        double newFluidLevel = firing.reachedState.marking.get(p).getReal() + passingFlow;
//                        EvaluatedFormula efFludLevel = new EvaluatedFormula(newFluidLevel);
                        firing.reachedState.marking.put(p, newFluidLevel);
                        firing.markingAtFlowEnd.put(p, newFluidLevel);
                        firing.markingDuringFiring.put(p, newFluidLevel);
                        firing.flowPassed.put(e, passingFlow);
                        break;
                    }
                }
            }
        }
    }
    
    
    // Apply a firing of an istantaneous transition (timed or immediate)
    private static void fireInstantaneousTransition(GspnPage gspn, ParserContext gspnContext,
                                                    Transition trn, ColorVarsBinding binding,
                                                    JointState startState, JointFiring firing)
    {
        assert trn.isFiringInstantaneous();
        assert startState.getEnabledFirableBindingsOf(trn).bindings.contains(binding);
        
        // Apply instantaneous transition firing
        for (Edge ee : gspn.edges) {
            if (ee instanceof GspnEdge) {
                GspnEdge e = (GspnEdge)ee;
                switch (e.getEdgeKind()) {
                    case INPUT: {
                        Place p = (Place)e.getTailNode();
                        Transition t = (Transition)e.getHeadNode();
                        if (t != trn)
                            break;
                        
                        EvaluatedFormula inMult = e.evaluateMultiplicity(gspnContext, startState, binding);
                        if (p.isDiscrete()) {
                            EvaluatedFormula inTokenCount = firing.reachedState.marking.get(p).subtract(inMult);
                            assert !inTokenCount.less(IntScalarValue.ZERO).getScalarBoolean();
//                            int inTokenCount = firing.reachedState.marking.get(p).getInt() - inMult.getInt();
//                            assert inTokenCount >= 0;
//                            EvaluatedFormula efTokenCount = new EvaluatedFormula(inTokenCount);
                            firing.reachedState.marking.put(p, inTokenCount);
                            firing.markingDuringFiring.put(p, inTokenCount);
                            firing.tokensPassed.put(e, inMult);
                            firing.maxInputEdgeLength = Math.max(firing.maxInputEdgeLength, 
                                                                    e.getApproxEdgeLength());
                        }
                        else { // p.isContinuous
//                            double inFluidLevel = firing.reachedState.marking.get(p).getReal() - inMult.getReal();
//                            if (Math.abs(inFluidLevel) < EPSILON)
//                                inFluidLevel = 0.0;
//                            assert inFluidLevel >= 0;
//                            EvaluatedFormula efFludLevel = new EvaluatedFormula(inFluidLevel);
                            
                            EvaluatedFormula inFluidLevel = firing.reachedState.marking.get(p).subtract(inMult);
                            if (inFluidLevel.abs().less(EPSILON).getScalarBoolean()) {
                                inFluidLevel = inFluidLevel.makeZero();
                            }
                                
                            firing.reachedState.marking.put(p, inFluidLevel);
                            firing.markingDuringFiring.put(p, inFluidLevel);
                            firing.tokensPassed.put(e, inMult);
                            firing.maxInputEdgeLength = Math.max(firing.maxInputEdgeLength, 
                                                                 e.getApproxEdgeLength());
                        }
                    }
                    break;

                    case OUTPUT: {
                        Place p = (Place)e.getHeadNode();
                        Transition t = (Transition)e.getTailNode();
                        if (t != trn)
                            break;
                        
                        EvaluatedFormula outMult = e.evaluateMultiplicity(gspnContext, startState, binding);
                        if (p.isDiscrete()) {
//                            int outTokenCount = firing.reachedState.marking.get(p).getInt() + outMult.getInt();
//                            assert outTokenCount >= 0;
//                            EvaluatedFormula efTokenCount = new EvaluatedFormula(outTokenCount);
                            EvaluatedFormula outTokenCount = firing.reachedState.marking.get(p).add(outMult);
                            assert !outTokenCount.less(IntScalarValue.ZERO).getScalarBoolean();
                            firing.reachedState.marking.put(p, outTokenCount);
                            firing.tokensPassed.put(e, outMult);
                            firing.maxOutputEdgeLength = Math.max(firing.maxOutputEdgeLength, 
                                                                  e.getApproxEdgeLength());
                        }
                        else { // p.isContinuous
//                            double outFluidLevel = firing.reachedState.marking.get(p).getReal() + outMult.getReal();
//                            if (Math.abs(outFluidLevel) < EPSILON)
//                                outFluidLevel = 0.0;
//                            assert outFluidLevel >= 0;
//                            EvaluatedFormula efFludLevel = new EvaluatedFormula(outFluidLevel);
                            EvaluatedFormula outFluidLevel = firing.reachedState.marking.get(p).add(outMult);
                            if (outFluidLevel.abs().less(EPSILON).getScalarBoolean())
                                outFluidLevel = outFluidLevel.makeZero();
                            firing.reachedState.marking.put(p, outFluidLevel);
                            firing.tokensPassed.put(e, outMult);
                            firing.maxInputEdgeLength = Math.max(firing.maxInputEdgeLength, 
                                                                 e.getApproxEdgeLength());
                        }
                    }
                    break;
                }
            }
        }
    }
    
    
    public static JointFiring fireTransition(GspnPage gspn, TemplateBinding gspnBinding,
                                             DtaPage dta, TemplateBinding dtaBinding,
                                             JointState startState, Firable firedNode, ColorVarsBinding binding,
                                             JointFiring prevFiring, double elapsedTime, double speedup) 
    {
        assert !startState.isBot() && !startState.isTop() && !startState.isErrorState();
        
        // Create a new marking
        JointFiring firing = new JointFiring(startState, firedNode, binding,
                                             prevFiring, elapsedTime, speedup);

        // Prepare the evaluation contexts
        ParserContext gspnContext = new ParserContext(gspn);
        ParserContext dtaContext = new ParserContext(dta);
        gspnContext.templateVarsBinding = gspnBinding;
        dtaContext.templateVarsBinding = dtaBinding;
        dtaContext.bindingContext = gspnContext;  // DTA -> GSPN
        gspnContext.bindingContext = gspnContext; // GSPN -> GSPN
        
        try {
            // Prepare parsed info (color classe)
            gspn.compileParsedInfo(gspnContext);
            if (dta != null)
                dta.compileParsedInfo(dtaContext);
            
            // Save the marking state before the firing
            for (Entry<Place, EvaluatedFormula> m : startState.marking.entrySet()) {
                firing.markingAtFlowBegin.put(m.getKey(), m.getValue());
                firing.markingAtFlowEnd.put(m.getKey(), m.getValue());
                firing.markingDuringFiring.put(m.getKey(), m.getValue());
            }
            
            // Let the time elapse
            if (startState.isTimedSimulation) {
                // Apply flow firing of continuous transitions
                if (elapsedTime > 0.0) {
                    for (Node node : gspn.nodes) {
                        if (node instanceof Transition) {
                            Transition trn = (Transition)node;
                            if (trn.isFiringFlow()) {
                                FirableWithBindings<Transition> enabledBind = startState.getEnabledFirableBindingsOf(trn);
                                if (enabledBind != null) {
                                    firing.enabledFlowTransitions.add(enabledBind);
                                    for (ColorVarsBinding b : enabledBind.bindings) {
                                        fireContinuousTransition(gspn, gspnContext, trn, b,
                                                                 startState, firing, elapsedTime);
                                    }
                                }
                            }
//                            if (trn.isFiringFlow() && startState.enabledTransitions.contains(trn)) {
//                                firing.enabledFlowTransitions.add(trn);
//                                fireContinuousTransition(gspn, gspnContext, trn, binding,
//                                                         startState, firing, elapsedTime);
//                            }
                        }
                    } 
                }
                
                // Advance the values of the DTA clock variables
                if (dta != null) {
                    for (Node node : dta.nodes) {
                        if (node instanceof ClockVar) {
                            ClockVar x = (ClockVar)node;
                            double val = startState.continuousVars.get(x);
                            firing.reachedState.continuousVars.put(x, val + elapsedTime);
                        }
                    }
                }
                
                // Advance the age of the general transitions
                for (Map.Entry<Tuple<Transition, ColorVarsBinding>, Double> e : startState.ageOfTransitions.entrySet()) {
                    if (e.getKey().x == firedNode && e.getKey().y.equals(binding))
                        continue; // fired transition loses its memory
                    firing.reachedState.ageOfTransitions.put(e.getKey(), e.getValue() + elapsedTime);
                }
            }

            if (firedNode instanceof Transition) {
                Transition firedTrn = (Transition)firedNode;
                
                if (startState.isTimedSimulation) {
                    assert (firedTrn.isTimed() && elapsedTime > 0.0) ||
                           (firedTrn.isImmediate() && elapsedTime == 0.0) ||
                           (firedTrn.isTimed() && elapsedTime == 0.0 && startState.timedEventsAreUrgent);
                }
                         
                // Apply instantaneous transition firing
                for (Node node : gspn.nodes) {
                    if (node instanceof Transition) {
                        Transition trn = (Transition)node;
                        if (trn.isFiringInstantaneous() && trn == firedTrn) {
                            fireInstantaneousTransition(gspn, gspnContext, trn, binding, startState, firing);
                        }
                    }
                }

                if (firing.reachedState.isTangible()) {
                    if (startState.currLoc == null && startState.isVanishing() && dta != null) {
                        // We have finished the initial vanishing. This is the first Tangible
                        // state in the path. Select the initial DTA location.
                        firing.reachedState.currLoc = initialLocationMatching(dta, firing.reachedState, dtaContext);
                        if (firing.reachedState.isErrorState())
                            return firing;
                    }
                }
            } // end of GSPN Transition firing
            else if (firedNode instanceof DtaEdge) {
                DtaEdge edge = (DtaEdge)firedNode;
                
                firing.reachedState.currLoc = (DtaLocation)edge.getHeadNode();
                firing.maxInputEdgeLength = firing.maxOutputEdgeLength = 5;
                if (startState.isTimedSimulation && edge.isReset()) {
                    // TODO: this is a very trivial implementation of the reset
                    for (Map.Entry<ClockVar, Double> e : firing.reachedState.continuousVars.entrySet())
                        if (e.getKey().getUniqueName().equals("x"))
                            e.setValue(0.0);
                }
            }
            else if (firedNode instanceof TimeElapse) {
                assert elapsedTime > 0.0;
                // Do nothing, just let the time elapse
            }
            else throw new IllegalStateException("Unknown event type.");
                                    
            firing.reachedState.markingText = firing.reachedState.getMarkingRepr(gspnContext);
            prepareEnabledSet(gspnContext, gspn, dtaContext, dta, firing.reachedState, firing);
        }
        catch (EvaluationException ee) {
            firing.tokensPassed.clear();
            firing.flowPassed.clear();
            firing.reachedState.setErrorState(ee.getMessage());
            ee.printStackTrace();
        }
        finally {
            gspn.compileParsedInfo(null);
            if (dta != null)
                dta.compileParsedInfo(null);
        }
        
        return firing;
    }
}
