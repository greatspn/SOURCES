/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import editor.domain.Expr;
import editor.domain.elements.GspnPage;
import editor.domain.grammar.ParserContext;
import editor.domain.io.XmlExchangeable;
import java.io.Serializable;

/** Parameters of solvers are contained in this class.
 *
 * @author elvio
 */
public abstract class SolverParams implements Serializable, XmlExchangeable {
    
    public static enum Solvers {
        GREATSPN("GreatSPN", GreatSPNSolverParams.class),
        MC4CSLTA("<html>MC<font color=blue>4</font>CSLTA</html>", MC4CSLTASolverParams.class),
//        RGMEDD("RGMEDD", RGMEDDSolverParams.class),
        RGMEDD2("RGMEDD2", RGMEDD2SolverParams.class),
        NSOLVE("NSolve", NSolveParams.class),
        COSMOS("Cosmos", CosmosSolverParams.class),
        ;
        
        private String text;
        private Class paramsClass;

        private Solvers(String text, Class paramsClass) {
            this.text = text;
            this.paramsClass = paramsClass;
        }

        @Override
        public String toString() { 
            return text;
        }

        public Class getParamsClass() {
            return paramsClass;
        }
    }
    
    public static class RealExpr extends Expr {

        public RealExpr() { }

        public RealExpr(String expr) {
            super(expr);
        }
        
        @Override
        protected String getExprDescr() {
            return "Real parameter.";
        }

        @Override
        protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
            return ParserContext.ParserEntryPoint.REAL_EXPR;
        }

        @Override
        protected int getParseFlags() { 
            return ParserContext.PF_CONST_EXPR; 
        }
    }
    
    public static class IntExpr extends Expr {

        public IntExpr() { }

        public IntExpr(String expr) {
            super(expr);
        }
        
        @Override
        protected String getExprDescr() {
            return "Integer parameter.";
        }

        @Override
        protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
            return ParserContext.ParserEntryPoint.INT_EXPR;
        }

        @Override
        protected int getParseFlags() { 
            return ParserContext.PF_CONST_EXPR; 
        }
    }
    
    public abstract Class getPanelClass();
    public abstract Solvers getSolver();
    public abstract SolverInvokator makeNewSolver();
    
    // Return null if it is supported, and a message string if it is not.
    public abstract String isLanguageSupported(FormulaLanguage lang);
    
    // May decide whether a GSPN will not be accepted by the solver
    public abstract void isGspnSupported(GspnPage gspn, MeasurePage errPage);
    
    public static String LANG_NOT_SUPPORTED = "Measure type not supported.";
}
