/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import editor.domain.Node;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.Transition;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import org.w3c.dom.Element;

/** parameters for the Nsolve tool of Peter Buchholz.
 *
 * @author elvio
 */
public class NSolveParams extends SolverParams {
    
    public static final String DEFAULT_MAX_NO_OF_STATES_LOCAL = "30000";
    public static final String DEFAULT_MAX_NO_OF_STATES_TOTAL = "30000000";
    public static final String DEFAULT_NUMERICAL_METHOD = "HSTR_POWER";
    public static final String DEFAULT_MAX_NO_OF_ITERATIONS = "2000";
    public static final String DEFAULT_MAX_CPU_TIME = "300";
    public static final String DEFAULT_MAX_MEMORY = "64";
    public static final String DEFAULT_RELAXATION_FACTOR = "0.95";
    public static final String DEFAULT_EPSILON = "1.0e-08";
    
    public IntExpr maxLocalStates = new IntExpr(DEFAULT_MAX_NO_OF_STATES_LOCAL);
    public IntExpr maxTotalStates = new IntExpr(DEFAULT_MAX_NO_OF_STATES_TOTAL);
    public String numericalMethod = DEFAULT_NUMERICAL_METHOD;
    public IntExpr maxIters = new IntExpr(DEFAULT_MAX_NO_OF_ITERATIONS);
    public IntExpr maxCpuTime = new IntExpr(DEFAULT_MAX_CPU_TIME);
    public IntExpr maxMemory = new IntExpr(DEFAULT_MAX_MEMORY);
    public RealExpr relaxFactor = new RealExpr(DEFAULT_RELAXATION_FACTOR);
    public RealExpr epsilon = new RealExpr(DEFAULT_EPSILON);
    
    public static final String[] kwnownNumericalMethods = {
        "MATGEN", "STD_LU", "STD_POWER", "STD_JOR", "STD_SOR", "STD_POWER_AD", "STD_JOR_AD", 
        "STD_SOR_AD", "STD_KMS", "STD_ML", "STD_GMRES", "STD_BICGSTAB", "STD_TFQMR", 
        "STD_ILU0_POWER", "STD_ILUTH_POWER", "STD_ILU0_GMRES", "STD_ILUTH_GMRES", 
        "STD_ILU0_BICGSTAB", "STD_ILUTH_BICGSTAB", "STD_ILU0_TFQMR", "STD_ILUTH_TFQMR", 
        "HSTR_POWER", "HSTR_JOR", "HSTR_SOR", "HSTR_RSOR", "HSTR_POWER_AD", "HSTR_JOR_AD", 
        "HSTR_SOR_AD", "HSTR_GMRES", "HSTR_DQGMRES", "HSTR_ARNOLDI", "HSTR_CGS", "HSTR_BICGSTAB", 
        "HSTR_TFQMR", "HSTR_GMRES_AD", "HSTR_DQGMRES_AD", "HSTR_ARNOLDI_AD", "HSTR_CGS_AD", 
        "HSTR_BICGSTAB_AD", "HSTR_PRE_POWER", "HSTR_PRE_GMRES", "HSTR_PRE_ARNOLDI", 
        "HSTR_PRE_CGS", "HSTR_PRE_BICGSTAB", "HSTR_PRE_TFQMR", "HSTR_BSOR_BICGSTAB", 
        "HSTR_BSOR_GMRES", "HSTR_BSOR_TFQMR", "HSTR_STB_JOR", "HSTR_STB_SOR", "HSTR_STB_SOR_T", 
        "HSTR_STB_JOR_AD", "HSTR_STB_SOR_AD", "HSTR_ML_POWER", "HSTR_ML_JOR", "HSTR_ML_SOR", 
        "HSTR_ML_POWER", "HSTR_ML_JOR", "HSTR_ML_SOR", "HSTR_ML_POWER_W", "HSTR_ML_JOR_W", 
        "HSTR_ML_SOR_W", "HSTR_ML_POWER_F", "HSTR_ML_JOR_F", "HSTR_ML_SOR_F", "HSTR_APP_POWER", 
        "HSTR_APP_GS_POWER", "HSTR_APP_BLOCK_GS", "HSTR_RANDOMIZATION", "HSTR_RANDOMIZATION_PT", 
        "HSTR_RANDOM_REWARD", "HSTR_RANDOM_REWARD_PT", "HSTR_APP_RANDOMIZATION", 
        "HSTR_APP_MULTI_RANDOMIZATION", 
    };
    
    @Override
    public Class getPanelClass() {
        return NSolvePanel.class;
    }

    @Override
    public Solvers getSolver() { return Solvers.NSOLVE; }

    @Override
    public SolverInvokator makeNewSolver() {
        return new NSolve();
    }
    
    @Override
    public String isLanguageSupported(FormulaLanguage lang) {
        switch (lang) {
            case ALL:
            case STAT:
                return null; // supported
                
            default:
                return LANG_NOT_SUPPORTED;
        }
    }
    
    @Override
    public void isGspnSupported(GspnPage gspn, MeasurePage errPage) {
        boolean notifyMissingPartitions = true;
        boolean notifyNumServers = true;
        for (Node node : gspn.nodes) {
            if (node instanceof Place) {
                Place pl = (Place)node;
                if (pl.getKroneckerPartition() == null ||
                    pl.getKroneckerPartition().length() == 0) {
                    if (notifyMissingPartitions) {
                        errPage.addPageError("nsolve requires that all places in the Petri net "
                                + "belong to a Kronecker partition.", null);
                        notifyMissingPartitions = false;
                    }
                }
            }
            else if (node instanceof Transition) {
                Transition trn = (Transition)node;
                if (trn.hasNumServers() && !trn.getNumServers().equals("1")) {
                    if (notifyNumServers) {
                        errPage.addPageError("nsolve requires that all EXP transitions have a single server.", null);
                        notifyNumServers = false;
                    }
                }
            }
        }
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        bindXMLAttrib(this, el, exDir, "max-local-states", "maxLocalStates.@Expr", DEFAULT_MAX_NO_OF_STATES_LOCAL);
        bindXMLAttrib(this, el, exDir, "max-total-states", "maxTotalStates.@Expr", DEFAULT_MAX_NO_OF_STATES_TOTAL);
        bindXMLAttrib(this, el, exDir, "method", "numericalMethod", DEFAULT_NUMERICAL_METHOD);
        bindXMLAttrib(this, el, exDir, "max-iters", "maxIters.@Expr", DEFAULT_MAX_NO_OF_ITERATIONS);
        bindXMLAttrib(this, el, exDir, "max-cpu-time", "maxCpuTime.@Expr", DEFAULT_MAX_CPU_TIME);
        bindXMLAttrib(this, el, exDir, "max-memory", "maxMemory.@Expr", DEFAULT_MAX_MEMORY);
        bindXMLAttrib(this, el, exDir, "relaxation", "relaxFactor.@Expr", DEFAULT_RELAXATION_FACTOR);
        bindXMLAttrib(this, el, exDir, "epsilon", "epsilon.@Expr", DEFAULT_EPSILON);
//        bindXMLAttrib(this, el, exDir, "mode", "mode", DEFAULT_SOLVER_MODE);
//        bindXMLAttrib(this, el, exDir, "sol-time", "solTime", DEFAULT_SOL_TIME);
//        bindXMLAttrib(this, el, exDir, "max-iters", "maxIters.@Expr", DEFAULT_MAX_ITERS_EXPR);
//        bindXMLAttrib(this, el, exDir, "time-T", "timeT.@Expr", DEFAULT_TIME_T_EXPR);
//        bindXMLAttrib(this, el, exDir, "max-dot-markings", "maxDotMarkings.@Expr", DEFAULT_MAX_DOT_MARKINGS);
    }
}
