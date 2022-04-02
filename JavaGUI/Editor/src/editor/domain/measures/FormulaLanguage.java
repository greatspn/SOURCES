/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.measures;

import editor.Main;
import editor.domain.grammar.ParserContext;
import editor.gui.ResourceFactory;
import javax.swing.Icon;

/** A language of performance indexes.
 *
 * @author Elvio
 */
public enum FormulaLanguage {
    ALL("Basic place and transition measures.", "basic measures", "property_all",
            null, 0),
    STAT("Basic tool statistics", "statistics", "property_stat", 
            null, 0),
    CTL("CTL formula", null, "property_ctl", 
            ParserContext.ParserEntryPoint.BOOLEAN_EXPR, ParserContext.PF_CTL_KEYWORDS),
    LTL("LTL formula", null, "property_ltl", 
            ParserContext.ParserEntryPoint.BOOLEAN_EXPR, ParserContext.PF_LTL_KEYWORDS),
    CTLSTAR("CTL* formula", null, "property_ctlstar", 
            ParserContext.ParserEntryPoint.BOOLEAN_EXPR, ParserContext.PF_CTLSTAR_KEYWORDS),
    FAIRNESS("Fairness constraint", null, "property_fair", 
            ParserContext.ParserEntryPoint.BOOLEAN_EXPR, ParserContext.PF_CTLSTAR_KEYWORDS),
    CSL("CSL formula", null, "property_csl", 
            ParserContext.ParserEntryPoint.INT_EXPR, 0),
    CSLTA("<html>CSL<font size=-1><sup>TA</sup></font> formula</html>", "CSLTA formula", "property_cslta", 
            ParserContext.ParserEntryPoint.CSLTA_EXPR, 0),
    PERFORMANCE_INDEX("Performance Index", null, "property_perf", 
            ParserContext.ParserEntryPoint.PERFORMANCE_MEASURE, 0),
    TRG("Tangible Reachability Graph", null, "property_trg", 
            null, 0),
    RG("Reachability Graph with Vanishings", null, "property_rg", 
        null, 0),
    DD("Decision Diagram graph", null, "property_dd", null, 0),
    INC("Incidence matrix", null, "property_inc", null, 0);
    
    private final String name, shortName;
    private final Icon icon24;
    private final ParserContext.ParserEntryPoint parseRule;
    private final int parseFlags;

    private FormulaLanguage(String name, String shortName, String icon24_name, 
                            ParserContext.ParserEntryPoint parseRule, int parseFlags)
    {
        this.name = name;
        this.shortName = (shortName == null) ? name : shortName;
        this.icon24 = ResourceFactory.loadPropertyIcon(icon24_name);
        this.parseRule = parseRule;
        this.parseFlags = parseFlags;
    }

    public String getName() {
        return name;
    }
    public String getShortName() {
        return shortName;
    }
    public Icon getIcon24() {
        return icon24;
    }  
    public ParserContext.ParserEntryPoint getParseRule() {
        return parseRule;
    }
    public int getParseFlags() {
        return parseFlags;
    }
    public boolean hasFormulaText() {
        return getParseRule() != null;
    }
    public String alternateToFormulaText() {
        switch (this) {
            case ALL: 
                return "All place distributions and transition throughputs.";
            case STAT:
                return "Tool statistics.";
            case TRG:
                return "Plot of the Tangible Reachability Graph.";
            case RG:
                return "Plot of the Reachability Graph with vanishing markings.";
            case DD:
                return "Plot of the Decision Diagram graph";
            case INC:
                return "Visualization of the Incidence matrix";
            default:
                return "---";
        }
    }
    // At most one entry per each formula page
    public boolean isUniqueInPage() { return !hasFormulaText(); }
    
    // Text shown in the button that will open the panel with the measure detail
    public String getViewResultButtonText() {
        switch (this) {
            case TRG:
            case RG:
            case STAT:
            case DD:
            case INC:
                return "View...";
            default: 
                return "Results...";
        }
    }
}
