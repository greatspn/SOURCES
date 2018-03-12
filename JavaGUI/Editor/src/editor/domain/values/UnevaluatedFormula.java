/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.values;

import editor.domain.elements.ColorClass;
import editor.domain.grammar.ExpressionLanguage;

/** A blocked formula that remains unevaluated, and thus is represented as a String.
 *
 * @author elvio
 */
public class UnevaluatedFormula extends EvaluatedFormula {
    
    // The type and color of this formula need to be known, to perform the proper
    // evaluation of the operators.
    private final Type type;
    private final ColorClass domain;
    
    // The blocked (unevaluated) formula
    private final String formula;

    public UnevaluatedFormula(Type type, ColorClass domain, String formula) {
        this.type = type;
        this.domain = domain;
        this.formula = formula;
    }
    
    @Override
    public boolean isBlocked() { 
        return true;
    }
    
    @Override
    public Type getType() {
        return type;
    }

    @Override
    public ColorClass getDomain() {
        return domain;
    }

    @Override
    public boolean isScalar() {
        return (type != Type.OBJECT) && domain.isNeutralDomain();
    }

    @Override
    public boolean isMultiSet() {
        return !domain.isNeutralDomain();
    }

    @Override
    public boolean isObject() {
        return type == Type.OBJECT;
    }

    @Override
    public String toStringFormat(ExpressionLanguage lang, String realFormat) {
        return formula;
    }

    @Override
    public boolean equalsZero() {
        return formula.equals("0") || formula.equals("0.0");
    }
}
