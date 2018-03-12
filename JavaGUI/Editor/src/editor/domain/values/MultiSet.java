/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.values;

import editor.domain.elements.ColorClass;
import editor.domain.elements.NeutralColorClass;
import editor.domain.grammar.DomainElement;
import java.util.Map;
import java.util.Set;

/** A multiset of colors in a color domain.
 *
 * @author elvio
 */
public class MultiSet extends EvaluatedFormula {
    
    // The color domain where this multi set is defined
    public final ColorClass domain;
    
    // The color tuples indices in domain
    protected final DomainElement[] entries;
    
    protected MultiSet(ColorClass domain, Set<DomainElement> colorSet) {
        assert domain != NeutralColorClass.INSTANCE;
//        assert getType() != Type.COLOR_SET || domain.isSimpleClass();
        this.domain = domain;
        entries = new DomainElement[colorSet.size()];
        int i = 0;
        for (DomainElement elem : colorSet) {
            assert elem.getDomain() == domain;
            entries[i] = elem;
            // Check the entries ordering
            if (i > 0) {
                if (entries[i].compareTo(entries[i - 1]) <= 0) {
                    throw new IllegalStateException("MultiSet entries are not ordered!");
                }
            }
            i++;
        }
    }
    
    public static MultiSet makeNew(ColorClass domain, Set<DomainElement> colorSet) {
        return new MultiSet(domain, colorSet);
    }
    public static MultiSet makeNew(Type type, ColorClass domain, Map<DomainElement, EvaluatedFormula> multiSet) {
        if (type == Type.BOOLEAN)
            return makeNew(domain, multiSet.keySet());
        return new ValuedMultiSet(type, domain, multiSet);
    }

    @Override 
    public Type getType() { 
        return Type.BOOLEAN;
    }

    @Override
    public ColorClass getDomain() { 
        return domain;
    }

    @Override
    public boolean isScalar() {
        return false;
    }

    @Override
    public boolean isMultiSet() {
        return true;
    }

    @Override
    public boolean isObject() {
        return false;
    }
    
    public final int numClassesInDomain() {
        return domain.getNumClassesInDomain();
    }

    public final int maxColorsInClass(int colClass) {
        return domain.getColorClass(colClass).numColors();
    }

    public final int getClassEntry(int element, int colClass) {
        return entries[element].getColor(colClass);
    }

    public final int numElements() {
        return entries.length;
    }
    
    public boolean isEmpty() {
        return numElements() == 0;
    }

    public final DomainElement getElement(int i) {
        return entries[i];
    }
    
    public EvaluatedFormula getValue(int element) {
        return BooleanScalarValue.TRUE;
    }
    
    @Override
    public boolean equals(Object obj) {
        if (!super.equals(obj))
            return false;
        
        MultiSet ms = (MultiSet)obj;
        if (numElements() != ms.numElements())
            return false;
        for (int e=0; e<numElements(); e++)
            if (!getElement(e).equals(ms.getElement(e)) || !getValue(e).equals(ms.getValue(e)))
                return false;
        return true;
    }

    @Override
    public boolean equalsZero() {
         return (numElements() == 0);
    }
    
    @Override
    public int hashCode() {
        int hash = super.hashCode();
        for (int e=0; e<numElements(); e++)
            hash = hash + getElement(e).hashCode() + getValue(e).hashCode();
        return hash;
    }
}
