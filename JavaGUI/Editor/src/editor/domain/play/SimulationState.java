///*
// * To change this license header, choose License Headers in Project Properties.
// * To change this template file, choose Tools | Templates
// * and open the template in the editor.
// */
//
//package editor.domain.play;
//
//import editor.domain.elements.DtaPage;
//import editor.domain.elements.GspnPage;
//import editor.domain.grammar.ParserContext;
//import java.util.Locale;
//
////    private static class SimulationException extends Exception {
//
////        SimulationException(String reason) { super(reason); }
//public class SimulationState {
//    // State data of the simulated model
//    public GspnSimulationEngine.ModelState state;
//    // Pre-computed textual state representation
//    public String markingText;
//    // != null if there is some unrecoverable error.
//    public String errorDescription;
//    // Cardinal number of this state in a path trace
//    public int stateNum;
//    // Remembers if this is a timed simulation or not.
//    public boolean isTimedSimulation;
//    // Elapsed time from the beginning of the simulation
//    public double time;
//    // Time boundary of the next event that changes the enabling of the transitions,
//    // or -1 if there is no time bound.
//    public double nextTimeBound;
//    // Suggested time instant for the next timed event
//    public double avgTimeNextTransition;
//    public boolean eventsAreUrgent;
//
//    public boolean isTangible() {
//        return state.isTangible() && !eventsAreUrgent;
//    }
//
//    public void setErrorState(String errDescr) {
//        state.setDeadState();
//        markingText = "<<ERROR>> " + errDescr;
//        errorDescription = errDescr;
//    }
//
//    public String getStateRepr(ParserContext gspnContext) {
//        // Prepare the marking text representation
//        StringBuilder sb = new StringBuilder();
//        if (isTimedSimulation) {
//            sb.append("@").append(String.format(Locale.US, "%.4f", time)).append(": ");
//        }
//        return sb.toString() + state.getStateRepr();
//    }
//    
//    private void prepareEnabledSet(ParserContext gspnContext, GspnPage gspn,
//                                    ParserContext dtaContext, DtaPage dta, 
//                                    JointFiring firing) 
//     {
//         nextTimeBound = -1;
//         avgTimeNextTransition = -1;
//     }
//}
