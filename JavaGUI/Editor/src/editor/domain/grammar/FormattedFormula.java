/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.grammar;

import java.io.Serializable;

/** A parsed string that has been formatted as a textual formula in a 
 *  target expression language, like LaTeX.
 *
 * @author elvio
 */
public class FormattedFormula implements Serializable {
        
    // The textual formula
    private final String formula;
    
    // The type of the term
    // true = the formula is a single mathematical term (a symbol, a parenthesized expr., etc...)
    // false = the formula is made by multiple terms (a sum, a multiplication, etc...)
    private final boolean isSimpleTerm;
    
    // Optional payload object
    private final FormulaPayload payload;
    
    // Expression language
    public final ExpressionLanguage lang;
    
    public static final FormattedFormula EMPTY_LATEX 
            = new FormattedFormula(ExpressionLanguage.LATEX, true, "");

    public FormattedFormula(ExpressionLanguage lang, boolean isSimpleTerm, String formula, FormulaPayload payload) {
        this.lang = lang;
        this.isSimpleTerm = isSimpleTerm;
        this.formula = formula;
        this.payload = payload;
    }
    
    public FormattedFormula(ExpressionLanguage lang, boolean isSimpleTerm, String formula) {
        this(lang, isSimpleTerm, formula, null);
    }

    public String getFormula() {
        return formula;
    }

    public boolean isSimpleTerm() {
        return isSimpleTerm;
    }
    
    public FormulaPayload getPayload() {
        return payload;
    }
    
    public FormattedFormula getAsSimpleTerm() {
        if (isSimpleTerm)
            return this;
        switch (lang) {
            case LATEX:
                return new FormattedFormula(lang, true, "\\left({"+getFormula()+"}\\right)", payload);
            case GREATSPN:
            case APNN:
            case PNPRO:
                return new FormattedFormula(lang, true, "("+getFormula()+")", payload);
            default:
                throw new UnsupportedOperationException();
        }
    }
    
    public FormattedFormula addPayload(FormulaPayload newPayload) {
        return new FormattedFormula(lang, isSimpleTerm, formula, newPayload);
    }
}
