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

/** Parameters of the Cosmos solver.
 *
 * @author benoit
 */
public class CosmosSolverParams extends SolverParams {

    public enum SolverMode implements ListRenderable {
        EGSPN("Basic GSPN"), SWN_ORD("SWN Ordinary"), SWN_SYM("SWN Symbolic");

        private SolverMode(String longName) {
            this.longName = longName;
        }
        String longName;

        @Override public String getDescription() { return longName; }
        @Override public Icon getIcon16() { return null; }
        @Override public int getTextSize() { return -1; /* default*/ }
    }
    public enum SolutionTime {
        STEADY_STATE, TRANSIENT, SIMULATED
    }
    
    public static final SolverMode DEFAULT_SOLVER_MODE = SolverMode.SWN_ORD;
    public static final SolutionTime DEFAULT_SOL_TIME = SolutionTime.STEADY_STATE;
    public static final String DEFAULT_TIME_T_EXPR = "1.0";
    public static final String DEFAULT_EPSILON_EXPR = "1.0e-7";
    public static final String DEFAULT_MAX_ITERS_EXPR = "10000";
    public static final String DEFAULT_MAX_DOT_MARKINGS = "80";
    
    public SolverMode mode = DEFAULT_SOLVER_MODE;
    public SolutionTime solTime = DEFAULT_SOL_TIME;
    public RealExpr timeT = new RealExpr(DEFAULT_TIME_T_EXPR);
    public RealExpr epsilon = new RealExpr(DEFAULT_EPSILON_EXPR);
    public IntExpr maxIters = new IntExpr(DEFAULT_MAX_ITERS_EXPR);
    public IntExpr maxDotMarkings = new IntExpr(DEFAULT_MAX_DOT_MARKINGS);

    @Override
    public Class getPanelClass() {
        return CosmosSolverPanel.class;
    }

    @Override
    public Solvers getSolver() { return Solvers.COSMOS; }

    @Override
    public SolverInvokator makeNewSolver() {
        return new CosmosSolver();
    }

    @Override
    public String isLanguageSupported(FormulaLanguage lang) {
        switch (lang) {
            case ALL:
            case STAT:
            case PERFORMANCE_INDEX:
                return null; // supported
                        
            default:
                return LANG_NOT_SUPPORTED;
        }
    }
    
    @Override
    public void isGspnSupported(GspnPage gspn, MeasurePage errPage) {
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        bindXMLAttrib(this, el, exDir, "epsilon", "epsilon.@Expr", DEFAULT_EPSILON_EXPR);
        bindXMLAttrib(this, el, exDir, "mode", "mode", DEFAULT_SOLVER_MODE);
        bindXMLAttrib(this, el, exDir, "sol-time", "solTime", DEFAULT_SOL_TIME);
        bindXMLAttrib(this, el, exDir, "max-iters", "maxIters.@Expr", DEFAULT_MAX_ITERS_EXPR);
        bindXMLAttrib(this, el, exDir, "time-T", "timeT.@Expr", DEFAULT_TIME_T_EXPR);
        bindXMLAttrib(this, el, exDir, "max-dot-markings", "maxDotMarkings.@Expr", DEFAULT_MAX_DOT_MARKINGS);
    }
}
