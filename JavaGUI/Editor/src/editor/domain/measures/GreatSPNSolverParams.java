/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import editor.domain.ListRenderable;
import editor.domain.elements.GspnPage;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import javax.swing.Icon;
import org.w3c.dom.Element;

/** Parameters of the GreatSPN solver.
 *
 * @author elvio
 */
public class GreatSPNSolverParams extends SolverParams {

    public enum SolverMode implements ListRenderable {
        EGSPN("<html>[<b><font color=#14ABDE>state space</font></b>] Basic GSPN (no colors)"), 
        SWN_ORD("<html>[<b><font color=#14ABDE>state space</font></b>] SWN Ordinary"), 
        SWN_SYM("<html>[<b><font color=#14ABDE>state space</font></b>] SWN Symbolic"),
        EGSPN_SIMUL("<html>[<b><font color=#DE1486>simulation</font></b>] Basic GSPN (no colors)"),
        SWN_ORD_SIMUL("<html>[<b><font color=#DE1486>simulation</font></b>] SWN Ordinary"),
        SWN_SYM_SIMUL("<html>[<b><font color=#DE1486>simulation</font></b>] SWN Symbolic");

        private SolverMode(String longName) {
            this.longName = longName;
        }
        String longName;
        
        public boolean isSimulation() { 
            return ((this == SWN_ORD_SIMUL) || 
                    (this == SWN_SYM_SIMUL) ||
                    (this == EGSPN_SIMUL)); 
        }

        @Override public String getDescription() { return longName; }
        @Override public Icon getIcon16() { return null; }
        @Override public int getTextSize() { return -1; /* default*/ }
    }
    public enum SolutionTime {
        STEADY_STATE, TRANSIENT
    }
    
    public enum Confidence implements ListRenderable {
        CONF_60("60 %", 60), CONF_70("70 %", 70), CONF_80("80 %", 80),
        CONF_90("90 %", 90), CONF_95("95 %", 95), CONF_99("99 %", 99);
        
        private Confidence(String longName, int confNum) {
            this.longName = longName;
            this.confNum = confNum;
        }
        String longName;
        int confNum;
        
        public int getConfidencePercentage() { return confNum; }
        @Override public String getDescription() { return longName; }
        @Override public Icon getIcon16() { return null; }
        @Override public int getTextSize() { return -1; /* default*/ }
    }
    
    public enum BatchLengthMode implements ListRenderable {
        BY_EVENT_COUNT("event count"), 
        BY_DURATION("duration");
        
        private BatchLengthMode(String longName) {
            this.longName = longName;
        }
        String longName;
        
        @Override public String getDescription() { return longName; }
        @Override public Icon getIcon16() { return null; }
        @Override public int getTextSize() { return -1; /* default*/ }
    }
    
    public static final SolverMode DEFAULT_SOLVER_MODE = SolverMode.SWN_ORD;
    public static final SolutionTime DEFAULT_SOL_TIME = SolutionTime.STEADY_STATE;
    public static final String DEFAULT_TIME_T_EXPR = "1.0";
    public static final String DEFAULT_EPSILON_EXPR = "1.0e-7";
    public static final String DEFAULT_MAX_ITERS_EXPR = "10000";
    public static final String DEFAULT_MAX_DOT_MARKINGS = "80";
    public static final Confidence DEFAULT_CONFIDENCE = Confidence.CONF_90;
    public static final String DEFAULT_APPROX = "15";
    public static final String DEFAULT_FIRST_TR_LENGTH = "10.0";
    public static final String DEFAULT_MIN_BATCH = "10";
    public static final String DEFAULT_MAX_BATCH = "10000";
    public static final String DEFAULT_SEED = "12345";
    public static final BatchLengthMode DEFAULT_BATCH_LENGTH_MODE = BatchLengthMode.BY_EVENT_COUNT;


    
    public SolverMode mode = DEFAULT_SOLVER_MODE;
    public SolutionTime solTime = DEFAULT_SOL_TIME;
    public RealExpr timeT = new RealExpr(DEFAULT_TIME_T_EXPR);
    public RealExpr epsilon = new RealExpr(DEFAULT_EPSILON_EXPR);
    public IntExpr maxIters = new IntExpr(DEFAULT_MAX_ITERS_EXPR);
    public IntExpr maxDotMarkings = new IntExpr(DEFAULT_MAX_DOT_MARKINGS);
    
    public Confidence confidence = DEFAULT_CONFIDENCE;
    public IntExpr approxPercentage = new IntExpr(DEFAULT_APPROX);
    public boolean useFirstTrLength = false;
    public RealExpr firstTrLength = new RealExpr(DEFAULT_FIRST_TR_LENGTH);
    public boolean useBatchConstraints = false;
    public RealExpr minBatch = new RealExpr(DEFAULT_MIN_BATCH);
    public RealExpr maxBatch = new RealExpr(DEFAULT_MAX_BATCH);
    public boolean useSeed = false;
    public IntExpr seed = new IntExpr(DEFAULT_SEED);
    public BatchLengthMode batchLengthMode = DEFAULT_BATCH_LENGTH_MODE;

    @Override
    public Class getPanelClass() {
        return GreatSPNSolverPanel.class;
    }

    @Override
    public Solvers getSolver() { return Solvers.GREATSPN; }

    @Override
    public SolverInvokator makeNewSolver() {
        return new GreatSPNSolver();
    }

    @Override
    public String isLanguageSupported(FormulaLanguage lang) {
        switch (lang) {
            case ALL:
            case STAT:
                return null; // supported
                
            case PERFORMANCE_INDEX:
                if (mode.isSimulation())
                    return "Only ALL and STAT measures are supported in simulation mode.";
                return null; // supported
                
                // RG generation is only supported by SWN solvers, and no simulation
            case RG:
                if (mode == SolverMode.EGSPN)
                    return SolverMode.EGSPN.longName+" mode does not generate RG plot.";
                if (mode.isSimulation())
                    return "Cannot plot the RG in simulation mode.";
                return null; // ok
                        
            default:
                return LANG_NOT_SUPPORTED;
        }
    }
    
    @Override
    public void isGspnSupported(GspnPage gspn, MeasurePage errPage) {
        if (mode == SolverMode.EGSPN || mode == SolverMode.EGSPN_SIMUL) {
            if (gspn.gspnHasColors())
                errPage.addPageError("Basic GSPN solver does not support models with colors.", null);
        }
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        bindXMLAttrib(this, el, exDir, "mode", "mode", DEFAULT_SOLVER_MODE);
        bindXMLAttrib(this, el, exDir, "epsilon", "epsilon.@Expr", DEFAULT_EPSILON_EXPR);
        if (exDir.XmlToFields() && el.getAttribute("sol-time").equals("SIMULATED"))
            solTime = SolutionTime.STEADY_STATE;
        else {
            bindXMLAttrib(this, el, exDir, "sol-time", "solTime", DEFAULT_SOL_TIME);
        }
        bindXMLAttrib(this, el, exDir, "max-iters", "maxIters.@Expr", DEFAULT_MAX_ITERS_EXPR);
        bindXMLAttrib(this, el, exDir, "time-T", "timeT.@Expr", DEFAULT_TIME_T_EXPR);
        bindXMLAttrib(this, el, exDir, "max-dot-markings", "maxDotMarkings.@Expr", DEFAULT_MAX_DOT_MARKINGS);
        bindXMLAttrib(this, el, exDir, "confidence", "confidence", DEFAULT_CONFIDENCE);
        bindXMLAttrib(this, el, exDir, "approx", "approxPercentage.@Expr", DEFAULT_APPROX);
        bindXMLAttrib(this, el, exDir, "use-first-transient", "useFirstTrLength", false);
        bindXMLAttrib(this, el, exDir, "first-transient-length", "firstTrLength.@Expr", DEFAULT_FIRST_TR_LENGTH);
        bindXMLAttrib(this, el, exDir, "use-batch-constraints", "useBatchConstraints", false);
        bindXMLAttrib(this, el, exDir, "min-batch", "minBatch.@Expr", DEFAULT_MIN_BATCH);
        bindXMLAttrib(this, el, exDir, "max-batch", "maxBatch.@Expr", DEFAULT_MAX_BATCH);
        bindXMLAttrib(this, el, exDir, "use-seed", "useSeed", false);
        bindXMLAttrib(this, el, exDir, "seed", "seed.@Expr", DEFAULT_SEED);
        bindXMLAttrib(this, el, exDir, "batch-length-mode", "batchLengthMode", DEFAULT_BATCH_LENGTH_MODE);
    }
}
