///*
// * To change this license header, choose License Headers in Project Properties.
// * To change this template file, choose Tools | Templates
// * and open the template in the editor.
// */
//
//package editor.domain.play;
//
//import editor.domain.Edge;
//import editor.domain.Node;
//import editor.domain.elements.ClockVar;
//import editor.domain.elements.DtaEdge;
//import editor.domain.elements.DtaLocation;
//import editor.domain.elements.DtaPage;
//import editor.domain.elements.GspnPage;
//import editor.domain.elements.Place;
//import editor.domain.elements.Transition;
//import editor.domain.grammar.ColorVarsBinding;
//import editor.domain.grammar.EvaluationException;
//import editor.domain.grammar.ParserContext;
//import editor.domain.grammar.TemplateBinding;
//import static editor.domain.play.GspnDtaPlayEngine.EPSILON_VALUE;
//import static editor.domain.play.JointState.BOTTOM_LOCATION;
//import editor.domain.values.EvaluatedFormula;
//import java.util.HashMap;
//import java.util.HashSet;
//import java.util.Iterator;
//import java.util.Locale;
//import java.util.Map;
//import java.util.Set;
//
///**
// *
// * @author elvio
// */
//public class SynchProductSimulationEngine implements SimulationEngine {
//    
//    private final GspnPage theGspn;
//    private final TemplateBinding gspnBinding;
//    private ParserContext gspnContext;
//    
//    private final DtaPage theDta;
//    private final TemplateBinding dtaBinding;
//    private ParserContext dtaContext;
//
//    public SynchProductSimulationEngine(GspnPage theGspn, TemplateBinding gspnBinding, 
//                                        DtaPage theDta, TemplateBinding dtaBinding) 
//    {
//        this.theGspn = theGspn;
//        this.gspnBinding = gspnBinding;
//        this.theDta = theDta;
//        this.dtaBinding = dtaBinding;
//    }
//    
//    private void initilizeContext() {
//        assert gspnContext == null && dtaContext == null;
//        gspnContext = new ParserContext(theGspn);
//        dtaContext = new ParserContext(theDta);
//        gspnContext.templateVarsBinding = gspnBinding;
//        dtaContext.templateVarsBinding = dtaBinding;
//        dtaContext.bindingContext = gspnContext;  // DTA -> GSPN
//        gspnContext.bindingContext = gspnContext; // GSPN -> GSPN
//        theGspn.compileParsedInfo(gspnContext);
//        theDta.compileParsedInfo(gspnContext);
//    }
//    
//    private void deinitializeContext() {
//        assert gspnContext != null && dtaContext != null;
//        theGspn.compileParsedInfo(null);
//        theDta.compileParsedInfo(null);
//        gspnContext = null;
//        dtaContext = null;
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
//            
//            SynchProductState slc0 = SynchProductState.initializeSynchProductState
//                    (theGspn, gspnContext, theDta, dtaContext, isTimedSimulation);
//            initState.state = slc0;
//            
//            initState.markingText = initState.state.getStateRepr();
//            slc0.prepareEnabledSet(gspnContext, theGspn, dtaContext, theDta, null);
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
//    
//    public static class SynchProductState extends GspnSimulationEngine.ModelState implements AbstractMarking {
//
//        private SynchProductState(boolean isTimed) {
//            super(isTimed);
//        }
//        
//        // GSPN marking
//        public GspnSimulationEngine.GspnMarking marking;
//        
//        // DTA location
//        public DtaLocation currLoc;
//
//        // DTA Inner/Boundary edges that can be triggered
//        public Set<FirableWithBindings<DtaEdge>> enabledDtaEdges = new HashSet<>();
//
//        // Is this a state where a DTA Inner edge must be triggered
//        public boolean isDtaInnerMove = false;
//
//        // The value of the continous variables (clocks)
//        public Map<ClockVar, Double> continuousVars = new HashMap<>();
//        
//        // Failed synchronization flag.
//        // Synchronization may fail when the DTA is not deterministic
//        public boolean dtaIsNotDet = false;
//        public boolean initialLocIsNotDet = false;
//        
//        // Some boundary edges are enabled (and therefore they are urgent,
//        // by definition of boundary edges).
//        public boolean boundaryDtaEdgesAreUrgent = false;
//    
//        public static SynchProductState initializeSynchProductState(GspnPage gspn, ParserContext gspnContext,
//                                                                     DtaPage dta, ParserContext dtaContext,
//                                                                     boolean isTimedSimulation) 
//        {
//            SynchProductState slc0 = new SynchProductState(isTimedSimulation);
//            slc0.marking = GspnSimulationEngine.GspnMarking.initializeMarking(isTimedSimulation, gspn, gspnContext);
//
//            // Initialize clock variables to zero.
//            if (isTimedSimulation) {
//                for (Node node : dta.nodes) {
//                    if (node instanceof ClockVar) {
//                        ClockVar x = (ClockVar)node;
//                        slc0.continuousVars.put(x, 0.0);
//                    }
//                }
//            }
//            if (slc0.marking.isTangible()) {
//                // Find the initial location of the DTA that matches the initial marking of the GSPN
//                slc0.matchInitialLocation(dta, dtaContext);
//            }
//            return slc0;
//        }
//        
//        public void matchInitialLocation(DtaPage dta, ParserContext dtaContext) {
//            assert currLoc == null;
//            for (Node n : dta.nodes) {
//                if (n instanceof DtaLocation) {
//                    DtaLocation loc = (DtaLocation)n;
//                    if (loc.isInitial() && loc.evaluateStatePropositionExpr(dtaContext, marking).getScalarBoolean()) {
//                        if (currLoc != null) {
//                            initialLocIsNotDet = true;
//                            currLoc = JointState.BOTTOM_LOCATION;
//                            return;
//                        }
//                        currLoc = loc;
//                    }
//                }
//            }
//            if (currLoc == null)
//                currLoc = JointState.BOTTOM_LOCATION;
//        }
//
//        
//        @Override
//        public boolean isTangible() { 
//            return marking.isTangible() && !isDtaMove();    
//        }
//        
//        public boolean isDtaMove() {
//            return isDtaInnerMove;
//        }
//        
//        public boolean isTop() {
//            return (currLoc != null) && (currLoc.isFinalAccepting());
//        }
//
//        public boolean isBot() {
//            if (currLoc == BOTTOM_LOCATION)
//                return true;
//            return (currLoc != null) && (currLoc.isFinalRejecting());
//        }
//
//        @Override
//        public String getErrorDescr() {
//            if (dtaIsNotDet)
//                return "DTA is not deterministic in location "+currLoc.getUniqueName();
//            if (initialLocIsNotDet)
//                return "Initial DTA location is not unique";
//            return null; // No error
//        }
//
//        @Override
//        public void setDeadState() {
//            marking.setDeadState();
//            currLoc = null;
//            enabledDtaEdges.clear();
//        }
//        
//        @Override
//        public String getStateRepr() {
//            StringBuilder sb = new StringBuilder();
//            sb.append("[");
//            sb.append(currLoc != null ? currLoc.getUniqueName() : "-");
//            for (Map.Entry<ClockVar, Double> e : continuousVars.entrySet())
//                sb.append(", ").append(e.getKey().getUniqueName())
//                  .append("=").append(String.format(Locale.US, "%.2f", e.getValue()));
//            sb.append("] ");
//            return sb.append(marking.getStateRepr()).toString();
//        }
//
//        @Override
//        public EvaluatedFormula getMarkingOfPlace(Place place) {
//            return marking.getMarkingOfPlace(place);
//        }
//
//        @Override
//        public double getValueOfClockVariable(ClockVar clockVar) {
//            return continuousVars.get(clockVar);
//        }
//
//        @Override
//        public void setTimeBound(ParserContext gspnContext, double bound) {
//            assert isTimed();
//            if (bound == -1)
//                return; // No new time bound.
//            if (marking.timeBound == -1 || marking.timeBound > bound) {
//                // We have a new stricter bound
//                marking.setTimeBound(gspnContext, bound);
//                
//                Iterator<FirableWithBindings<DtaEdge>> edgeIt = enabledDtaEdges.iterator();
//                while (edgeIt.hasNext()) {
//                    FirableWithBindings<DtaEdge> e = edgeIt.next();
//                    assert e.firable.isBoundary();
//                    assert e.bindings.size() <= 1; // This code does not enumerate color bindings!
//                    double timeToFire = e.firable.getBoundOf(dtaContext, this);
//                    if (bound < timeToFire - EPSILON_VALUE) {
//                        // This boundary edge cannot fire before the time bound!
//                        edgeIt.remove();
//                    }
//                }
//            }
//        }
//        
//        private void prepareEnabledSet(ParserContext gspnContext, GspnPage gspn,
//                                       ParserContext dtaContext, DtaPage dta, 
//                                       JointFiring firing) 
//        {
//            enabledDtaEdges.clear();
//
//            if (isTop() || isBot()) {
//                return;
//            }
//            marking.prepareEnabledSet(gspnContext, gspn, firing);
//            
//            // Is this a corresponding Innere edge move?
//            if (firing != null && isTangible() && firing.firedNode instanceof Transition) {
//                // An Inner edge must fire.
//                marking.setDeadState();
//                isDtaInnerMove = true;
//                for (Edge e : dta.edges) {
//                    if (e instanceof DtaEdge) {
//                        DtaEdge edge = (DtaEdge) e;
//                        if (edge.isInner()
//                                && edge.getTailNode() == currLoc
//                                && edge.evaluateActionSetExpr(dtaContext, marking, (Transition) firing.firedNode,
//                                        firing.colorVarsBinding).getScalarBoolean()
//                                && ((DtaLocation) edge.getHeadNode()).evaluateStatePropositionExpr(dtaContext, firing.reachedState).getScalarBoolean()) 
//                        {
//                            FirableWithBindings<DtaEdge> fwb = new FirableWithBindings<>(edge);
//                            fwb.bindings.add(ColorVarsBinding.EMPTY_BINDING);
//                            enabledDtaEdges.add(fwb);
////                        if (innerEdge != null) {
////                            firing.reachedState.setErrorState("Inner edge choice is not deterministic.");
////                            state.currLoc = JointState.BOTTOM_LOCATION;
////                            return;
////                        }
////                        innerEdge = edge;
//                        }
//                    }
//                }
//                System.out.println("There are "+enabledDtaEdges.size()+" Inner DTA edges enabled.");
//                if (enabledDtaEdges.isEmpty()) {
//                    currLoc = JointState.BOTTOM_LOCATION;
//                }
////            else
////                state.enabledInnerEdges.add(innerEdge);
//                return;
//            }
//            
//            // Find the time bound of the DTA Boundary edges.
//            double nextTimeBound = marking.timeBound;
//            if (marking.isTangible() && isTimed()) {
//                for (Edge e : dta.edges) {
//                    if (e instanceof DtaEdge) {
//                        DtaEdge edge = (DtaEdge) e;
//                        if (edge.isBoundary()) {
//                            double bound = edge.getBoundOf(dtaContext, this);
//                            if (bound != -1) {
//                                if (nextTimeBound == -1 || nextTimeBound > bound) {
//                                    nextTimeBound = bound;
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//            
//            // Add DTA boundary edges
//            for (Edge e : dta.edges) {
//                if (e instanceof DtaEdge) {
//                    DtaEdge edge = (DtaEdge) e;
//                    AbstractMarking evalState = firing == null ? this : firing.reachedState;
//                    if (edge.isBoundary()
//                            && edge.getTailNode() == currLoc
//                            && ((DtaLocation) edge.getHeadNode()).evaluateStatePropositionExpr(dtaContext, evalState).getScalarBoolean()) {
//                        // For untimed simulation, always add the boundary edges.
//                        // For timed simulation, the edge must also be enabled before
//                        // the time bound.
//                        double bound = -1;
//                        if (isTimed()) {
//                            bound = edge.getBoundOf(dtaContext, this);
//                            if (bound != -1 && bound > nextTimeBound) {
//                                continue;
//                            }
//                            if (bound != -1 && bound <= 0 + GspnDtaPlayEngine.EPSILON_VALUE) {
//                                // Boundary edge is urgent!
//                                marking.setDeadState();
////                                state.priorityLevel = Integer.MAX_VALUE;
//                                boundaryDtaEdgesAreUrgent = true;
//                                assert !isTangible();
//                            }
//                        }
//
//                        FirableWithBindings<DtaEdge> fwb = new FirableWithBindings<>(edge);
//                        fwb.bindings.add(ColorVarsBinding.EMPTY_BINDING);
//                        enabledDtaEdges.add(fwb);
////                        if (bound != -1) {
////                            state.fixedTimeToFire.put(new Tuple<Firable, ColorVarsBinding>(edge, null), bound);
////                        }
//                    }
//                }
//            }
//            
//            // Set the new time bound
//            if (isTimed()) {
//                if (!isTangible())
//                    marking.timeBound = 0.0;
//                else
//                    setTimeBound(gspnContext, nextTimeBound);
//            }
//        }
//    }
//}
