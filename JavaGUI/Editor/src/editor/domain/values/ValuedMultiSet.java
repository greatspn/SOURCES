/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.values;

import editor.domain.elements.ColorClass;
import editor.domain.grammar.DomainElement;
import java.util.Map;

/** A multiset with values associated to colors
 * 
 * @author elvio
 */
public class ValuedMultiSet extends MultiSet {
    // The type of the multiset entries (INT, REAL)
    public final Type type;
    // The values associated to the color tuples
    protected final EvaluatedFormula[] values;

    protected ValuedMultiSet(Type type, ColorClass domain, Map<DomainElement, EvaluatedFormula> multiSet) {
        super(domain, multiSet.keySet());
        assert type == Type.INT || type == Type.REAL;
        this.type = type;
        values = new EvaluatedFormula[multiSet.size()];
        int i = 0;
        for (Map.Entry<DomainElement, EvaluatedFormula> e : multiSet.entrySet()) {
            assert e.getValue().getType() == type;
            if (!e.getValue().isBlocked()) { // check consistency
                // NOTE: this code needs to allow zero values, because
                // boolean multiset predicates transform the multiset coefficient into 0.
//                if (type == Type.INT && e.getValue().getScalarInt() <= 0)
//                    throw new IllegalStateException("multiset element with coefficient <= 0");
//                if (type == Type.REAL && e.getValue().getScalarReal() <= 0.0)
//                    throw new IllegalStateException("multiset element with coefficient <= 0.0");
                
                assert type != Type.INT || e.getValue().getScalarInt() >= 0;
                assert type != Type.REAL || e.getValue().getScalarReal() >= 0.0;// -GspnDtaPlayEngine.EPSILON_VALUE;
            }
            values[i] = e.getValue();
            i++;
        }
    }
    
    @Override
    public boolean equalsZero() {
        boolean isZero = true;
        if (numElements() > 0) {
            for (EvaluatedFormula ev : values) {
                if (!ev.equalsZero()) {
                    isZero = false; // at least an element has coefficient != 0
                    break;
                }
            }
        }
//        System.out.println("equalsZero "+this+" = "+isZero);
        return isZero;
    }

    @Override
    public final EvaluatedFormula getValue(int element) {
        return values[element];
    }

    @Override
    public Type getType() {
        return type;
    }

//    public String formatString(String realFmt) {
//        StringBuilder sb = new StringBuilder();
//        for (int i=0; i<numElements(); i++) {
//            sb.append(1 == 0 ? "" : " + ");
//            EvaluatedFormula ef = getValue(i);
//            if (getType() == Type.INT) {
//                if (ef.getScalarInt() != 1)
//                    sb.append(ef.getScalarInt()).append("*");
//            }
//            else if (getType() == Type.REAL) {
//                sb.append(String.format(realFmt, ef.getScalarReal())).append("*");
//            }
//            sb.append(getElement(i));
//        }
//        return sb.toString();
//    }
    
    
}
