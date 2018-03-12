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
public class RGMEDD2SolverParams extends SolverParams {
    
    public enum VariableOrder implements ListRenderable {
        NO_ORDER("No variable ordering.", "", false),
        FROM_FILE("Read the variable order from the <netname>.place file. ", "-F", false),
        META_HEURISTIC("Meta-heuristic.", "-META", true),
        HEURISTIC_P("Derive order using P-semiflows.", "-P", true),
        HEURISTIC_FORCE("Use FORCE algorithm point spans metric.", "-FORCE", false),
        HEURISTIC_FORCE_NES("Use FORCE algorithm with NES metric.", "-FORCE-NES", false),
        HEURISTIC_FORCE_WES1("Use FORCE algorithm with WES(1) metric.", "-FORCE-WES1", false),
        HEURISTIC_FORCE_P("Use FORCE weighted by P-semiflows.", "-FORCE-P", true),
        HEURISTIC_CM("Use Cuthill-McKee method.", "-CM", false),
        HEURISTIC_KING("Use King ordering method.", "-KING", false),
        HEURISTIC_SLOAN("Use Sloan ordering with W1=1, W2=2.", "-SLO", false),
        HEURISTIC_SLOAN_16("Use Sloan ordering with W1=1, W2=16.", "-SLO-16", false),
        HEURISTIC_NOACK("Use Noack ordering.", "-NOACK", false),
        HEURISTIC_TOVCHIGRECHKO("Use Tovchigrechko ordering.", "-TOV", false),
        HEURISTIC_GRADIENT_P("Use Gradient-P ordering.", "-GP", true),
        HEURISTIC_MCL("Use Markov Clustering ordering.", "-MCL", true);

        private VariableOrder(String description, String cmdOption, boolean usesPinvars) {
            this.description = description;
            this.cmdOption = cmdOption;
            this.usesPinvars = usesPinvars;
        }
        private final String description;
        private final String cmdOption;
        private final boolean usesPinvars;

        @Override public String getDescription() { return description; }
        public String getCmdOption() {
            return cmdOption;
        }
        public boolean usesPinvars() {
            return usesPinvars;
        }
        @Override public Icon getIcon16() { return null; }
        @Override public int getTextSize() { return -1; /* default*/ }
    }
    
    public static final VariableOrder DEFAULT_VARIABLE_ORDER = VariableOrder.META_HEURISTIC;
    public static final boolean DEFAULT_GEN_COUNTEREXAMPLES = false;

    
    public VariableOrder varOrder = DEFAULT_VARIABLE_ORDER;
    public boolean genCounterExamples = DEFAULT_GEN_COUNTEREXAMPLES;
    
    @Override
    public Class getPanelClass() {
        return RGMEDD2SolverPanel.class;
    }

    @Override
    public Solvers getSolver() {
        return Solvers.RGMEDD2;
    }

    @Override
    public SolverInvokator makeNewSolver() {
        return new RGMEDD2Solver();
    }
    
    @Override
    public String isLanguageSupported(FormulaLanguage lang) {
        switch (lang) {
            case CTL:
            case STAT:
            case DD:
            case INC:
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
        bindXMLAttrib(this, el, exDir, "variable-order", "varOrder", DEFAULT_VARIABLE_ORDER);
    }
}
