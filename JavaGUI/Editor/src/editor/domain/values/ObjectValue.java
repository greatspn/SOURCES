/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.values;

import editor.domain.elements.ColorClass;
import editor.domain.elements.NeutralColorClass;
import editor.domain.grammar.EvaluationSymbol;
import editor.domain.grammar.ExpressionLanguage;

// Base class of object values

public class ObjectValue extends EvaluatedFormula {

    @Override
    public ColorClass getDomain() {
        return NeutralColorClass.INSTANCE;
    }

    @Override
    public boolean isScalar() {
        return true;
    }

    @Override
    public boolean isMultiSet() {
        return false;
    }

    @Override
    public boolean isObject() {
        return true;
    }

    @Override
    public Type getType() {
        return Type.OBJECT;
    }
    private final Object obj; // The contained object

    public ObjectValue(Object obj) {
        this.obj = obj;
    }

    public Object getObject() {
//        if (obj.getClass().equals(objClass)) {
            return obj;
//        }
//        throw new IllegalStateException("Type mismatch for ObjectValue.");
    }

    public String toStringFormat(ExpressionLanguage lang) {
        if (obj == EvaluationSymbol.INFINITY) {
            switch (lang) {
                case LATEX:     return "\\infty";
                case PNPRO:     return "Infinite";
                default:        return "Infinite";
            }
        }
        return obj.toString();
    }

    @Override
    public boolean equalsZero() {
        // An object is never treated as a zero.
        return false;
    }
}
