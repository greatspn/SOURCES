/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import editor.domain.values.EvaluatedFormula;
import editor.domain.grammar.EvaluationArguments;
import editor.domain.grammar.EvaluationException;
import editor.domain.grammar.ExprRewriter;
import editor.domain.grammar.ExpressionLanguage;
import editor.domain.grammar.FormattedFormula;
import editor.domain.grammar.FormulaPayload;
import editor.domain.grammar.ParserContext;
import java.io.Serializable;
import latex.LatexFormula;
import latex.LatexProvider;
import latex.LatexProviderImpl;

/** An expression object, that is formatted and evaluated inside a ParserContext.
 *
 * @author elvio
 */
public abstract class Expr implements Serializable {
    
    private String expr;
    transient protected FormattedFormula formattedExpr;
    private LatexFormula latexExpr;
    private boolean foundParseErrors = false;

    public Expr() { }

    public Expr(String expr) {
        this.expr = expr;
    }
    
    protected abstract String getExprDescr();
    protected abstract ParserContext.ParserEntryPoint getParseRule(String exprText);
    protected abstract int getParseFlags();
    
    public void setExpr(String newExpr) {
        assert newExpr != null;
        if (newExpr.length() == 0)
            newExpr = getDefaultEmptyExpr();
        if (getExpr() == null || !getExpr().equals(newExpr)) {
            this.expr = newExpr;
            formattedExpr = null;
            setLatexExpr(null);
        }
    }
    
    public String getExpr() {
        return expr;
    }
    
    // Overwrite this method to provide a default expression that substitutes the empty text.
    public String getDefaultEmptyExpr() { return ""; }
    
    public boolean prepareContext(ParserContext context) { return true; }
    
    public boolean isValidExpr(ParserContext context, String text) {
        if (text.length() == 0)
            text = getDefaultEmptyExpr();
        if (prepareContext(context)) {
            FormattedFormula ff = context.parseFormula(text, getParseRule(text), 
                                                       getExprDescr(), getParseFlags());
            return !context.hasParseErrors() && ff != null;
        }
        return true;
    }
    
    public void checkExprCorrectness(ParserContext context, ProjectPage page, Selectable selObj) {
        if (prepareContext(context)) {
            formattedExpr = context.parseFormula(getExpr(), getParseRule(getExpr()), getExprDescr(), getParseFlags());
            foundParseErrors = context.hasParseErrors();
            if (foundParseErrors && selObj != null)
                page.addErrorWarningList(context.formatErrors(page, selObj));
            if (getLatexExpr() != null && !latexExpr.getLatex().equals(getVisualizedExpr()))
                setLatexExpr(null);
        }
    }
    
    public boolean isFormatted() {
        return formattedExpr != null;
    }
    
    public boolean isFormattedAndCorrect() {
        return formattedExpr != null && !foundParseErrors;
    }
    
    public String getFormatted() {
        assert formattedExpr != null;
        return formattedExpr.getFormula();
    }
    public FormulaPayload getParserPayload() {
        assert formattedExpr != null;
        return formattedExpr.getPayload();
    }
    
    public String getVisualizedExpr() {
        if (isFormatted()) // Formatted, with or without errors
            return getFormatted();
        if (foundParseErrors) {
            // Not formatted and with errors - draw in red
            return "\\textcolor{FF0000}{${"+LatexProviderImpl.basicLatexForm(getExpr())+"}$}";
        }
        return getExpr();
    }
    
    public EvaluatedFormula evaluate(ParserContext context, EvaluationArguments args) {
        assert isFormatted();
//        System.out.println("context="+context);
//        System.out.println("expr="+expr);
//        System.out.println("getParseRule()="+getParseRule());
        if (!prepareContext(context))
            return null;
        EvaluatedFormula result;
        result = context.evaluate(getExpr(), getParseRule(getExpr()), args, getExprDescr(), getParseFlags());
//        System.out.println("result = "+result);
        return result;
    }
    
    public String reduce(ParserContext context, EvaluationArguments args, ExpressionLanguage lang) {
        assert isFormatted();
//        System.out.println("context="+context);
//        System.out.println("expr="+expr);
//        System.out.println("getParseRule()="+getParseRule());
        if (!prepareContext(context))
            return null;
        String result;
        result = context.reduce(getExpr(), getParseRule(getExpr()), args, 
                               getExprDescr(), getParseFlags(), lang, true /* apply reduction step */);
//        System.out.println("result = "+result.toStringFormat("%.2f"));
        return result;
    }
    
    public void rewrite(ParserContext context, ExprRewriter rewriter) {
//        assert isFormatted();
        if (prepareContext(context)) {
            try {
                String result = context.rewrite(expr, getParseRule(expr), getParseFlags(), rewriter);
//              System.out.println("Rewriting: "+getExpr()+" into: "+result);
                setExpr(result);
            }
            catch (EvaluationException e) { 
                System.out.println("REWRITE ERROR: "+e.getMessage());
                // Will not rewrite anything.
            }
        }
    }
    
    public String convertLang(ParserContext context, EvaluationArguments args, ExpressionLanguage lang) {
        assert isFormatted();
        if (!prepareContext(context))
            return null;
        String result;
        result = context.reduce(getExpr(), getParseRule(getExpr()), args, 
                                getExprDescr(), getParseFlags(), lang, false /* do not reduce */);
        return result;
    }
    
    protected ParserContext createParseContext(ProjectData proj, ProjectPage page) {
        return new ParserContext(page);
    }
    
    public EditableValue getEditableValue() {
        return new EditableValue() {
            @Override public Object getValue() { return getExpr(); }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
                ParserContext context = createParseContext(proj, page);
                return isValidExpr(context, (String)value);
            }
            @Override public boolean isCurrentValueValid() { return isFormattedAndCorrect(); }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) { 
                setExpr((String)value);
            }
            @Override public boolean isEditable() { return true; }
        };
    }

    @Override
    public String toString() {
        return "Expr("+getExpr()+")";
    }

    public LatexFormula getLatexExpr() {
        return latexExpr;
    }

    public void setLatexExpr(LatexFormula latexExpr) {
        this.latexExpr = latexExpr;
    }
}
