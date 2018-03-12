/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import editor.domain.elements.GspnPage;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import org.w3c.dom.Element;

/** The parameters used to invoke the MC4CSLTA solver
 *
 * @author elvio
 */
public class MC4CSLTASolverParams extends SolverParams {
    public enum MrpMethod {
        IMPLICIT, EXPLICIT, COMPONENT_METHOD
    }
    public enum LinearAlgoType {
        FORWARD_SOR, BACKWARD_SOR, SYMMETRIC_SOR, JOR,
        GMRES, CGS, BICGSTAB, POWER_METHOD
    }
    public enum McSolutionType {
        FORWARD, BACKWARD
    }
    public enum PerformanceIndexTime {
        STEADY_STATE, TRANSIENT
    }

    public static final String DEFAULT_TIME_T_EXPR = "1.0";
    public static final String DEFAULT_EPSILON_EXPR = "1.0e-7";
    public static final MrpMethod DEFAULT_MRP_METHOD = MrpMethod.IMPLICIT;
    public static final LinearAlgoType DEFAULT_LINALGO_METHOD = LinearAlgoType.GMRES;
    public static final McSolutionType DEFAULT_MC_SOL_TYPE = McSolutionType.FORWARD;
    public static final PerformanceIndexTime DEFAULT_PERF_TIME = PerformanceIndexTime.STEADY_STATE;
    public static final boolean DEFAULT_ON_THE_FLY = true;
    public static final String DEFAULT_MAX_DOT_MARKINGS = "80";
    public static final boolean DEFAULT_DOT_CLUSTERS = false;
        
    public RealExpr epsilon = new RealExpr(DEFAULT_EPSILON_EXPR);
    public MrpMethod mrpMethod = DEFAULT_MRP_METHOD;
    public LinearAlgoType linAlgo = DEFAULT_LINALGO_METHOD;
    public McSolutionType mcSolType = DEFAULT_MC_SOL_TYPE;
    public boolean onTheFly = DEFAULT_ON_THE_FLY;
    public PerformanceIndexTime piTime = DEFAULT_PERF_TIME;
    public RealExpr timeT = new RealExpr(DEFAULT_TIME_T_EXPR);
    public IntExpr maxDotMarkings = new IntExpr(DEFAULT_MAX_DOT_MARKINGS);
    public boolean dotClusters = DEFAULT_DOT_CLUSTERS;
    
    @Override
    public Class getPanelClass() {
        return MC4CSLTASolverPanel.class;
    }

    @Override
    public Solvers getSolver() {
        return Solvers.MC4CSLTA;
    }

    @Override
    public SolverInvokator makeNewSolver() {
        return new MC4CSLTASolver();
    }
    
    @Override
    public String isLanguageSupported(FormulaLanguage lang) {
        switch (lang) {
            case ALL:
            case PERFORMANCE_INDEX:
            case RG:
            case TRG:
            case CSLTA:
            case STAT:
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
        bindXMLAttrib(this, el, exDir, "mrp-method", "mrpMethod", DEFAULT_MRP_METHOD);
        bindXMLAttrib(this, el, exDir, "linear-algorithm", "linAlgo", DEFAULT_LINALGO_METHOD);
        bindXMLAttrib(this, el, exDir, "mc-sol-type", "mcSolType", DEFAULT_MC_SOL_TYPE);
        bindXMLAttrib(this, el, exDir, "on-the-fly", "onTheFly", DEFAULT_ON_THE_FLY);
        bindXMLAttrib(this, el, exDir, "index-time", "piTime", DEFAULT_PERF_TIME);
        bindXMLAttrib(this, el, exDir, "max-dot-markings", "maxDotMarkings.@Expr", DEFAULT_MAX_DOT_MARKINGS);
        bindXMLAttrib(this, el, exDir, "dot-clusters", "dotClusters", DEFAULT_DOT_CLUSTERS);
    }
}
