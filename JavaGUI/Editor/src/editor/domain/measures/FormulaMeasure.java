/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.measures;

import editor.domain.EditableValue;
import editor.domain.Expr;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.grammar.ParserContext;
import editor.domain.grammar.VarListMultiAssignment;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import java.util.ArrayList;
import java.util.Map;
import java.util.TreeMap;
import org.w3c.dom.Element;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;

/** A measure that is a single formula string in a given language
 *
 * @author Elvio
 */
public class FormulaMeasure extends AbstractMeasure {
    
    public class FormulaExpr extends Expr {
        public FormulaExpr(String expr) { super(expr); }
        @Override protected String getExprDescr() { return "Measure expression."; }
        @Override protected int getParseFlags() { 
            return language.getParseFlags();
        }
        @Override protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
            return language.getParseRule(); 
        }

        @Override
        protected ParserContext createParseContext(ProjectData proj, ProjectPage page) {
            assert page instanceof MeasurePage;
            MeasurePage mpage = (MeasurePage)page;
            ParserContext context = new ParserContext(mpage.targetGspn);
            context.knownDtas = mpage.knownDtas;
            return context;
        }
    }

    // The formula, in a given formula language
    private final FormulaExpr formula = new FormulaExpr("");
    // The language of the formula
    private FormulaLanguage language;

    public FormulaMeasure(String comment, boolean commentShown, 
                          String formula, FormulaLanguage language) 
    {
        super(comment, commentShown);
        this.formula.setExpr(formula);
        this.language = language;
    }
    
    public FormulaMeasure() { }

    @Override
    public void checkMeasureCorrectness(MeasurePage page, ParserContext context) {
        String reason;
        if (null != (reason = page.solverParams.isLanguageSupported(language))) {
            String err = page.solverParams.getSolver().toString()+
                         " does not support a measure of type: "+language.getShortName()+". ";
            if (!reason.equals(SolverParams.LANG_NOT_SUPPORTED))
                err += reason;
            page.addPageError(err, this);
        }
        if (getLanguage().hasFormulaText())
            formula.checkExprCorrectness(context, page, this);
    }
    
    public FormulaExpr getFormula() {
        return formula;
    }

    public void setFormula(String formula) {
        this.formula.setExpr(formula);
    }

    public FormulaLanguage getLanguage() {
        return language;
    }

    public void setLanguage(FormulaLanguage language) {
        this.language = language;
    }
    
    public EditableValue getFormulaEditable() { 
        return formula.getEditableValue();
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir);
        
        bindXMLAttrib(this, el, exDir, "language", "language", null,  FormulaLanguage.class);
        
        if (language.hasFormulaText()) {
            bindXMLAttrib(this, el, exDir, "expr", "formula.@Expr", "");
            if (exDir.XmlToFields()) { // backward compatibility
                if (el.getTextContent() != null && 
                    el.getTextContent().length() > 0 &&
                    !el.hasAttribute("expr"))
                    formula.setExpr(el.getTextContent());
            }
        }
    }

    @Override
    public void getResultsAsWorkSheets(VarListMultiAssignment vma, ArrayList<ExcelXml.WorkSheet> ws) {
        switch (language) {
            case RG:
            case TRG:
            case DD:
            case INC:
                break;
                
            case ALL: {
                int count = 0;
                for (ResultEntry e : getResults().table) {
                    ((AllMeasuresResult)e).getResultsAsWorkSheets(ws, count++);
                }
            }
            break;
                
            case STAT: {
                // Prepare columns:
                ArrayList<String> colNames = new ArrayList<>();
                for (int i=0; i<vma.assigns.size(); i++)
                    colNames.add(vma.assigns.get(i).varName);
                Map<String, Integer> keyToColumn = new TreeMap<>();
                for (ResultEntry e : getResults().table) {
                    StatResultEntry stat = (StatResultEntry)e;
                    for (Map.Entry<String, String> sv : stat.statValues.entrySet()) {
                        if (keyToColumn.containsKey(sv.getKey()))
                            continue;
                        keyToColumn.put(sv.getKey(), keyToColumn.size() + vma.assigns.size());
                        colNames.add(sv.getKey());
                    }
                }
                // Fill table data
                Object[][] data = new Object[getResults().table.size()][colNames.size()];
                for (int r=0; r<getResults().table.size(); r++) {
                    StatResultEntry stat = (StatResultEntry)getResults().table.get(r);
                    for (int c=0; c<colNames.size(); c++)
                        data[r][c] = "-";
                    for (int c=0; c<vma.assigns.size(); c++) {
                        String varName = vma.assigns.get(c).varName;
                        data[r][c] = stat.assign.binding.get(varName);
                    }
                    for (Map.Entry<String, String> sv : stat.statValues.entrySet())
                        data[r][keyToColumn.get(sv.getKey())] = sv.getValue();
                }
                ws.add(new ExcelXml.WorkSheet("Tool statistics", 
                        "Tool statistics ", 
                        getComment(), colNames.toArray(new String[colNames.size()]), data));
            }
            break;
                
            default: {
                String[] colNames = new String[1 + vma.assigns.size()];
                for (int i=0; i<vma.assigns.size(); i++)
                    colNames[i] = vma.assigns.get(i).varName;
                colNames[colNames.length-1] = getFormula().getExpr();

                // Prepare table data
                Object[][] data = new Object[getResults().table.size()][colNames.length];
                for (int r=0; r<getResults().table.size(); r++) {
                    ResultEntry entry = getResults().table.get(r);
                    for (int c=0; c<vma.assigns.size(); c++) {
                        String varName = vma.assigns.get(c).varName;
                        data[r][c] = entry.assign.binding.get(varName);
                    }
                    data[r][colNames.length-1] = entry.isComputedOk() ? entry.toString() : "-";
                }
                ws.add(new ExcelXml.WorkSheet("Results of "+language.getShortName(), 
                        "Results of "+language.getShortName() + ": " + formula.getExpr(), 
                        getComment(), colNames, data));                
            }
            break;
        }
    }

    @Override
    public boolean canExportResultsAsWorkSheets() {
        switch (language) {
            case RG:
            case TRG:
            case DD:
            case INC:
                return false;
                
            default:
                return hasResults();
        }
    }
    
    @Override
    public String getSuggestedWorkSheetTitle() {
        String title = "Results of " + language.getShortName();
        if (language.hasFormulaText())
            title += " "+formula.getExpr();
        return title;
    }
}
