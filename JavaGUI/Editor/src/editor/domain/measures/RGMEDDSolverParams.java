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

/**
 *
 * @author elvio
 */
public class RGMEDDSolverParams extends SolverParams {
    
    public enum VariableOrder implements ListRenderable {
        NO_ORDER("No variable ordering."),
        FROM_FILE("Read from manually-provided file."),
        HEURISTIC_PINV("Use P-invariants heuristic."),
        HEURISTIC_FORCE_WES1("Use FORCE-WES(1) heuristic."),
        HEURISTIC_FORCE_PINV("Use FORCE + P-invariants heuristic.");
        
        private VariableOrder(String longName) {
            this.description = longName;
        }
        private final String description;

        @Override public String getDescription() { return description; }
        @Override public Icon getIcon16() { return null; }
        @Override public int getTextSize() { return -1; /* default*/ }
    }
    
    public enum PlaceBoundMethod implements ListRenderable {
        MANUALLY_SPECIFIED("Specify upper-bound manually."),
        FROM_PINVAR("Derived automatically from P-invariants.");
        
        private PlaceBoundMethod(String longName) {
            this.description = longName;
        }
        private final String description;

        @Override public String getDescription() { return description; }
        @Override public Icon getIcon16() { return null; }
        @Override public int getTextSize() { return -1; /* default*/ }
    }
    
    public static final String DEFAULT_PLACE_BOUND_EXPR = "";
    public static final VariableOrder DEFAULT_VARIABLE_ORDER = VariableOrder.NO_ORDER;
    public static final PlaceBoundMethod DEFAULT_PLACE_BOUND_METHOD = PlaceBoundMethod.FROM_PINVAR;
    public static final boolean DEFAULT_GEN_COUNTEREXAMPLES = false;

    
    public VariableOrder varOrder = DEFAULT_VARIABLE_ORDER;
    public PlaceBoundMethod plBoundMeth = DEFAULT_PLACE_BOUND_METHOD;
    public boolean genCounterExamples = DEFAULT_GEN_COUNTEREXAMPLES;
    public IntExpr placeBound = new IntExpr(DEFAULT_PLACE_BOUND_EXPR);
    
    @Override
    public Class getPanelClass() {
        return RGMEDDSolverPanel.class;
    }

    @Override
    public Solvers getSolver() {
        return Solvers.RGMEDD;
    }

    @Override
    public SolverInvokator makeNewSolver() {
        return new RGMEDDSolver();
    }
    
    @Override
    public String isLanguageSupported(FormulaLanguage lang) {
        switch (lang) {
            case CTL:
            case STAT:
                return null; // Supported
                
            default:
                return LANG_NOT_SUPPORTED;
        }
    }
    
    @Override
    public void isGspnSupported(GspnPage gspn, MeasurePage errPage) {
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        bindXMLAttrib(this, el, exDir, "counter-examples", "genCounterExamples", DEFAULT_GEN_COUNTEREXAMPLES);
        bindXMLAttrib(this, el, exDir, "place-bound-method", "plBoundMeth", DEFAULT_PLACE_BOUND_METHOD);
        bindXMLAttrib(this, el, exDir, "variable-order", "varOrder", DEFAULT_VARIABLE_ORDER);
        bindXMLAttrib(this, el, exDir, "place-bound", "placeBound.@Expr", DEFAULT_PLACE_BOUND_EXPR);
    }
}
