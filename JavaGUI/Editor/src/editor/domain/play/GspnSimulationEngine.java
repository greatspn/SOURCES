///*
// * To change this license header, choose License Headers in Project Properties.
// * To change this template file, choose Tools | Templates
// * and open the template in the editor.
// */
//
//package editor.domain.play;
//
//import common.Tuple;
//import editor.domain.Edge;
//import editor.domain.Node;
//import editor.domain.elements.ClockVar;
//import editor.domain.elements.ColorClass;
//import editor.domain.elements.ColorVar;
//import editor.domain.elements.GspnEdge;
//import editor.domain.elements.GspnPage;
//import editor.domain.elements.Place;
//import editor.domain.elements.Transition;
//import editor.domain.grammar.ColorVarsBinding;
//import editor.domain.grammar.DomainElement;
//import editor.domain.grammar.EvaluationException;
//import editor.domain.grammar.ExpressionLanguage;
//import editor.domain.grammar.ParserContext;
//import editor.domain.grammar.TemplateBinding;
//import static editor.domain.play.GspnDtaPlayEngine.EPSILON;
//import static editor.domain.play.GspnDtaPlayEngine.EPSILON_VALUE;
//import static editor.domain.play.JointState.EMPTY_MAP;
//import editor.domain.values.EvaluatedFormula;
//import editor.domain.values.IntScalarValue;
//import editor.domain.values.MultiSet;
//import editor.domain.values.RealScalarValue;
//import java.util.HashMap;
//import java.util.HashSet;
//import java.util.Iterator;
//import java.util.Map;
//import java.util.Set;
//import java.util.TreeSet;
//
///**
// *
// * @author elvio
// */
//public class GspnSimulationEngine implements SimulationEngine {
//    
//    private final GspnPage theGspn;
//    private final TemplateBinding gspnBinding;
//    private ParserContext gspnContext;
//
//    public GspnSimulationEngine(GspnPage theGspn, TemplateBinding gspnBinding) {
//        this.theGspn = theGspn;
//        this.gspnBinding = gspnBinding;
//    }
//    
//    private void initilizeContext() {
//        assert gspnContext == null;
//        gspnContext = new ParserContext(theGspn);
//        gspnContext.templateVarsBinding = gspnBinding;
//        gspnContext.bindingContext = gspnContext; // GSPN -> GSPN
//        theGspn.compileParsedInfo(gspnContext);
//    }
//    
//    private void deinitializeContext() {
//        assert gspnContext != null;
//        theGspn.compileParsedInfo(null);
//        gspnContext = null;
//    }
//    
//    public SimulationState getInitialState(boolean isTimedSimulation) 
//    {
//        SimulationState initState = new SimulationState();
//        initState.stateNum = 0;
//        initState.isTimedSimulation = isTimedSimulation;
//        initState.time = 0.0;
//        initState.nextTimeBound = -1;
//        initState.eventsAreUrgent = false;
//        
//        try {
//            initilizeContext();
//            GspnMarking m0 = GspnMarking.initializeMarking(isTimedSimulation, theGspn, gspnContext);
//            initState.state = m0;
//            initState.markingText = initState.state.getStateRepr();
//            m0.prepareEnabledSet(gspnContext, theGspn, null);
//        }
//        catch (EvaluationException ee) {
//            initState.setErrorState(ee.getMessage());
//        }
//        finally {
//            deinitializeContext();
//        }
//        return initState;
//    }    
//    
//    public static abstract class ModelState {
//        
//        final private boolean isTimedSimulation;
//
//        public ModelState(boolean isTimed) {
//            this.isTimedSimulation = isTimed;
//        }
//        
//        public final boolean isTimed() { return isTimedSimulation; }
//        
//        public abstract String getStateRepr();
//        
//        public abstract boolean isTangible();
//        
//        // Error description or null
//        public abstract String getErrorDescr();
//        
//        // Disable all the enabled transitions.
//        public abstract void setDeadState();
//        
//        // Set a new time bound for this tangible state.
//        // Some transition may become disabled if their time-to-fire is
//        // higher than the time bound, and this method should disable them
//        public abstract void setTimeBound(ParserContext gspnContext, double time);
//    }
//    
//    public static class GspnMarking extends ModelState implements AbstractMarking {
//        // Place markings 
//        public Map<Place, EvaluatedFormula> marking = new HashMap<>();
//
//        // GSPN transitions that can fire
//        public Set<FirableWithBindings<Transition>> enabledTransitions 
//                = new HashSet<>();
//
//        // The flow of the continuous places
//        public Map<Place, EvaluatedFormula> flowFunction = new HashMap<>();
//
//        // The priority level of the enabled transitions (0 = timed)
//        public int priorityLevel;
//
//        // Age of the general transitions
//        public Map<Tuple<Transition, ColorVarsBinding>, Double> ageOfTransitions 
//                = new HashMap<>();
//        
//        // Fixed time step for those events that are triggered at specific time points,
//        // like boundary DTA edges (fire when x = value) or general transitions.
//        // Exponential transitions do not have a fixed time-to-fire.
////        public Map<Tuple<Firable, ColorVarsBinding>, Double> fixedTimeToFire 
////                = new HashMap<>();
//        
//        // This marking has a time bound, when some timed transition with finite support
//        // (like a determnistic) will surely fire, or when some transition changes its enabling
//        // due to a continuous transition flow. Variable @nextTimeBound is the
//        // smallest of the time bounds, or -1 if there is no bound.
//        public double timeBound = -1;
//        
//        // Timed events are urgent (i.e. behave like immediate events)
//        // This happens when general transitions with finite supports (like determinisitc) 
//        // have no time left before firing
//        public boolean timedEventsAreUrgent = false;
//
//        public static GspnMarking initializeMarking(boolean isTimed, GspnPage gspn, ParserContext gspnContext) {
//            GspnMarking m0 = new GspnMarking(isTimed);
//
//            for (Node n : gspn.nodes) {
//                if (n instanceof Place) {
//                    Place place = (Place)n;
//                    EvaluatedFormula ef = place.evaluateInitMarking(gspnContext);
//                    m0.marking.put(place, ef);
//                }
//            } 
//            return m0;
//        }
//        
//        private GspnMarking(boolean isTimed) {
//            super(isTimed);
//        }
//        
//        @Override
//        public boolean isTangible() { 
//            return priorityLevel == 0;    
//        }
//
//        @Override
//        public String getErrorDescr() {
//            return null;
//        }
//
//        @Override
//        public void setDeadState() {
//            enabledTransitions.clear();
//            timeBound = -1;
//            timedEventsAreUrgent = false;
//            priorityLevel = 0;
//        }
//
//        @Override
//        public double getValueOfClockVariable(ClockVar clockVar) {
//            throw new UnsupportedOperationException(); 
//        }
//        
//        @Override
//        public EvaluatedFormula getMarkingOfPlace(Place place) {
//            EvaluatedFormula m = marking.get(place);
//            if (m != null)
//                return m;
//            if (place.isInNeutralDomain())
//                return (place.isDiscrete() ? IntScalarValue.ZERO : RealScalarValue.ZERO);
//            else
//                return MultiSet.makeNew(place.isDiscrete() ? EvaluatedFormula.Type.INT 
//                                                           : EvaluatedFormula.Type.REAL,
//                                        place.getColorDomain(), EMPTY_MAP);
//        }
//        
//        @Override
//        public String getStateRepr() {
//            // Prepare the marking text representation
//            StringBuilder sb = new StringBuilder();
//            boolean emptyMarking = true;
//            int i = 0;
//            Iterator<Map.Entry<Place, EvaluatedFormula>> it = marking.entrySet().iterator();
//            while (it.hasNext()) {
//                Map.Entry<Place, EvaluatedFormula> tkn = it.next();
//                if (tkn.getValue().equalsZero())
//                    continue;
//                if (i++ > 0)
//                    sb.append(", ");
//                sb.append(tkn.getKey().getUniqueName());
//                sb.append("=");
//                sb.append(tkn.getValue().toStringFormat(ExpressionLanguage.PNPRO, "%.2f"));
//                emptyMarking = false;
//            }
//            if (emptyMarking)
//                sb.append("<<empty marking>>");
//
//            return sb.toString();
//        }
//        
//        public FirableWithBindings<Transition> getEnabledFirableBindingsOf(Transition trn) {
//            for (FirableWithBindings<Transition> fwb : enabledTransitions)
//                if (fwb.firable == trn)
//                    return fwb;
//            return null; // trn is not enabled
//        }
//        
//        // Suggest next time to fire
//        public double getSuggestNextTimeToFire(ParserContext gspnContext) {
//            assert isTangible() && isTimed();
//            double avgTimeNextTransition = 0.0;
//            for (FirableWithBindings<Transition> fwb : enabledTransitions) {
//                if (fwb.firable.isFiringInstantaneous()) {
//                    double delay = fwb.firable.evaluateDelay(gspnContext, this).getScalarReal();
//                    avgTimeNextTransition += fwb.firable.isExponential() ? 1 / delay : delay;
//                }
//            }
//            avgTimeNextTransition /= enabledTransitions.size();
//            if (Double.isInfinite(avgTimeNextTransition)
//                    || Double.isNaN(avgTimeNextTransition)
//                    || avgTimeNextTransition <= EPSILON_VALUE) {
//                avgTimeNextTransition = 1.0; // Default value
//            }
//            return avgTimeNextTransition;
//        }
//
//        @Override
//        public void setTimeBound(ParserContext gspnContext, double bound) {
//            assert isTimed();
//            if (bound == -1)
//                return; // No new time bound.
//            assert bound >= 0;
//            if (timeBound == -1 || timeBound > bound) {
//                // We have a new stricter bound
//                timeBound = bound;
//                
//                Iterator<FirableWithBindings<Transition>> fwbIt = enabledTransitions.iterator();
//                while (fwbIt.hasNext()) {
//                    FirableWithBindings<Transition> fwb = fwbIt.next();
////                    FirableWithBindings<Transition> urgentFirable = new FirableWithBindings<>(fwb.firable);
//                    if (fwb.firable.isGeneral()) {
//                        for (ColorVarsBinding binding : fwb.bindings) {
//                            Tuple<Transition, ColorVarsBinding> trnBind = new Tuple<>(fwb.firable, binding);
//                            assert ageOfTransitions.containsKey(trnBind);
//                            // Remove a deterministic/general timed transitions
//                            // if it is enabled but cannot fire before the new time bound.
//                            // In that case, the transition is removed, but its age memory is kept.
//                            double age = ageOfTransitions.get(trnBind);
//                            double delay = fwb.firable.evaluateDelay(gspnContext, this).getScalarReal();
//                            double timeToFire = delay - age;
//                            assert delay >= age;
//                            if (timeBound < timeToFire - EPSILON_VALUE) {
//                                // There is no time to fire! Remove this transition
//                                fwbIt.remove();
//                            }
//                        }
//                    }
//                }
//            }
//        }
//        
//        // Determine the list of color variable bindings that could enable a transition
//        private static Set<ColorVarsBinding> getBindingsOfTransition(GspnPage gspn, ParserContext gspnContext, 
//                                                                     Transition trn)
//        {
//            Set<ColorVar> trnColorVars = new HashSet<>();
//            if (trn.getColorVarsInUse() != null)
//                trnColorVars.addAll(trn.getColorVarsInUse());
//
//            for (Edge ee : gspn.edges) {
//                if (!(ee instanceof GspnEdge))
//                    continue;
//
//                GspnEdge e = (GspnEdge)ee;
//                if (e.getConnectedTransition() != trn)
//                    continue;
//
//                trnColorVars.addAll(e.getColorVarsInUse());
//            }
//
//            Set<ColorVarsBinding> outBindings = new HashSet<>();
//            if (trnColorVars.isEmpty()) {
//                outBindings.add(ColorVarsBinding.EMPTY_BINDING);
//                return outBindings;
//            }
//
//            // Generate all the possible color bindings of the involved color variables.
//            ColorVar[] vars = new ColorVar[trnColorVars.size()];
//            trnColorVars.toArray(vars);
//            int[] domainSizes = new int[vars.length];
//            ColorClass[] colorClasses = new ColorClass[vars.length];
//            MultiSet[] values = new MultiSet[vars.length];
//            for (int i=0; i<vars.length; i++) {
//                colorClasses[i] = vars[i].findColorClass(gspn); 
//                domainSizes[i] = colorClasses[i].numColors();
//            }
//
//            generateBindings(gspnContext, 0, vars, domainSizes, 
//                             values, colorClasses, outBindings);
//
//            return outBindings;
//        }
//
//        private static void generateBindings(ParserContext gspnContext, int level, ColorVar[] vars, int[] domainSizes, 
//                                              MultiSet[] values, ColorClass[] colorClasses,
//                                              Set<ColorVarsBinding> outBindings) 
//        {
//            if (level == vars.length) {
//                ColorVarsBinding cvb = new ColorVarsBinding();
//                for (int i=0; i<vars.length; i++)
//                    cvb.bind(vars[i], values[i]);
//                outBindings.add(cvb);
//            }
//            else {
//                Set<DomainElement> set = new TreeSet<>();
//                int[] color = new int[1];
//                for (int i=0; i<domainSizes[level]; i++) {
//                    color[0] = i;
//                    set.add(new DomainElement(colorClasses[level], color));
//                    values[level] = MultiSet.makeNew(colorClasses[level], set);
//
//                    generateBindings(gspnContext, level + 1, vars, domainSizes, 
//                                     values, colorClasses, outBindings);
//
//                    set.clear();
//                }
//            }
//        }
//
//        // determines if a given GSPN transition has concession in a marking
//        private static boolean hasTransitionConcession(GspnPage gspn, ParserContext gspnContext, 
//                                                       Transition trn, ColorVarsBinding binding, 
//                                                       AbstractMarking marking)
//        {
//            if (trn.hasGuard() && !trn.evaluateGuard(gspnContext, marking, binding).getScalarBoolean())
//                return false;
//
//            for (Edge ee : gspn.edges) {
//                if (ee instanceof GspnEdge) {
//                    GspnEdge e = (GspnEdge)ee;
//                    boolean enabled = true;
//
//                    switch (e.getEdgeKind()) {
//                        case INPUT: {
//                            Place p = (Place)e.getTailNode();
//                            Transition t = (Transition)e.getHeadNode();
//                            if (trn != t)
//                                break;
//
//                            EvaluatedFormula inMult = e.evaluateMultiplicity(gspnContext, marking, binding);
//                            if (p.isDiscrete()) {
//                                enabled = marking.getMarkingOfPlace(p).greaterEqual(inMult).getScalarBoolean();
//                            }
//                            else if (t.isFiringInstantaneous()) { // continuous place, instantaneous transition
//                                enabled = marking.getMarkingOfPlace(p).greaterEqual(inMult).getScalarBoolean();
//                            }
//                            else { // continuous place, flow transition
//                                enabled = marking.getMarkingOfPlace(p).greater(EPSILON).getScalarBoolean();
//                            }
//                            break;
//                        }
//                        case OUTPUT: {
//                            break;
//                        }
//                        case INHIBITOR: {
//                            Place p = (Place)e.getTailNode();
//                            Transition t = (Transition)e.getHeadNode();
//                            if (trn != t)
//                                break;
//                            EvaluatedFormula outMult = e.evaluateMultiplicity(gspnContext, marking, binding);
//                            if (p.isDiscrete()) {
//                                enabled = marking.getMarkingOfPlace(p).less_ZeroAsInf(outMult).getScalarBoolean();
//                            }
//                            else if (t.isFiringInstantaneous()) { // continuous place, instantaneous transition
//                                enabled = marking.getMarkingOfPlace(p).less_ZeroAsInf(outMult).getScalarBoolean();
//                            }
//                            else { // continuous place, flow transition
//                                enabled = marking.getMarkingOfPlace(p).less_ZeroAsInf(outMult.subtract(EPSILON)).getScalarBoolean();
//                            }
//                            break;
//                        }
//                    }
//
//                    if (!enabled) 
//                        return false;
//                }
//            }
//            return true;
//        }
//
//
//        // Return the overall flow of a continuous place moved by enabled flow transitions.
//        private static EvaluatedFormula flowOfPlace(GspnPage gspn, ParserContext gspnContext, 
//                                                    Place plc, GspnMarking state) {
//            assert plc.isContinuous() && state.isTangible() && state.isTimed();
//            EvaluatedFormula flow = EvaluatedFormula.makeZero(EvaluatedFormula.Type.REAL, plc.getColorDomain());
//
//            // Determine the flow of the continuous places from the enabled timed transitions
//            for (Edge ee : gspn.edges) {
//                if (ee instanceof GspnEdge) {
//                    GspnEdge e = (GspnEdge)ee;
//                    if (e.isDiscrete() || e.isFiringInstantaneous())
//                        continue;
//                    switch (e.getEdgeKind()) {
//                        case INPUT: {
//                            Place p = (Place)e.getTailNode();
//                            Transition t = (Transition)e.getHeadNode();
//                            if (p != plc)
//                                break;
//
//                            FirableWithBindings<Transition> enabledBind = state.getEnabledFirableBindingsOf(t);
//                            if (enabledBind != null) {
//                                for (ColorVarsBinding binding : enabledBind.bindings) {
//                                    // Remove the input flow from the place
//                                    EvaluatedFormula inMult = e.evaluateMultiplicity(gspnContext, state, binding);
//                                    flow = flow.subtract(inMult);
//                                }
//                            }
//    //                        EvaluatedFormula inMult = e.evaluateMultiplicity(gspnContext, state);
//    //                        if (state.enabledTransitions.contains(t))
//    //                            flow -= inMult.getReal();
//                            break;
//                        }
//                        case OUTPUT: {
//                            Place p = (Place)e.getHeadNode();
//                            Transition t = (Transition)e.getTailNode();
//                            if (p != plc)
//                                break;
//
//                            FirableWithBindings<Transition> enabledBind = state.getEnabledFirableBindingsOf(t);
//                            if (enabledBind != null) {
//                                for (ColorVarsBinding binding : enabledBind.bindings) {
//                                    // Add the output flow to the place
//                                    EvaluatedFormula outMult = e.evaluateMultiplicity(gspnContext, state, binding);
//                                    flow = flow.add(outMult);
//                                }
//                            }
//    //                        // Add the output flow to the place
//    //                        EvaluatedFormula outMult = e.evaluateMultiplicity(gspnContext, state);
//    //                        if (state.enabledTransitions.contains(t)) 
//    //                            flow += outMult.getReal();
//                            break;
//                        }
//                        case INHIBITOR:
//                            break;
//                    }
//                }
//            }
//            System.out.println("Place "+plc.getUniqueName()+" has flow "+flow);
//            return flow;
//        }
//
//
//        // Returns the time bound when this transition will become 
//        // enabled/disabled/inhibited due to the continuous flow
//        private static double timeBoundOfTransition(GspnPage gspn, ParserContext gspnContext, 
//                                                    Transition trn, boolean isEnabled,
//                                                    ColorVarsBinding binding, GspnMarking state) 
//        {
//            assert state.isTangible() && state.isTimed();
//    //        boolean isEnabled = state.enabledTransitions.contains(trn);
//            double upperBound = -1;
//            for (Edge ee : gspn.edges) {
//                if (ee instanceof GspnEdge) {
//                    GspnEdge e = (GspnEdge) ee;
//    //                double newUpperBound = -1;
//                    EvaluatedFormula allBounds = null;
//                    if (/*e.isFiringFlow() && */e.isContinuous()) {
//                        switch (e.getEdgeKind()) {
//                            case INPUT: {
//                                Place p = (Place) e.getTailNode();
//                                Transition t = (Transition)e.getHeadNode();
//                                if (t != trn)
//                                    break;
//    //                            EvaluatedFormula mult = e.evaluateMultiplicity(gspnContext, state);
//                                EvaluatedFormula placeFlow = state.flowFunction.get(p);
//
//                                if (e.isFiringFlow()) {
//                                    if (isEnabled) { // Place p is being drained, test when it goes to 0
//                                        allBounds = state.getMarkingOfPlace(p).safeDivide(placeFlow.opposite());
//                                    }
//    //                                if (placeFlow < 0.0 && isEnabled) { // Place p is being drained, test when it goes to 0
//    //                                    newUpperBound = state.getMarkingOfPlace(p).getReal() / -placeFlow;
//    //                                    assert newUpperBound > 0.0;
//    //                                }
//                                }
//                                else { // instantaneous transition
//                                    if (isEnabled) {
//                                        // Test when trn will become disabled
//                                        EvaluatedFormula inMult = e.evaluateMultiplicity(gspnContext, state, binding);
//                                        EvaluatedFormula placeMark = state.getMarkingOfPlace(p);
//    //                                    assert !placeMark.lessEqual(inMult).getScalarBoolean();
//                                        allBounds = placeMark.subtract(inMult).safeDivide(placeFlow.opposite());
//                                    }
//                                    else {
//                                        // Transition could become enabled
//                                        EvaluatedFormula inMult = e.evaluateMultiplicity(gspnContext, state, binding);
//                                        EvaluatedFormula placeMark = state.getMarkingOfPlace(p);
//    //                                    assert !placeMark.greaterEqual(inMult).getScalarBoolean();
//                                        allBounds = placeMark.subtract(inMult).safeDivide(placeFlow);
//                                    }
//
//    //                                if (placeFlow < 0.0 && isEnabled) { 
//    //                                    // Test when trn will become disabled
//    //                                    EvaluatedFormula inMult = e.evaluateMultiplicity(gspnContext, state, binding);
//    //                                    EvaluatedFormula placeMark = state.getMarkingOfPlace(p);
//    //                                    assert !state.getMarkingOfPlace(p).lessEqual(inMult).getScalarBoolean();
//    //                                    newUpperBound = (state.getMarkingOfPlace(p).getReal() - inMult.getReal()) / -placeFlow;
//    //                                    assert newUpperBound > 0.0;
//    //                                }
//    //                                else if (placeFlow > 0.0 && !isEnabled) {
//    //                                    // Transition could become enabled
//    //                                    EvaluatedFormula inMult = e.evaluateMultiplicity(gspnContext, state, binding);
//    //                                    assert state.getMarkingOfPlace(p).getReal() < inMult.getReal();
//    //                                    newUpperBound = (inMult.getReal() - state.getMarkingOfPlace(p).getReal()) / placeFlow;
//    //                                    assert newUpperBound > 0.0;
//    //                                }
//                                }
//                                break;
//                            }
//
//                            case OUTPUT: 
//                                break;
//
//                            case INHIBITOR: {
//                                Place p = (Place) e.getTailNode();
//                                Transition t = (Transition)e.getHeadNode();
//                                if (t != trn)
//                                    break;
//                                EvaluatedFormula mult = e.evaluateMultiplicity(gspnContext, state, binding);
//                                EvaluatedFormula placeFlow = state.flowFunction.get(p);
//                                EvaluatedFormula placeMark = state.getMarkingOfPlace(p);
//    //                            EvaluatedFormula inhibLevel = mult.getReal();
//
//                                if (placeMark.less(mult.add(EPSILON)).getScalarBoolean() && isEnabled) {
//                                    // Transition will be inhibited
//                                    allBounds = mult.subtract(placeMark).safeDivide(placeFlow);
//                                }
//                                else if (placeMark.greaterEqual(mult.add(EPSILON)).getScalarBoolean() && !isEnabled) {
//                                    // Transition could become enabled
//                                    allBounds = placeMark.subtract(mult).safeDivide(placeFlow.opposite());
//                                }
//    //                            
//    //                            if (placeMark < inhibLevel + EPSILON_VALUE && placeFlow > 0.0 && isEnabled) {
//    //                                // Transition will be inhibited
//    //                                newUpperBound = (inhibLevel - placeMark) / placeFlow;
//    //                                assert newUpperBound > 0.0;
//    //                            } 
//    //                            else if (placeMark >= inhibLevel - EPSILON_VALUE && placeFlow < 0.0 && !isEnabled) {
//    //                                // Transition could become enabled
//    //                                newUpperBound = (placeMark - inhibLevel) / -placeFlow;
//    //                                assert newUpperBound > 0.0;
//    //                            }
//
//                                break;
//                            }
//                        }
//                    }
//                    if (allBounds != null) {
//    //                    System.out.println("allBounds = "+allBounds);
//                        double newUpperBound = -1;
//                        if (allBounds.isScalar()) {
//                            newUpperBound = allBounds.getScalarRealOrIntAsReal();
//                        }
//                        else {
//                            allBounds.multisetFilterNegatives();
//                            if (!((MultiSet)allBounds).isEmpty()) {
//                                newUpperBound = allBounds.multisetMin().getScalarReal();
//                            }
//                        }
//                        if (newUpperBound > 0) {
//                            if (upperBound == -1 || newUpperBound < upperBound)
//                                upperBound = newUpperBound;
//                        }
//                    }
//                }
//            }
//            // Consider the bound of general (deterministic) transitions
//            if (trn.isGeneral() && isEnabled) {
//                double age = 0.0, delay;
//                Tuple<Transition, ColorVarsBinding> trnBind = new Tuple<>(trn, binding);
//                if (state.ageOfTransitions.containsKey(trnBind))
//                    age = state.ageOfTransitions.get(trnBind);
//                delay = trn.evaluateDelay(gspnContext, state).getScalarReal();
//                double fireBound = (delay - age);
//                assert fireBound >= 0.0;
//                if (upperBound == -1 || fireBound < upperBound)
//                    upperBound = fireBound;
//            }
//
//            System.out.println("Bound of "+trn.getUniqueName()+binding+"  is "+upperBound);
//            return upperBound;
//        }
//
//        public void prepareEnabledSet(ParserContext gspnContext, GspnPage gspn, JointFiring firing) 
//        {
//            enabledTransitions.clear();
//            priorityLevel = 0;
//
//            // Determine the enabled GSPN transitions
//            for (Node node : gspn.nodes) {
//                if (node instanceof Transition) {
//                    Transition trn = (Transition) node;
//                    // Generate all the possible bindings that could enable this transition
//                    Set<ColorVarsBinding> allBindings = getBindingsOfTransition(gspn, gspnContext, trn);
//                    FirableWithBindings<Transition> firable = new FirableWithBindings<>(trn);
//                    for (ColorVarsBinding binding : allBindings) {
//                        if (hasTransitionConcession(gspn, gspnContext, trn, binding, this)) {
//                            // Transition has concession in marking. Determine if it is enabled
//                            int trnPrio;
//                            if (trn.isImmediate()) {
//                                trnPrio = trn.evaluatePriority(gspnContext, this).getScalarInt();
//                            } else {
//                                trnPrio = 0;
//                            }
//
//                            if (trnPrio >= priorityLevel) {
////                            state.enabledTransitions.add(trn);
//                                firable.bindings.add(binding);
//                                if (trnPrio > priorityLevel) {
//                                    // Update the priority level
//                                    priorityLevel = trnPrio;
//                                    // Remove transition that have less priority than trn
//                                    Iterator<FirableWithBindings<Transition>> trnIt = enabledTransitions.iterator();
//                                    while (trnIt.hasNext()) {
//                                        FirableWithBindings<Transition> fwb = trnIt.next();
//                                        if (fwb.firable.isTimed() 
//                                            || priorityLevel > fwb.firable.evaluatePriority(gspnContext, this).getScalarInt()) {
//                                            trnIt.remove();
//                                        }
//                                    }
//                                }
//                            }
//                        }
//                    }
//                    if (!firable.bindings.isEmpty()) {
//                        enabledTransitions.add(firable);
//                    }
//                }
//            }
//
////            if (dta != null && firing != null && state.isTangible() && firing.firedNode instanceof Transition) {
////                // An Inner edge must fire.
////                state.enabledTransitions.clear();
////                state.isDtaInnerMove = true;
////                for (Edge e : dta.edges) {
////                    if (e instanceof DtaEdge) {
////                        DtaEdge edge = (DtaEdge) e;
////                        if (edge.isInner()
////                                && edge.getTailNode() == state.currLoc
////                                && edge.evaluateActionSetExpr(dtaContext, state, (Transition) firing.firedNode,
////                                        firing.colorVarsBinding).getScalarBoolean()
////                                && ((DtaLocation) edge.getHeadNode()).evaluateStatePropositionExpr(dtaContext, firing.reachedState).getScalarBoolean()) {
////                            state.enabledDtaEdges.add(edge);
//////                        if (innerEdge != null) {
//////                            firing.reachedState.setErrorState("Inner edge choice is not deterministic.");
//////                            state.currLoc = JointState.BOTTOM_LOCATION;
//////                            return;
//////                        }
//////                        innerEdge = edge;
////                        }
////                    }
////                }
////                if (state.enabledDtaEdges.isEmpty()) {
////                    state.currLoc = JointState.BOTTOM_LOCATION;
////                }
//////            else
//////                state.enabledInnerEdges.add(innerEdge);
////                return;
////            }
//
//            // Prepare the flow function for the continuous places 
//            // from the enabled continuous transitions
//            for (Node node : gspn.nodes) {
//                if (node instanceof Place && ((Place) node).isContinuous()) {
//                    Place plc = (Place) node;
//                    if (isTangible() && isTimed()) {
//                        EvaluatedFormula flow = flowOfPlace(gspn, gspnContext, plc, this);
//                        flowFunction.put(plc, flow);
//                    } else {
//                        flowFunction.put(plc, EvaluatedFormula.makeZero(EvaluatedFormula.Type.REAL,
//                                plc.getColorDomain()));
//                    }
//                }
//            }
//
//            double nextTimeBound = -1;
//            if (isTangible() && isTimed()) {
//                // Determine the time bound when the continuous flow will change
//                // the enabling of some transition.
//                for (Node node : gspn.nodes) {
//                    if (node instanceof Transition) {
//                        Transition trn = (Transition) node;
//                        Set<ColorVarsBinding> allBindings = getBindingsOfTransition(gspn, gspnContext, trn);
//                        FirableWithBindings<Transition> enabledBindings = getEnabledFirableBindingsOf(trn);
//                        for (ColorVarsBinding binding : allBindings) {
//                            boolean isEnabled = (enabledBindings != null
//                                    && enabledBindings.bindings.contains(binding));
//                            double bound = timeBoundOfTransition(gspn, gspnContext, trn,
//                                                                 isEnabled, binding, this);
//                            if (bound != -1) {
//                                if (nextTimeBound == -1 || nextTimeBound > bound) {
//                                    nextTimeBound = bound;
//                                }
//                            }
//                        }
//                    }
//                }
//
////                // Find the time bound of the DTA Inner edges.
////                if (dta != null) {
////                    for (Edge e : dta.edges) {
////                        if (e instanceof DtaEdge) {
////                            DtaEdge edge = (DtaEdge) e;
////                            if (edge.isBoundary()) {
////                                double bound = edge.getBoundOf(dtaContext, state);
////                                if (bound != -1) {
////                                    if (state.nextTimeBound == -1 || state.nextTimeBound > bound) {
////                                        state.nextTimeBound = bound;
////                                    }
////                                }
////                            }
////                        }
////                    }
////                }
//            }
//
////            // Add DTA boundary edges
////            if (dta != null) {
////                for (Edge e : dta.edges) {
////                    if (e instanceof DtaEdge) {
////                        DtaEdge edge = (DtaEdge) e;
////                        AbstractMarking evalState = firing == null ? this : firing.reachedState;
////                        if (edge.isBoundary()
////                                && edge.getTailNode() == state.currLoc
////                                && ((DtaLocation) edge.getHeadNode()).evaluateStatePropositionExpr(dtaContext, evalState).getScalarBoolean()) {
////                      // For untimed simulation, always add the boundary edges.
////                            // For timed simulation, the edge must also be enabled before
////                            // the upperBound.
////                            double bound = -1;
////                            if (state.isTimedSimulation) {
////                                bound = edge.getBoundOf(dtaContext, state);
////                                if (bound != -1 && bound > state.nextTimeBound) {
////                                    continue;
////                                }
////                                if (bound != -1 && bound <= 0 + EPSILON_VALUE) {
////                                    // Boundary edge is urgent!
////                                    state.enabledTransitions.clear();
////                                    state.priorityLevel = Integer.MAX_VALUE;
////                                    state.boundaryDtaEdgesAreUrgent = true;
////                                    assert !state.isTangible();
////                                }
////                            }
////
////                            state.enabledDtaEdges.add(edge);
////                            if (bound != -1) {
////                                state.fixedTimeToFire.put(new Tuple<Firable, ColorVarsBinding>(edge, null), bound);
////                            }
////                        }
////                    }
////                }
////            }
//
//            System.out.println("GSPN time bound is " + nextTimeBound);
//
//            // Manage general transitions
//            if (isTimed() && isTangible()) {
//                // Clear the memory of the disabled general transitions
//                Iterator<Map.Entry<Tuple<Transition, ColorVarsBinding>, Double>> ageIt;
//                ageIt = ageOfTransitions.entrySet().iterator();
//                while (ageIt.hasNext()) {
//                    Map.Entry<Tuple<Transition, ColorVarsBinding>, Double> e = ageIt.next();
//                    FirableWithBindings<Transition> enabledBindings = getEnabledFirableBindingsOf(e.getKey().x);
//                    if (!enabledBindings.bindings.contains(e.getKey().y)) {
//                        ageIt.remove();
//                    }
//                }
//
//                // Set of urgent general transitions, that will fire immediately
//                Set<FirableWithBindings<Transition>> urgentTransitions = new HashSet<>();
//
//                // Add the memory of the newly enabled general transitions
//                Iterator<FirableWithBindings<Transition>> fwbIt = enabledTransitions.iterator();
//                while (fwbIt.hasNext()) {
//                    FirableWithBindings<Transition> fwb = fwbIt.next();
//                    FirableWithBindings<Transition> urgentFirable = new FirableWithBindings<>(fwb.firable);
//                    if (fwb.firable.isGeneral()) {
//                        for (ColorVarsBinding binding : fwb.bindings) {
//                            Tuple<Transition, ColorVarsBinding> trnBind = new Tuple<>(fwb.firable, binding);
//                            if (!ageOfTransitions.containsKey(trnBind)) {
//                                // t is newly enabled
//                                System.out.println("Transition " + fwb.firable.getUniqueName() + binding + " is newly enabled.");
//                                ageOfTransitions.put(trnBind, 0.0);
//                            }
//                            // Now remove those deterministic/general timed transitions
//                            // that are enabled but cannot fire before the time bound.
//                            // In that case, the transition is removed, but its age is kept.
//                            double age = ageOfTransitions.get(trnBind);
//                            double delay = fwb.firable.evaluateDelay(gspnContext, this).getScalarReal();
//                            double timeToFire = delay - age;
//                            assert delay >= age;
//                            if (nextTimeBound < timeToFire - EPSILON_VALUE) {
//                                // There is no time to fire! Remove this transition
//                                fwbIt.remove();
//                            } else { // May fire
//                                if (timeToFire < EPSILON_VALUE) {
//                                    // Treat this general transition like an immediate transition
//                                    urgentFirable.bindings.add(binding);
////                                urgentTransitions.add(trnBind);
//                                }
//                                else {
//                                    if (nextTimeBound == -1 || nextTimeBound > timeToFire)
//                                        nextTimeBound = timeToFire;
//                                }
////                                fixedTimeToFire.put(new Tuple<Firable, ColorVarsBinding>(fwb.firable, binding), timeToFire);
//                            }
//                        }
//                        if (!urgentFirable.bindings.isEmpty()) {
//                            urgentTransitions.add(urgentFirable);
//                        }
//                    }
//                }
//                for (Map.Entry<Tuple<Transition, ColorVarsBinding>, Double> e : ageOfTransitions.entrySet()) {
//                    System.out.println("Age of " + e.getKey().x.getUniqueName() + e.getKey().y + " is " + e.getValue());
//                }
//
//                // Do we have some urgent transitions?
//                if (!urgentTransitions.isEmpty()) {
//                    enabledTransitions = urgentTransitions;
//                    priorityLevel = 1;
//                    timedEventsAreUrgent = true;
//                }
//            }
//            
//            // Remove transitions that cannot fire before the time bound of this state.
//            if (isTimed()) {
//                if (!isTangible())
//                    timeBound = 0.0;
//                else
//                    setTimeBound(gspnContext, nextTimeBound);
//            }
//
////            // Finally, setup the suggested timing of the next event
////            if (isTimedSimulation && isTangible()) {
////                state.avgTimeNextTransition = 0.0;
////                for (FirableWithBindings<Transition> fwb : state.enabledTransitions) {
////                    if (fwb.firable.isFiringInstantaneous()) {
////                        double delay = fwb.firable.evaluateDelay(gspnContext, state).getScalarReal();
////                        state.avgTimeNextTransition += fwb.firable.isExponential() ? 1 / delay : delay;
////                    }
////                }
////                state.avgTimeNextTransition /= state.enabledTransitions.size();
////                if (Double.isInfinite(state.avgTimeNextTransition)
////                        || Double.isNaN(state.avgTimeNextTransition)
////                        || state.avgTimeNextTransition <= EPSILON_VALUE) {
////                    state.avgTimeNextTransition = 1.0; // Default value
////                }
////            }
//        }
//    }
//    
//    
//}
